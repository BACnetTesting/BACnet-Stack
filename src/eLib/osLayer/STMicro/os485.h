/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#ifndef OS485_HEADER
#define OS485_HEADER

#if defined(STM32F103xB)
	#include "stm32f1xx_hal.h"
#elif defined(STM32F429xx)
	#include "stm32f4xx_hal.h"
#elif defined(STM32G071xx)
  #include "stm32g0xx_hal.h"
#else
	#error "Not supported platform."
#endif

// #include "boardLED.h"

typedef struct {
    // BOARD_LED_CB		led_cb ;
    uint32_t  			baudRate ;
    UART_HandleTypeDef	*uartHandle ;
} RS485_CB2 ;


void os485_init(
		RS485_CB2 *rs485cb,
		const char *comport ) ;

void os485_send(
		RS485_CB2 *rs485cb,
		uint8_t *buffer,
		uint16_t len ) ;


#endif // OS485_HEADER
