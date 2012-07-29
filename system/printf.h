#ifndef PRINTF_H_
#define PRINTF_H_
/*=============================================================================================*//**
@file    printf.h

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

#ifdef __cplusplus
   extern "C" {
#endif

/*==================================================================================================
                                            Include files
==================================================================================================*/
#include <stdarg.h>
#include "system.h"


/*==================================================================================================
                                 Exported symbolic constants/macros
==================================================================================================*/
#define print(...)                  fprint(stdout, __VA_ARGS__)
#define putChar(c)                  fputChar(stdout, c)
#define getChar()                   fgetChar(stdin)
#define clearSTDIN()                fclearSTDIO(stdin)
#define clearSTDOUT()               fclearSTDIO(stdout)

/** VT100 terminal commands */
#define clrscr()                    print("\x1B[2J")
#define eraseLine()                 print("\x1B[2K")
#define cursorHome()                print("\x1B[H")
#define resetAttr()                 print("\x1B[0m")
#define fontBlink()                 print("\x1B[5m")
#define fontUnderl()                print("\x1B[4m")
#define fontBlack()                 print("\x1B[30m")
#define fontRed()                   print("\x1B[31m")
#define fontGreen()                 print("\x1B[32m")
#define fontYellow()                print("\x1B[33m")
#define fontBlue()                  print("\x1B[34m")
#define fontMagenta()               print("\x1B[35m")
#define fontCyan()                  print("\x1B[36m")
#define fontWhite()                 print("\x1B[37m")
#define bgBlack()                   print("\x1B[40m")
#define bgRed()                     print("\x1B[41m")
#define bgGreen()                   print("\x1B[42m")
#define bgYellow()                  print("\x1B[43m")
#define bgBlue()                    print("\x1B[44m")
#define bgMagenta()                 print("\x1B[45m")
#define bgCyan()                    print("\x1B[46m")
#define bgWhite()                   print("\x1B[47m")


/*==================================================================================================
                                  Exported types, enums definitions
==================================================================================================*/


/*==================================================================================================
                                     Exported object declarations
==================================================================================================*/


/*==================================================================================================
                                     Exported function prototypes
==================================================================================================*/
extern ch_t  *itoa(i32_t value, ch_t *buffer, u8_t base);
extern u32_t snprint(ch_t *stream, u32_t size, const ch_t *format, ...);
extern u32_t kprint(const ch_t *format, ...);
extern u32_t fprint(stdioFIFO_t *stdout, const ch_t *format, ...);
extern void  kprintEnable(void);
extern void  fputChar(stdioFIFO_t *stdout, ch_t c);
extern ch_t  fgetChar(stdioFIFO_t *stdin);
extern void  fclearSTDIO(stdioFIFO_t *stdio);


#ifdef __cplusplus
   }
#endif

#endif /* PRINTF_H_ */
/*==================================================================================================
                                            End of file
==================================================================================================*/
