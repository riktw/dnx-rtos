/*=============================================================================================*//**
@file    regdrv.c

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
#include "regdrv.h"
#include "systypes.h"
#include <string.h>

/* include here drivers headers */
#include "uart.h"
#include "gpio.h"
#include "pll.h"
#include "i2c.h"
#include "ether.h"


/*==================================================================================================
                                  Local symbolic constants/macros
==================================================================================================*/


/*==================================================================================================
                                   Local types, enums definitions
==================================================================================================*/
typedef struct
{
      ch_t     *drvName;
      stdRet_t (*init)(dev_t);
      stdRet_t (*open)(dev_t);
      stdRet_t (*close)(dev_t);
      stdRet_t (*write)(dev_t, void*, size_t, size_t);
      stdRet_t (*read)(dev_t, void*, size_t, size_t);
      stdRet_t (*ioctl)(dev_t, IORq_t, void*);
      stdRet_t (*release)(dev_t);
      dev_t    device;
} regDrv_t;





/*==================================================================================================
                                      Local function prototypes
==================================================================================================*/


/*==================================================================================================
                                      Local object definitions
==================================================================================================*/
static const regDrv_t drvList[] =
{
      {"uart1", UART_Init,  UART_Open,  UART_Close,  UART_Write,  UART_Read,  UART_IOCtl,  UART_Release,  UART_DEV_1   },
      {"gpio",  GPIO_Init,  GPIO_Open,  GPIO_Close,  GPIO_Write,  GPIO_Read,  GPIO_IOCtl,  GPIO_Release,  GPIO_DEV_NONE},
      {"pll",   PLL_Init,   PLL_Open,   PLL_Close,   PLL_Write,   PLL_Read,   PLL_IOCtl,   PLL_Release,   PLL_DEV_NONE },
      {"i2c1",  I2C_Init,   I2C_Open,   I2C_Close,   I2C_Write,   I2C_Read,   I2C_IOCtl,   I2C_Release,   I2C_DEV_1    },
      {"eth0",  ETHER_Init, ETHER_Open, ETHER_Close, ETHER_Write, ETHER_Read, ETHER_IOCtl, ETHER_Release, ETH_DEV_1    },
};


static struct devName_struct
{
      ch_t *node[ARRAY_SIZE(drvList)];
} *devName;


/*==================================================================================================
                                     Exported object definitions
==================================================================================================*/


/*==================================================================================================
                                        Function definitions
==================================================================================================*/

//================================================================================================//
/**
 * @brief Function find driver name and then initialize device
 *
 * @param *drvName            driver name
 * @param *nodeName           file name in /dev/ directory
 *
 * @return driver depending value, all not equal to STD_RET_OK are errors
 */
//================================================================================================//
stdRet_t InitDrv(const ch_t *drvName, ch_t *nodeName)
{
      stdRet_t status = STD_RET_ERROR;
      u32_t i;

      if (drvName && nodeName)
      {
            if (devName == NULL)
            {
                  devName = (struct devName_struct*)Malloc(ARRAY_SIZE(drvList) * sizeof(ch_t*));

                  if (devName == NULL)
                  {
                        goto InitDrv_End;
                  }
            }

            for (i = 0; i < ARRAY_SIZE(drvList); i++)
            {
                  if (strcmp(drvList[i].drvName, drvName) == 0)
                  {
                        status = drvList[i].init(drvList[i].device);

                        if (status == STD_RET_OK)
                        {
                              devName->node[i] = nodeName;
                              kprint("Created node /dev/%s\n", nodeName);
                        }
                        break;
                  }
            }
      }

      InitDrv_End:

      return status;
}


//================================================================================================//
/**
 * @brief Function find driver name and then release device
 *
 * @param *drvName            driver name
 *
 * @return driver depending value, all not equal to STD_RET_OK are errors
 */
//================================================================================================//
stdRet_t ReleaseDrv(const ch_t *drvName)
{
      stdRet_t status = STD_RET_ERROR;
      u32_t i;

      if (drvName)
      {
            for (i = 0; i < ARRAY_SIZE(drvList); i++)
            {
                  if (strcmp(drvList[i].drvName, drvName) == 0)
                  {
                        status = drvList[i].release(drvList[i].device);

                        if (status == STD_RET_OK && devName->node[i])
                        {
                              Free(devName->node[i]);
                              devName->node[i] = NULL;
                        }
                        break;
                  }
            }
      }

      return status;
}


//================================================================================================//
/**
 * @brief Function returns regisered driver list
 *
 * @param *drvNode            name of driver in /dev/ path
 *
 * @return driver depending value, all not equal to STD_RET_OK are errors
 */
//================================================================================================//
regDrvData_t GetDrvData(const ch_t *drvNode)
{
      regDrvData_t drvPtrs;
      drvPtrs.open   = NULL;
      drvPtrs.close  = NULL;
      drvPtrs.write  = NULL;
      drvPtrs.read   = NULL;
      drvPtrs.ioctl  = NULL;
      drvPtrs.device = 0;

      if (drvNode)
      {
            for (u8_t i = 0; i < ARRAY_SIZE(drvList); i++)
            {
                  if (strcmp(devName->node[i], drvNode) == 0)
                  {
                        drvPtrs.open   = drvList[i].open;
                        drvPtrs.close  = drvList[i].close;
                        drvPtrs.write  = drvList[i].write;
                        drvPtrs.read   = drvList[i].read;
                        drvPtrs.ioctl  = drvList[i].ioctl;
                        drvPtrs.device = drvList[i].device;

                        break;
                  }
            }
      }

      return drvPtrs;
}



#ifdef __cplusplus
}
#endif

/*==================================================================================================
                                            End of file
==================================================================================================*/
