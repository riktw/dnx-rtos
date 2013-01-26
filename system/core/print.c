/*=============================================================================================*//**
@file    print.c

@author  Daniel Zorychta

@brief   This file support message printing

@note    Copyright (C) 2012 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


*//*==============================================================================================*/

/*==================================================================================================
                                            Include files
==================================================================================================*/
#include "print.h"
#include <string.h>
#include "vfs.h"
#include "memman.h"
#include "oswrap.h"

#if (CONFIG_PRINT_ENABLE == 1)

/*==================================================================================================
                                  Local symbolic constants/macros
==================================================================================================*/
#define calloc(nmemb, msize)              memman_calloc(nmemb, msize)
#define malloc(size)                      memman_malloc(size)
#define free(mem)                         memman_free(mem)

#define fopen(path, mode)                 vfs_fopen(path, mode)
#define fclose(file)                      vfs_fclose(file)
#define fwrite(ptr, isize, nitems, file)  vfs_fwrite(ptr, isize, nitems, file)
#define fread(ptr, isize, nitems, file)   vfs_fread(ptr, isize, nitems, file)
#define fseek(file, offset, mode)         vfs_fseek(file, offset, mode)
#define ftell(file)                       vfs_ftell(file)
#define ioctl(file, rq, data)             vfs_ioctl(file, rq, data)


/*==================================================================================================
                                   Local types, enums definitions
==================================================================================================*/


/*==================================================================================================
                                      Local function prototypes
==================================================================================================*/
static void  reverseBuffer(ch_t *begin, ch_t *end);
static int_t vsnprint(ch_t *stream, size_t size, const ch_t *format, va_list arg);
static ch_t *itoa(i32_t value, ch_t *buffer, u8_t base, bool_t unsignedValue, u8_t zerosRequired);
static int_t CalcFormatSize(const ch_t *format, va_list arg);


/*==================================================================================================
                                      Local object definitions
==================================================================================================*/
static FILE_t *kprintFile;


/*==================================================================================================
                                     Exported object definitions
==================================================================================================*/


/*==================================================================================================
                                        Function definitions
==================================================================================================*/

//================================================================================================//
/**
 * @brief Enable kprint functionality
 *
 * @param filename      path to file used to write kernel log
 */
//================================================================================================//
void pr_kprintEnable(ch_t *filename)
{
      /* close file if opened */
      if (kprintFile) {
            fclose(kprintFile);
            kprintFile = NULL;
      }

      /* open new file */
      if (kprintFile == NULL) {
            kprintFile = fopen(filename, "w");
      }
}


//================================================================================================//
/**
 * @brief Disable kprint functionality
 */
//================================================================================================//
void pr_kprintDisable(void)
{
      fclose(kprintFile);
      kprintFile = NULL;
}


//================================================================================================//
/**
 * @brief Function reverse buffer
 *
 * @param *begin     buffer begin
 * @param *end       buffer end
 */
//================================================================================================//
static void reverseBuffer(ch_t *begin, ch_t *end)
{
    ch_t temp;

    while (end > begin) {
        temp     = *end;
        *end--   = *begin;
        *begin++ = temp;
    }
}


//================================================================================================//
/**
 * @brief Function convert value to the character
 *
 * @param value         converted value
 * @param *buffer       result buffer
 * @param base          conversion base
 *
 * @return pointer in the buffer
 */
//================================================================================================//
static ch_t *itoa(i32_t value, ch_t *buffer, u8_t base, bool_t unsignedValue, u8_t zerosRequired)
{
      static const ch_t digits[] = "0123456789ABCDEF";

      ch_t *bufferCopy = buffer;
      i32_t sign    = 0;
      u8_t  zeroCnt = 0;
      i32_t quot;
      i32_t rem;

      if ((base >= 2) && (base <= 16)) {
            if (unsignedValue) {
                  do {
                        quot = (u32_t)((u32_t)value / (u32_t)base);
                        rem  = (u32_t)((u32_t)value % (u32_t)base);
                        *buffer++   = digits[rem];
                        zeroCnt++;
                  } while ((value = quot));
            } else {
                  if ((base == 10) && ((sign = value) < 0)) {
                        value = -value;
                  }

                  do {
                        quot = value / base;
                        rem  = value % base;
                        *buffer++ = digits[rem];
                        zeroCnt++;
                  } while ((value = quot));
            }

            while (zerosRequired > zeroCnt) {
                  *buffer++ = '0';
                  zeroCnt++;
            }

            if (sign < 0) {
                  *buffer++ = '-';
            }

            reverseBuffer(bufferCopy, (buffer - 1));
      }

      *buffer = '\0';

      return bufferCopy;
}


