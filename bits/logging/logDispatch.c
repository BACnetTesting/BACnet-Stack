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

#include <stdio.h>
#ifdef _MSC_VER
#include <conio.h>
#endif
#include <stdarg.h>
#include <stdlib.h>

#include "logging.h"

void log_printf(const char *format, ...)
{
    va_list ap;
    char tbuf[1000];

    va_start(ap, format);
    vsnprintf(tbuf, sizeof(tbuf)-1, format, ap);
    va_end(ap);

    tbuf[sizeof(tbuf) - 1] = 0;
    log_puts(tbuf);
}

void log_lev_printf(const LLEV lev, const char *format, ...)
{
    (void)lev; // for now
    va_list ap;
    char tbuf[1000];

    va_start(ap, format);
    vsnprintf(tbuf, sizeof(tbuf)-1, format, ap);
    va_end(ap);

    tbuf[sizeof(tbuf) - 1] = 0;
    log_puts(tbuf);
}

void log_puts(
    const char *msg)
{
    puts(msg);
    fflush(stdout);

#if ( LOG_TO_FILE )
    log_puts_toFile(msg);
#endif
}

void log_putc(const char val)
{
#ifdef _MSC_VER
    _putch(val);
#else
    putchar(val);
#endif
}

void sys_log_lev_trace(const LLEV level, const char *file, const int line)
{
    // log_printf("trace: %d %s\n", line, file);
    char tbuf[1000];
    snprintf(tbuf, sizeof(tbuf) - 1, "trace: %d %s\n", line, file );
    tbuf[sizeof(tbuf) - 1] = 0;
#if ( LOG_TO_FILE )
    log_puts_toFile(tbuf);
#endif
}


void sys_log_assert(const int assrt, const char *file, const int line)
{
    if (assrt != 0) return;
    log_printf("Assert failed : Line:%d File:%s\n", line, file);
    exit(-1);
}
