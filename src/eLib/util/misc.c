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
#include "bacnet/bits/util/bitsUtil.h"
#include "eLib/util/eLibDebug.h"
#include "eLib/util/eLibUtil.h"

#ifdef _MSC_VER
#include <conio.h>
#pragma comment(lib, "Winmm.lib")           // for timeGetTime()
#endif

#ifdef __GNUC__
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
//#include "timerCommon.h"
#endif

#include "string.h"

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


uint16_t bits_strlen(const char *orig)
{
    for (int i = 0; i <= 1000; i++) {
        if (orig[i] == 0) return i;
    }
    panicDesc("String too long to make any sense");
    return 0;
}
 
 
uint16_t bits_strlcpy(char *dest, const char *orig, const uint destLen )
{
	if (destLen == 0)
	{
		// makes 0 sense
		panic();
		return 0;
	}

	if (orig == NULL )
	{
		// not a string then, do we want to fail silently? I doubt it.
		dest[0] = 0;
		panic();
		return 0;
	}

    int copyLen = bits_strlen(orig) + 1 ;
    if ( copyLen > destLen) {
        dest[0] = 0;
        panic();
        return 0 ;
    }

    bits_memcpy(dest, orig, copyLen );
    return copyLen - 1;
}


void bits_memcpy(void *dest, const void *src, uint16_t destLen)
{
    for (int i = 0; i < destLen; i++) {
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
    }
}


void bits_memset(void *dest, uint8_t val, uint16_t destLen)
{
    for (int i = 0; i < destLen; i++) {
        ((uint8_t *)dest)[i] = val ;
    }
}

