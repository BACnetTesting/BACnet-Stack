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
#include "os485.h"
#include "boardLED.h"
#include "main.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void board_rs485_init(
		RS485_CB *rs485cb,
		const char *comport )
{
	switch ( comport[bits_strlen( comport )-1] )
	{
	case '1' :
		rs485cb->uartHandle = &huart1 ;
		break;

	case '2' :
		rs485cb->uartHandle = &huart2 ;
		break;

	default:
		// todo 1 panic();
		rs485cb->uartHandle = NULL ;
		break;
	}
}

