#ifndef _UART_DEF_H_
#define _UART_DEF_H_
/*=========================================================================*//**
@file    usart_def.h

@author  Daniel Zorychta

@brief   This file support statuses and request of USART peripherals

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

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "system/ioctl_macros.h"
#include "stm32f1/uart_cfg.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
/** UART minor number */
#define UART_MINOR_NUMBER                       0

/** port names */
enum UART_MAJOR_NUMBER
{
#if defined(RCC_APB2ENR_USART1EN) && (UART_1_ENABLE > 0)
        UART_DEV_1,
#endif

#if defined(RCC_APB1ENR_USART2EN) && (UART_2_ENABLE > 0)
        UART_DEV_2,
#endif

#if defined(RCC_APB1ENR_USART3EN) && (UART_3_ENABLE > 0)
        UART_DEV_3,
#endif

#if defined(RCC_APB1ENR_UART4EN) && (UART_4_ENABLE > 0)
        UART_DEV_4,
#endif

#if defined(RCC_APB1ENR_UART5EN) && (UART_5_ENABLE > 0)
        UART_DEV_5,
#endif

        UART_DEV_COUNT
};

/** IO request for UART driver (0x5452 - "TR") */
#define UART_IORQ_ENABLE_WAKEUP_IDLE            _IORQ(0x00, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_ENABLE_WAKEUP_ADDRESS_MARK    _IORQ(0x01, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_ENABLE_PARITY_CHECK           _IORQ(0x02, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_DISABLE_PARITY_CHECK          _IORQ(0x03, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_ODD_PARITY                _IORQ(0x04, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_EVEN_PARITY               _IORQ(0x05, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_ENABLE_RECEIVER_WAKEUP_MUTE   _IORQ(0x06, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_DISABLE_RECEIVER_WAKEUP_MUTE  _IORQ(0x07, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_ENABLE_LIN_MODE               _IORQ(0x08, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_DISABLE_LIN_MODE              _IORQ(0x09, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_1_STOP_BIT                _IORQ(0x0A, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_2_STOP_BITS               _IORQ(0x0B, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_LIN_BRK_DETECTOR_11_BITS  _IORQ(0x0C, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_LIN_BRK_DETECTOR_10_BITS  _IORQ(0x0D, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_SET_ADDRESS_NODE              _IORQ(0x0E, 0x5452, _DIR_WR, sizeof(int))
#define UART_IORQ_ENABLE_CTS                    _IORQ(0x0F, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_DISABLE_CTS                   _IORQ(0x10, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_ENABLE_RTS                    _IORQ(0x11, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_DISABLE_RTS                   _IORQ(0x12, 0x5452, _DIR_NONE, 0)
#define UART_IORQ_GET_BYTE                      _IORQ(0x13, 0x5452, _DIR_RD, sizeof(u8_t*))
#define UART_IORQ_GET_BYTE_BLOCKING             _IORQ(0x14, 0x5452, _DIR_RD, sizeof(u8_t*))
#define UART_IORQ_SEND_BYTE                     _IORQ(0x15, 0x5452, _DIR_WR, sizeof(int))
#define UART_IORQ_SET_BAUDRATE                  _IORQ(0x16, 0x5452, _DIR_WR, sizeof(int))

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
 Exported function prototypes
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _UART_DEF_H_ */
/*==============================================================================
  End of file
==============================================================================*/
