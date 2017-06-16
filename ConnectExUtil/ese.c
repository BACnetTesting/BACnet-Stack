/**************************************************************************
*
* Copyright (C) 2014-2017 ConnectEx, Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
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
        if (sys_err_buffer[i++] == (uint8_t) value + 0x80 ) return;
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



