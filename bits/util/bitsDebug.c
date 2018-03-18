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


// for future ref: embedded string / printf functions: 
//      http://elm-chan.org/fsw/strf/xprintf.html
//      http://www.sparetimelabs.com/tinyprintf/tinyprintf.php

#ifdef _MSC_VER
#include <wtypes.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
// #include "debug.h"
#include "debug.h"
#include "logging.h"
#include "btaDebug.h"

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

void ipdump ( const char *title, unsigned char *ip )
{
    if ( title ) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]  );
}


void DumpIpPort ( const char *title, uint32_t ipAddr, uint16_t hoPort )
{
    uint8_t *ip = (uint8_t *)&ipAddr;

    if ( title ) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u:%u", ip[0], ip[1], ip[2], ip[3], hoPort );
}

char *NwoIPAddrToString( char *tbuf, const uint32_t nwoIpAddr)
{
    // static char tbuf[30];
    uint8_t *ip = (uint8_t *)&nwoIpAddr;
#if defined ( _MSC_VER  )
    sprintf_s(tbuf, sizeof(tbuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3] );
#else
    sprintf(tbuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3] );
#endif
    return tbuf;
}

void sys_panic( const char *file, const int line )
{
    log_printf("Panic, File:%s, line:%d\n", file, line);
    // recursing!
    // dbTraffic(DB_UNEXPECTED_ERROR, "Panic: %s %d", file, line);
    // SendBTApanicInt( file, line );
}

void sys_panic_desc(const char *file, const int line, const char *description )
{
    log_printf("Panic:%s, File:%s, line:%d\n", description, file, line);
    // recursing!
    //    dbTraffic(DB_UNEXPECTED_ERROR, "Panic: %s at %s %d\n\r", description, file, line);
    // SendBTApanicInt( file, line );
}



#if DB_TRAFFIC

void sys_dbTraffic(DB_LEVEL lev, const char *format, ...);

static DB_LEVEL	dbTrafficLevel = DB_NOTE ;

void sys_dbTrafficAssert(DB_LEVEL lev, bool assertion, const char *message )
{
    if (assertion) {
        return;
    }
    sys_dbTraffic(lev, message);
}


#if defined ( _MSC_VER  )
HANDLE hIOMutex ;
#endif

void sys_dbTraffic(DB_LEVEL lev, const char *format, ...)
{
    va_list ap;
    char tbuf[1000];

#if defined ( _MSC_VER  )
	if ( hIOMutex == NULL ) hIOMutex = CreateMutex(NULL, FALSE, NULL);
    WaitForSingleObject(hIOMutex, INFINITE);
#endif

    if (lev >= dbTrafficLevel) {
        va_start(ap, format);
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
        printf("\n\r");
#endif        
        vsprintf(tbuf, format, ap);
        
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
        printf("%s", tbuf);

        FILE *fp = fopen("BACnet.log", "a");
        if (fp) {
            fprintf(fp, "%s", tbuf );
            fprintf(fp, "\n");
            fclose(fp);
        }
#endif // _MSC_VER

        if (lev == DB_UNEXPECTED_ERROR)
        {
            SendBTApanicMessage(tbuf);
        }
        else
        {
            SendBTAmessage(tbuf);
        }
        
        va_end(ap);
    }

#if defined ( _MSC_VER  )
    ReleaseMutex(hIOMutex);
#endif
}


void sys_dbTrafficSetLevel(DB_LEVEL lev)
{
    dbTrafficLevel = lev;
}

#endif



/* More sophisticated error logging

    Throwing the message

    Type        Eg, BBMD transactions, All messages, Application, Startup, Shutdown
    Priority    Of the message

    Configuring the filter

    Type        Same
    Threshold   Only messages that meet or exceed this threshold get processed
    Destination List of destinations for this combination

    */



