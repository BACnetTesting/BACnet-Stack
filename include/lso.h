/**************************************************************************
*
* Copyright (C) 2006 John Minack
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

#ifndef LSO_H
#define LSO_H

//#include <stdint.h>
//#include <stdbool.h>
//#include "bacenum.h"
//#include "bacdef.h"
//#include "bacstr.h"
#include "device.h"

/* Life Safety Operation Service */

typedef struct {
    uint32_t processId;
    BACNET_CHARACTER_STRING requestingSrc;
    BACNET_LIFE_SAFETY_OPERATION operation;
    BACNET_OBJECT_ID targetObject;
} BACNET_LSO_DATA;


int lso_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_LSO_DATA * data);

/* decode the service request only */
int lso_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_LSO_DATA * data);

uint8_t Send_Life_Safety_Operation_Data(
    DEVICE_OBJECT_DATA *destDev,
    uint32_t device_id,
    BACNET_LSO_DATA * data);

#ifdef TEST
#include "ctest.h"
void testLSO(
    Test * pTest);
#endif

#endif
