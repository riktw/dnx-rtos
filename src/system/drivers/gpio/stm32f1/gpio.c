/*=========================================================================*//**
@file    gpio.c

@author  Daniel Zorychta

@brief   This driver support GPIO. GPIO driver not provide any interface
         functions. All operations on ports should be made direct via definitions
         (much faster). When operation on a ports are needed please write own
         driver which controls pins directly and register it in the VFS if
         needed.

@note    Copyright (C) 2012  Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "drivers/gpio.h"
#include "stm32f1/stm32f10x.h"

MODULE_NAME(GPIO);

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
/** define pin configure size (CNF[1:0] + MODE[1:0]) */
#define GPIO_PIN_CFG_SIZE                       4U

/** define CRL configuration macro */
#define GPIO_SET_CRL(CFG, PIN)                  ( CFG << (GPIO_PIN_CFG_SIZE * PIN) )

/** define CRH configuration macro */
#define GPIO_SET_CRH(CFG, PIN)                  ( CFG << (GPIO_PIN_CFG_SIZE * (PIN - 8)) )

/** define macro used to set port's pins */
#define SET_PIN(port, pin_mask)                 port->BSRR = pin_mask

/** define macro used to clear port's pins */
#define CLEAR_PIN(port, pin_mask)               port->BRR = pin_mask

#if (GPIOA_EN != 0)
/** CRL register value for GPIOA */
#define GPIOA_CRL    ( GPIO_SET_CRL(GPIOA_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOA_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOA_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOA_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOA_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOA_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOA_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOA_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOA */
#define GPIOA_CRH    ( GPIO_SET_CRH(GPIOA_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOA_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOA_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOA_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOA_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOA_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOA_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOA_PIN_15_MODE, 15) )

/** ODR register value for GPIOA */
#define GPIOA_ODR    ( (GPIOA_PIN_00_DEFAULT_STATE << 0 ) | (GPIOA_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOA_PIN_02_DEFAULT_STATE << 2 ) | (GPIOA_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOA_PIN_04_DEFAULT_STATE << 4 ) | (GPIOA_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOA_PIN_06_DEFAULT_STATE << 6 ) | (GPIOA_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOA_PIN_08_DEFAULT_STATE << 8 ) | (GPIOA_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOA_PIN_10_DEFAULT_STATE << 10) | (GPIOA_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOA_PIN_12_DEFAULT_STATE << 12) | (GPIOA_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOA_PIN_14_DEFAULT_STATE << 14) | (GPIOA_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOB_EN != 0)
/** CRL register value for GPIOB */
#define GPIOB_CRL    ( GPIO_SET_CRL(GPIOB_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOB_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOB_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOB_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOB_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOB_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOB_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOB_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOB */
#define GPIOB_CRH    ( GPIO_SET_CRH(GPIOB_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOB_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOB_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOB_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOB_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOB_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOB_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOB_PIN_15_MODE, 15) )

/** ODR register value for GPIOB */
#define GPIOB_ODR    ( (GPIOB_PIN_00_DEFAULT_STATE << 0 ) | (GPIOB_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOB_PIN_02_DEFAULT_STATE << 2 ) | (GPIOB_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOB_PIN_04_DEFAULT_STATE << 4 ) | (GPIOB_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOB_PIN_06_DEFAULT_STATE << 6 ) | (GPIOB_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOB_PIN_08_DEFAULT_STATE << 8 ) | (GPIOB_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOB_PIN_10_DEFAULT_STATE << 10) | (GPIOB_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOB_PIN_12_DEFAULT_STATE << 12) | (GPIOB_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOB_PIN_14_DEFAULT_STATE << 14) | (GPIOB_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOC_EN != 0)
/** CRL register value for GPIOC */
#define GPIOC_CRL    ( GPIO_SET_CRL(GPIOC_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOC_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOC_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOC_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOC_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOC_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOC_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOC_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOC */
#define GPIOC_CRH    ( GPIO_SET_CRH(GPIOC_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOC_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOC_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOC_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOC_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOC_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOC_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOC_PIN_15_MODE, 15) )