//================================================================================================//
/**
 * @brief Function convert ASCII to the number
 * When function find any other character than number (depended of actual base) immediately finished
 * operation and return pointer when bad character was found
 *
 * @param[in]  *string       string to decode
 * @param[in]  base          decode base
 * @param[out] *value        pointer to result
 *
 * @return pointer in string when operation was finished
 */
//================================================================================================//
ch_t *pr_atoi(ch_t *string, u8_t base, i32_t *value)
{
      ch_t   character;
      i32_t  sign      = 1;
      bool_t charFound = FALSE;

      *value = 0;

      if (base < 2 && base > 16) {
            goto atoi_end;
      }

      while ((character = *string) != ASCII_NULL) {
            /* if space exist, atoi continue finding correct character */
            if ((character == ' ') && (charFound == FALSE)) {
                  string++;
                  continue;
            } else {
                  charFound = TRUE;
            }

            /* check signum */
            if (character == '-') {
                  if (base == 10) {
                        if (sign == 1) {
                              sign = -1;
                        }

                        string++;
                        continue;
                  } else {
                        goto atoi_sign;
                  }
            }

            /* check character range */
            if (character >= 'a') {
                  character -= 'a' - 10;
            } else if (character >= 'A') {
                  character -= 'A' - 10;
            } else if (character >= '0') {
                  character -= '0';
            } else {
                  goto atoi_sign;
            }

            /* check character range according to actual base */
            if (character >= base) {
                  break;
            }

            /* compute value */
            *value = *value * base;
            *value = *value + character;

            string++;
      }

      atoi_sign:
      *value *= sign;

      atoi_end:
      return string;
}


//================================================================================================//
/**
 * @brief Function convert arguments to stream
 *
 * @param[in] *format        message format
 * @param[in] arg            argument list
 *
 * @return size of snprintf result
 */
//================================================================================================//
static int_t CalcFormatSize(const ch_t *format, va_list arg)
{
      ch_t  chr;
      int_t size = 1;

      while ((chr = *format++) != ASCII_NULL) {
            if (chr != '%') {
                  if (chr == '\n') {
                        size += 2;
                  } else {
                        size++;
                  }
            } else {
                  chr = *format++;

                  if (chr == '%' || chr == 'c') {
                        if (chr == 'c') {
                              chr = va_arg(arg, i32_t);
                        }

                        size++;
                        continue;
                  }

                  if (chr == 's') {
                        size += strlen(va_arg(arg, ch_t*));
                        continue;
                  }

                  if (chr == 'd' || chr == 'x' || chr == 'u') {
                        chr = va_arg(arg, i32_t);
                        size += 11;
                        continue;
                  }
            }
      }

      return size;
}


//================================================================================================//
/**
 * @brief Function send to buffer formated output string
 *
 * @param *stream             output buffer
 * @param size                buffer size
 * @param *format             formated text
 * @param ...                 format arguments
 *
 * @retval number of written characters
 */
//================================================================================================//
int_t pr_snprintf(ch_t *stream, u32_t size, const ch_t *format, ...)
{
      va_list args;
      int_t   n = 0;

      if (stream) {
            va_start(args, format);
            n = vsnprint(stream, size, format, args);
            va_end(args);
      }

      return n;
}


//================================================================================================//
/**
 * @brief Function send on a standard output string
 *
 * @param *file               file
 * @param *format             formated text
 * @param ...                 format arguments
 *
 * @retval number of written characters
 */
//================================================================================================//
int_t pr_fprintf(FILE_t *file, const ch_t *format, ...)
{
      va_list args;
      int_t   n = 0;

      if (file) {
            va_start(args, format);
            u32_t size = CalcFormatSize(format, args);
            va_end(args);

            ch_t *str  = calloc(1, size);

            if (str) {
                  va_start(args, format);
                  n = vsnprint(str, size, format, args);
                  va_end(args);

                  fwrite(str, sizeof(ch_t), size, file);

                  free(str);
            }
      }

      return n;
}


//================================================================================================//
/**
 * @brief Function send kernel message on terminal
 *
 * @param *format             formated text
 * @param ...                 format arguments
 *
 * @retval number of written characters
 */
