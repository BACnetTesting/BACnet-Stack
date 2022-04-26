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

#include <stdint.h>
#include "timerCommon.h"
#include "ledCommon.h"
#include "stm32f1xx.h"
#include "main.h"
#include "boardLED.h"

BOARD_LED_CB	boardLeds[3] ;

static BOARD_LED_CB *bits_board_led_find_indexed ( BOARD_LED_FUNCTION func, unsigned int index )
{
	for( int i=0; i<sizeof(boardLeds) / sizeof(BOARD_LED_CB); i++)
	{
		if ( boardLeds[i].board_led_function == func &&
				boardLeds[i].board_led_function_ix == index ) return &boardLeds[i] ;
	}
	return NULL ;
}


static BOARD_LED_CB *bits_board_led_find ( BOARD_LED_FUNCTION func )
{
	return bits_board_led_find_indexed( func, 0 ) ;
}


void bits_board_led_assert ( BOARD_LED_CB *bdLedCb, bool state )
{
	bdLedCb->currentState = state ;
	HAL_GPIO_WritePin(
			bdLedCb->GPIOx,
			bdLedCb->GPIO_Pin,
			( state ) ? GPIO_PIN_SET : GPIO_PIN_RESET );
}


void bits_board_led_toggle ( BOARD_LED_CB *bdLedCb )
{
	bits_board_led_assert ( bdLedCb, ! bdLedCb->currentState ) ;
}


void bits_board_led_function_assert_indexed ( BOARD_LED_FUNCTION func, unsigned index, bool state )
{
	BOARD_LED_CB *ble = bits_board_led_find_indexed ( func, index ) ;
	if ( ble ) bits_board_led_assert( ble, state );
}


void bits_board_led_function_assert ( BOARD_LED_FUNCTION func,  bool state )
{
	BOARD_LED_CB *ble = bits_board_led_find ( func ) ;
	if ( ble ) bits_board_led_assert( ble, state );
}


void bits_board_led_heartbeat_toggle ( void )
{
	// does not seem to work HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, ( state ) ? GPIO_PIN_SET : GPIO_PIN_RESET );	// green
	// HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, ( state ) ? GPIO_PIN_SET : GPIO_PIN_RESET );	// blue
	// HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, ( state ) ? GPIO_PIN_SET : GPIO_PIN_RESET );	// red

	// since this gets called often, do some memoirization
	static BOARD_LED_CB *ble ;
	static bool failed ;

	if ( ble ) {
		bits_board_led_toggle ( ble ) ;
		return ;
	}

	if ( failed ) return ;

	ble = bits_board_led_find ( BOARD_LED_FUNCTION_HEARTBEAT ) ;
	if ( ble ) {
		bits_board_led_toggle ( ble ) ;
		return ;
	}

	// not set up, so fail.
	failed = true ;
}


// set msPulseTime to 0 if function not desired
//void bits_board_led_init ( LED_CB *ledCB, GPIO_TypeDef* GPIOx, unsigned int16_t GPIO_Pin, unsigned int16_t msPulseTime )
//{
//	ledCB->GPIO_Pin = GPIO_Pin ;
//	ledCB->GPIOx = GPIOx ;
//	ledCB->msPulseTime = msPulseTime ;
//}

void bits_board_led_pulse_indexed (
		BOARD_LED_FUNCTION func,
		unsigned int index,
		unsigned int durationMillisecs )
{
	BOARD_LED_CB *ble = bits_board_led_find_indexed ( func, index ) ;
	if ( ! ble ) return ;

	if ( ! ble->timeRemaining ) {
		ble->timeRemaining = durationMillisecs ;
		bits_board_led_assert( ble, true );
	}
}


void bits_board_led_idle( unsigned int milliSecs )
{
	for( int i=0; i<sizeof(boardLeds) / sizeof(BOARD_LED_CB); i++)
	{
		BOARD_LED_CB *ble = &boardLeds[i] ;
		if ( ble->timeRemaining )
		{
			if ( ble->timeRemaining <= milliSecs )
			{
				bits_board_led_toggle( ble );
				// set up timer if there is a repeat time, else zero it by default
				ble->timeRemaining = ble->repeatTime ;
			}
			else
			{
				ble->timeRemaining -= milliSecs ;
			}
		}
	}
}


void bits_board_led_init (
		void )
{
	boardLeds[BOARD_LED_BLUE].GPIOx = LD2_GPIO_Port ;
	boardLeds[BOARD_LED_BLUE].GPIO_Pin = LD2_Pin ;
	boardLeds[BOARD_LED_BLUE].board_led_function = BOARD_LED_FUNCTION_HEARTBEAT ;
	// boardLeds[BOARD_LED_BLUE].board_led_function = BOARD_LED_FUNCTION_DATALINK_OTHER_TX ;

//	boardLeds[BOARD_LED_RED].GPIOx = LD3_GPIO_Port ;
//	boardLeds[BOARD_LED_RED].GPIO_Pin = LD3_Pin ;
	boardLeds[BOARD_LED_RED].board_led_function = BOARD_LED_FUNCTION_DATALINK_OTHER_TX ;
	// boardLeds[BOARD_LED_RED].board_led_function = BOARD_LED_FUNCTION_HEARTBEAT ;
	boardLeds[BOARD_LED_RED].board_led_function_ix = 0 ;

	// leave green uninitialized for now
}

