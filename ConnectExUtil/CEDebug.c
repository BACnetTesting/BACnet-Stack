/**************************************************************************
*
* Copyright (C) 2014-2016 ConnectEx, Inc. <info@connect-ex.com>
*
* Permission is hereby granted, to whom a copy of this software and 
* associated documentation files (the "Software") is provided by ConnectEx, Inc.
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
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


// for future ref: embedded string / printf functions: 
//      http://elm-chan.org/fsw/strf/xprintf.html
//      http://www.sparetimelabs.com/tinyprintf/tinyprintf.php


#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
// #include "CEDebug.h"
#include "debug.h"

void hexdump(
    char *title,
    void *mem,
    int size)
{
    int i;
    unsigned char *bptr = (unsigned char *) mem;

    printf("%s", title);
    printf("\n");

    for (i = 0; i < size; i++) {
        printf("%02x ", bptr[i]);
    }
    printf("\n");
}

void ipdump ( char *title, unsigned char *ip )
{
    if ( title ) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]  );
}


void DumpIpPort ( char *title, uint32_t ipAddr, uint16_t hoPort )
{
    uint8_t *ip = (uint8_t *)&ipAddr;

    if ( title ) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u:%u", ip[0], ip[1], ip[2], ip[3], hoPort );
}

char *NwoIPAddrToString(const uint32_t nwoIpAddr)
{
    static char tbuf[30];
    uint8_t *ip = (uint8_t *)&nwoIpAddr;
    sprintf_s(tbuf, sizeof(tbuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3] );
    return tbuf;
}

void sys_panic( const char *file, const int line )
{
    dbTraffic(DB_UNEXPECTED_ERROR, "Panic: %s %d", file, line);
}

void sys_panic_desc(const char *file, const int line, const char *description )
{
    dbTraffic(DB_UNEXPECTED_ERROR, "Panic: %s at %s %d\n\r", description, file, line);
}

