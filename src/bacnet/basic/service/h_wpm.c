/**************************************************************************
 *
 * Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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
#include "bacnet/reject.h"
#include "bacnet/wpm.h"
/* device object has the handling for all objects */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"

/** @file h_wpm.c  Handles Write Property Multiple requests. */


/** Handler for a WriteProperty Service request.
 * @ingroup DSWP
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - an ACK if Device_Write_Property_Multiple() succeeds
 * - an Error if Device_Write_PropertyMultiple() encounters an error
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
 
#if 0
void handler_write_property_multiple(
	BACNET_ROUTE *rxDetails, 
    DEVICE_OBJECT_DATA *sendingDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    int apdu_len = 0;
    int npdu_len = 0;
    int pdu_len = 0;
    int decode_len = 0;
    bool error = false;
    BACNET_WRITE_PROPERTY_DATA wp_data;
    BACNET_NPCI_DATA npci_data;
    // BACNET_PATH my_address;
    // int bytes_sent = 0;

	DLCB *dlcb = alloc_dlcb_response('=', rxDetails->portParams);
	if (dlcb == NULL) return;

    if (service_data->segmented_message) {
        wp_data.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
        len = BACNET_STATUS_ABORT;

        dbMessage(DB_UNEXPECTED_ERROR, "WPM: Segmented message.  Sending Abort!\n");

        goto WPM_ABORT;
    }

    /* decode service request */
    decode_len = 0;
    do {
        /* decode Object Identifier */
        len =
            wpm_decode_object_id(&service_request[decode_len],
                                 service_len - decode_len, &wp_data);
        if (len > 0) {
            uint8_t tag_number = 0;

            decode_len += len;
            /* Opening tag 1 - List of Properties */
            if (decode_is_opening_tag_number(&service_request[decode_len++],
                                             1)) {
                do {
                    /* decode a 'Property Identifier'; (3) an optional 'Property Array Index' */
                    /* (4) a 'Property Value'; and (5) an optional 'Priority'. */
                    len =
                        wpm_decode_object_property(&service_request
                                                   [decode_len], service_len - decode_len, &wp_data);
                    if (len > 0) {
                        decode_len += len;
                            if (validated) {

                                dbMessage(DB_UNEXPECTED_ERROR,
	                                "WPM: type=%lu instance=%lu property=%lu priority=%lu index=%ld\n",
	                                (unsigned long) wp_data.object_type,
	                                (unsigned long) wp_data.object_instance,
	                                (unsigned long) wp_data.object_property,
	                                (unsigned long) wp_data.priority,
	                                (long) wp_data.array_index);

                        if (Device_Write_Property(pDev, &wp_data) == false) {
                            error = true;
                            len = BACNET_STATUS_ERROR;
                            goto WPM_ABORT;
                        }
                    }
                    else {

                        dbMessage(DB_UNEXPECTED_ERROR, "WPM: Bad Encoding!\n");

                        error = true;
                        goto WPM_ABORT;
                    }

                    /* Closing tag 1 - List of Properties */
                    if (decode_is_closing_tag_number(&service_request
                                                     [decode_len], 1)) {
                        tag_number = 1;
                        decode_len++;
                        } else {
                        tag_number = 0; /* it was not tag 1, decode next Property Identifier ... */
                }
                    } while (tag_number != 1);  /* end decoding List of Properties for "that" object */

                if (error) {
                    goto WPM_ABORT;
                }
            }
        } 
        else {

                dbMessage(DB_UNEXPECTED_ERROR, "WPM: Bad Encoding!\n");

            error = true;
            goto WPM_ABORT;
        }
    } while (decode_len < service_len);
        
        validated++;
    }
WPM_ABORT:
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, NULL,
                        &npci_data);
    apdu_len = 0;
    /* handle any errors */
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id,
                                  abort_convert_error_code(wp_data.error_code), true);

            dbMessage(DB_UNEXPECTED_ERROR, "WPM: Sending Abort!\n");

        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len =
                wpm_error_ack_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                          service_data->invoke_id, &wp_data);

            dbMessage(DB_UNEXPECTED_ERROR, "WPM: Sending Error!\n");

        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                   service_data->invoke_id,
                                   reject_convert_error_code(wp_data.error_code));

            dbMessage(DB_UNEXPECTED_ERROR, "WPM: Sending Reject!\n");

        }
    } else {
        apdu_len =
            wpm_ack_encode_apdu_init(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id);

        dbMessage(DB_UNEXPECTED_ERROR, "WPM: Sending Ack!\n");

    }

    pdu_len = npdu_len + apdu_len;
    dlcb->optr = pdu_len;
    rxDetails->portParams->SendPdu(dlcb);
}

#endif
