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

#include "configProj.h"

#if (INTRINSIC_REPORTING_B == 1)

#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "bacnet/basic/services.h"


/** @file h_alarm_sum.c  Handles Get Alarm Summary request. */

static get_alarm_summary_function Get_Alarm_Summary[MAX_BACNET_OBJECT_TYPE];

#if ( BACNET_PROTOCOL_REVISION <= 13 )

void handler_get_alarm_summary_set(
    BACNET_OBJECT_TYPE object_type,
    get_alarm_summary_function pFunction)
{
    if (object_type < MAX_BACNET_OBJECT_TYPE) {
        Get_Alarm_Summary[object_type] = pFunction;
    }
}

void handler_get_alarm_summary(
	// PORT_SUPPORT *portParams, 
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    int pdu_len = 0;
    int apdu_len = 0;
//    int bytes_sent ;
    int alarm_value = 0;
    unsigned i = 0;
    unsigned j = 0;
    bool error = false;
    //BACNET_GLOBAL_ADDRESS my_address;
    BACNET_NPCI_DATA npci_data;
    BACNET_GET_ALARM_SUMMARY_DATA getalarm_data;



    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &src->bacnetPath->adr, NULL,
                        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        apdu_len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                              true);
#if PRINT_ENABLED
        fprintf(stderr,
                "GetAlarmSummary: Segmented message. Sending Abort!\n");
#endif
        goto GET_ALARM_SUMMARY_ABORT;
    }

    /* init header */
    apdu_len =
        get_alarm_summary_ack_encode_apdu_init(&Handler_Transmit_Buffer
                [pdu_len], service_data->invoke_id);


    for (i = 0; i < MAX_BACNET_OBJECT_TYPE; i++) {
        if (Get_Alarm_Summary[i]) {
            for (j = 0; j < 0xffff; j++) {
                alarm_value = Get_Alarm_Summary[i] (pDev, j, &getalarm_data);
                if (alarm_value > 0) {
                    len =
                        get_alarm_summary_ack_encode_apdu_data
                        (&Handler_Transmit_Buffer[pdu_len + apdu_len],
                         service_data->max_resp - apdu_len, &getalarm_data);
                    if (len <= 0) {
                        error = true;
                        goto GET_ALARM_SUMMARY_ERROR;
                    } else
                        apdu_len += len;
                } else if (alarm_value < 0) {
                    break;
                }
            }
        }
    }


#if PRINT_ENABLED
    fprintf(stderr, "GetAlarmSummary: Sending response!\n");
#endif

GET_ALARM_SUMMARY_ERROR:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            /* BACnet APDU too small to fit data, so proper response is Abort */
            apdu_len =
                abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                  service_data->invoke_id,
                                  ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
#if PRINT_ENABLED
            fprintf(stderr,
                    "GetAlarmSummary: Reply too big to fit into APDU!\n");
#endif
        } else {
            apdu_len =
                bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_GET_ALARM_SUMMARY,
                                     ERROR_CLASS_PROPERTY, ERROR_CODE_OTHER);
#if PRINT_ENABLED
            fprintf(stderr, "GetAlarmSummary: Sending Error!\n");
#endif
        }
    }


GET_ALARM_SUMMARY_ABORT:
    pdu_len += apdu_len;
    src->portParams->SendPdu(src->portParams, pDev, &src->bacnetPath->localMac, &npci_data, &Handler_Transmit_Buffer[0],
                          pdu_len);
}

#endif // ( BACNET_PROTOCOL_REVISION <= 13 )

#endif // (INTRINSIC_REPORTING_B == 1)
