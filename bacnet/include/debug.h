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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bacdef.h"

// finer grained debugging
// Set the following to 0 to disable. Any value > 0 adds more and more detail...

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

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


#if defined ( _MSC_VER  )

// Compiling with /Wall, but have to suppress the following, and then re-enable some after Microsoft includes.

// Warnings disabled in property manager file "bacnet solutions settings.prop"
// 4820; 4214; 4244; 4996; 4668; 4100; 4242; 4255; 4710; 4267; 4206; 4389; 4548;

#pragma warning( error : 4255)    // function prototype not declared

#endif

#endif // DEBUG_H
