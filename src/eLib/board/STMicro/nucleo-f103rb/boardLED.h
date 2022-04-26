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

#pragma once

#include <stdint.h>

#ifdef STM32F103xB
	#include "stm32f1xx_hal.h"
#endif
#ifdef STM32F429xx
	#include "stm32f4xx_hal.h"
#endif

#include "ledCommon.h"

// the nucleo-f429zi only has 3 leds, and the green one currently is not working !

typedef enum {
	BOARD_LED_GREEN,
	BOARD_LED_BLUE,
	BOARD_LED_RED,
} BOARD_LED ;

typedef enum {
	BOARD_LED_FUNCTION_NONE,
	BOARD_LED_FUNCTION_HEARTBEAT,
	BOARD_LED_FUNCTION_SYS_ERR,
	BOARD_LED_FUNCTION_CONFIG_ERR,
	BOARD_LED_FUNCTION_PERSISTING,
	BOARD_LED_FUNCTION_CHECK_STATS,
	BOARD_LED_FUNCTION_DATALINK_OTHER_TX,
	BOARD_LED_FUNCTION_DATALINK_OTHER_RX,
	BOARD_LED_FUNCTION_DATALINK_OTHER_ERR,
	BOARD_LED_FUNCTION_DATALINK_MSTP_TX,
	BOARD_LED_FUNCTION_DATALINK_MSTP_RX,
	BOARD_LED_FUNCTION_DATALINK_MSTP_ERR,
	BOARD_LED_FUNCTION_DATALINK_ETH_TX,
	BOARD_LED_FUNCTION_DATALINK_ETH_RX,
	BOARD_LED_FUNCTION_DATALINK_ETH_ERR,
} BOARD_LED_FUNCTION ;

typedef struct {
	GPIO_TypeDef* 		GPIOx ;
	uint16_t 			GPIO_Pin ;
	BOARD_LED_FUNCTION 	board_led_function ;
	unsigned int				board_led_function_ix ;
	unsigned int				timeRemaining ;
	unsigned int				repeatTime ;
	bool				currentState ;
} BOARD_LED_CB ;


//void bits_board_led_init (
//		LED_CB *ledCB,
//		GPIO_TypeDef* GPIOx,
//		uint16_t GPIO_Pin,
//		uint16_t msPulseTime ) ;

void bits_board_led_init (
		void ) ;

void bits_board_led_idle(
		unsigned int milliSecs ) ;

void bits_board_led_pulse_indexed (
		BOARD_LED_FUNCTION func,
		unsigned int index,
		unsigned int durationMillisecs ) ;

void bits_board_led_toggle (
		BOARD_LED_CB *bdLedCb ) ;

void bits_board_led_assert (
		BOARD_LED_CB *bdLedCb,
		bool state ) ;

void bits_board_led_heartbeat_toggle ( void ) ;
