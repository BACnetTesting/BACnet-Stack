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

#ifndef ATOMIC_READ_FILE_H
#define ATOMIC_READ_FILE_H

#include <stdint.h>
#include <stdbool.h>
#include "bacdcode.h"
#include "bacstr.h"

#ifndef BACNET_READ_FILE_RECORD_COUNT
#define BACNET_READ_FILE_RECORD_COUNT 1
#endif

typedef struct BACnet_Atomic_Read_File_Data {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    BACNET_FILE_ACCESS_METHOD access;
    union {
        struct {
            int32_t fileStartPosition;
            uint32_t requestedOctetCount;
        } stream;
        struct {
            int32_t fileStartRecord;
            /* requested or returned record count */
            uint32_t RecordCount;
        } record;
    } type;
    // todo2 - Karg's **RELEASE 0.8.0 branch** does not have an array here.... Investigate further and remove if not necessary.
    BACNET_OCTET_STRING fileData[BACNET_READ_FILE_RECORD_COUNT];
    bool endOfFile;
} BACNET_ATOMIC_READ_FILE_DATA;


/* Atomic Read File */
/* encode service */
    int arf_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_ATOMIC_READ_FILE_DATA * data);

/* decode the service request only */
    int arf_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_ATOMIC_READ_FILE_DATA * data);

    int arf_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        BACNET_ATOMIC_READ_FILE_DATA * data);

/* Atomic Read File Ack */

/* encode service */
    int arf_ack_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_ATOMIC_READ_FILE_DATA * data);

/* decode the service request only */
    int arf_ack_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_ATOMIC_READ_FILE_DATA * data);

    int arf_ack_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        BACNET_ATOMIC_READ_FILE_DATA * data);

#ifdef TEST
#include "ctest.h"

    void test_AtomicReadFile(
        Test * pTest);
    void test_AtomicReadFileAck(
        Test * pTest);
#endif

#endif
