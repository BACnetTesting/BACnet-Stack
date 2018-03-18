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

#include "bitsUtil.h"

#ifdef _MSC_VER
#include <conio.h>
#pragma comment(lib, "Winmm.lib")           // for timeGetTime()
#endif

#ifdef __GNUC__
#include <unistd.h>
#endif

#include "timer.h"
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

// ms since machine was started
uint32_t timer_get_time(void)
{
	return timeGetTime();
}

uint16_t timer_delta_time(uint32_t startTime)
{
    uint32_t timeNow = timeGetTime();
    uint32_t deltaTime;

    if (timeNow < startTime)
    {
        deltaTime = timeNow - startTime;
    }
    else
    {
        deltaTime = (UINT32_MAX - startTime + timeNow + 1);
    }

	if (deltaTime > UINT16_MAX) return UINT16_MAX;			// todo, this will still fail WHEN the timer wraps on 32 bits...
	return (uint16_t)deltaTime;
}


#ifdef _MSC_VER
bool bitsKBhit(void)
{
    return _kbhit();
}

int bitsGetch(void)
{
    return _getch();
}
#endif


bool isMatchCaseInsensitive(const char *stringA, const char *stringB)
{
#ifdef _MSC_VER
    return ( _strcmpi(stringA, stringB) == 0 ) ;
#else
    return ( strcasecmp (stringA, stringB) == 0 ) ;
#endif
}
