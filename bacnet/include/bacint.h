/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*****************************************************************************************
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#ifndef BACINT_H
#define BACINT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* unsigned value encoding and decoding */
uint8_t encode_unsigned8(
	uint8_t * apdu,
	uint8_t value);

uint8_t encode_unsigned16(
    uint8_t * apdu,
    uint16_t value);

uint8_t decode_unsigned16(
    const uint8_t * apdu,
    uint16_t * value);

uint8_t encode_unsigned24(
    uint8_t * apdu,
    uint32_t value);

uint8_t decode_unsigned24(
    const uint8_t * apdu,
    uint32_t * value);

uint8_t encode_unsigned32(
    uint8_t * apdu,
    uint32_t value);

uint8_t decode_unsigned32(
    const uint8_t * apdu,
    uint32_t * value);

#ifdef UINT64_MAX
int decode_unsigned64(
    uint8_t * buffer,
    uint64_t * value);
#endif

/* signed value encoding and decoding */
uint8_t encode_signed8(
    uint8_t * apdu,
    int8_t value);

uint8_t decode_signed8(
    uint8_t * apdu,
    int32_t * value);

uint8_t encode_signed16(
    uint8_t * apdu,
    int16_t value);

uint8_t decode_signed16(
    uint8_t * apdu,
    int32_t * value);

uint8_t encode_signed24(
    uint8_t * apdu,
    int32_t value);

uint8_t decode_signed24(
    uint8_t * apdu,
    int32_t * value);

uint8_t encode_signed32(
    uint8_t * apdu,
    int32_t value);

uint8_t decode_signed32(
    uint8_t * apdu,
    int32_t * value);

#ifdef UINT64_MAX
int encode_unsigned64(
    uint8_t * buffer,
    uint64_t value);
#endif

#ifdef TEST
#include "ctest.h"
void testBACnetIntegers(
    Test * pTest);
#endif

#endif
