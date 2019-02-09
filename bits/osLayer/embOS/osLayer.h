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
#include "RTOS.h"

#define SemaDefine(a)   OS_RSEMA a
#define SemaInit(a)     OS_CREATERSEMA(&a)
#define SemaWait(a)     OS_Use(&a)
#define SemaFree(a)     OS_Unuse(&a)

typedef struct {
  uint32_t timeLastReset ;
} TimerControl ;

// A very basic timer of time elapsed since previous time

uint32_t bits_sysTimer_get_time(
                                void ) ;

uint32_t bits_sysTimer_elapsed_milliseconds(
                                            uint32_t prevTime ) ;


// "Count up" timers that allow multiple instances
void bits_multiTimer_init(
                                 volatile TimerControl *timerControl );

uint32_t bits_multiTimer_elapsed_milliseconds(
                                              volatile TimerControl *timerControl );

void bits_multiTimer_reset(
                           volatile TimerControl *timerControl );

