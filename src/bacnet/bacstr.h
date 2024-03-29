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

#ifndef BACSTR_H
#define BACSTR_H

#include <stdint.h>
#ifdef __GNUC__
#include <stdbool.h>
#endif

//#include <stddef.h>
#include "bacnet/bacdef.h"
#include "configProj.h"
#include "bacnet/bacenum.h"

// in bacdef.h (where Karg originally had it)
//typedef struct BACnet_Object_Id {
//        BACNET_OBJECT_TYPE type;
//        uint32_t instance;
//    } BACNET_OBJECT_ID;

/* bit strings
   They could be as large as 256/8=32 octets */
typedef struct BACnet_Bit_String {
    uint8_t bits_used;
    uint8_t value[MAX_BITSTRING_BYTES];
} BACNET_BIT_STRING;

typedef struct BACnet_Character_String {
    uint8_t length;
    BACNET_CHARACTER_STRING_ENCODING encoding;
    /* limit - 6 octets is the most our tag and type could be */
    char value[MAX_CHARACTER_STRING_BYTES];
} BACNET_CHARACTER_STRING;

/* FIXME: convert the bacdcode library to use BACNET_OCTET_STRING
   for APDU buffer to prevent buffer overflows */
typedef struct BACnet_Octet_String {
    uint8_t length;
    /* limit - 6 octets is the most our tag and type could be */
    uint8_t value[MAX_OCTET_STRING_BYTES];
} BACNET_OCTET_STRING;

void bitstring_init(
    BACNET_BIT_STRING * bit_string);

void bitstring_set_bit(
    BACNET_BIT_STRING * bit_string,
    const uint8_t bit_number,
    const bool value);

bool bitstring_bit(
    BACNET_BIT_STRING * bit_string,
    uint8_t bit_number);
uint8_t bitstring_bits_used(
    BACNET_BIT_STRING * bit_string);

/* returns the number of bytes that a bit string is using */
uint8_t bitstring_bytes_used(
    BACNET_BIT_STRING * bit_string);

uint8_t bitstring_bits_capacity(
    BACNET_BIT_STRING * bit_string);

/* used for encoding and decoding from the APDU */
uint8_t bitstring_octet(
    BACNET_BIT_STRING * bit_string,
    uint8_t octet_index);

bool bitstring_set_octet(
    BACNET_BIT_STRING * bit_string,
    uint8_t index,
    uint8_t octet);

bool bitstring_set_bits_used(
    BACNET_BIT_STRING * bit_string,
    uint8_t bytes_used,
    uint8_t unused_bits);

bool bitstring_copy(
    BACNET_BIT_STRING * dest,
    BACNET_BIT_STRING * src);

bool bitstring_same(
    BACNET_BIT_STRING * bitstring1,
    BACNET_BIT_STRING * bitstring2);
bool bitstring_init_ascii(
    BACNET_BIT_STRING * bit_string,
    const char *ascii);

/* returns false if the string exceeds capacity
   initialize by using length=0 */
bool characterstring_init(
    BACNET_CHARACTER_STRING * char_string,
    BACNET_CHARACTER_STRING_ENCODING encoding,
    const char *value,
    uint16_t length);
/* used for ANSI C-Strings */

bool characterstring_init_ansi(
    BACNET_CHARACTER_STRING * char_string,
    const char *value);

BACNET_CHARACTER_STRING *characterstring_create_ansi(
    const char *value);

bool characterstring_copy(
    BACNET_CHARACTER_STRING * dest,
    const BACNET_CHARACTER_STRING * src);

bool characterstring_ansi_copy(
    char *dest,
    uint16_t dest_max_len,
    BACNET_CHARACTER_STRING * src);
/* returns true if the strings are the same length, encoding, value */
bool characterstring_same(
    BACNET_CHARACTER_STRING * dest,
    BACNET_CHARACTER_STRING * src);
bool characterstring_ansi_same(
    BACNET_CHARACTER_STRING * dest,
    const char *src);
/* returns false if the string exceeds capacity */
bool characterstring_append(
    BACNET_CHARACTER_STRING * char_string,
    const char *value,
    uint16_t length);

/* This function sets a new length without changing the value.
   If length exceeds capacity, no modification happens and
   function returns false.  */
bool characterstring_truncate(
    BACNET_CHARACTER_STRING * char_string,
    uint16_t length);

bool characterstring_set_encoding(
    BACNET_CHARACTER_STRING *char_string,
    BACNET_CHARACTER_STRING_ENCODING encoding);

/* Returns the value */
char *characterstring_value(
    const BACNET_CHARACTER_STRING *char_string);

/* returns the length */
uint16_t characterstring_length(
    const BACNET_CHARACTER_STRING *char_string);

BACNET_CHARACTER_STRING_ENCODING characterstring_encoding(
    const BACNET_CHARACTER_STRING * char_string);

uint16_t characterstring_capacity(
    BACNET_CHARACTER_STRING * char_string);

bool characterstring_printable(
    BACNET_CHARACTER_STRING * char_string);

bool characterstring_valid(
    BACNET_CHARACTER_STRING * char_string);

bool utf8_isvalid(
    const char *str,
    uint16_t length);

/* returns false if the string exceeds capacity
   initialize by using length=0 */
bool octetstring_init(
    BACNET_OCTET_STRING * octet_string,
    const uint8_t * value,
    const uint16_t length);

#if PRINT_ENABLED
/* converts an null terminated ASCII Hex string to an octet string.
   returns true if successfully converted and fits; false if too long */
bool octetstring_init_ascii_hex(
    BACNET_OCTET_STRING * octet_string,
    const char *ascii_hex);
#endif

bool octetstring_copy(
    BACNET_OCTET_STRING * dest,
    BACNET_OCTET_STRING * src);

uint16_t octetstring_copy_value(
    uint8_t * dest,
    uint16_t length,
    BACNET_OCTET_STRING * src);

/* returns false if the string exceeds capacity */
bool octetstring_append(
    BACNET_OCTET_STRING * octet_string,
    uint8_t * value,
    uint16_t length);

/* This function sets a new length without changing the value.
   If length exceeds capacity, no modification happens and
   function returns false.  */
bool octetstring_truncate(
    BACNET_OCTET_STRING * octet_string,
    uint16_t length);

/* Returns the value */
uint8_t *octetstring_value(
    BACNET_OCTET_STRING * octet_string);
/* Returns the length.*/

uint16_t octetstring_length(
    BACNET_OCTET_STRING * octet_string);

uint16_t octetstring_capacity(
    BACNET_OCTET_STRING * octet_string);

/* returns true if the same length and contents */
bool octetstring_value_same(
    BACNET_OCTET_STRING * octet_string1,
    BACNET_OCTET_STRING * octet_string2);

#ifdef TEST
#include "ctest.h"
void testBACnetStrings(
    Test * pTest);
#endif

#endif