//================================================================================================//
int_t pr_kprint(const ch_t *format, ...)
{
      va_list args;
      int_t   n = 0;

      if (kprintFile) {
            va_start(args, format);
            int_t size = CalcFormatSize(format, args);
            va_end(args);

            ch_t *buffer = calloc(size, sizeof(ch_t));

            if (buffer) {
                  va_start(args, format);
                  n = vsnprint(buffer, size, format, args);
                  va_end(args);

                  fwrite(buffer, sizeof(ch_t), size, kprintFile);

                  free(buffer);
            }
      }

      return n;
}


//================================================================================================//
/**
 * @brief Function put character into file
 *
 * @param stdout              file
 * @param c                   character
 */
//================================================================================================//
void pr_putchar(FILE_t *stdout, ch_t c)
{
      if (stdout) {
            ch_t chr[2] = {c, 0};
            fwrite(chr, sizeof(ch_t), ARRAY_SIZE(chr), stdout);
      }
}


//================================================================================================//
/**
 * @brief Function get character from file
 *
 * @param stdin            source file
 *
 * @retval character
 */
//================================================================================================//
ch_t pr_getchar(FILE_t *stdin)
{
      ch_t  chr  = 0;
      u16_t dcnt = 0;

      if (stdin) {
            while (fread(&chr, sizeof(chr), 1, stdin) < 1) {
                  if (dcnt >= 60000) {
                        TaskDelay(200);
                  } else if (dcnt >= 5000) {
                        dcnt += 100;
                        TaskDelay(100);
                  } else {
                        dcnt += 20;
                        TaskDelay(20);
                  }
            }
      }

      return chr;
}


//================================================================================================//
/**
 * @brief Function get character from virtual terminal in unblocking mode
 *
 * @param stdin            source file
 *
 * @retval character
 */
//================================================================================================//
ch_t pr_ugetchar(FILE_t *stdin)
{
      ch_t chr = 0;

      if (stdin) {
            fread(&chr, sizeof(ch_t), 1, stdin);
      }

      return chr;
}


//================================================================================================//
/**
 * @brief Function convert arguments to stream
 *
 * @param[in] *stream        buffer for stream
 * @param[in] size           buffer size
 * @param[in] *format        message format
 * @param[in] arg            argument list
 *
 * @return number of printed characters
 */
//================================================================================================//
static int_t vsnprint(ch_t *stream, size_t size, const ch_t *format, va_list arg)
{
      #define putCharacter(character)           \
      {                                         \
            if ((size_t)slen < size)  {         \
                  *stream++ = character;        \
                  slen++;                       \
            }  else {                           \
                  goto vsnprint_end;            \
            }                                   \
      }


      ch_t  chr;
      int_t slen = 1;

      /* analyze format */
      while ((chr = *format++) != ASCII_NULL) {
            if (chr != '%') {
                  if (chr == ASCII_LF) {
                        putCharacter(ASCII_CR);
                  }

                  putCharacter(chr);
            } else {
                  chr = *format++;

                  if (chr == '%' || chr == 'c') {
                        if (chr == 'c') {
                              chr = va_arg(arg, i32_t);
                        }

                        putCharacter(chr);

                        continue;
                  }

                  if (chr == 's' || chr == 'd' || chr == 'x' || chr == 'u') {
                        ch_t result[11];
                        ch_t *resultPtr;

                        if (chr == 's') {
                              resultPtr = va_arg(arg, ch_t*);
                        } else {
                              u8_t zeros = *format++;

                              if (zeros >= '0' && zeros <= '9') {
                                    zeros -= '0';
                              } else {
                                    zeros = 0;
                                    format--;
                              }

                              u8_t   base  = ((chr == 'd') || (chr == 'u') ? 10 : 16);
                              bool_t uint  = ((chr == 'x') || (chr == 'u') ? TRUE : FALSE);

                              resultPtr = itoa(va_arg(arg, i32_t), result, base, uint, zeros);
                        }

                        while ((chr = *resultPtr++)) {
                              putCharacter(chr);
                        }

                        continue;
                  }
            }
      }

      vsnprint_end:
      *stream = 0;
      return (slen - 1);

      #undef putChar
}


//================================================================================================//
/**
 * @brief Function convert arguments to the stdio and gets data from stdin
 *
 * @param[in]  *stream        file of virtual terminal (keyboard)
 * @param[in]  *format        message format
 * @param[out] *var           output
 *
 * @return number of printed characters
 */
