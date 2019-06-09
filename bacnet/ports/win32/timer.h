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
****************************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>

#include <sys/timeb.h>

/* Timer Module */
#ifndef MAX_MILLISECOND_TIMERS
#define TIMER_SILENCE 0
#define MAX_MILLISECOND_TIMERS 1
#endif

int gettimeofday(
struct timeval *tp,
    void *tzp);

void timer_init(
    void);

uint32_t timer_milliseconds(
    unsigned index);

bool timer_elapsed_milliseconds(
    unsigned index,
    uint32_t value);

bool timer_elapsed_seconds(
    unsigned index,
    uint32_t value);

bool timer_elapsed_minutes(
    unsigned index,
    uint32_t seconds);

uint32_t timer_milliseconds_set(
    unsigned index,
    uint32_t value);

uint32_t timer_reset(
    unsigned index);

#endif
