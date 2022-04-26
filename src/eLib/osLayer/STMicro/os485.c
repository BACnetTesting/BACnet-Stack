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

#include "board485.h"
#include "bitsUtil.h"

#if defined(STM32F103xB)
	#include "stm32f1xx_hal.h"
#elif defined(STM32F429xx)
	#include "stm32f4xx_hal.h"
#elif defined(STM32G071xx)
  #include "stm32g0xx_hal.h"
#else
	#error "Not supported platform."
#endif


void os485_init(
		RS485_CB *rs485cb,
		const char *comport )
{
	board_rs485_init( rs485cb, comport ) ;
}


void os485_send( RS485_CB *rs485cb, uint8_t *buffer, uint16_t len )
{
	bits_board_led_pulse_indexed ( BOARD_LED_FUNCTION_DATALINK_OTHER_TX, 0, 200 ) ;

	// send in blocking mode (buffer can be ephemeral)
	HAL_UART_Transmit( rs485cb->uartHandle, buffer, len, 100 ) ;

//	rs485cb->uartHandle->Instance->DR = buffer[0];
//	rs485cb->uartHandle->Instance->CR1 |= USART_CR1_TXEIE ;

//    huart1.Instance->DR = tx_byte ;
//    huart1.Instance->CR1 |= USART_CR1_TXEIE ;
//    SilenceTimerReset(mstp_port);
}

