/**************************************************************************
 *
 * Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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

#if ( BACNET_SVC_RR_B == 1 )

#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacerror.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/reject.h"
#include "bacnet/readrange.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"

/** @file h_rr.c  Handles Read Range requests. */

static uint8_t Temp_Buf[MAX_LPDU_IP] = { 0 };  // todo 3 - use mallocs - note, this static allocation takes place all over the place. 

/* Encodes the property APDU and returns the length,
   or sets the error, and returns -1 */
static int Encode_RR_payload(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest)
{
    int apdu_len = BACNET_STATUS_ERROR ;
    rr_info_function info_fn_ptr = NULL;
    RR_PROP_INFO PropInfo;

    /* initialize the default return values */
    pRequest->error_class = ERROR_CLASS_SERVICES;
    pRequest->error_code = ERROR_CODE_OTHER;

    /* handle each object type */
    info_fn_ptr = Device_Objects_RR_Info(pDev, pRequest->object_type);

    if ((info_fn_ptr != NULL) ) {
        if (info_fn_ptr(pDev, pRequest, &PropInfo) != false) {
            /* We try and do some of the more generic error checking here to cut down on duplication of effort */
            if ((pRequest->RequestType == RR_BY_POSITION) && (pRequest->Range.RefIndex == 0)) {     /* First index is 1 so can't accept 0 */
                pRequest->error_code = ERROR_CODE_OTHER;    /* I couldn't see anything more appropriate so... */
            }
            else if (((PropInfo.RequestTypes & RR_ARRAY_OF_LISTS) == 0) &&
                (pRequest->array_index != 0) &&
                (pRequest->array_index != BACNET_ARRAY_ALL)) {
                /* Array access attempted on a non array property */
                pRequest->error_class = ERROR_CLASS_PROPERTY;
                pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
            }
            else if ((pRequest->RequestType != RR_READ_ALL) &&
                ((PropInfo.RequestTypes & pRequest->RequestType) == 0)) {
                /* By Time or By Sequence not supported - By Position is always required */
                pRequest->error_code = ERROR_CODE_OTHER;    /* I couldn't see anything more appropriate so... */
            }
            else if ((pRequest->Count == 0) && (pRequest->RequestType != RR_READ_ALL)) {  /* Count cannot be zero */
                pRequest->error_code = ERROR_CODE_OTHER;    /* I couldn't see anything more appropriate so... */
            }
            else if (PropInfo.Handler != NULL) {
                apdu_len = PropInfo.Handler(pDev, apdu, pRequest);
            }
        }
        else {
            // info_fn_ptr may have set some other errors 
            // leave apdu_len set to BACNET_STATUS_ERROR
        }
    } else {
        /* Either we don't support RR for this property yet or it is not a list or array of lists */
        // pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
        // unrecognized service is a Reject, not an error
        return BACNET_STATUS_REJECT;
    }

    return apdu_len;
}


void handler_read_range(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t* service_request,
    uint16_t service_len,
    BACNET_ROUTE* srcRoute,
    BACNET_CONFIRMED_SERVICE_DATA* service_data)
{
    BACNET_READ_RANGE_DATA data;
    int len ;
    int pdu_len ;
    BACNET_NPCI_DATA npci_data;
    bool error = false;

	DLCB *dlcb = alloc_dlcb_response('m', &srcRoute->bacnetPath, pDev->datalink->max_lpdu );
	if (dlcb == NULL) return;

    data.error_class = ERROR_CLASS_OBJECT;
    data.error_code = ERROR_CODE_UNKNOWN_OBJECT;
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &srcRoute->bacnetPath.glAdr, NULL, // &my_address,
        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                              true);
#if PRINT_ENABLED
        dbTraffic(DB_UNEXPECTED_ERROR, "RR: Segmented message.  Sending Abort!\n");
#endif
        goto RR_ABORT;
    }
    memset(&data, 0, sizeof(data));     /* start with blank canvas */
    len = rr_decode_service_request(service_request, service_len, &data);
#if PRINT_ENABLED
    if (len <= 0)
        dbTraffic(DB_UNEXPECTED_ERROR, "RR: Unable to decode Request!\n");
#endif
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
        dbTraffic(DB_UNEXPECTED_ERROR, "RR: Bad Encoding.  Sending Abort!\n");
#endif
        goto RR_ABORT;
    }

    /* assume that there is an error */
    error = true;
    len = Encode_RR_payload(pDev, &Temp_Buf[0], &data);
    if (len >= 0) {
        /* encode the APDU portion of the packet */
        data.application_data = &Temp_Buf[0];
        data.application_data_len = len;
        /* FIXME: probably need a length limitation sent with encode */
        len =
            rr_ack_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                               service_data->invoke_id, &data);
#if PRINT_ENABLED
        dbTraffic(DB_UNEXPECTED_ERROR, "RR: Sending Ack!\n");
#endif
        error = false;
    }
    if (error) {
        if (len == BACNET_STATUS_ABORT ) {
            /* BACnet APDU too small to fit data, so proper response is Abort */
            len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                  service_data->invoke_id,
                                  ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
#if PRINT_ENABLED
            dbTraffic(DB_UNEXPECTED_ERROR, "RR: Reply too big to fit into APDU!\n");
#endif
        }
        else if (len == BACNET_STATUS_REJECT) {
            len =
                reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    REJECT_REASON_UNRECOGNIZED_SERVICE);                        // the reject reason is a big assumption for now... 
        }
        else {
            len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_READ_RANGE,
                                     data.error_class, data.error_code);
#if PRINT_ENABLED
            dbTraffic(DB_UNEXPECTED_ERROR, "RR: Sending Error!\n");
#endif
        }
    }

RR_ABORT:
    pdu_len += len;

    dlcb->optr = (uint16_t) pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);
}

#endif // #if  (BACNET_SVC_RR_B == 1)
