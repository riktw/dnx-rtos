/****************************************************************************
 * ARMv7M ELF loader
 * Copyright (c) 2013-2015 Martin Ribelotta
 * Copyright (c) 2019 Johannes Taelman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "loader.h"
#include "elf.h"
#include "loader_config.h"

#define IS_FLAGS_SET(v, m) ((v&m) == m)
#define SECTION_OFFSET(e, n) (e->sectionTable + n * sizeof(Elf32_Shdr))
#define EI_MAGIC_SIZE 4

#ifndef DOX

typedef struct {
  void *data;
  int secIdx;
  off_t relSecIdx;
} ELFSection_t;

typedef struct ELFExec {

  LOADER_USERDATA_T user_data;

  size_t sections;
  off_t sectionTable;
  off_t sectionTableStrings;

  size_t symbolCount;
  off_t symbolTable;
  off_t symbolTableStrings;
  off_t entry;

  ELFSection_t text;
  ELFSection_t rodata;
  ELFSection_t data;
  ELFSection_t bss;
  ELFSection_t init_array;
  ELFSection_t fini_array;
  ELFSection_t sdram_rodata;
  ELFSection_t sdram_data;
  ELFSection_t sdram_bss;

  unsigned int fini_array_size;

} ELFExec_t;


#endif

typedef enum {
  FoundERROR = 0,
  FoundSymTab = (1 << 0),
  FoundStrTab = (1 << 2),
  FoundText = (1 << 3),
  FoundRodata = (1 << 4),
  FoundData = (1 << 5),
  FoundBss = (1 << 6),
  FoundRelText = (1 << 7),
  FoundRelRodata = (1 << 8),
  FoundRelData = (1 << 9),
  FoundRelBss = (1 << 10),
  FoundInitArray = (1 << 11),
  FoundRelInitArray = (1 << 12),
  FoundFiniArray = (1 << 13),
  FoundRelFiniArray = (1 << 14),
  FoundSDRamRodata = (1 << 15),
  FoundSDRamData = (1 << 16),
  FoundSDRamBss = (1 << 17),
  FoundRelSDRamRodata = (1 << 18),
  FoundRelSDRamData = (1 << 19),
  FoundRelSDRamBss = (1 << 20),
  FoundValid = FoundSymTab | FoundStrTab,
  FoundExec = FoundValid | FoundText,
  FoundAll = FoundSymTab | FoundStrTab | FoundText | FoundRodata | FoundData
      | FoundBss | FoundRelText | FoundRelRodata | FoundRelData | FoundRelBss
      | FoundInitArray | FoundRelInitArray
      | FoundFiniArray | FoundRelFiniArray
} FindFlags_t;

static int readSectionName(ELFExec_t *e, off_t off, char *buf, size_t max) {
  int ret = -1;
  off_t offset = e->sectionTableStrings + off;
  off_t old = LOADER_TELL(e->user_data);
  if (LOADER_SEEK_FROM_START(e->user_data, offset) == 0)
    if (LOADER_READ(e->user_data, buf, max) == 0)
      ret = 0;
  (void) LOADER_SEEK_FROM_START(e->user_data, old);
  return ret;
}

static int readSymbolName(ELFExec_t *e, off_t off, char *buf, size_t max) {
  int ret = -1;
  off_t offset = e->symbolTableStrings + off;
  off_t old = LOADER_TELL(e->user_data);
  if (LOADER_SEEK_FROM_START(e->user_data, offset) == 0)
    if (LOADER_READ(e->user_data, buf, max) == 0)
      ret = 0;
  (void) LOADER_SEEK_FROM_START(e->user_data, old);
  return ret;
}

static void freeSection(ELFSection_t *s) {
  if (s->data)
    LOADER_FREE(s->data);
}

static uint32_t swabo(uint32_t hl) {
  return ((((hl) >> 24)) | /* */
  (((hl) >> 8) & 0x0000ff00) | /* */
  (((hl) << 8) & 0x00ff0000) | /* */
  (((hl) << 24))); /* */
}

static void dumpData(uint8_t *data, size_t size) {
#if 0
  int i = 0;
  while (i < size) {
    if ((i & 0xf) == 0)
      DBG("\n  %04X: ", i);
    DBG("%08x ", swabo(*((uint32_t* )(data + i))));
    i += sizeof(uint32_t);
  }
  DBG("\n");
#endif
}

