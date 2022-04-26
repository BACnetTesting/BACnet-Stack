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

//#include "datalink.h"
#include "bacnet/cov.h"

/** @file s_cov.c  Send a Change of Value (COV) update or a Subscribe COV request. */
#if 0
/** Encodes an Unconfirmed COV Notification.
 * @ingroup DSCOV
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param buffer_len [in] Number of bytes in the buffer
 * @param dest [in] Destination address
 * @param npci_data [in] Network Layer information
 * @param cov_data [in]  The COV update information to be encoded.
 * @return Size of the message sent (bytes), or a negative value on error.
 */
int ucov_notify_encode_pdu(
	DLCB *dlcb,
    DEVICE_OBJECT_DATA *pDev,
    BACNET_PATH * dest,
    BACNET_NPCI_DATA * npci_data,
    BACNET_COV_DATA * cov_data)
{
    int len = 0;
    int pdu_len = 0;

    /* unconfirmed is a broadcast */
    bacnet_path_set_broadcast_global(dest);

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(dlcb->Handler_Transmit_Buffer, &dest->glAdr, NULL, npci_data);

    /* encode the APDU portion of the packet */
    len = ucov_notify_encode_apdu(&buffer[pdu_len],
        dlcb->used - pdu_len, cov_data);
    if (len) {
        pdu_len += len;
    } else {
        pdu_len = 0;
    }

    return pdu_len;
}

/** Sends an Unconfirmed COV Notification.
 * @ingroup DSCOV
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param buffer_len [in] Number of bytes in the buffer
 * @param cov_data [in]  The COV update information to be encoded.
 * @return Size of the message sent (bytes), or a negative value on error.
 */
#if 0

int Send_UCOV_Notify(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *pDev,
    DLCB *dlcb,
    BACNET_COV_DATA * cov_data)
{
    int pdu_len = 0;
    BACNET_PATH dest;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    pdu_len = ucov_notify_encode_pdu( portParams, pDev, buffer, &dest, &npci_data, cov_data);
    dlcb->bufSize = pdu_len;
    bytes_sent = portParams->SendPdu(dlcb);

    return bytes_sent;
}
#endif

#if 0 // client side

/** Sends a COV Subscription request.
 * @ingroup DSCOV
 *
 * @param device_id [in] ID of the destination device
 * @param cov_data [in]  The COV subscription information to be encoded.
 * @return invoke id of outgoing message, or 0 if communication is disabled or
 *         no slot is available from the tsm for sending.
 */
uint8_t Send_COV_Subscribe(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *pDev,
    uint32_t device_id,
    BACNET_SUBSCRIBE_COV_DATA * cov_data)
{
    BACNET_ADDRESS dest;
    BACNET_ADDRESS my_address;
    uint16_t max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
    BACNET_NPCI_DATA npci_data;

    if (!dcc_communication_enabled(pDev)) {
        return 0;
    }
    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status) {
        invoke_id = tsm_next_free_invokeID(routerApplicationEntity);
    }
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        // datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest, NULL,
                            &npci_data);
        /* encode the APDU portion of the packet */
        len =
            cov_subscribe_encode_apdu(dlcb, invoke_id, cov_data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
           dlcb->optr = pdu_len ;
           // todo1, make sure other occurences set optr before submittal to tsm!
            tsm_set_confirmed_unsegmented_transaction( portParams, pDev, invoke_id, &dest,
                &npci_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);
            bytes_sent =
                portParams->SendPdu(portParams, &dest, &npdu_data, 


            if (bytes_sent <= 0) {
                dbMessage(DB_UNEXPECTED_ERROR, "Failed to Send SubscribeCOV Request (%s)!\n",
                        strerror(errno));
            }
        } else {
            tsm_free_invoke_id(pDev, invoke_id);
            invoke_id = 0;

            dbMessage(DB_UNEXPECTED_ERROR,
                    "Failed to Send SubscribeCOV Request "
                    "(exceeds destination maximum APDU)!\n");

        }
    }

    return invoke_id;
}
#endif // 0  client side

#endif // ( BACNET_SVC_COV_B == 1 )

