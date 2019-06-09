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

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )

// obsoleting 2019.03.10 
//#define LOG_INFO    1
//#define LOG_ERR     2
//#define LOG_DAEMON  3
//#define LOG_PID     4

// obsoleting 2019.03.10 
// obsoleting #define openlog(a,b,c)
// obsoleting #define syslog(a,...) printf ( __VA_ARGS__ ) ; printf ( "\n" ) 
// obsoleting #define closelog()

// int indicating verbosity - modelled (roughly) on linux levels
typedef enum 
{
    DB_ALWAYS,
    DB_FATAL_ERROR,
    DB_UNEXPECTED_ERROR,
    DB_ERROR,
    DB_BTC_ERROR,               // we do expect these errors during BTC regression testing.
    DB_NOTE,                    // default. Startup messages etc.
    // DB_NOTE_SUPPRESSED,
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
    DBD_Config          	= (1 << 10),
    DBD_OOB_TRAFFIC     	= (1 << 11),            // Out of band traffic
    DBD_Queues_and_Buffers 	= (1 << 12),
    DBD_BVLL                = (1 << 13),
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
#define BAC_DEBUG	0
#endif
#endif

#if ( BAC_DEBUG == 1 )
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
#define PRINT(debug_level, a, ...) printf(a, __VA_ARGS__ )
#else
#define PRINT(debug_level, ...) printf(__VA_ARGS__)
// #define PRINT(debug_level, ...) if(debug_level <= DEBUG_LEVEL) fprintf(stderr, __VA_ARGS__)
#endif
#else
#define PRINT(...)
#endif

#define DB_TRAFFIC  1

#if DB_TRAFFIC
    // this should move to bitsDebug.h

#define	dbTrafficSetLevel(db_level)					sys_dbTrafficSetLevel((DB_LEVEL)db_level)
#define	dbTraffic(dbd_debugDomain, db_level, ...)	sys_dbTraffic(dbd_debugDomain, db_level, __VA_ARGS__)

#define	dbTrafficAssert(lev, assertion, msg)	sys_dbTrafficAssert(lev, assertion, msg)
#define	dbUser(a,...)      						printf(a,__VA_ARGS__)

void sys_dbTraffic(DBD_DebugDomain domain, DB_LEVEL lev, const char *format, ...);

void sys_dbTrafficAssert(
    DB_LEVEL lev,
    bool assertion,
    const char *message);

// Sets level filter to only display debug messages of priority of lev or above
void sys_dbTrafficSetLevel(DB_LEVEL lev);

#else

#define	dbTrafficSetLevel(lev)
#define	dbTraffic(lev, ...)
#define	dbTrafficAssert(lev, assertion, msg)

#endif


char ToHexadecimal(uint8_t val);
void hexdump( char *title, void *mem, int size) ;
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

// has been moved to oslayer.h
//// this is not a debug thing, it belongs in osLayer.h
//#ifdef _MSC_VER
//// Microsoft
//#define LockDefine(mutexName)               HANDLE mutexName 
//#define LockExtern(mutexName)               extern HANDLE mutexName
//#define LockTransactionInit(mutexName)      mutexName = CreateMutex(NULL, FALSE, NULL)
//#define LockTransaction(mutexName)          if ( WaitForSingleObject( mutexName, INFINITE ) != 0 ) panic () 
//#define UnlockTransaction(mutexName)        ReleaseMutex ( mutexName)
//#elif defined ( __IAR_SYSTEMS_ICC__ )
//// IAR
//#define LockTransactionCreate(mutexName) OS_CREATERSEMA( &mutexName )
//#define LockTransaction(mutexName) OS_Use ( &mutexName )
//#define UnlockTransaction(mutexName) OS_Unuse( &mutexName )
//#else
//// Linux
//// Moved to bacTarget.h
////#define LockDefine(mutexName)               pthread_mutex_t mutexName
////#define LockExtern(mutexName)               extern pthread_mutex_t mutexName
////#define LockTransactionInit(mutexName)
////#define LockTransaction(mutexName) 			pthread_mutex_lock( &mutexName )
////#define UnlockTransaction(mutexName) 		pthread_mutex_unlock( &mutexName )
//#endif

