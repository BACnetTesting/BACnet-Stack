/****************************************************************************************
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
#include <pthread.h>
#include <stdbool.h>

#include "configProj.h"

#define osLayerLinux        #error      // deprecated permanently as of now 2020-08-08 

#undef _MSC_VER

#define __STDC_LIMIT_MACROS
#define OS_LAYER_LINUX

typedef unsigned uint;

typedef int SOCKET ;

// 2020-08-02 watch out here, I just defined these arbitrarily, may have to reverse sequence
#define LOG_CRIT    1
#define LOG_DEBUG   2
#define LOG_ERR     3
#define LOG_INFO    4
#define LOG_NOTICE  5

typedef int bitsThreadVar;
void bitsCreateThread(void(*threadFunc)(void *arg), void *argListPtr);

#define bitsDetachThread(threadVar)
#define bitsThreadFunction(threadFuncName,argList)  void (threadFuncName) ( void *argList )

// Be aware: mutexs in windows have thread affinity (they can be locked recursively in same thread), linux mutexs do not
// I should have used CreateSemaphore() Stack Overflow: http://goo.gl/h96Wh9  to avoid this difference...
// MSDN: http://goo.gl/hSD2Yh

// 2019.04.01 adding recursive initializer (PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
// 2020-08-02 they do not exist on Pi, removing, but we need to resolve - todo 1

#define bits_mutex_define(mutexName)        pthread_mutex_t mutexName = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP ;
#define bits_mutex_static(mutexName)        static pthread_mutex_t mutexName = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP ;
// #define bits_mutex_define(mutexName)        pthread_mutex_t mutexName = PTHREAD_MUTEX_INITIALIZER ;
// #define bits_mutex_static(mutexName)        static pthread_mutex_t mutexName = PTHREAD_MUTEX_INITIALIZER ;

#define bits_mutex_extern(mutexName)        extern pthread_mutex_t mutexName
#define bits_mutex_init(mutexName)
#define bits_mutex_lock(mutexName)          pthread_mutex_lock( &mutexName )
#define bits_mutex_unlock(mutexName)        pthread_mutex_unlock( &mutexName )

#define closesocket(a) close(a)

bool read_config(const char *filepath) ;
void read_config_auxiliary(const char *filepath);
// moved to bitsUtil.h bool parse_command_line(int argc, char *argv[]) ;
bool bits_FileExists(const char *name);

int osGetch(void);
int osKBhit(void);
 