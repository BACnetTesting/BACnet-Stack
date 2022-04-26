/****************************************************************************************
*
*   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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
#include "bitsUtil.h"
#include "bitsDebug.h"

#ifdef _MSC_VER
#include <conio.h>
#pragma comment(lib, "Winmm.lib")           // for timeGetTime()
#endif

#ifdef __GNUC__
#include <stdint.h>
#include <unistd.h>
#include "timerCommon.h"
#endif

#include "string.h"

#if defined ( _MSC_VER  )
void msSleep(uint16_t ms)
{
    Sleep(ms);
}
#else
void msSleep(uint16_t ms)
{
    usleep(1000 * ms);
}
#endif


// all this is now handled in osLayer.c and timerCommon.h
//// ms since machine was started
//uint32_t timer_get_time(void)
//{
//#ifdef OS_LAYER_WIN
//	return timeGetTime();
//#else
//    panic();
//#endif
//}
//
//uint16_t timer_delta_time(uint32_t startTime)
//{
//#ifdef OS_LAYER_WIN
//    uint32_t timeNow = timeGetTime();
//#else
//    panic();
//    // uint32_t timeNow = timeGetTime();
//    uint32_t timeNow = 11 ;
//#endif
//    uint32_t deltaTime;
//
//    if (timeNow < startTime)
//    {
//        deltaTime = timeNow - startTime;
//    }
//    else
//    {
//        deltaTime = (UINT32_MAX - startTime + timeNow + 1);
//    }
//
//	if (deltaTime > UINT16_MAX) return UINT16_MAX;			// todo, this will still fail WHEN the timer wraps on 32 bits...
//	return (uint16_t)deltaTime;
//}


//#ifdef _MSC_VER
//bool bitsKBhit(void)
//{
//    return _kbhit();
//}
//
//int bitsGetch(void)
//{
//    return _getch();
//}
//#endif


bool isMatchCaseInsensitive(const char *stringA, const char *stringB)
{
#ifdef _MSC_VER
    return ( _strcmpi(stringA, stringB) == 0 ) ;
#else
    return ( strcasecmp (stringA, stringB) == 0 ) ;
#endif
}


bool IsSubstring(const char* source, const char* substring)
{
    if (strstr(source, substring) != NULL) {
        return true;
    }
    else {
        return false;
    }
}


char bits_toupper(const char orig)
{
    if (orig < 'a') return orig;
    return (orig - 'a' + 'A');
}


size_t bits_strlen(const char *orig)
{
    for (int i = 0; i <= 4096; i++) {
        if (orig[i] == 0) return i;
    }
    // todo 0 panicDesc("String too long for BACnet");
    return 0;
}


size_t bits_strlcpy(char *dest, const char *orig, const uint destLen )
{
    if (bits_strlen(orig) > (destLen - 1)) {
        dest[0] = 0;
        panic();
        return 0 ;
    }

    bits_memcpy(dest, orig, destLen);
    return destLen - 1;
}


void bits_memcpy(void *dest, const void *src, uint destLen)
{
    for (int i = 0; i < destLen; i++) {
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
    }
}


void bits_memset(void *dest, uint8_t val, uint destLen)
{
    for (int i = 0; i < destLen; i++) {
        ((uint8_t *)dest)[i] = val ;
    }
}

