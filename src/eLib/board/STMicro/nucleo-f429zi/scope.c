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

#ifdef todo1
#include "stm32f4xx_hal_gpio.h"
#endif

#include "scope.h"

void bits_ScopeInit(void)
{
#ifdef todo1
    GPIO_InitTypeDef GPIO_InitStruct;

    /*Configure GPIO pin : PC1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif
}

static uint pulsetimerMs;

void bits_ScopeTrigger(void)
{
    pulsetimerMs = 4;
#ifdef todo1
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
#endif
}


void bits_ScopeIdle(uint deltaMs)
{
    if (pulsetimerMs)
    {
        if (deltaMs > pulsetimerMs)
        {
            pulsetimerMs = 0;
        }
        else
        {
            pulsetimerMs -= deltaMs;
        }

        if (pulsetimerMs == 0)
        {
#ifdef todo1
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
#endif
        }
    }
    else
    {
        pulsetimerMs -= deltaMs;
    }
}
