/*=========================================================================*//**
@file    tty_def.h

@author  Daniel Zorychta

@brief   This file support global definitions of TTY

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


*//*==========================================================================*/

#ifndef _TTY_DEF_H_
#define _TTY_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "tty_cfg.h"
#include "system/ioctl_macros.h"

/*==============================================================================
  Exported object types
==============================================================================*/
/** define part count */
#define TTY_MINOR_NUMBER        0

/** devices number */
enum TTY_major_number {
#if TTY_NUMBER_OF_VT > 0
        TTY_DEV_0,
#endif
#if TTY_NUMBER_OF_VT > 1
        TTY_DEV_1,
#endif
#if TTY_NUMBER_OF_VT > 2
        TTY_DEV_2,
#endif
#if TTY_NUMBER_OF_VT > 3
        TTY_DEV_3,
#endif
#if TTY_NUMBER_OF_VT > 4
#error "TTY support 4 virtual terminals!"
#endif
        TTY_DEV_COUNT
};

/* IO requests */
#define TTY_IORQ_GET_CURRENT_TTY                _IOR('T', 0x00, int*)
#define TTY_IORQ_SWITCH_TTY_TO                  _IOW('T', 0x01, int )
#define TTY_IORQ_GET_COL                        _IOR('T', 0x02, int*)
#define TTY_IORQ_GET_ROW                        _IOR('T', 0x03, int*)
#define TTY_IORQ_CLEAR_SCR                      _IO( 'T', 0x04)
#define TTY_IORQ_ECHO_ON                        _IO( 'T', 0x05)
#define TTY_IORQ_ECHO_OFF                       _IO( 'T', 0x06)

#ifdef __cplusplus
}
#endif

#endif /* _TTY_DEF_H_ */
/*==============================================================================
  End of file
==============================================================================*/