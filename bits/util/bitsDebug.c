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

// for future ref: embedded string / printf functions:
//      http://elm-chan.org/fsw/strf/xprintf.html
//      http://www.sparetimelabs.com/tinyprintf/tinyprintf.php

//#ifdef _MSC_VER
//#include <wtypes.h>
//#endif

#include <stdio.h>
#include <stdint.h>
//#include <stdarg.h>
//// #include "debug.h"
//#include "debug.h"
#include "logging/logging.h"
#include "btaDebug.h"
#include "bitsDebug.h"

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

#if defined ( _MSC_VER ) || defined ( __GNUC__ )
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
#endif

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

void sys_dbTraffic(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...);

static DB_LEVEL	dbTrafficLevel = DB_NOTE ;

//void sys_dbTrafficAssert(DB_LEVEL lev, bool assertion, const char *message )
//{
//    if (assertion) {
//        return;
//    }
//    sys_dbTraffic(lev, message);
//}


#if defined ( _MSC_VER  )
HANDLE hIOMutex ;
#endif

void sys_dbTraffic(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...)
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



