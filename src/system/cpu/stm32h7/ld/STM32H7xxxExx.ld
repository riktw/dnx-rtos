SEARCH_DIR(.);
SEARCH_DIR(src/system/cpu/stm32h7/ld);

MEMORY
{
   flash   (rx)  : org = 0x08000000, len = 512k
   axisram (rwx) : org = 0x24000000, len = 128k
   sram1   (rwx) : org = 0x30000000, len = 16k
   sram2   (rwx) : org = 0x30020000, len = 16k
   sram3   (rwx) : org = 0x30040000, len = 0k
   sram4   (rwx) : org = 0x38000000, len = 16k
   axiitcm (rwx) : org = 0x24020000, len = 192k
   itcm    (rwx) : org = 0x00000000, len = 64k
   dtcm    (rw)  : org = 0x20000000, len = 128k
   bkp     (rw)  : org = 0x38800000, len = 4k
}

INCLUDE common.ld
