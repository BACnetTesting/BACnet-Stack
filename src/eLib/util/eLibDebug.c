/****************************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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

#ifdef _MSC_VER
//#include <wtypes.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
// #include "debug.h"
 // #include "eLib/util/logging.h"
#include "btaDebug.h"
#include "eLib/util/eLibDebug.h"
#include "osLayer.h"

#define DB_TRAFFIC  1

bits_mutex_extern(bacnetStackMutex);

char ToHexadecimal(uint8_t val)
{
    if (val > 15) return '?';
    if (val > 9) return 'A' + val - 10;
    return '0' + val;
}


void hexdump(
    const char *title,
    void *mem,
    int size)
{
    SendBTAhexdump(title, mem, size);

    int i;
    unsigned char *bptr = (unsigned char *)mem;

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
    printf("%u.%u.%u.%u:%u", ip[0], ip[1], ip[2], ip[3], hoPort);
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
    // log_printf("Panic, File:%s, line:%d\n", file, line);
    // recursing!
    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Panic: %s %d", file, line);
    // SendBTApanicInt( file, line );
}

void sys_panic_desc(const char *file, const int line, const char *description )
{
    // log_printf("Panic:%s, File:%s, line:%d\n", description, file, line);
    // recursing!
    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Panic: %s at %s %d\r\n", description, file, line);
    // SendBTApanicInt( file, line );
}



#if DB_TRAFFIC

static DB_LEVEL	dbMessageLevel = DB_UNUSUAL_TRAFFIC;
bits_mutex_define(hIOMutex);

// chosen subset
#if 1
    // All
static uint enabledLoggingDomain = ~0;
#else
    // exclude list
static uint enabledLoggingDomain = ~0 & ~(DBD_BBMD | DBD_ReadOperations)  ;
#endif


void sys_dbMessage_Enable(DBD_DebugDomain domainToLog)
{
    enabledLoggingDomain |= domainToLog;
}


void sys_dbMessage_Hourly(DBD_DebugDomain domain, DB_LEVEL lev, const char* format, ...)
{
    // todo 4, figure out a way to send this message hourly,
    // for now, there is only one message, but I will need to extend this as soon as we hit the panic for the first time...
    static char* prior;
    static time_t lastTime;
    bool sendIt = false;

    if (prior)
    {
        if (strcmp(format, prior) == 0)
        {
            // same message again is one hour up yet?
            time_t thisTime;
            time(&thisTime);
            if (thisTime > lastTime + 60 * 3600)
            {
                sendIt = true;
            }
        }
        else
        {
            panic();
            // someone has introduced a second message, update this..
            sendIt = true;
            prior = (char*)format;
        }
    }
    else
    {
        prior = (char *) format;
        sendIt = true;
    }

    if (sendIt)
    {
        time(&lastTime);
        va_list ap;
        va_start(ap, format);
        sys_dbMessage(domain, lev, format, ap);

        char tbuf[1000];
        vsprintf(tbuf, format, ap);

		// Need some platform specific way of sending..
		
        va_end(ap);
    }
}


void sys_dbMessage(DBD_DebugDomain domain, DB_LEVEL lev, const char* format, ...)
{
#ifdef OS_LAYER_WIN
    bits_mutex_init(bacnetStackMutex);
#endif
    bits_mutex_lock(bacnetStackMutex);
    DB_LEVEL tlev = dbMessageLevel;
    bits_mutex_unlock(bacnetStackMutex);

    va_list ap;
    char tbuf[200];

    // if ((domain & debugDomain) == 0) return;
    bool shallIlog = false;

    if (lev <= tlev )
    {
        // global setting.. (using 'e' flag)
        shallIlog = true;
    }

    // regardless of the above, if we don't care about the domain, don't log it
    if(!(domain & enabledLoggingDomain))
    {
        shallIlog = false;
    }

    if (!shallIlog) {
        return;
    }

    bits_mutex_init(hIOMutex);
    bits_mutex_lock(hIOMutex);

    va_start(ap, format);
    vsnprintf(tbuf, sizeof(tbuf), format, ap);
    tbuf[sizeof(tbuf) - 1] = 0;

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
    if ( domain & DBD_UI ) {
        printf("%s\n", tbuf);
        bits_mutex_unlock(hIOMutex);
        return;
    }
#endif      

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
    case DB_UNUSUAL_TRAFFIC :
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

#if defined ( __GNUC__  )
    // shut down syslog after 200 reports so we don't swamp disk
#define MX_SYSLOG   200

    static uint syslogCount = 0;
    if (syslogCount < MX_SYSLOG) 
    {
        syslogCount++;
        syslog(priority, "%s", tbuf);
        if (syslogCount == MX_SYSLOG)
            syslog(priority, "Logging stopped at %d", MX_SYSLOG);
    }
#endif // __GNUC__


#if defined ( _MSC_VER  )
    FILE* fp = fopen("BACnet.log", "a");
    if (fp) {
        fprintf(fp, "%s\n", tbuf);
        fclose(fp);
    }
#endif // _MSC_VER

#if defined BAC_DEBUG
    if (lev <= DB_ERROR) {
        // unitTestFail = true;
    }
#endif

    if (lev <= DB_UNEXPECTED_ERROR) {
        // todo 1 SendBTApanicMessage(tbuf);
    }
    else {
        // todo 1 SendBTAmessage(tbuf);
    }

    va_end(ap);

    bits_mutex_unlock(hIOMutex);
    }


void sys_dbMessageSetLevel(DB_LEVEL lev)
{
    dbMessageLevel = lev;
}

void sys_dbMessage_Level_Incr(void)
{
    dbMessageLevel = (DB_LEVEL) ((int) dbMessageLevel+1) ;
    if ((int)dbMessageLevel > 9) dbMessageLevel = DB_ALWAYS;
}

void sys_dbMessage_Level_Decr(void)
{
    if (!(int)dbMessageLevel) return;
    dbMessageLevel = (DB_LEVEL) ((int) dbMessageLevel - 1);
}

uint sys_dbMessage_Level_Get(void)
{
    return (uint)dbMessageLevel;
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



