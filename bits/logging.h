/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
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

// Logging levels

typedef enum
{
    Debug,
    Release,
    None
} LLEV ;

void log_printf(const char *fmt, ...);
void log_puts(const char *msg );
void log_putc(const char c);
void log_puts_toFile(const char *msg);

void log_lev_printf( const LLEV lev, const char *fmt, ...);


void sys_log_lev_trace(const LLEV level, const char *file, const int line);
void sys_log_assert(int assrt, const char *file, const int line);

#define log_lev_trace(a)    sys_log_lev_trace(a, __FILE__, __LINE__ )
#define log_assert(a)       sys_log_assert(a, __FILE__, __LINE__ )
// in bitsDebug.h #define panic()             log_printf("Panic %d, %s\n", __LINE__, __FILE__ )