typedef enum {
  sram = 0,
  sdram = 1
} MemType_t;

static int loadSecData(ELFExec_t *e, ELFSection_t *s, Elf32_Shdr *h, MemType_t memType) {
  if (!h->sh_size) {
    MSG(" No data for section");
    return 0;
  }
  if (memType == sdram) {
    s->data = LOADER_ALIGN_ALLOC_SDRAM(h->sh_size, h->sh_addralign, h->sh_flags);
  } else {
    s->data = LOADER_ALIGN_ALLOC(h->sh_size, h->sh_addralign, h->sh_flags);
  }
  if (!s->data) {
    ERR("    GET MEMORY fail");
    return -1;
  }
  if (h->sh_type == SHT_NOBITS) {
    // init with zeros
    char *p = s->data;
    int i, sz = h->sh_size;
    for (i = 0; i < sz; i++) {
      *p++ = 0;
    }
  } else {
    if (LOADER_SEEK_FROM_START(e->user_data, h->sh_offset) != 0) {
      ERR("    seek fail");
      freeSection(s);
      return -1;
    }
    if (LOADER_READ(e->user_data, s->data, h->sh_size) != h->sh_size) {
      ERR("     read data fail");
      return -1;
    }
    /* DBG("DATA: "); */
    dumpData(s->data, h->sh_size);
  }
  return 0;
}

