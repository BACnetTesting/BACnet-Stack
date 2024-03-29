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

#ifndef READPROPERTY_H
#define READPROPERTY_H

#include <stdint.h>
#include <stdbool.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacenum.h"

typedef struct BACnet_Read_Property_Data {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    BACNET_PROPERTY_ID object_property;
    BACNET_ARRAY_INDEX array_index;
    uint8_t *application_data;
    int application_data_len;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
} BACNET_READ_PROPERTY_DATA;

/* Forward declaration of RPM-style data structure */
struct BACnet_Read_Access_Data;
typedef struct devObj_s DEVICE_OBJECT_DATA;

/** Reads one property for this object type of a given instance.
 * A function template; @see device.c for assignment to object types.
 * @ingroup ObjHelpers
 *
 * @param rp_data [in] Pointer to the BACnet_Read_Property_Data structure,
 *                     which is packed with the information from the RP request.
 * @return The length of the apdu encoded or -1 for error or
 *         -2 for abort message.
 */
typedef int (
    *read_property_function) (
    DEVICE_OBJECT_DATA *pDev,
    BACNET_READ_PROPERTY_DATA * rp_data);


/* encode service */
int rp_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_READ_PROPERTY_DATA * rpdata);

/* decode the service request only */
int rp_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_READ_PROPERTY_DATA * rpdata);

/* method to encode the ack without extra buffer */
int rp_ack_encode_apdu_init(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_READ_PROPERTY_DATA * rpdata);

int rp_ack_encode_apdu_object_property_end(
    uint8_t * apdu);

/* method to encode the ack using extra buffer */
int rp_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_READ_PROPERTY_DATA * rpdata);

int rp_ack_decode_service_request(
    uint8_t * apdu,
    int apdu_len,   /* total length of the apdu */
    BACNET_READ_PROPERTY_DATA * rpdata);

/* Decode instead to RPM-style data structure. */
int rp_ack_fully_decode_service_request(
    uint8_t * apdu,
    int apdu_len,
    struct BACnet_Read_Access_Data *read_access_data);

#ifdef TEST
#include "ctest.h"
int rp_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_READ_PROPERTY_DATA * rpdata);

int rp_ack_decode_apdu(
    uint8_t * apdu,
    int apdu_len,   /* total length of the apdu */
    uint8_t * invoke_id,
    BACNET_READ_PROPERTY_DATA * rpdata);

void test_ReadProperty(
    Test * pTest);
void test_ReadPropertyAck(
    Test * pTest);
#endif

/** @defgroup DataShare Data Sharing BIBBs
 * These BIBBs prescribe the BACnet capabilities required to interoperably
 * perform the data sharing functions enumerated in 22.2.1.1 for the BACnet
     * devices defined therein.
 */

/** @defgroup DSRP Data Sharing -Read Property Service (DS-RP)
 * @ingroup DataShare
 * 15.5 ReadProperty Service <br>
 * The ReadProperty service is used by a client BACnet-user to request the
 * value of one property of one BACnet Object. This service allows read access
 * to any property of any object, whether a BACnet-defined object or not.
 */
#endif