/** ODR register value for GPIOC */
#define GPIOC_ODR    ( (GPIOC_PIN_00_DEFAULT_STATE << 0 ) | (GPIOC_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOC_PIN_02_DEFAULT_STATE << 2 ) | (GPIOC_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOC_PIN_04_DEFAULT_STATE << 4 ) | (GPIOC_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOC_PIN_06_DEFAULT_STATE << 6 ) | (GPIOC_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOC_PIN_08_DEFAULT_STATE << 8 ) | (GPIOC_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOC_PIN_10_DEFAULT_STATE << 10) | (GPIOC_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOC_PIN_12_DEFAULT_STATE << 12) | (GPIOC_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOC_PIN_14_DEFAULT_STATE << 14) | (GPIOC_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOD_EN != 0)
/** CRL register value for GPIOD */
#define GPIOD_CRL    ( GPIO_SET_CRL(GPIOD_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOD_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOD_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOD_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOD_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOD_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOD_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOD_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOD */
#define GPIOD_CRH    ( GPIO_SET_CRH(GPIOD_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOD_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOD_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOD_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOD_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOD_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOD_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOD_PIN_15_MODE, 15) )

/** ODR register value for GPIOD */
#define GPIOD_ODR    ( (GPIOD_PIN_00_DEFAULT_STATE << 0 ) | (GPIOD_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOD_PIN_02_DEFAULT_STATE << 2 ) | (GPIOD_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOD_PIN_04_DEFAULT_STATE << 4 ) | (GPIOD_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOD_PIN_06_DEFAULT_STATE << 6 ) | (GPIOD_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOD_PIN_08_DEFAULT_STATE << 8 ) | (GPIOD_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOD_PIN_10_DEFAULT_STATE << 10) | (GPIOD_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOD_PIN_12_DEFAULT_STATE << 12) | (GPIOD_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOD_PIN_14_DEFAULT_STATE << 14) | (GPIOD_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOE_EN != 0)
/** CRL register value for GPIOE */
#define GPIOE_CRL    ( GPIO_SET_CRL(GPIOE_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOE_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOE_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOE_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOE_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOE_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOE_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOE_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOE */
#define GPIOE_CRH    ( GPIO_SET_CRH(GPIOE_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOE_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOE_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOE_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOE_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOE_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOE_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOE_PIN_15_MODE, 15) )

/** ODR register value for GPIOE */
#define GPIOE_ODR    ( (GPIOE_PIN_00_DEFAULT_STATE << 0 ) | (GPIOE_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOE_PIN_02_DEFAULT_STATE << 2 ) | (GPIOE_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOE_PIN_04_DEFAULT_STATE << 4 ) | (GPIOE_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOE_PIN_06_DEFAULT_STATE << 6 ) | (GPIOE_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOE_PIN_08_DEFAULT_STATE << 8 ) | (GPIOE_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOE_PIN_10_DEFAULT_STATE << 10) | (GPIOE_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOE_PIN_12_DEFAULT_STATE << 12) | (GPIOE_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOE_PIN_14_DEFAULT_STATE << 14) | (GPIOE_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOF_EN != 0)
/** CRL register value for GPIOF */
#define GPIOF_CRL    ( GPIO_SET_CRL(GPIOF_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOF_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOF_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOF_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOF_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOF_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOF_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOF_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOF */
#define GPIOF_CRH    ( GPIO_SET_CRH(GPIOF_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOF_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOF_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOF_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOF_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOF_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOF_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOF_PIN_15_MODE, 15) )

/** ODR register value for GPIOF */
#define GPIOF_ODR    ( (GPIOF_PIN_00_DEFAULT_STATE << 0 ) | (GPIOF_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOF_PIN_02_DEFAULT_STATE << 2 ) | (GPIOF_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOF_PIN_04_DEFAULT_STATE << 4 ) | (GPIOF_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOF_PIN_06_DEFAULT_STATE << 6 ) | (GPIOF_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOF_PIN_08_DEFAULT_STATE << 8 ) | (GPIOF_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOF_PIN_10_DEFAULT_STATE << 10) | (GPIOF_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOF_PIN_12_DEFAULT_STATE << 12) | (GPIOF_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOF_PIN_14_DEFAULT_STATE << 14) | (GPIOF_PIN_15_DEFAULT_STATE << 15) )
#endif

