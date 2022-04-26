/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
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

#include "osLayer.h"

#include <stdbool.h>
#include <windows.h>
#include <conio.h>
//#pragma comment(lib, "Winmm.lib")           // for timeGetTime()
//#include <timeapi.h>
//
//#include <process.h>

//#include "timerCommon.h"
//#include "bacdef.h"
// #include "datalink.h"
//#include "bacnet/basic/services.h"
// #include "dlenv.h"
//#include "bitsUtil.h"
// #include "eLib/util/eLibDebug.h"
// #include "bip.h"

// #pragma comment(lib, "Ws2_32.lib")


void bitsCreateThread(void (*threadFunc) (void *arg), void *argListPtr)
{
    // second parameter is stack size
    _beginthread((_beginthread_proc_type)threadFunc, 0, argListPtr );
}



static time_t oldMultiTime ;

//TimerHandle bits_multiTimer_init(
//                            void)
//{
//    oldMultiTime = bits_sysTimer_get_milliseconds();
//    return 0;
//}


uint32_t bits_timer_delta_time(
    uint32_t prevTime,
    uint32_t newTime )
{
  if ( prevTime <= newTime ) return newTime - prevTime ;
  // we have wrapped
  return (UINT32_MAX - prevTime) + newTime + 1;
}


uint32_t bits_sysTimer_get_milliseconds(
    void )
{
    return (uint32_t) timeGetTime();
}


uint32_t bits_sysTimer_elapsed_milliseconds(
                                         uint32_t prevTime )
{
    return bits_timer_delta_time ( prevTime, bits_sysTimer_get_milliseconds() ) ;
}


//uint32_t bits_multiTimer_elapsed_milliseconds(
//                                         TimerHandle handle )
//{
//    uint32_t newTime = bits_sysTimer_get_milliseconds() ;
//
//    return ( bits_timer_delta_time( oldMultiTime, newTime ) ) ;
//}
//
//
//void bits_multiTimer_reset(
//                      TimerHandle handle )
//{
//  oldMultiTime = bits_sysTimer_get_milliseconds();
//}


void msSleep(uint16_t ms)
{
    Sleep((DWORD)ms);
}

uint64_t SystemTimeNowInMilliseconds(void)
{
    ULONGLONG st = GetTickCount64();
    return ( st );
}



bool osKBhit(void)
{
    return _kbhit();
}


uint8_t osGetch(void)
{
    return (uint8_t) _getch();
}


bool bits_FileExists(const char* name)
{
    DWORD dwAttrib = GetFileAttributesA(name);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


void bits_sys_mutex_init(bits_mutex_t *mutexName)
{
    // We are depending on a recursive mutex (that can be relocked multiple times within the same thread)

    if (mutexName->SpinCount == 0) { (void)InitializeCriticalSectionAndSpinCount(mutexName, 0x400); }    // safe to re-init this
}


// isalnum not available in c++
bool isalnum(uint8_t ch)
{
    if (ch >= '0' || ch <= '9') return true;
    if (ch >= 'A' || ch <= 'Z') return true;
    if (ch >= 'a' || ch <= 'z') return true;
    return false;
}