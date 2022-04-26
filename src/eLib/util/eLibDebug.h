/****************************************************************************************
*
*   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
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

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "osLayer.h"

#if defined ( OS_LAYER_WIN  ) || defined ( OS_LAYER_LINUX )


// int indicating verbosity
typedef enum 
{
    DB_ALWAYS,
    DB_FATAL_ERROR,
    DB_UNEXPECTED_ERROR,
    DB_ERROR,
    DB_BTC_ERROR,               // We do expect these errors during BTC regression testing.
    DB_NOTE,                    // <--- Default level. Startup messages etc.
    DB_UNUSUAL_TRAFFIC,
    DB_EXPECTED_ERROR_TRAFFIC,
    DB_NORMAL_TRAFFIC,
    DB_INFO,
    DB_DEBUG,
    DB_NONE			
} DB_LEVEL;


// 32-bit mask indicating domains of interest
typedef enum 
{
    DBD_WriteOperations = 1,
    DBD_TimeOperations  = (1 << 1),
    DBD_FileOperations  = (1 << 2),
    DBD_COVoperations   = (1 << 3),
    DBD_ESEerr          = (1 << 4),
    DBD_ALL             = (1 << 5),
    DBD_BBMD            = (1 << 6),
    DBD_FD              = (1 << 7),
    DBD_ReadOperations  = (1 << 8),
    DBD_Intrinsic       = (1 << 9),
    DBD_Config              = (1 << 10),
    DBD_OOB_TRAFFIC         = (1 << 11),             // Out of band traffic
    DBD_Queues_and_Buffers 	= (1 << 12),
    DBD_BVLL                = (1 << 13),
    DBD_Application         = (1 << 14),
    DBD_RouterOperations    = (1 << 15),
    DBD_UI                  = (1 << 16)
} DBD_DebugDomain ;


/*
#define DEBUG_LEAK_DETECT
*/
#define MEM_OVERFLOW_CHECK

#endif

// you should define BAC_DEBUG in your makefile, or IDE environment, to allow switching between configurations
// at compile time without modifying code
#if ! defined ( BAC_DEBUG )
#if defined ( _DEBUG ) || defined ( DEBUG_ENABLED ) || defined ( DEBUG )
#define BAC_DEBUG   1
#else
#define BAC_DEBUG   0
#endif
#endif

#define DEBUG_LEVEL 3
#ifdef DEBUG_LEVEL
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
#define PRINT(debug_level, a, ...) printf(a, __VA_ARGS__ )
#else
#define PRINT(debug_level, ...) printf(__VA_ARGS__)
// #define PRINT(debug_level, ...) if(debug_level <= DEBUG_LEVEL) fprintf(stderr, __VA_ARGS__)
// #define PRINT(debug_level, ...) syslog(LOG_INFO, __VA_ARGS__)
#endif
#else
#define PRINT(...)
#endif

#define DB_TRAFFIC  1
#if DB_TRAFFIC

#define	dbMessageSetLevel(db_level)					        sys_dbMessageSetLevel((DB_LEVEL)db_level)
#define	dbMessage(dbd_debugDomain, db_level, ...)           sys_dbMessage(dbd_debugDomain, db_level, __VA_ARGS__)
#define	dbMessage_Hourly(dbd_debugDomain, db_level, ...)    sys_dbMessage_Hourly(dbd_debugDomain, db_level, __VA_ARGS__)
#define	dbMessage_Enable(dbd_debugDomain)	                sys_dbMessage_Enable(dbd_debugDomain)
#define	dbMessageAssert(lev, assertion, msg)	            sys_dbMessageAssert(lev, assertion, msg)
#define	dbMessage_Level_Incr                                sys_dbMessage_Level_Incr
#define	dbMessage_Level_Decr                                sys_dbMessage_Level_Decr
#define	dbMessage_Level_Get                                 sys_dbMessage_Level_Get

void sys_dbMessage(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...);
void sys_dbMessage_Hourly(DBD_DebugDomain domain, DB_LEVEL lev, const char* format, ...);
void sys_dbMessage_Enable(DBD_DebugDomain domain);
void sys_dbMessage_Level_Incr(void);
void sys_dbMessage_Level_Decr(void);
uint sys_dbMessage_Level_Get(void);

void sys_dbMessageAssert(
    DB_LEVEL lev,
    bool assertion,
    const char *message);

// Sets level filter to only display debug messages of priority of lev or above
void sys_dbMessageSetLevel(DB_LEVEL lev);

#else

#define	dbMessageSetLevel(lev)
#define	dbMessage(lev, ...)
#define	dbMessage_Enable(dbd_debugDomain)
#define	dbMessageAssert(lev, assertion, msg)
#define	dbMessage_Hourly(dbd_debugDomain, db_level, ...)
#define	dbMessage_Level_Incr()
#define	dbMessage_Level_Decr()
#define	dbMessage_Level_Get()

#endif


char ToHexadecimal(uint8_t val);
void hexdump(const char *title, void *mem, int size) ;
void ipdump(const char *title, unsigned char *mem);
void DumpIpPort(const char *title, uint32_t ipAddr, uint16_t hoPort);
char *NwoIPAddrToString(char *temp, const uint32_t nwoIpAddr );

#define panicDesc(desc) sys_panic_desc ( __FILE__, __LINE__, desc )		// deprecated
#define panicstr(desc)  sys_panic_desc ( __FILE__, __LINE__, desc )		// preferred (one day there will be panicInt, etc.)
#define panic()			sys_panic( __FILE__, __LINE__ )

#ifdef _MSC_VER
#pragma warning(suppress : 4127)
#endif

#define panictodo(a)	sys_panic( __FILE__, __LINE__ )

void sys_panic_desc(const char *file, const int line, const char *description );
void sys_panic(const char *file, const int line);
// void sys_dbLog( char *a, ...);