static int readSecHeader(ELFExec_t *e, int n, Elf32_Shdr *h) {
  off_t offset = SECTION_OFFSET(e, n);
  if (LOADER_SEEK_FROM_START(e->user_data, offset) != 0)
    return -1;
  if (LOADER_READ(e->user_data, h, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
    return -1;
  return 0;
}

static int readSection(ELFExec_t *e, int n, Elf32_Shdr *h, char *name,
    size_t nlen) {
  if (readSecHeader(e, n, h) != 0)
    return -1;
  if (h->sh_name)
    return readSectionName(e, h->sh_name, name, nlen);
  return 0;
}

static int readSymbol(ELFExec_t *e, int n, Elf32_Sym *sym, char *name,
    size_t nlen) {
  int ret = -1;
  off_t old = LOADER_TELL(e->user_data);
  off_t pos = e->symbolTable + n * sizeof(Elf32_Sym);
  if (LOADER_SEEK_FROM_START(e->user_data, pos) == 0)
    if (LOADER_READ(e->user_data, sym, sizeof(Elf32_Sym)) == sizeof(Elf32_Sym)) {
      if (sym->st_name)
        ret = readSymbolName(e, sym->st_name, name, nlen);
      else {
        Elf32_Shdr shdr;
        ret = readSection(e, sym->st_shndx, &shdr, name, nlen);
      }
    }
  (void) LOADER_SEEK_FROM_START(e->user_data, old);
  return ret;
}

static const char *typeStr(int symt) {
#define STRCASE(name) case name: return #name;
  switch (symt) {
  STRCASE(R_ARM_NONE)
  STRCASE(R_ARM_ABS32)
  STRCASE(R_ARM_THM_PC22)
  STRCASE(R_ARM_THM_JUMP24)
  STRCASE(R_ARM_TARGET1)
  default:
    return "R_<unknow>";
  }
#undef STRCASE
}

static void relJmpCall(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
  uint16_t upper_insn = ((uint16_t *) relAddr)[0];
  uint16_t lower_insn = ((uint16_t *) relAddr)[1];
  uint32_t S = (upper_insn >> 10) & 1;
  uint32_t J1 = (lower_insn >> 13) & 1;
  uint32_t J2 = (lower_insn >> 11) & 1;

  int32_t offset = (S << 24) | /* S     -> offset[24] */
  ((~(J1 ^ S) & 1) << 23) | /* J1    -> offset[23] */
  ((~(J2 ^ S) & 1) << 22) | /* J2    -> offset[22] */
  ((upper_insn & 0x03ff) << 12) | /* imm10 -> offset[12:21] */
  ((lower_insn & 0x07ff) << 1); /* imm11 -> offset[1:11] */
  if (offset & 0x01000000)
    offset -= 0x02000000;

  offset += symAddr - relAddr;

  S = (offset >> 24) & 1;
  J1 = S ^ (~(offset >> 23) & 1);
  J2 = S ^ (~(offset >> 22) & 1);

  upper_insn = ((upper_insn & 0xf800) | (S << 10) | ((offset >> 12) & 0x03ff));
  ((uint16_t*) relAddr)[0] = upper_insn;

  lower_insn = ((lower_insn & 0xd000) | (J1 << 13) | (J2 << 11)
      | ((offset >> 1) & 0x07ff));
  ((uint16_t*) relAddr)[1] = lower_insn;
}

static int relocateSymbol(Elf32_Addr relAddr, int type, Elf32_Addr symAddr) {
  switch (type) {
  case R_ARM_ABS32:
    *((uint32_t*) relAddr) += symAddr;
    DBG("  R_ARM_ABS32 relocated is 0x%08X\n", *((uint32_t* )relAddr));
    break;
  case R_ARM_THM_PC22:
  case R_ARM_THM_JUMP24:
    relJmpCall(relAddr, type, symAddr);
    DBG("  R_ARM_THM_CALL/JMP relocated is 0x%08X\n", *((uint32_t* )relAddr));
    break;
  case R_ARM_TARGET1:
    // quoting https://sourceware.org/binutils/docs/ld/ARM.html :
    // "interpreted as either ‘R_ARM_REL32’ or ‘R_ARM_ABS32’, depending on the target"
    // implementation here is as R_ARM_ABS32
    *((uint32_t*) relAddr) += symAddr;
    DBG("  R_ARM_TARGET1 relocated is 0x%08X\n", *((uint32_t* )relAddr));
    break;
  case R_ARM_THM_PC11:
    MSG("  R_ARM_THM_JUMP11 DISCARDED!\n");
    // TODO : implement relocation type R_ARM_THM_JUMP11
    break;
  default:
    DBG("  Undefined relocation %d\n", type);
    return -1;
  }
  return 0;
}

static ELFSection_t *sectionOf(ELFExec_t *e, int index) {
#define IFSECTION(sec, i) \
  do { \
    if ((sec).secIdx == i) \
      return &(sec); \
  } while(0)
  IFSECTION(e->text, index);
  IFSECTION(e->rodata, index);
  IFSECTION(e->data, index);
  IFSECTION(e->bss, index);
  IFSECTION(e->sdram_rodata, index);
  IFSECTION(e->sdram_data, index);
  IFSECTION(e->sdram_bss, index);
  IFSECTION(e->init_array, index);
  IFSECTION(e->fini_array, index);
#undef IFSECTION
  return NULL;
}

static Elf32_Addr addressOf(ELFExec_t *e, Elf32_Sym *sym, const char *sName) {
  if (sym->st_shndx == SHN_UNDEF) {
    return LOADER_GETUNDEFSYMADDR(&e->user_data, sName);
  } else {
    ELFSection_t *symSec = sectionOf(e, sym->st_shndx);
    if (symSec)
      return ((Elf32_Addr) symSec->data) + sym->st_value;
  }
  DBG("  Can't find address for symbol %s\n", sName);
  return 0xffffffff;
}

static int relocate(ELFExec_t *e, Elf32_Shdr *h, ELFSection_t *s,
    const char *name) {
  if (s->data) {
    Elf32_Rel rel;
    size_t relEntries = h->sh_size / sizeof(rel);
    size_t relCount;
    (void) LOADER_SEEK_FROM_START(e->user_data, h->sh_offset);
    DBG(" Offset   Info     Type             Name\n");
    for (relCount = 0; relCount < relEntries; relCount++) {
      if (LOADER_READ(e->user_data, &rel, sizeof(rel)) == sizeof(rel)) {
        Elf32_Sym sym;
        Elf32_Addr symAddr;

        char name[LOADER_MAX_SYM_LENGTH] = "<unnamed>";
        int symEntry = ELF32_R_SYM(rel.r_info);
        int relType = ELF32_R_TYPE(rel.r_info);
        Elf32_Addr relAddr = ((Elf32_Addr) s->data) + rel.r_offset;

        readSymbol(e, symEntry, &sym, name, sizeof(name));
        DBG(" %08X %08X %-16s %s\n", rel.r_offset, rel.r_info, typeStr(relType),
            name);

        symAddr = addressOf(e, &sym, name);
        if (symAddr != 0xffffffff) {
          DBG("  symAddr=%08X relAddr=%08X\n", symAddr, relAddr);
          if (relocateSymbol(relAddr, relType, symAddr) == -1) {
            ERR("relocate failed of sym %s, type %d", name, relType);
            return -1;
          }
        } else {
          DBG("  No symbol address of %s\n", name);
          return -1;
        }
      }
    }
    return 0;
  } else {
    MSG("Section not loaded");
  }
  return -1;
}

static int placeInfo(ELFExec_t *e, Elf32_Shdr *sh, const char *name, int n) {
  if (LOADER_STREQ(name, ".symtab")) {
    e->symbolTable = sh->sh_offset;
    e->symbolCount = sh->sh_size / sizeof(Elf32_Sym);
    return FoundSymTab;
  } else if (LOADER_STREQ(name, ".strtab")) {
    e->symbolTableStrings = sh->sh_offset;
    return FoundStrTab;
  } else if (LOADER_STREQ(name, ".text")) {
    if (loadSecData(e, &e->text, sh, sram) == -1)
      return FoundERROR;
    e->text.secIdx = n;
    return FoundText;
  } else if (LOADER_STREQ(name, ".rodata")) {
    if (loadSecData(e, &e->rodata, sh, sram) == -1)
      return FoundERROR;
    e->rodata.secIdx = n;
    return FoundRodata;
  } else if (LOADER_STREQ(name, ".data")) {
    if (loadSecData(e, &e->data, sh, sram) == -1)
      return FoundERROR;
    e->data.secIdx = n;
    return FoundData;
  } else if (LOADER_STREQ(name, ".bss")) {
    if (loadSecData(e, &e->bss, sh, sram) == -1)
      return FoundERROR;
    e->bss.secIdx = n;
    return FoundBss;
  } else if (LOADER_STREQ(name, ".sdram_rodata")) {
    if (loadSecData(e, &e->sdram_rodata, sh, sdram) == -1)
      return FoundERROR;
    e->sdram_rodata.secIdx = n;
    return FoundSDRamRodata;
  } else if (LOADER_STREQ(name, ".sdram_data")) {
    if (loadSecData(e, &e->sdram_data, sh, sdram) == -1)
      return FoundERROR;
    e->sdram_data.secIdx = n;
    return FoundSDRamData;
  } else if (LOADER_STREQ(name, ".sdram_bss")) {
    if (loadSecData(e, &e->sdram_bss, sh, sdram) == -1)
      return FoundERROR;
    e->sdram_bss.secIdx = n;
    return FoundSDRamBss;
  } else if (LOADER_STREQ(name, ".init_array")) {
    if (loadSecData(e, &e->init_array, sh, sram) == -1)
      return FoundERROR;
    e->init_array.secIdx = n;
    return FoundInitArray;
  } else if (LOADER_STREQ(name, ".fini_array")) {
    if (loadSecData(e, &e->fini_array, sh, sram) == -1)
      return FoundERROR;
    e->fini_array.secIdx = n;
    e->fini_array_size = sh->sh_size;
    return FoundFiniArray;
  } else if (LOADER_STREQ(name, ".rel.text")) {
    e->text.relSecIdx = n;
    return FoundRelText;
  } else if (LOADER_STREQ(name, ".rel.rodata")) {
    e->rodata.relSecIdx = n;
    return FoundRelRodata;
  } else if (LOADER_STREQ(name, ".rel.data")) {
    e->data.relSecIdx = n;
    return FoundRelData;
  } else if (LOADER_STREQ(name, ".rel.sdram_rodata")) {
    e->sdram_rodata.relSecIdx = n;
    return FoundRelSDRamRodata;
  } else if (LOADER_STREQ(name, ".rel.sdram_data")) {
    e->sdram_data.relSecIdx = n;
    return FoundRelSDRamData;
  } else if (LOADER_STREQ(name, ".rel.init_array")) {
    e->init_array.relSecIdx = n;
    return FoundRelInitArray;
  } else if (LOADER_STREQ(name, ".rel.fini_array")) {
    e->fini_array.relSecIdx = n;
    return FoundRelFiniArray;
  }
  /* BSS not need relocation */
#if 0
  else if (LOADER_STREQ(name, ".rel.bss")) {
    e->bss.relSecIdx = n;
    return FoundRelText;
  }
#endif
  return 0;
}

static int loadSymbols(ELFExec_t *e) {
  unsigned int n;
  int founded = 0;
  MSG("Scan ELF indexes...");
  for (n = 1; n < e->sections; n++) {
    Elf32_Shdr sectHdr;
    char name[LOADER_MAX_SYM_LENGTH] = "<unamed>";
    if (readSecHeader(e, n, &sectHdr) != 0) {
      ERR("Error reading section");
      return -1;
    }
    if (sectHdr.sh_name)
      readSectionName(e, sectHdr.sh_name, name, sizeof(name));
    DBG("Examining section %d %s\n", n, name);
    founded |= placeInfo(e, &sectHdr, name, n);
    if (IS_FLAGS_SET(founded, FoundAll))
      return FoundAll;
  }
  MSG("Done");
  return founded;
}

static int initElf(ELFExec_t *e) {
  Elf32_Ehdr h;
  Elf32_Shdr sH;

  if (!LOADER_FD_VALID(e->user_data))
    return -1;

  if (LOADER_READ(e->user_data, &h, sizeof(h)) != sizeof(h))
    return -1;

  const char elfmagic[EI_MAGIC_SIZE] = ELFMAG;
  if (h.e_ident[EI_MAG0] != elfmagic[EI_MAG0]) return 1;
  if (h.e_ident[EI_MAG1] != elfmagic[EI_MAG1]) return 1;
  if (h.e_ident[EI_MAG2] != elfmagic[EI_MAG2]) return 1;
  if (h.e_ident[EI_MAG3] != elfmagic[EI_MAG3]) return 1;
  if (h.e_ident[EI_CLASS] != ELFCLASS32) return 1;
  if (h.e_type != ET_REL) return 1;
  if (h.e_machine != EM_ARM) return 1;
  if (h.e_version != EV_CURRENT) return 1;

  if (LOADER_SEEK_FROM_START(e->user_data, h.e_shoff + h.e_shstrndx * sizeof(sH)) != 0)
    return -1;
  if (LOADER_READ(e->user_data, &sH, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
    return -1;

  e->entry = h.e_entry;
  e->sections = h.e_shnum;
  e->sectionTable = h.e_shoff;
  e->sectionTableStrings = sH.sh_offset;

  return 0;
}

static void freeElf(ELFExec_t *e) {
  freeSection(&e->text);
  freeSection(&e->rodata);
  freeSection(&e->data);
  freeSection(&e->bss);
  freeSection(&e->sdram_rodata);
  freeSection(&e->sdram_data);
  freeSection(&e->sdram_bss);
  freeSection(&e->init_array);
  freeSection(&e->fini_array);
  LOADER_CLOSE(e->user_data);
}

static int relocateSection(ELFExec_t *e, ELFSection_t *s, const char *name) {
  DBG("Relocating section %s\n", name);
  if (s->relSecIdx) {
    Elf32_Shdr sectHdr;
    if (readSecHeader(e, s->relSecIdx, &sectHdr) == 0)
      return relocate(e, &sectHdr, s, name);
    else {
      ERR("Error reading section header");
      return -1;
    }
  } else {
    MSG("No relocation index"); /* Not an error */
  }
  return 0;
}

static int relocateSections(ELFExec_t *e) {
  return relocateSection(e, &e->text, ".text")
      | relocateSection(e, &e->rodata, ".rodata")
      | relocateSection(e, &e->data, ".data")
      | relocateSection(e, &e->sdram_rodata, ".sdram_rodata")
      | relocateSection(e, &e->sdram_data, ".sdram_data")
      | relocateSection(e, &e->init_array, ".init_array")
      | relocateSection(e, &e->fini_array, ".fini_array")
      /* BSS not need relocation */
#if 0
      | relocateSection(e, &e->bss, ".bss")
#endif
  ;
}

int jumpTo(ELFExec_t *e) {
  if (e->entry) {
    entry_t *entry = (entry_t*) (e->text.data + e->entry);
    LOADER_JUMP_TO(entry);
    return 0;
  } else {
    MSG("No entry defined.");
    return -1;
  }
}

static void do_init(ELFExec_t *e) {

  if (e->init_array.data) {
    MSG("Processing section .init_array.");
    Elf32_Shdr sectHdr;
    if (readSecHeader(e, e->init_array.secIdx, &sectHdr) != 0) {
      ERR("Error reading section header");
      return;
    }
    entry_t **entry = (entry_t**) (e->init_array.data);
    int i;
    int n = sectHdr.sh_size >> 2;
    for(i=0;i<n;i++) {
      DBG("Processing .init_array[%d] : %08x->%08x\n", i, (int)entry, (int)*entry);
      (*entry)();
      entry++;
    }
  } else {
    MSG("No .init_array"); // and that's fine
  }
}

static void do_fini(ELFExec_t *e) {
  if (e->fini_array.data) {
    entry_t **entry = (entry_t**) (e->fini_array.data);
    int i;
    int n = e->fini_array_size >> 2;
    for(i=0;i<n;i++) {
      DBG("Processing .fini_array[%d] : %08x->%08x\n", i, (int)entry, (int)*entry);
      (*entry)();
      entry++;
    }
  } else {
    MSG("No .fini_array"); // and that's fine too
  }
}

void* get_sym(ELFExec_t *exec, const char *sym_name, int symbol_type) {
  off_t old = LOADER_TELL(exec->user_data);
  off_t pos = exec->symbolTable;
  if (LOADER_SEEK_FROM_START(exec->user_data, pos) != 0) {
    MSG("seek err");
    return 0;
  }
  unsigned int i;
  entry_t *addr = 0;
  for (i = 0; i < exec->symbolCount; i++) {
    Elf32_Sym sym;
    if (LOADER_READ(exec->user_data, &sym, sizeof(Elf32_Sym)) == sizeof(Elf32_Sym)) {
      if (sym.st_name && (ELF32_ST_TYPE(sym.st_info) == symbol_type)) {
        char name[LOADER_MAX_SYM_LENGTH] = "<unnamed>";
        int ret = readSymbolName(exec, sym.st_name, name, sizeof(name));
        if (!ret) {
          DBG("readSymbolName failed");
        } else {
          //DBG("sym %d = \"%s\" @ %08x, st_shndx = %d\n",i,name,sym.st_value, sym.st_shndx);
          if (LOADER_STREQ(name, sym_name)) {
            ELFSection_t *symSec = sectionOf(exec, sym.st_shndx);
            if (symSec) {
              addr = (entry_t*) (((Elf32_Addr) symSec->data) + sym.st_value);
              DBG("sym \"%s\" found @ %08x\n", name, addr);
              break;
            } else if (symbol_type == STT_NOTYPE) {
              addr = (entry_t *)sym.st_value;
              DBG("sym \"%s\" found @ %08x\n", name, addr);
              break;
            }
          }
        }
      }
    }
  }
  (void) LOADER_SEEK_FROM_START(exec->user_data, old);
  if (!addr) {
    DBG("sym \"%s\" not found\n", sym_name);
  }
  return addr;
}

void* get_obj(ELFExec_t *exec, const char *obj_name) {
  return get_sym(exec, obj_name, STT_OBJECT);
}

void* get_func(ELFExec_t *exec, const char *func_name) {
  return get_sym(exec, func_name, STT_FUNC);
}

static void clearELFExec(ELFExec_t *e) {
  char *c = (char *)e;
  unsigned int i;
  for(i=0;i<sizeof(ELFExec_t);i++){
    *c++=0;
  }
}

int load_elf(const char *path, LOADER_USERDATA_T user_data, ELFExec_t **exec_ptr) {
  ELFExec_t *exec;
  exec = LOADER_ALIGN_ALLOC(sizeof(ELFExec_t), 4, ELF_SEC_READ | ELF_SEC_WRITE);
  if (!exec) {
    DBG("allocation failed\n\n");
    return -1;
  }
  clearELFExec(exec);
  exec->user_data = user_data;
  LOADER_OPEN_FOR_RD(exec->user_data, path);
  if (initElf(exec) != 0) {
    DBG("Invalid elf %s\n", path);
    return -1;
  }
  if (!IS_FLAGS_SET(loadSymbols(exec), FoundValid)) {
    freeElf(exec);
    LOADER_FREE(exec);
    return -2;
  }
  if (relocateSections(exec) != 0) {
    freeElf(exec);
    LOADER_FREE(exec);
    return -3;
  }
  do_init(exec);
  *exec_ptr = exec;
  return 0;
}

int unload_elf(ELFExec_t *exec) {
  do_fini(exec);
  freeElf(exec);
  LOADER_FREE(exec);
  return 0;
}

