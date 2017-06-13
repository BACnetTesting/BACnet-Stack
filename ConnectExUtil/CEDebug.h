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

#pragma once

#include <stdint.h>

// #include "net.h"

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
void ipdump(char *title, unsigned char *mem);
void DumpIpPort(char *title, uint32_t ipAddr, uint16_t hoPort);
char *NwoIPAddrToString(const uint32_t nwoIpAddr );

#define MyPortsTraffic  41797
#define MyPortsTerminal 48796
#define MyPortsPanic    502

// #define dbLog(a,...)    sys_dbLog(a, _VA_ARGS__)
#define panicDesc(desc) sys_panic_desc ( __FILE__, __LINE__, desc )
#define panic()			sys_panic( __FILE__, __LINE__ )

#ifdef _MSC_VER
#pragma warning(suppress : 4127)
#endif

void sys_panic_desc(const char *file, const int line, const char *description );
void sys_panic(const char *file, const int line);
// void sys_dbLog( char *a, ...);

#ifdef _MSC_VER
// Microsoft
#define LockTransactionCreate(mutexName) mutexName = CreateMutex(NULL, FALSE, NULL)
#define LockTransaction(mutexName) if ( WaitForSingleObject( mutexName, INFINITE ) != 0 ) panic () 
#define UnlockTransaction(mutexName) ReleaseMutex ( mutexName)
#elif defined ( __IAR_SYSTEMS_ICC__ )
// IAR
#define LockTransactionCreate(mutexName) OS_CREATERSEMA( &mutexName )
#define LockTransaction(mutexName) OS_Use ( &mutexName )
#define UnlockTransaction(mutexName) OS_Unuse( &mutexName )
#else
// Linux
#define LockTransaction(mutexName) pthread_mutex_lock( &mutexName )
#define UnlockTransaction(mutexName) pthread_mutex_unlock( &mutexName )
#endif

