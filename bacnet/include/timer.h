/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*****************************************************************************************
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
*   June 30, 2018   BITS    Renamed this from timer.h to timerCommon.h because of collisions
*                           with h files in other platforms. Moved it to include/ because
*                           the functions are general amongst most platforms. Platform
*                           specific declarations can remain in ports\*\something.h
*
****************************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __GNUC__
// #include <sys/time.h>   /* for timeval */
#endif

// Older versions of GCC does not have these
#ifndef UINT32_MAX
#define UINT32_MAX  ((uint32_t)-1)
#define UINT16_MAX  ((uint16_t)-1)
#endif

/* Timer Module */
#ifndef MAX_MILLISECOND_TIMERS
#define MAX_MILLISECOND_TIMERS 1
#endif

/* elapsed timer structure */
typedef struct etimer {
    uint32_t start;
} ETIMER ;

/* interval timer structure */
struct itimer {
    uint32_t start;
    uint32_t interval;
};

void timer_elapsed_start(
    struct etimer *t);

uint32_t timer_elapsed_time(
    volatile struct etimer *t);

//uint32_t timeGetTime(
//    void);

void timer_init(
    void);

uint32_t target_timer_milliseconds(
    void);

uint32_t timer_milliseconds(
    unsigned index);

bool timer_elapsed_milliseconds(
    struct etimer *t,
    uint32_t milliseconds);

bool timer_elapsed_seconds(
    struct etimer * t,
    uint32_t seconds);

bool timer_elapsed_minutes(
    struct etimer * t,
    uint32_t minutes);

uint32_t timer_milliseconds_set(
    unsigned index,
	uint32_t value);

uint32_t timer_reset(
    unsigned index);

#endif
