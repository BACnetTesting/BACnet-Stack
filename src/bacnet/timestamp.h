/**************************************************************************
 *
 * Copyright (C) 2008 John Minack
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

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "bacdcode.h"

typedef enum {
    TIME_STAMP_TIME = 0,
    TIME_STAMP_SEQUENCE = 1,
    TIME_STAMP_DATETIME = 2
} BACNET_TIMESTAMP_TAG;

typedef uint8_t TYPE_BACNET_TIMESTAMP_TYPE;

typedef struct BACnet_Timestamp {
    TYPE_BACNET_TIMESTAMP_TYPE tag;
    union {
        BACNET_TIME time;
        uint16_t sequenceNum;
        BACNET_DATE_TIME dateTime;
    } value;
} BACNET_TIMESTAMP;


void bacapp_timestamp_sequence_set(
    BACNET_TIMESTAMP * dest,
    uint16_t sequenceNum);

void bacapp_timestamp_time_set(
    BACNET_TIMESTAMP * dest,
    BACNET_TIME *btime);

void bacapp_timestamp_sequence_set(
    BACNET_TIMESTAMP * dest,
    uint16_t sequenceNum);

void bacapp_timestamp_time_set(
    BACNET_TIMESTAMP * dest,
    BACNET_TIME *btime);

void bacapp_timestamp_datetime_set(
    BACNET_TIMESTAMP * dest,
    BACNET_DATE_TIME * bdateTime);

void bacapp_timestamp_copy(
    BACNET_TIMESTAMP * dest,
    BACNET_TIMESTAMP * src);

int bacapp_encode_timestamp(
    uint8_t * apdu,
    BACNET_TIMESTAMP * value);

int bacapp_decode_timestamp(
    uint8_t * apdu,
    BACNET_TIMESTAMP * value);


int bacapp_encode_context_timestamp(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_TIMESTAMP * value);
int bacapp_decode_context_timestamp(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_TIMESTAMP * value);

#endif
