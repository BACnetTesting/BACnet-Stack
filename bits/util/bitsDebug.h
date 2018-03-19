/**************************************************************************

Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.

This program is free software : you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.

For more information : info@bac-test.com
For access to source code : info@bac-test.com
or www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#pragma once

#include <stdint.h>

#ifdef _MSC_VER

#define LOG_INFO    1
#define LOG_ERR     2
#define LOG_DAEMON  3
#define LOG_PID     4

#define openlog(a,b,c)
#define syslog(a,...) printf ( "\n" ); printf ( __VA_ARGS__ ) 
#define closelog()

/*
#define DEBUG_LEAK_DETECT
*/
#define MEM_OVERFLOW_CHECK

#endif

#if defined ( _DEBUG ) || defined ( DEBUG_ENABLED )
#define BAC_DEBUG   1
#else
// obviosly want to do something about this! todo3
#define BAC_DEBUG	1
#endif

#define DEBUG_LEVEL 3
#ifdef DEBUG_LEVEL
#ifdef _MSC_VER
#define PRINT(debug_level, a, ...) printf(a, __VA_ARGS__ )
#else
#define PRINT(debug_level, ...) printf(__VA_ARGS__)
// #define PRINT(debug_level, ...) if(debug_level <= DEBUG_LEVEL) fprintf(stderr, __VA_ARGS__)
// #define PRINT(debug_level, ...) syslog(LOG_INFO, __VA_ARGS__)
#endif
#else
#define PRINT(...)
#endif


void hexdump( char *title, void *mem, int size) ;
void ipdump(const char *title, unsigned char *mem);
void DumpIpPort(const char *title, uint32_t ipAddr, uint16_t hoPort);
char *NwoIPAddrToString(char *temp, const uint32_t nwoIpAddr );

// #define dbLog(a,...)    sys_dbLog(a, _VA_ARGS__)
#define panicDesc(desc) sys_panic_desc ( __FILE__, __LINE__, desc )
#define panic()			sys_panic( __FILE__, __LINE__ )

#ifdef _MSC_VER
#pragma warning(suppress : 4127)
#endif

#define panictodo(a)	sys_panic( __FILE__, __LINE__ )

void sys_panic_desc(const char *file, const int line, const char *description );
void sys_panic(const char *file, const int line);
// void sys_dbLog( char *a, ...);

#ifdef _MSC_VER
// Microsoft
#define LockDefine(mutexName)               HANDLE mutexName 
#define LockExtern(mutexName)               extern HANDLE mutexName
#define LockTransactionInit(mutexName)      mutexName = CreateMutex(NULL, FALSE, NULL)
#define LockTransaction(mutexName)          if ( WaitForSingleObject( mutexName, INFINITE ) != 0 ) panic () 
#define UnlockTransaction(mutexName)        ReleaseMutex ( mutexName)
#elif defined ( __IAR_SYSTEMS_ICC__ )
// IAR
#define LockTransactionCreate(mutexName) OS_CREATERSEMA( &mutexName )
#define LockTransaction(mutexName) OS_Use ( &mutexName )
#define UnlockTransaction(mutexName) OS_Unuse( &mutexName )
#else
// Linux
// Moved to bacTarget.h
//#define LockDefine(mutexName)               pthread_mutex_t mutexName
//#define LockExtern(mutexName)               extern pthread_mutex_t mutexName
//#define LockTransactionInit(mutexName)
//#define LockTransaction(mutexName) 			pthread_mutex_lock( &mutexName )
//#define UnlockTransaction(mutexName) 		pthread_mutex_unlock( &mutexName )
#endif

