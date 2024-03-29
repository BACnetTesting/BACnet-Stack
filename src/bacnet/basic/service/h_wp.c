/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "configProj.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacerror.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/wp.h"
/* device object has the handling for all objects */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"
//#include "debug.h"
#include "bacnet/bactext.h"
#include "bacnet/basic/service/h_wp.h"

/** @file h_wp.c  Handles Write Property requests. */

/** Handler for a WriteProperty Service request.
 * @ingroup DSWP
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - an ACK if Device_Write_Property() succeeds
 * - an Error if Device_Write_Property() fails
 *   or there isn't enough room in the APDU to fit the data.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_write_property(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_WRITE_PROPERTY_DATA wp_data;
    int16_t len ;
    uint16_t pdu_len ;
    BACNET_NPCI_DATA npci_data;

	DLCB *dlcb = alloc_dlcb_response('n', &src->bacnetPath, pDev->datalink->max_lpdu );
	if (dlcb == NULL) return;

	/* encode the NPDU portion of the packet */
    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &src->bacnetPath.glAdr, NULL, &npci_data);

    dbMessage(DBD_ALL, DB_UNUSUAL_TRAFFIC, "WP: Received Request!");

    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                              true);
        dbMessage(DBD_ALL, DB_ERROR, "WP: Segmented message.  Sending Abort!");
        goto WP_ABORT;
    }   /* decode the service request only */
    len = wp_decode_service_request(service_request, service_len, &wp_data);

    /* bad decoding or something we didn't understand - send an abort */
    if (len <= 0) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_OTHER, true);
        dbMessage(DBD_ALL, DB_ERROR, "WP: Bad Encoding. Sending Abort!");
        goto WP_ABORT;
    }
    if (Device_Write_Property(pDev, &wp_data)) {
        len =
            encode_simple_ack(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, SERVICE_CONFIRMED_WRITE_PROPERTY);
        dbMessage(DBD_ALL, DB_UNUSUAL_TRAFFIC, "WP: Sending Simple Ack!");
    } else {
        len =
            bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                 service_data->invoke_id, SERVICE_CONFIRMED_WRITE_PROPERTY,
                                 wp_data.error_class, wp_data.error_code);
        dbMessage(DBD_ALL, DB_UNUSUAL_TRAFFIC, "WP: Sending Error Class:%s Code:%s!", bactext_error_class_name( wp_data.error_class), bactext_error_code_name(wp_data.error_code)  );
    }

WP_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);
}


/** Perform basic validation of Write Property argument based on
 * the assumption that it is a string. Check for correct data type,
 * correct encoding (fixed here as ANSI X34),correct length, and
 * finally if it is allowed to be empty.
 */

bool WPValidateString(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint16_t iMaxLen,
    bool bEmptyAllowed,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    bool bResult;

    /* Save on a bit of code duplication by pre selecting the most
     * common outcomes from the tests (not necessarily the most likely
     * outcome of the tests).
     */
    bResult = false;
    *pErrorClass = ERROR_CLASS_PROPERTY;

    if (pValue->tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
        if (characterstring_encoding(&pValue->type.Character_String) ==
            CHARACTER_ANSI_X34) {
            if ((bEmptyAllowed == false) &&
                (characterstring_length(&pValue->type.Character_String) ==
                    0)) {
                *pErrorCode = ERROR_CODE_VALUE_OUT_OF_RANGE;
            } else if ((bEmptyAllowed == false) &&
                (!characterstring_printable(&pValue->type.Character_String))) {
                /* assumption: non-empty also means must be "printable" */
                *pErrorCode = ERROR_CODE_VALUE_OUT_OF_RANGE;
            } else if (characterstring_length(&pValue->type.Character_String) >
                (uint16_t) iMaxLen) {
                *pErrorClass = ERROR_CLASS_RESOURCES;
                *pErrorCode = ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
            } else
                bResult = true; /* It's all good! */
        } else
            *pErrorCode = ERROR_CODE_CHARACTER_SET_NOT_SUPPORTED;
    } else
        *pErrorCode = ERROR_CODE_INVALID_DATA_TYPE;

    return (bResult);
}

/** Perform simple validation of type of Write Property argument based
 * the expected type vs the actual. Set up error response if the
 * validation fails. Cuts out reams of repeated code in the object code.
 */

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    BACNET_APPLICATION_TAG ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    bool bResult;

    /*
     * start out assuming success and only set up error
     * response if validation fails.
     */
    bResult = true;
    if (pValue->tag != ucExpectedTag) {
        bResult = false;
        *pErrorClass = ERROR_CLASS_PROPERTY;
        *pErrorCode = ERROR_CODE_INVALID_DATA_TYPE;
    }

    return (bResult);
}
