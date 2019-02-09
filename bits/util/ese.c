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
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

/*
    ESE  -   Embedded System Error reporting

  Embedded systems cannot easliy indicate errors - typically no printf, debug cable attached etc.
  The ESE library allows system errors to be captured and displayed later in a number of ways:
    - Flashing LEDs (Flasherr.c - and the reason why we disallow error numbers with '0' in them
    - Debugger memory examination (at symbol sys_err_buffer[] )
    - Using ese_available() and/or ese_dequeu() to dig out error numbers and printf/BTA etc if available

  Errors can be ese_enqueue() once, or ese_enqueu() to record multiple instances

  Error buffer can be 'one-shot' until fill, or circular

*/

#include "ese.h"

// ESE options
// Option 1 - circulating queue - most useful when BTA available to receive debug messages
// Option 2 - one-shot then no more reports - more useful for embedded debugging - shows first errors

#define ESE_OPTION  1

uint8_t sys_err_buffer[MX_ESE];
uint8_t eseHead ;
static uint8_t eseTail ;

void ese_enqueue_uint8( uint8_t value)
{
#if (ESE_OPTION == 2 )
    // add to queue until full, then freeze
    if ( eseHead < MX_ESE )
        {
        sys_err_buffer[eseHead++] = value;
        }
#else
    // warning, if there are lots of errors, head will catch up with tail. So be it.
    sys_err_buffer[eseHead++] = value;
    if (eseHead >= MX_ESE) eseHead = 0;
#endif
    
    // and if we want to flash something....
    FlashErr_enqueue(value);
}

void ese_enqueue( ESE value )
{
    ese_enqueue_uint8( (uint8_t) value  ) ;
}

int8_t initFail;

void ese_enqueue_init_fail(ESE rc)
{
    initFail = (int8_t) rc;
}

void ese_enqueue_once(ESE value)
{
    uint8_t i = eseTail;
    while (i != eseHead)
        {
        if (sys_err_buffer[i++] == (uint8_t) value ) return;
        if (i >= MX_ESE) i = 0;
        }
    ese_enqueue(value);
}


uint8_t ese_dequeue(void)
{
    uint8_t rc = 0 ;
    if (eseTail != eseHead)
        {
        rc = sys_err_buffer[eseTail++];
#if (ESE_OPTION == 1 )
        // this avoids tail pointer looping if head pointer is maxed on one-shot mode
        if (eseTail >= MX_ESE) eseTail = 0;
#endif
        }
    return rc;
}

bool ese_available(void)
{
    return eseHead != eseTail;
}



