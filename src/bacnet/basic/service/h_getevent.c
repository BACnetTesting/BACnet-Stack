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

#include "configProj.h"
#include "bacerror.h"
#include "abort.h"
#include "bacnet/basic/services.h"

/** @file h_getevent.c  Handles Get Event Information request. */

#if (INTRINSIC_REPORTING_B == 1)

// todo 3 - this is a HUGE array to cycle through each time. Reconsider the approach soon.
static get_event_info_function Get_Event_Info[MAX_BACNET_OBJECT_TYPE];


/** print eventState
 */
void ge_ack_print_data(
    BACNET_GET_EVENT_INFORMATION_DATA * data,
    uint32_t device_id)
{
    BACNET_GET_EVENT_INFORMATION_DATA *act_data = data;
    const char* state_strs[] = {"NO", "FA", "ON", "HL", "LL"};
    printf("DeviceID\tType\tInstance\teventState\n");
    printf("--------------- ------- --------------- ---------------\n");
    unsigned count = 0;
    while (act_data) {
        printf("%u\t\t%u\t%u\t\t%s\n",
               device_id,
               act_data->objectIdentifier.type,
               act_data->objectIdentifier.instance,
               state_strs[data->eventState]
              );
        act_data = act_data->next;
        count++;
    }
    printf("\n%u\t Total\n",count);
}

void handler_get_event_information_set(
    BACNET_OBJECT_TYPE object_type,
    get_event_info_function pFunction)
{
    if (object_type < MAX_BACNET_OBJECT_TYPE) {
        Get_Event_Info[object_type] = pFunction;
    }
}

void handler_get_event_information(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t* service_request,
    uint16_t service_len,
    BACNET_ROUTE* rxDetails,
    BACNET_CONFIRMED_SERVICE_DATA* service_data)
{
    int len = 0;
    int pdu_len = 0;
    int apdu_len = 0;
    BACNET_NPCI_DATA npci_data;
    bool error = false;
    bool more_events = false;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_OBJECT;
    BACNET_ERROR_CODE error_code = ERROR_CODE_UNKNOWN_OBJECT;
    BACNET_OBJECT_ID object_id;
    unsigned i = 0, j = 0;      /* counter */
    BACNET_GET_EVENT_INFORMATION_DATA getevent_data;
    int valid_event = 0;

	DLCB *dlcb = alloc_dlcb_response('d', &rxDetails->bacnetPath, pDev->datalink->max_lpdu );
	if (dlcb == NULL) return;

    /* initialize type of 'Last Received Object Identifier' using max value */
    object_id.type = MAX_BACNET_OBJECT_TYPE;

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &rxDetails->bacnetPath.glAdr, NULL, // &my_address,
                        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                              true);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
            "GetEventInformation: " "Segmented message. Sending Abort!\n");
        goto GET_EVENT_ABORT;
    }

    len =
        getevent_decode_service_request(service_request, service_len,
                                        &object_id);
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                              service_data->invoke_id, ABORT_REASON_OTHER, true);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
            "GetEventInformation: Bad Encoding.  Sending Abort!\n");
        goto GET_EVENT_ABORT;
    }
    len =
        getevent_ack_encode_apdu_init(&dlcb->Handler_Transmit_Buffer[pdu_len],
        dlcb->optr - pdu_len, service_data->invoke_id);
    if (len <= 0) {
        error = true;
        goto GET_EVENT_ERROR;
    }
    pdu_len += len;
    apdu_len = len;
    for (i = 0; i < MAX_BACNET_OBJECT_TYPE; i++) {
        if (Get_Event_Info[i]) {
            for (j = 0; j < 0xffff; j++) {
                valid_event = Get_Event_Info[i] (pDev, j, &getevent_data);
                if (valid_event > 0) {
                    /* encode GetEvent_data only when type of object_id has max value */
                    if (object_id.type != MAX_BACNET_OBJECT_TYPE) {
                        if ((object_id.type ==
                                getevent_data.objectIdentifier.type) &&
                            (object_id.instance ==
                                getevent_data.objectIdentifier.instance)) {
                            /* found 'Last Received Object Identifier'
                               so should set type of object_id to max value */
                            object_id.type = MAX_BACNET_OBJECT_TYPE;
                        }
                        continue;
                    }

                    getevent_data.next = NULL;
                    len =
                        getevent_ack_encode_apdu_data(&dlcb->Handler_Transmit_Buffer[pdu_len], 
                                 dlcb->optr - pdu_len, &getevent_data);
                    if (len <= 0) {
                        error = true;
                        goto GET_EVENT_ERROR;
                    }
                    apdu_len += len;
                    if ((apdu_len >= service_data->max_resp - 2)  ||
                        (apdu_len >= MAX_LPDU_IP - 2)) {    // todo use datalink / dlcb lengths
                        /* Device must be able to fit minimum
                           one event information.
                           Length of one event informations needs
                           more than 50 octets. */
                        if ((service_data->max_resp < 128) ||
                            (MAX_LPDU_IP < 128)) {
                            len = BACNET_STATUS_ABORT;
                            error = true;
                            goto GET_EVENT_ERROR;
                        } 
                        else {
                            more_events = true;
                        }
                        break;
                    } 
                    else {
                        pdu_len += len;
                    }
                } else if (valid_event < 0) {
                    break;
                }
            }
        }
    }
    len =
        getevent_ack_encode_apdu_end(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                     dlcb->optr - pdu_len, more_events);
    if (len <= 0) {
        error = true;
        goto GET_EVENT_ERROR;
    }
    dbMessage(DBD_Intrinsic, DB_UNEXPECTED_ERROR, "Got a GetEventInformation request: Sending Ack!\n");
GET_EVENT_ERROR:
    if (error) {
        pdu_len =
            npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &rxDetails->bacnetPath.glAdr, NULL, // &my_address,
                            &npci_data);

        if (len == -2) {
            /* BACnet APDU too small to fit data, so proper response is Abort */
            len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                  service_data->invoke_id,
                                  ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
            dbMessage(DBD_Intrinsic, DB_UNEXPECTED_ERROR,
                "GetEventInformation: " "Reply too big to fit into APDU!\n");
        }
        else {
            len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                                     error_class, error_code);
            dbMessage(DBD_Intrinsic, DB_UNEXPECTED_ERROR, "GetEventInformation: Sending Error!\n");
        }
    }

GET_EVENT_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);
}
#endif // INTRINSIC_REPORTING_B