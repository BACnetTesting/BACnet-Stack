/**************************************************************************
 *
 * Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
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

#if ( BACNET_SVC_COV_B == 1 )

#include "bacnet/bacdef.h"
#include "bacdcode.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
/* special for this module */
#include "cov.h"
#include "bactext.h"
#include "bacnet/basic/services.h"
//#include "datalink.h"

#ifndef MAX_COV_PROPERTIES
#define MAX_COV_PROPERTIES 2
#endif

/** @file h_ucov.c  Handles Unconfirmed COV Notifications. */

/*  */
/** Handler for an Unconfirmed COV Notification.
 * @ingroup DSCOV
 * Decodes the received list of Properties to update,
 * and print them out with the subscription information.
 * @note Nothing is specified in BACnet about what to do with the
 *       information received from Unconfirmed COV Notifications.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_GLOBAL_ADDRESS of the source of the message (unused)
 */
void handler_ucov_notification(
    BACNET_ROUTE *rxDetails, 
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len)
{
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE property_value[MAX_COV_PROPERTIES];
    BACNET_PROPERTY_VALUE *pProperty_value ;
#if PRINT_ENABLED
    int len ;
#endif
    unsigned index = 0;

    /* src not needed for this application */
//     (void) src;
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

#if PRINT_ENABLED
    dbMessage(DB_UNEXPECTED_ERROR, "UCOV: Received Notification!\n");
    /* decode the service request only */
    len =
#endif
        /* decode the service request only */
        cov_notify_decode_service_request(service_request, service_len,
                                          &cov_data);

#if PRINT_ENABLED
    if (len > 0) {
        dbMessage(DB_UNEXPECTED_ERROR, "UCOV: PID=%u ", cov_data.subscriberProcessIdentifier);
        dbMessage(DB_UNEXPECTED_ERROR, "instance=%u ", cov_data.initiatingDeviceIdentifier);
        dbMessage(DB_UNEXPECTED_ERROR, "%s %u ",
                bactext_object_type_name(cov_data.monitoredObjectIdentifier.type),
                cov_data.monitoredObjectIdentifier.instance);
        dbMessage(DB_UNEXPECTED_ERROR, "time remaining=%u seconds ", cov_data.timeRemaining);
        dbMessage(DB_UNEXPECTED_ERROR, "\n");
        pProperty_value = &property_value[0];
        while (pProperty_value) {
            dbMessage(DB_UNEXPECTED_ERROR, "UCOV: ");
            if (pProperty_value->propertyIdentifier < 512) {
                dbMessage(DB_UNEXPECTED_ERROR, "%s ",
                        bactext_property_name
                        (pProperty_value->propertyIdentifier));
            } else {
                dbMessage(DB_UNEXPECTED_ERROR, "proprietary %u ",
                        pProperty_value->propertyIdentifier);
            }
            if (pProperty_value->propertyArrayIndex != BACNET_ARRAY_ALL) {
                dbMessage(DB_UNEXPECTED_ERROR, "%u ", pProperty_value->propertyArrayIndex);
            }
            dbMessage(DB_UNEXPECTED_ERROR, "\n");
            pProperty_value = pProperty_value->next;
        }
    } else {
        dbMessage(DB_UNEXPECTED_ERROR, "UCOV: Unable to decode service request!\n");
    }
#endif
}

#endif // ( BACNET_SVC_COV_B == 1 )

