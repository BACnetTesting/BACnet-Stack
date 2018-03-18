/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
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

#include <stdio.h>

#include <pthread.h>

typedef unsigned uint;

typedef int bitsThreadVar;
// #define bitsCreateThread(threadVar, threadFunc, argListPtr)   _beginthread( threadFunc, 0, (void *) argListPtr)
#define bitsDetachThread(threadVar)
#define bitsThreadFunction(threadFuncName,argList)  void (threadFuncName) ( void *argList )

// Be aware: mutexs in windows have thread affinity (they can be locked recursively in same thread), linux mutexs do not
// I should have used CreateSemaphore() Stack Overflow: http://goo.gl/h96Wh9  to avoid this difference...
// MSDN: http://goo.gl/hSD2Yh

// Also, use critical sections (lighter, limited to threads in-process) 
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682530(v=vs.85).aspx

typedef HANDLE bits_mutex_t ;

// Note, the convoluted * defeferencing is to allow the calls to closely match the linux mutex locks... keep it that way
#define bits_mutex_init(mutexName)        *mutexName = CreateMutex(NULL, FALSE, NULL)
#define bits_mutex_lock(mutexName)        sys_bits_mutex_lock( mutexName, INFINITE )
#define bits_mutex_trylock(mutexName,ms)  sys_bits_mutex_lock( mutexName, ms )
#define bits_mutex_unlock(mutexName)      ReleaseMutex ( *(mutexName) )

//void sys_bits_mutex_init(bits_mutex_t *mutexName);
int sys_bits_mutex_lock(bits_mutex_t *mutexName, int ms );
//void sys_bits_mutex_trylock(bits_mutex_t *mutexName);
//void sys_bits_mutex_unlock(bits_mutex_t *mutexName);

void *bitsCreateThread( bitsThreadVar threadId, void *(*threadFunc) (void *arg ), void *argListPtr);