//================================================================================================//
int_t pr_fscanf(FILE_t *stream, const ch_t *format, void *var)
{
      ch_t   chr;
      int_t  read_fields = 0;
      int_t  value       = 0;
      uint_t slen        = 0;

      if (!stream || !format || !var) {
            read_fields = EOF;
      }

      while ((chr = *format++) != '\0') {
            if (chr == '%') {
                  chr = *format++;

                  /* read value from format */
                  while (chr >= '0' && chr <= '9') {
                        value *= 10;
                        value += chr - '0';
                        chr    = *format++;
                  }

                  /* check if digital value is to decode */
                  if (chr == 'd' || chr == 'u') {
                        i32_t  *dec  = var;
                        i32_t   sign = 1;
                        bool_t  uint = (chr == 'u' ? TRUE : FALSE);

                        *dec = 0;
                        read_fields++;

                        while (TRUE) {
                              chr = pr_getchar(stream);

                              if (  (chr >= '0' && chr <= '9')
                                 || (chr == '-' && !uint && sign == 1) ) {

                                    if (chr == '-' && sign == 1 && !uint) {
                                          sign = -1;
                                    }

                                    if (chr >= '0' && chr <= '9') {
                                          chr -= '0';

                                          *dec *= 10;
                                          *dec += chr;
                                    }

                                    slen++;

                              } else if (chr == ASCII_CR || chr == ASCII_LF) {
                                    *dec *= sign;
                                    goto tscan_end;
                              } else if ((chr == ASCII_BS) && (slen > 1)) {
                                    if (slen == 2 && sign == -1) {
                                          sign = 1;
                                    } else {
                                          *dec /= 10;
                                    }

                                    slen--;
                                    continue;
                              }
                        }

                        goto tscan_end;
                  }

                  /* check if hex value is to decode */
                  if (chr == 'x') {
                        u32_t *hex = var;

                        *hex = 0;
                        read_fields++;

                        while (TRUE) {
                              chr = pr_getchar(stream);

                              if (  ((chr >= '0') && (chr <= '9'))
                                 || ((chr >= 'A') && (chr <= 'F'))
                                 || ((chr >= 'a') && (chr <= 'f')) ) {

                                    if ((chr >= 'A') && (chr <= 'F')) {
                                          chr = chr - 'A' + 0x0A;
                                    } else if ((chr >= 'a') && (chr <= 'f')) {
                                          chr = chr - 'a' + 0x0A;
                                    } else if ((chr >= '0') && (chr <= '9')) {
                                          chr -= '0';
                                    }

                                    *hex <<= 4;
                                    *hex |= chr;

                                    slen++;

                              } else if (chr == ASCII_CR || chr == ASCII_LF) {
                                    goto tscan_end;
                              } else if ((chr == ASCII_BS) && (slen > 1)) {
                                    *hex >>= 4;
                                    slen--;
                                    continue;
                              }
                        }
                  }

                  /* check if binary value is to decode */
                  if (chr == 'b') {
                        u32_t *bin = var;

                        *bin = 0;
                        read_fields++;

                        while (TRUE) {
                              chr = pr_getchar(stream);

                              if (chr == '0' || chr == '1') {

                                    chr -= '0';

                                    *bin <<= 1;
                                    *bin |= chr;

                                    slen++;

                              } else if (chr == ASCII_CR || chr == ASCII_LF) {
                                    goto tscan_end;
                              } else if ((chr == ASCII_BS) && (slen > 1)) {
                                    *bin >>= 1;
                                    slen--;
                                    continue;
                              }
                        }
                  }

                  /* check if text string is to gets */
                  if (chr == 's') {
                        u16_t bfrSize = UINT16_MAX;
                        ch_t *string  = var;
                        u16_t strLen  = 0;

                        if (*(--format) != '%' && value) {
                              bfrSize = value - 1;
                        }

                        read_fields++;

                        while (TRUE) {
                              chr = pr_getchar(stream);

                              /* put character */
                              if (chr == ASCII_CR || chr == ASCII_LF) {
                                    *(string++) = 0x00;
                                    goto tscan_end;
                              } else if ((chr == ASCII_BS) && (slen > 1) && strLen) {
                                    *(--string) = 0x00;
                                    slen--;
                                    strLen--;
                                    continue;
                              } else if ((chr >= ' ') && (strLen < bfrSize)) {
                                    *(string++) = chr;
                                    strLen++;
                                    slen++;
                              }
                        }
                  }
            }
      }

      read_fields = EOF;

      tscan_end:
      return read_fields;
}

#endif

/*==================================================================================================
                                            End of file
==================================================================================================*/