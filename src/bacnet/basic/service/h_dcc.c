/**************************************************************************
 *
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

#include "bacnet/bacdcode.h"
#include "bacnet/bacerror.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/reject.h"
#include "bacnet/dcc.h"
//#include "datalink.h"

/** @file h_dcc.c  Handles Device Communication Control request. */

static char My_Password[32] = "BACnet Testing 01";

/** Sets (non-volatile hold) the password to be used for DCC requests.
 * @param new_password [in] The new DCC password, of up to 31 characters.
 */
//void handler_dcc_password_set(
//    char *new_password)
//{
//    uint16_t i = 0;       /* loop counter */
//
//    if (new_password) {
//        for (i = 0; i < (sizeof(My_Password) - 1); i++) {
//            My_Password[i] = new_password[i];
//            My_Password[i + 1] = 0;
//            if (new_password[i] == 0) {
//                break;
//            }
//        }
//    }
//    else {
//        for (i = 0; i < sizeof(My_Password); i++) {
//            My_Password[i] = 0;
//        }
//    }
//}

/** Gets (non-volatile hold) the password to be used for DCC requests.
 * @return DCC password
 */
//const char *handler_dcc_password(void)
//{
//    return MyPassword;
//}

/** Handler for a Device Communication Control (DCC) request.
 * @ingroup DMDCC
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 *   - if not a known DCC state
 * - an Error if the DCC password is incorrect
 * - else tries to send a simple ACK for the DCC on success,
 *   and sets the DCC state requested.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ROUTE of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_device_communication_control(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    uint16_t timeDuration = 0;
    BACNET_COMMUNICATION_ENABLE_DISABLE state = COMMUNICATION_ENABLE;
    BACNET_CHARACTER_STRING password;
    int len ;
    int pdu_len ;
    BACNET_NPCI_DATA npci_data;
    // BACNET_GLOBAL_ADDRESS my_address;
    
    DLCB *dlcb = alloc_dlcb_response('a', &src->bacnetPath, pDev->datalink->max_lpdu);
    if (dlcb == NULL) return;

    /* encode the NPDU portion of the reply packet */
    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &src->bacnetPath.glAdr, NULL, // &my_address,
            &npci_data);
    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "DeviceCommunicationControl!\n");
    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
                true);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
            "DeviceCommunicationControl: "
            "Sending Abort - segmented message.\n");
        goto DCC_ABORT;
    }
    /* decode the service request only */
    len =
        dcc_decode_service_request(service_request, service_len, &timeDuration,
            &state, &password);
    if (len > 0)
        dbMessage(DBD_ALL, DB_ERROR,
            "DeviceCommunicationControl: " "timeout=%u state=%u password=%s\n",
            (unsigned)timeDuration, (unsigned)state,
            characterstring_value(&password));
    /* bad decoding or something we didn't understand - send an abort */
    if (len < 0) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id, ABORT_REASON_OTHER, true);
        dbMessage(DBD_ALL, DB_ERROR,
            "DeviceCommunicationControl: "
            "Sending Abort - could not decode.\n");
        goto DCC_ABORT;
    }
    if (state >= MAX_BACNET_COMMUNICATION_ENABLE_DISABLE) {
        len =
            reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id, REJECT_REASON_UNDEFINED_ENUMERATION);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
            "DeviceCommunicationControl: "
            "Sending Reject - undefined enumeration\n");
    }
    else {
//#if BAC_ROUTING
//        /* Check to see if the current Device supports this service. */
//        len =
//            Routed_Device_Service_Approval
//            (SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL, (int)state,
//                &dlcb->Handler_Transmit_Buffer[pdu_len], service_data->invoke_id);
//        if (len > 0) {
//            goto DCC_ABORT;
//        }
//#endif

        if (characterstring_ansi_same(&password, My_Password)) {
            len =
                encode_simple_ack(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL);
            dbMessage(DBD_ALL, DB_INFO,
                "DeviceCommunicationControl: " "Sending Simple Ack!\n");
            dcc_set_status_duration(pDev, state, timeDuration);
        }
        else {
            len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
                    ERROR_CLASS_SECURITY, ERROR_CODE_PASSWORD_FAILURE);
            dbMessage(DBD_ALL, DB_ERROR,
                "DeviceCommunicationControl: "
                "Sending Error - password failure.\n");
        }
    }

DCC_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);

}
