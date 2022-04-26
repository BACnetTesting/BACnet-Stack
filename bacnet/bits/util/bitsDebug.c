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
*   As a special exception, if other files instantiate templates or
*   use macros or inline functions from this file, or you compile
*   this file and link it with other works to produce a work based
*   on this file, this file does not by itself cause the resulting
*   work to be covered by the GNU General Public License. However
*   the source code for this file must still be made available in
*   accordance with section (3) of the GNU General Public License.
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

#include <stdio.h>
#include <stdint.h>
#include "logging.h"
#include "btaDebug.h"
#include "bitsDebug.h"
#include "osLayer.h"

#ifdef __GNUC__
#include "syslog.h"
#endif

#if _MSC_VER
bool logToConsole = true;
#else
bool logToConsole;
#endif

#if defined BAC_DEBUG
bool unitTestFail;
#endif

char ToHexadecimal(uint8_t val)
{
    if (val > 15) return '?';
    if (val > 9) return 'A' + val - 10;
    return '0' + val;
}


void hexdump(
    char *title,
    void *mem,
    int size)
{
    SendBTAhexdump(title, mem, size);
    /*   int i;
       unsigned char *bptr = (unsigned char *)mem;

       printf("%s", title);
       printf("\n");

       for (i = 0; i < size; i++) {
           printf("%02x ", bptr[i]);
       }
       printf("\n");
   */
}

void ipdump(const char *title, unsigned char *ip)
{
    if (title) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}


void DumpIpPort(const char *title, uint32_t ipAddr, uint16_t hoPort)
{
    uint8_t *ip = (uint8_t *)&ipAddr;

    if (title) {
        printf("%s", title);
        printf(": ");
    }
    printf("%u.%u.%u.%u:%u", ip[0], ip[1], ip[2], ip[3], hoPort);
}

#if defined ( _MSC_VER ) || defined ( __GNUC__ )
char *NwoIPAddrToString(char *tbuf, const uint32_t nwoIpAddr)
{
    // static char tbuf[30];
    uint8_t *ip = (uint8_t *)&nwoIpAddr;
#if defined ( _MSC_VER  )
    sprintf_s(tbuf, sizeof(tbuf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
#else
    sprintf(tbuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
#endif
    return tbuf;
}
#endif

void sys_panic(const char *file, const int line)
{
    // log_printf("Panic, File:%s, line:%d\n", file, line);
    // recursing!
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Panic: %s %d", file, line);
    // SendBTApanicInt( file, line );
}

void sys_panic_desc(const char *file, const int line, const char *description)
{
    // log_printf("Panic:%s, File:%s, line:%d\n", description, file, line);
    // recursing!
    dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Panic: %s at %s %d\r\n", description, file, line);
    // SendBTApanicInt( file, line );
}



#if DB_TRAFFIC

void sys_dbTraffic(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...);

#ifdef TARGET_IzoT_Router_Release
static DB_LEVEL	dbTrafficLevel = DB_NOTE;
#else
// static DB_LEVEL	dbTrafficLevel = DB_DEBUG;
static DB_LEVEL	dbTrafficLevel = DB_UNUSUAL_TRAFFIC;
#endif

bits_mutex_define(hIOMutex);

#if 0
// all debug messages
static uint debugDomain = UINT_MAX;
#else
// chosen subset
// static uint debugDomain = UINT_MAX & ~(DBD_BBMD | DBD_ReadOperations | DBD_RouterOperations );
static uint debugDomain = ~0 & ~(DBD_BBMD | DBD_ReadOperations);
#endif

void sys_dbTraffic(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...)
{
    va_list ap;
    char tbuf[1000];

    if ((domain & debugDomain) == 0) return;

    bits_mutex_init(hIOMutex);
    bits_mutex_lock(hIOMutex);

    if (lev <= dbTrafficLevel) {
        va_start(ap, format);
        vsprintf(tbuf, format, ap);

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )

        if (logToConsole) {
            printf("%s\n", tbuf);
        }
#endif      

#if defined ( __GNUC__  )

        int priority = LOG_CRIT;
        /*
        LOG_EMERG      system is unusable
        LOG_ALERT      action must be taken immediately
        LOG_CRIT       critical conditions
        LOG_ERR        error conditions
        LOG_WARNING    warning conditions
        LOG_NOTICE     normal, but significant, condition
        LOG_INFO       informational message
        LOG_DEBUG      debug - level message
        */
        switch (lev)
        {
        case DB_ALWAYS:
        case DB_FATAL_ERROR:
            // aleady set appropriately above
            break;

        case DB_UNEXPECTED_ERROR:
        case DB_ERROR:
        case DB_BTC_ERROR:                // we do expect these errors during BTC regression testing.
            priority = LOG_ERR;
            break;

        case DB_NOTE:                     // default
        // case DB_NOTE_SUPPRESSED:
        case DB_UNUSUAL_TRAFFIC:
            priority = LOG_NOTICE;
            break;

        case DB_EXPECTED_ERROR_TRAFFIC:
        case DB_NORMAL_TRAFFIC:
        case DB_INFO:
            priority = LOG_INFO;
            break;

        case DB_DEBUG:
        case DB_NONE:
            priority = LOG_DEBUG;
            break;
        }

        syslog(priority, "%s", tbuf);

#endif // __GNUC__

#if defined ( _MSC_VER  )
        FILE *fp = fopen("BACnet.log", "a");
        if (fp) {
            fprintf(fp, "%s\n", tbuf);
            fclose(fp);
        }
#endif // _MSC_VER

#if defined BAC_DEBUG
        if (lev <= DB_ERROR) {
            unitTestFail = true;
        }
#endif

        if (lev <= DB_UNEXPECTED_ERROR) {
            SendBTApanicMessage(tbuf);
        }
        else {
            SendBTAmessage(tbuf);
        }

        va_end(ap);
    }

    bits_mutex_unlock(hIOMutex);
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