#if (GPIOG_EN != 0)
/** CRL register value for GPIOG */
#define GPIOG_CRL    ( GPIO_SET_CRL(GPIOG_PIN_00_MODE, 0 ) | GPIO_SET_CRL(GPIOG_PIN_01_MODE, 1 ) \
                     | GPIO_SET_CRL(GPIOG_PIN_02_MODE, 2 ) | GPIO_SET_CRL(GPIOG_PIN_03_MODE, 3 ) \
                     | GPIO_SET_CRL(GPIOG_PIN_04_MODE, 4 ) | GPIO_SET_CRL(GPIOG_PIN_05_MODE, 5 ) \
                     | GPIO_SET_CRL(GPIOG_PIN_06_MODE, 6 ) | GPIO_SET_CRL(GPIOG_PIN_07_MODE, 7 ) )

/** CRH register value for GPIOG */
#define GPIOG_CRH    ( GPIO_SET_CRH(GPIOG_PIN_08_MODE, 8 ) | GPIO_SET_CRH(GPIOG_PIN_09_MODE, 9 ) \
                     | GPIO_SET_CRH(GPIOG_PIN_10_MODE, 10) | GPIO_SET_CRH(GPIOG_PIN_11_MODE, 11) \
                     | GPIO_SET_CRH(GPIOG_PIN_12_MODE, 12) | GPIO_SET_CRH(GPIOG_PIN_13_MODE, 13) \
                     | GPIO_SET_CRH(GPIOG_PIN_14_MODE, 14) | GPIO_SET_CRH(GPIOG_PIN_15_MODE, 15) )

/** ODR register value for GPIOG */
#define GPIOG_ODR    ( (GPIOG_PIN_00_DEFAULT_STATE << 0 ) | (GPIOG_PIN_01_DEFAULT_STATE << 1 ) \
                     | (GPIOG_PIN_02_DEFAULT_STATE << 2 ) | (GPIOG_PIN_03_DEFAULT_STATE << 3 ) \
                     | (GPIOG_PIN_04_DEFAULT_STATE << 4 ) | (GPIOG_PIN_05_DEFAULT_STATE << 5 ) \
                     | (GPIOG_PIN_06_DEFAULT_STATE << 6 ) | (GPIOG_PIN_07_DEFAULT_STATE << 7 ) \
                     | (GPIOG_PIN_08_DEFAULT_STATE << 8 ) | (GPIOG_PIN_09_DEFAULT_STATE << 9 ) \
                     | (GPIOG_PIN_10_DEFAULT_STATE << 10) | (GPIOG_PIN_11_DEFAULT_STATE << 11) \
                     | (GPIOG_PIN_12_DEFAULT_STATE << 12) | (GPIOG_PIN_13_DEFAULT_STATE << 13) \
                     | (GPIOG_PIN_14_DEFAULT_STATE << 14) | (GPIOG_PIN_15_DEFAULT_STATE << 15) )
#endif

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/
static void init_GPIOx(GPIO_t *gpio, u32_t crl, u32_t crh, u32_t odr);
static void init_AFIO(void);

