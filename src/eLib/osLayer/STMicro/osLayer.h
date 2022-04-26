/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
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

#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>

typedef unsigned uint;

typedef int bitsThreadVar;
void bitsCreateThread(void(*threadFunc)(void *arg), void *argListPtr);

#define bitsDetachThread(threadVar)
#define bitsThreadFunction(threadFuncName,argList)  void (threadFuncName) ( void *argList )

// Be aware: mutexs in windows have thread affinity (they can be locked recursively in same thread), linux mutexs do not
// I should have used CreateSemaphore() Stack Overflow: http://goo.gl/h96Wh9  to avoid this difference...
// MSDN: http://goo.gl/hSD2Yh


#define closesocket(a) close(a)

#define LockDefine(mutexName)               pthread_mutex_t mutexName = PTHREAD_MUTEX_INITIALIZER
#define LockExtern(mutexName)               extern pthread_mutex_t mutexName
#define LockTransactionInit(mutexName)
#define LockTransaction(mutexName) 			pthread_mutex_lock( &mutexName )
#define UnlockTransaction(mutexName) 		pthread_mutex_unlock( &mutexName )

//void *bitsCreateThread(
//    bitsThreadVar threadId, 
//    void *(*threadFunc)(void *arg),
//	void *argListPtr);

bool read_config(char *filepath) ;
bool parse_cmd(int argc, char *argv[]) ;
int osGetch(void);
int osKBhit(void);

typedef uint8_t TimerHandle ;

// A very basic timer of time elapsed since previous time

//// now in timerCommon.h
//uint32_t bits_sysTimer_get_time(
//                                void ) ;
//
//// now in timerCommon.h
//uint32_t bits_sysTimer_elapsed_milliseconds(
//                                            uint32_t prevTime ) ;


// "Count up" timers that allow multiple instances
TimerHandle bits_multiTimer_init(
                                 void);

uint32_t bits_multiTimer_elapsed_milliseconds(
                                              TimerHandle handle );

void bits_multiTimer_reset(
                           TimerHandle handle );

