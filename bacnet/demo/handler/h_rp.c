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

//#include <stddef.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
//#include <errno.h>
//#include "config.h"
// #include "txbuf.h"
//#include "bacdef.h"
//#include "bacdcode.h"
#include "bacerror.h"
//#include "bacdevobjpropref.h"
#include "apdu.h"
//#include "npdu.h"
#include "abort.h"
#include "reject.h"
//#include "rp.h"
/* device object has custom handler for all objects */
#include "device.h"
#include "handlers.h"
#include "datalink.h"

/** @file h_rp.c  Handles Read Property requests. */


/** Handler for a ReadProperty Service request.
 * @ingroup DSRP
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 *   - if the response would be too large
 * - the result from Device_Read_Property(), if it succeeds
 * - an Error if Device_Read_Property() fails
 *   or there isn't enough room in the APDU to fit the data.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_read_property(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * srcRoute,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    int len ;
    int pdu_len ;
    int apdu_len = 0 ;
    int npdu_len ;
    BACNET_NPCI_DATA npci_data;
    bool error = true;  /* assume that there is an error */
    // int bytes_sent = 0;
//    BACNET_PATH my_address;

    DLCB *dlcb = alloc_dlcb_response('f', srcRoute);
    if (dlcb == NULL) return;

    /* configure default error code as an abort since it is common */
    rpdata.error_class = ERROR_CLASS_OBJECT;
    rpdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;

    /* encode the NPDU portion of the packet */
    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &srcRoute->bacnetPath.glAdr, NULL,
        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len = BACNET_STATUS_ABORT;
        dbTraffic(DBD_ALL, DB_ERROR, "RP: Segmented message.  Sending Abort!");
        goto RP_FAILURE;
    }
    len = rp_decode_service_request(service_request, service_len, &rpdata);
//    dbTrafficAssert(DB_ERROR, (len > 0 ), "RP: Unable to decode Request!");
    if (len < 0) {
        /* bad decoding - skip to error/reject/abort handling */
        error = true;
        dbTraffic(DBD_ALL, DB_ERROR, "RP: Bad Encoding.");
        goto RP_FAILURE;
    }
    /* Test for case of indefinite Device object instance */
    if ((rpdata.object_type == OBJECT_DEVICE) &&
        (rpdata.object_instance == BACNET_MAX_INSTANCE)) {
        rpdata.object_instance = Device_Object_Instance_Number();
    }

    apdu_len =
        rp_ack_encode_apdu_init(&dlcb->Handler_Transmit_Buffer[npdu_len],
        service_data->invoke_id, &rpdata);
    /* configure our storage */
    rpdata.application_data = &dlcb->Handler_Transmit_Buffer[npdu_len + apdu_len];
    rpdata.application_data_len =
        dlcb->lpduMax - (npdu_len + apdu_len);
    len = Device_Read_Property(&rpdata);
    if (len >= 0) {
        apdu_len += len;
        len =
            rp_ack_encode_apdu_object_property_end(&dlcb->Handler_Transmit_Buffer
            [npdu_len + apdu_len]);
        apdu_len += len;
        if (apdu_len > service_data->max_resp) {
            /* too big for the sender - send an abort
             * Setting of error code needed here as read property processing may
             * have overriden the default set at start */
            rpdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
            len = BACNET_STATUS_ABORT;
            dbTraffic(DBD_ALL, DB_ERROR, "RP: Message too large."); // todo1 - btc, test 00029 raised an oveflow err here??? (stack crash etc.. something got corrupted!)
            // Confirmed 2016.12.02 , bit00044 bombed this during emulation. todo2
            // (apdu len was 1778 ), reading schedule default I think.
        } else {
            dbTraffic(DBD_ALL, DB_INFO, "RP: Sending Ack!");
            error = false;
        }
    } else {
#if PRINT_ENABLED_off
        if (len == BACNET_STATUS_ABORT) {
            dbTraffic(DB_ERROR, "RP: Device_Read_Property: Abort!");
        } else if (len == BACNET_STATUS_ERROR || len == BACNET_STATUS_UNKNOWN_PROPERTY ) {
            dbTraffic(DB_ERROR, "RP: Device_Read_Property: Error. Class:%s, Code:%s  for  %s:%d.%s !", 
                bactext_error_class_name(rpdata.error_class), 
                bactext_error_code_name(rpdata.error_code), 
                bactext_object_type_name(rpdata.object_type), 
                rpdata.object_instance, 
                bactext_property_name(rpdata.object_property) );
        } else if (len == BACNET_STATUS_REJECT) {
            dbTraffic(DB_ERROR, "RP: Device_Read_Property: Reject!");
        } else {
            dbTraffic(DB_ERROR, "RP: Device_Read_Property: Unknown Len=%d", len);
        }
#endif
    }

RP_FAILURE:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id,
                abort_convert_error_code(rpdata.error_code), true);
            dbTraffic(DBD_ALL, DB_UNUSUAL_TRAFFIC, "RP: Sending Abort!");
        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                                     rpdata.error_class, rpdata.error_code);
            dbTraffic(DBD_ALL, DB_EXPECTED_ERROR_TRAFFIC, "RP: Sending Error!");
        } else if (len == BACNET_STATUS_UNKNOWN_PROPERTY) {
            apdu_len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                                     rpdata.error_class, rpdata.error_code);
            // and no error message
        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                    service_data->invoke_id,
                    reject_convert_error_code(rpdata.error_code));
            dbTraffic(DBD_ALL, DB_UNUSUAL_TRAFFIC, "RP: Sending Reject!");
        }
        else
        {
            panic();
            return;
        }
    }

    pdu_len = npdu_len + apdu_len;
    dlcb->optr = (uint16_t) pdu_len;
    srcRoute->portParams->SendPdu(dlcb );
}