/*==============================================================================
  Local object definitions
==============================================================================*/

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Initialise all GPIOs
 *
 * @param[out] **drvhdl         driver's memory handle
 * @param[in]  dev              device number
 * @param[in]  part             device part
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_init(void **drvhdl, uint dev, uint part)
{
        (void)drvhdl;
        (void)dev;
        (void)part;

#if (AFIO_EN > 0)
        init_AFIO();
#endif

#if (GPIOA_EN > 0)
        init_GPIOx(GPIOA, GPIOA_CRL, GPIOA_CRH, GPIOA_ODR);
#endif

#if (GPIOB_EN > 0)
        init_GPIOx(GPIOB, GPIOB_CRL, GPIOB_CRH, GPIOB_ODR);
#endif

#if (GPIOC_EN > 0)
        init_GPIOx(GPIOC, GPIOC_CRL, GPIOC_CRH, GPIOC_ODR);
#endif

#if (GPIOD_EN > 0)
        init_GPIOx(GPIOD, GPIOD_CRL, GPIOD_CRH, GPIOD_ODR);
#endif

#if (GPIOE_EN > 0)
        init_GPIOx(GPIOE, GPIOE_CRL, GPIOE_CRH, GPIOE_ODR);
#endif

#if (GPIOF_EN > 0)
        init_GPIOx(GPIOF, GPIOF_CRL, GPIOF_CRH, GPIOF_ODR);
#endif

#if (GPIOG_EN > 0)
        init_GPIOx(GPIOG, GPIOG_CRL, GPIOG_CRH, GPIOG_ODR);
#endif

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Release GPIO devices. Here is removed driver node and reseted device
 *
 * @param[in] *drvhdl           driver's memory handle
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_release(void *drvhdl)
{
        (void)drvhdl;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Open device
 *
 * @param[in] *drvhdl           driver's memory handle
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_open(void *drvhdl)
{
        (void)drvhdl;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Close device
 *
 *
 * @param[in] *drvhdl           driver's memory handle
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_close(void *drvhdl)
{
        (void)drvhdl;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Write to the device
 *
 * @param[in] *drvhdl           driver's memory handle
 * @param[in] *src              source
 * @param[in] size              size
 * @param[in] seek              seek
 *
 * @retval number of written nitems
 */
//==============================================================================
size_t GPIO_write(void *drvhdl, const void *src, size_t size, size_t nitems, size_t seek)
{
        (void)drvhdl;
        (void)src;
        (void)size;
        (void)seek;
        (void)nitems;

        return 0;
}

//==============================================================================
/**
 * @brief Read from device
 *
 * @param[in]  *drvhdl          driver's memory handle
 * @param[out] *dst             destination
 * @param[in]  size             size
 * @param[in]  seek             seek
 *
 * @retval number of read nitems
 */
//==============================================================================
size_t GPIO_read(void *drvhdl, void *dst, size_t size, size_t nitems, size_t seek)
{
        (void)drvhdl;
        (void)dst;
        (void)size;
        (void)seek;
        (void)nitems;

        return 0;
}

//==============================================================================
/**
 * @brief IO control
 *
 * @param[in]     *drvhdl       driver's memory handle
 * @param[in]     ioRq          IO reqest
 * @param[in,out] args          additinal arguments
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_ioctl(void *drvhdl, int iorq, va_list args)
{
        (void)drvhdl;
        (void)args;

        switch (iorq) {
        case GPIO_IORQ_SD_SELECT:
                CLEAR_PIN(SD_CS_PORT, SD_CS_BM);
                break;

        case GPIO_IORQ_SD_DESELECT:
                SET_PIN(SD_CS_PORT, SD_CS_BM);
                break;

        default:
                return STD_RET_ERROR;
        }

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Function flush device
 *
 * @param[in] *drvhdl           driver's memory handle
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
stdret_t GPIO_flush(void *drvhdl)
{
        (void)drvhdl;

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Initialize GPIOx
 *
 * @param[in] *gpio             GPIO address
 * @param[in] crl               CRL register value
 * @param[in] crh               CRH register value
 * @param[in] odr               ODR register value
 */
//==============================================================================
static void init_GPIOx(GPIO_t *gpio, u32_t crl, u32_t crh, u32_t odr)
{
        /* enable peripherals */
        switch ((u32_t)gpio) {
#if (GPIOA_EN > 0)
        case GPIOA_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
                break;
#endif

#if (GPIOB_EN > 0)
        case GPIOB_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
                break;
#endif

#if (GPIOC_EN > 0)
        case GPIOC_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
                break;
#endif

#if (GPIOD_EN > 0)
        case GPIOD_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
                break;
#endif

#if (GPIOE_EN > 0)
        case GPIOE_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
                break;
#endif

#if (GPIOF_EN > 0)
        case GPIOF_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
                break;
#endif

#if (GPIOG_EN > 0)
        case GPIOG_BASE:
                RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;
                break;
#endif

        default:
                return;
        }

        /* pin 0-7 configuration */
        gpio->CRL = crl;

        /* pin 8-15 configuration */
        gpio->CRH = crh;

        /* configure outputs and inputs initial state */
        gpio->ODR = odr;
}

//==============================================================================
/**
 * @brief Initialize AFIO peripheral
 */
//==============================================================================
static void init_AFIO(void)
{
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

#if (SPI3_REMAP > 0)
        AFIO->MAPR |= AFIO_MAPR_SPI3_REMAP;
#endif
}


#ifdef __cplusplus
}
#endif

/*==============================================================================
                                             End of file
==============================================================================*/
