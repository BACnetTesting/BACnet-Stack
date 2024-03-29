/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include "configProj.h"
#include "bacerror.h"
#include "apdu.h"
#include "abort.h"
#include "reject.h"
/* device object has custom handler for all objects */
#include "device.h"
#include "bacnet/basic/services.h"
#include "bactext.h"

#if (INTRINSIC_REPORTING_B == 1)

/** @file h_alarm_ack.c  Handles Alarm Acknowledgment. */

static alarm_ack_function Alarm_Ack[MAX_BACNET_OBJECT_TYPE];    // todo 3 - this is a very inefficient array

void handler_alarm_ack_set(
    BACNET_OBJECT_TYPE object_type,
    alarm_ack_function pFunction)
{
    if (object_type < MAX_BACNET_OBJECT_TYPE) {
        Alarm_Ack[object_type] = pFunction;
    }
}

/** Handler for an Alarm/Event Acknowledgement.
 * @ingroup ALMACK
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - Otherwise, sends a simple ACK
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_alarm_ack(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t *service_request,
    uint16_t service_len,
    BACNET_ROUTE * srcRoute,
    BACNET_CONFIRMED_SERVICE_DATA *service_data)
{
    int len = 0;
    int pdu_len ;
    int ack_result = 0;
    BACNET_NPCI_DATA npci_data;
    BACNET_ALARM_ACK_DATA data;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code = ERROR_CODE_OTHER ;

    DLCB *dlcb = alloc_dlcb_response('A', &srcRoute->bacnetPath, srcRoute->portParams->max_lpdu );
    if (dlcb == NULL) return;

    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &srcRoute->bacnetPath.glAdr, NULL,
        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
        dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Ack: Segmented message.  Sending Abort!\n");
        goto AA_ABORT;
    }

    len =
        alarm_ack_decode_service_request(service_request, service_len, &data);
#if PRINT_ENABLED
    if (len <= 0)
        dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Ack: Unable to decode Request!\n");
#endif
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
        dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Ack: Bad Encoding.  Sending Abort!\n");
        goto AA_ABORT;
    }

    dbMessage(DBD_ALL, DB_BTC_ERROR,
        "Alarm Ack Operation: Received acknowledge for object id (%d, %lu) from %s for process id %lu \n",
        data.eventObjectIdentifier.type,
        (unsigned long) data.eventObjectIdentifier.instance,
        data.ackSource.value, (unsigned long) data.ackProcessIdentifier);

    /* 	BACnet Testing Observed Incident oi00105
    	ACK of a non-existent object returned the incorrect error code
    	Revealed by BACnet Test Client v1.8.16 ( www.bac-test.com/bacnet-test-client-download )
    		BC 135.1: 9.1.3.3-A
    	Any discussions can be directed to edward@bac-test.com */
    if (!Device_Valid_Object_Id(pDev, data.eventObjectIdentifier.type, data.eventObjectIdentifier.instance)) {
        len =
			bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
				service_data->invoke_id,
				SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM, ERROR_CLASS_OBJECT, ERROR_CODE_UNKNOWN_OBJECT);
	}
    else if (Alarm_Ack[data.eventObjectIdentifier.type]) {

        ack_result =
            Alarm_Ack[data.eventObjectIdentifier.type] (pDev, &data, &error_class, &error_code);

        switch (ack_result) {
            case 1:
                len =
                    encode_simple_ack(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM);
                dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Acknowledge: " "Sending Simple Ack!\n");
                break;

            case -1:
                len =
                    bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM, error_class,
                    error_code);
                dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Acknowledge: error %s!\n",
                    bactext_error_code_name(error_code));
                break;

            default:
                len =
                    abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, ABORT_REASON_OTHER, true);
                dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Acknowledge: abort other!\n");
                break;
        }
    } else {
        len =
            bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM,
            ERROR_CLASS_OBJECT, ERROR_CODE_NO_ALARM_CONFIGURED);
        dbMessage(DBD_ALL, DB_BTC_ERROR, "Alarm Acknowledge: error %s!\n",
            bactext_error_code_name(ERROR_CODE_NO_ALARM_CONFIGURED));
    }

AA_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb );

}

#endif // INTRINSIC_REPORTING
