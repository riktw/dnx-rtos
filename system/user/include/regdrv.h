#ifndef REGDRV_H_
#define REGDRV_H_
/*=============================================================================================*//**
@file    regdrv.h

@author  Daniel Zorychta

@brief   This file is used to registration drivers

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
#include "systypes.h"


/*==================================================================================================
                                 Exported symbolic constants/macros
==================================================================================================*/


/*==================================================================================================
                                  Exported types, enums definitions
==================================================================================================*/


/*==================================================================================================
                                     Exported object declarations
==================================================================================================*/


/*==================================================================================================
                                     Exported function prototypes
==================================================================================================*/
extern stdRet_t InitDrv(const ch_t *drvName, const ch_t *nodeName);
extern stdRet_t ReleaseDrv(const ch_t *drvName);

#ifdef __cplusplus
}
#endif

#endif /* REGDRV_H_ */
/*==================================================================================================
                                            End of file
==================================================================================================*/