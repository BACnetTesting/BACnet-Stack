/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

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

