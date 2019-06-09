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
#include <stdint.h>

#include <WinSock2.h>       // For HANDLE (used by mutexs)
#include <process.h>

#define OS_LAYER_WIN

typedef unsigned uint;

void *bitsCreateThread(void (*threadFunc)(void *arg), void *argListPtr);
    
// Be aware: mutexs in windows have thread affinity (they can be locked recursively in same thread), linux mutexs do not
// I should have used CreateSemaphore() Stack Overflow: http://goo.gl/h96Wh9  to avoid this difference...
// MSDN: http://goo.gl/hSD2Yh

// Also, use critical sections (lighter, limited to threads in-process) 
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682530(v=vs.85).aspx

// Microdort critical sectoins are recursive
// https://docs.microsoft.com/en-us/windows/desktop/Sync/critical-section-objects

typedef CRITICAL_SECTION bits_mutex_t ;

#define bits_mutex_define(criticalSection)        bits_mutex_t criticalSection
#define bits_mutex_extern(criticalSection)        extern bits_mutex_t criticalSection
#define bits_mutex_init(criticalSection)          if ( criticalSection.SpinCount == 0 ) { InitializeCriticalSectionAndSpinCount( &criticalSection, 0x400) ; }    // save to re-init
#define bits_mutex_lock(criticalSection)          EnterCriticalSection ( &criticalSection ) // ; if ( ( criticalSection.LockCount & 0x01 ) == 0 ) panic ()             // lowest bit == 0 -> locked
// #define bits_mutex_trylock(criticalSection,ms)    sys_bits_mutex_lock( criticalSection, ms )
#define bits_mutex_unlock(criticalSection)        LeaveCriticalSection ( &criticalSection )

// int sys_bits_mutex_lock(bits_mutex_t mutexName, int ms );

// typedef uint8_t TimerHandle ;

// A very basic timer of time elapsed since previous time

// in timerCommon.h
//uint32_t bits_sysTimer_get_time(
//                                void ) ;
//
//uint32_t bits_sysTimer_elapsed_milliseconds(
//                                            uint32_t prevTime ) ;
//
//
//// "Count up" timers that allow multiple instances
//TimerHandle bits_multiTimer_init(
//                                 void);
//
//uint32_t bits_multiTimer_elapsed_milliseconds(
//                                              TimerHandle handle );
//
//void bits_multiTimer_reset(
//                           TimerHandle handle );

int osGetch(void);
int osKBhit(void);
