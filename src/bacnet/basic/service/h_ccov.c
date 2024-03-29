/**************************************************************************
 *
 * Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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

#if ( BACNET_SVC_COV_A == 1 )

#include "bacnet/bacdef.h"
#include "bacdcode.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
/* special for this module */
#include "cov.h"
#include "bactext.h"
#include "bacnet/basic/services.h"

#ifndef MAX_COV_PROPERTIES
#define MAX_COV_PROPERTIES 2
#endif

/** @file h_ccov.c  Handles Confirmed COV Notifications. */

/*  */
/** Handler for an Confirmed COV Notification.
 * @ingroup DSCOV
 * Decodes the received list of Properties to update,
 * and print them out with the subscription information.
 * @note Nothing is specified in BACnet about what to do with the
 *       information received from Confirmed COV Notifications.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_ccov_notification(
    // PORT_SUPPORT *portParams,
    const DEVICE_OBJECT_DATA    *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_NPCI_DATA npci_data;
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE property_value[MAX_COV_PROPERTIES];
    BACNET_PROPERTY_VALUE *pProperty_value = NULL;
    unsigned index = 0;
    int len = 0;
    int pdu_len ;
    int bytes_sent ;
    //BACNET_GLOBAL_ADDRESS my_address;

    /* create linked list to store data if more
       than one property value is expected */
    pProperty_value = &property_value[0];
    while (pProperty_value) {
        index++;
        if (index < MAX_COV_PROPERTIES) {
            pProperty_value->next = &property_value[index];
        } else {
            pProperty_value->next = NULL;
        }
        pProperty_value = pProperty_value->next;
    }
    cov_data.listOfValues = &property_value[0];
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &src->bacnetPath->adr, NULL,
                        &npci_data);
#if PRINT_ENABLED
    fprintf(stderr, "CCOV: Received Notification!\n");
#endif
    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                              true);
#if PRINT_ENABLED
        fprintf(stderr, "CCOV: Segmented message.  Sending Abort!\n");
#endif
        goto CCOV_ABORT;
    }
    /* decode the service request only */
    len =
        cov_notify_decode_service_request(service_request, service_len,
                                          &cov_data);
#if PRINT_ENABLED
    if (len > 0) {
        fprintf(stderr, "CCOV: PID=%u ", cov_data.subscriberProcessIdentifier);
        fprintf(stderr, "instance=%u ", cov_data.initiatingDeviceIdentifier);
        fprintf(stderr, "%s %u ",
                bactext_object_type_name(cov_data.monitoredObjectIdentifier.type),
                cov_data.monitoredObjectIdentifier.instance);
        fprintf(stderr, "time remaining=%u seconds ", cov_data.timeRemaining);
        fprintf(stderr, "\n");
        pProperty_value = &property_value[0];
        while (pProperty_value) {
            fprintf(stderr, "CCOV: ");
            if (pProperty_value->propertyIdentifier < 512) {
                fprintf(stderr, "%s ",
                        bactext_property_name
                        (pProperty_value->propertyIdentifier));
            } else {
                fprintf(stderr, "proprietary %u ",
                        pProperty_value->propertyIdentifier);
            }
            if (pProperty_value->propertyArrayIndex != BACNET_ARRAY_ALL) {
                fprintf(stderr, "%u ", pProperty_value->propertyArrayIndex);
            }
            fprintf(stderr, "\n");
            pProperty_value = pProperty_value->next;
        }
    }
#endif

    if (len <= 0) {
        /* bad decoding or something we didn't understand - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
        fprintf(stderr, "CCOV: Bad Encoding. Sending Abort!\n");
#endif
        goto CCOV_ABORT;
    } 
    else {
        len =
            encode_simple_ack(&Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, SERVICE_CONFIRMED_COV_NOTIFICATION);
#if PRINT_ENABLED
        fprintf(stderr, "CCOV: Sending Simple Ack!\n");
#endif
    }

CCOV_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len ;
    bytes_sent =
        src->portParams->SendPdu(dlcb);
#if PRINT_ENABLED
    if (bytes_sent <= 0) {
        fprintf(stderr, "CCOV: Failed to send PDU (%s)!\n", strerror(errno));
    }
#else
    (void) bytes_sent;
#endif

}

#endif // ( BACNET_SVC_COV_B == 1 )

