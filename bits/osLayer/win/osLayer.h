/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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
#include <stdint.h>

#include <WinSock2.h>       // For HANDLE (used by mutexs)
#include <process.h>

#define OS_LAYER_WIN

typedef unsigned uint;

// typedef int bitsThreadVar;
void *bitsCreateThread(void (*threadFunc)(void *arg), void *argListPtr);
    
    // #define bitsCreateThread(threadVar,
                                         // threadFunc, argListPtr)
                                         // _beginthread( threadFunc, 0, (void
                                         // *) argListPtr)
// #define bitsDetachThread(threadVar)
// #define bitsThreadFunction(threadFuncName,argList)  void (threadFuncName) ( void *argList )

// Be aware: mutexs in windows have thread affinity (they can be locked recursively in same thread), linux mutexs do not
// I should have used CreateSemaphore() Stack Overflow: http://goo.gl/h96Wh9  to avoid this difference...
// MSDN: http://goo.gl/hSD2Yh

// Also, use critical sections (lighter, limited to threads in-process) 
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682530(v=vs.85).aspx

typedef HANDLE bits_mutex_t ;

// todo 0 - consolidate
#define SemaDefine(a)   HANDLE a 
#define SemaInit(a)     a = CreateMutex(NULL, FALSE, NULL)
#define SemaWait(a)     WaitForSingleObject(a, INFINITE)
#define SemaFree(a)     ReleaseMutex(a);

// Note, the convoluted * defeferencing is to allow the calls to closely match the linux mutex locks... keep it that way
#define bits_mutex_init(mutexName)        *mutexName = CreateMutex(NULL, FALSE, NULL)
#define bits_mutex_lock(mutexName)        sys_bits_mutex_lock( mutexName, INFINITE )
#define bits_mutex_trylock(mutexName,ms)  sys_bits_mutex_lock( mutexName, ms )
#define bits_mutex_unlock(mutexName)      ReleaseMutex ( *(mutexName) )

//void sys_bits_mutex_init(bits_mutex_t *mutexName);
int sys_bits_mutex_lock(bits_mutex_t *mutexName, int ms );
//void sys_bits_mutex_trylock(bits_mutex_t *mutexName);
//void sys_bits_mutex_unlock(bits_mutex_t *mutexName);

// void *bitsCreateThread( bitsThreadVar threadId, void *(*threadFunc) (void *arg ), void *argListPtr);

typedef uint8_t TimerHandle ;

// A very basic timer of time elapsed since previous time

uint32_t bits_sysTimer_get_time(
                                void ) ;

uint32_t bits_sysTimer_elapsed_milliseconds(
                                            uint32_t prevTime ) ;


// "Count up" timers that allow multiple instances
TimerHandle bits_multiTimer_init(
                                 void);

uint32_t bits_multiTimer_elapsed_milliseconds(
                                              TimerHandle handle );

void bits_multiTimer_reset(
                           TimerHandle handle );

int osKBhit(void);
int osGetch(void);