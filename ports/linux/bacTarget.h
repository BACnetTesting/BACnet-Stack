/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
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

// bacTarget.h - supplies OS specific defines for BBRS
// was platform.h, target.h, but these collided with other common libraries

#pragma once

// todo 2 - move to osLayer.h

#include <semaphore.h>
#include <pthread.h>

// todo2

#define SemaDefine(a) sem_t a
#define SemaInit(a)   sem_init( &a, 0, 0);
#define SemaWait(a)   sem_wait( &a );
#define SemaFree(a)   sem_destroy( &a );

#define LockDefine(mutexName)               pthread_mutex_t mutexName
#define LockExtern(mutexName)               extern pthread_mutex_t mutexName
#define LockTransactionInit(mutexName)
#define LockTransaction(mutexName) 			pthread_mutex_lock( &mutexName )
#define UnlockTransaction(mutexName) 		pthread_mutex_unlock( &mutexName )

