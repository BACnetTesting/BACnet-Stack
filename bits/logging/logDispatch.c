/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
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

void log_lev_printf(const BL_LEV lev, const char *format, ...)
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

void sys_log_lev_trace(const BL_LEV level, const char *file, const int line)
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


static char _keyinput;

void bits_KeyInput(char key)
{
    _keyinput = key;
}


bool bits_kbhit(void)
{
    return (_keyinput != 0 ) ? true : false;
}

int bits_getch(void)
{
    int keyinput = _keyinput;
    _keyinput = 0;
    return keyinput;
}

int bits_getch_toupper(void)
{
    int keyinput = _keyinput ;
    _keyinput = 0;
    if (keyinput >= 'a') return keyinput - 'a' + 'A';
    return keyinput;
}