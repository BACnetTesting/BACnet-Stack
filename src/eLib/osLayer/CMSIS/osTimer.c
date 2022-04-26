/**************************************************************************
 *
 * Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
 *
 * Created: 10/24/2013 8:58:56 PM
 * Author: Steve
 *
 * @page License
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
    Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

    This file contains changes made by BACnet Interoperability Testing
    Services, Inc. These changes are subject to the permissions,
    warranty terms and limitations above.
    For more information: info@bac-test.com
    For access to source code:  info@bac-test.com
            or      www.github.com/bacnettesting/bacnet-stack
 *
 **************************************************************************
 *
 *  Module Description:
 *
 *      Generate a periodic timer tick for use by generic timers in the code.
 *
 *************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "timerCommon.h"
#include "debug.h"
#include "stm32f4xx_hal.h"

/* counter for the various timers */
static volatile uint32_t Millisecond_Counter[MAX_MILLISECOND_TIMERS];

/*************************************************************************
* Description: compares the current time count with a value
* Returns: true if the time has elapsed
* Notes: none
*************************************************************************/
//bool timer_elapsed_seconds(
//    unsigned index,
//    uint32_t seconds)
//{
//    return ((timer_milliseconds(index) / 1000) >= seconds);
//}

/*************************************************************************
* Description: compares the current time count with a value
* Returns: true if the time has elapsed
* Notes: none
*************************************************************************/
//bool timer_elapsed_minutes(
//    unsigned index,
//    uint32_t minutes)
//{
//    return ((timer_milliseconds(index) / (1000 * 60)) >= minutes);
//}

/*************************************************************************
* Description: Sets the timer counter to zero.
* Returns: none
* Notes: none
*************************************************************************/
uint32_t timer_reset(
    unsigned index)
{
    uint32_t timer_value = timer_milliseconds(index);
    if (index < MAX_MILLISECOND_TIMERS) {
        Millisecond_Counter[index] = target_timer_milliseconds();
    }
    return timer_value;
}


/*************************************************************************
* Description: Shut down for timer
* Returns: none
* Notes: none
*************************************************************************/
static void timer_cleanup(
    void)
{
}

/*************************************************************************
* Description: returns the current millisecond count for a given timer
* Returns: none
* Notes: none
*************************************************************************/
uint32_t timer_milliseconds(
    unsigned index)
{
    uint32_t msNow = target_timer_milliseconds();
    uint32_t msStart = Millisecond_Counter[index];
    
    if (msNow >= msStart) return msNow - msStart ;
    // we have a rollover
    return (0xFFFFFFFFu - msNow) + 1 + msStart;
}


/*************************************************************************
* Description: returns the current SYSTEM millisecond count, rolls over
* Returns: none
* Notes: none
*************************************************************************/
uint32_t target_timer_milliseconds(
    void)
{
    // Supplied by STM32cubeMX templates.
    return HAL_GetTick();
}

/*************************************************************************
* Description: Timer setup for 1 millisecond timer
* Returns: none
* Notes: none
*************************************************************************/
void timer_init(
    void)
{
    // Using the STM32cubeMX system, the systick timer is already set up for a 1ms tick. No further action required.
}

