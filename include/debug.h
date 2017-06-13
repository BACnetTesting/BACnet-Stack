/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

//#include <stdint.h>
#include <stdbool.h>
//#include <stdio.h>
//#include "bacdef.h"

// finer grained debugging
// Set the following to 0 to disable. Any value > 0 adds more and more detail...

#define	DB_TRAFFIC	1

typedef enum {
    DB_DEBUG,
    DB_INFO,
    DB_NORMAL_TRAFFIC,
    DB_NOTE,
    DB_EXPECTED_ERROR_TRAFFIC,
    DB_UNUSUAL_TRAFFIC,
    DB_ERROR,
    DB_BTC_ERROR,                   // Errors that are expected during BTC testing.
    DB_UNEXPECTED_ERROR,
    DB_NONE			// so we can turn off a single statement easily
} DB_LEVEL;

#define	dbTrafficSetLevel(lev)			        sys_dbTrafficSetLevel(lev)
#define	dbTraffic(lev, ...)			            sys_dbTraffic(lev,__VA_ARGS__)
#define	dbTrafficAssert(lev, assertion, msg)	sys_dbTrafficAssert(lev, assertion, msg)
        
void sys_dbTraffic( DB_LEVEL lev, const char *format, ...);
void sys_dbTrafficAssert(DB_LEVEL lev, bool assertion, const char *message);

// Sets level filter to only display debug messages of priority of lev or above
void sys_dbTrafficSetLevel(DB_LEVEL lev);

void debug_printf(
    const char *format,
    ...);

#if DEBUG_ENABLED
/* Nothing more here */
#else
    /* If your compiler supports it, this is more compact:
       inline void debug_printf(
   const char *format,
   ...) {
   format = format;
   }
 */
#endif
#else // DB_TRAFFIC not defined

//#define	dbTrafficSetLevel(lev)
//#define	dbTraffic(lev, ...)
//#define dbTrafficAssert(lev, ...)
    
#endif // DB_TRAFFIC



void print_address_cache(void);
void print_tsm_cache(void);
