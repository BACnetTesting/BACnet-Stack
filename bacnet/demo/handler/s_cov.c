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

#include "address.h"
#include "tsm.h"
#include "dcc.h"
#include "datalink.h"
#include "cov.h"

/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_cov.c  Send a Change of Value (COV) update or a Subscribe COV request. */

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
    uint8_t * buffer,
    unsigned buffer_len,
    BACNET_ADDRESS * dest,
    BACNET_NPCI_DATA * npci_data,
    BACNET_COV_DATA * cov_data)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_ADDRESS my_address;
    datalink_get_my_address(&my_address);

    /* unconfirmed is a broadcast */
    datalink_get_broadcast_address(dest);
    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&buffer[0], dest, &my_address, npci_data);

    /* encode the APDU portion of the packet */
    len = ucov_notify_encode_apdu(&buffer[pdu_len],
        buffer_len - pdu_len, cov_data);
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
int Send_UCOV_Notify(
    uint8_t * buffer,
    unsigned buffer_len,
    BACNET_COV_DATA * cov_data)
{
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    pdu_len = ucov_notify_encode_pdu(buffer, buffer_len, &dest, &npci_data, cov_data);
    bytes_sent = datalink_send_pdu(&dest, &npci_data, &buffer[0], pdu_len);

    return bytes_sent;
}

/** Sends a COV Subscription request.
 * @ingroup DSCOV
 *
 * @param device_id [in] ID of the destination device
 * @param cov_data [in]  The COV subscription information to be encoded.
 * @return invoke id of outgoing message, or 0 if communication is disabled or
 *         no slot is available from the tsm for sending.
 */
uint8_t Send_COV_Subscribe(
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
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    if (!dcc_communication_enabled())
        return 0;
    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status) {
        invoke_id = tsm_next_free_invokeID();
    }
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
            &npci_data);
        /* encode the APDU portion of the packet */
        len =
            cov_subscribe_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            sizeof(Handler_Transmit_Buffer)-pdu_len, invoke_id, cov_data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, &dest,
                &npci_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);
            bytes_sent =
                datalink_send_pdu(&dest, &npci_data,
                &Handler_Transmit_Buffer[0], pdu_len);
            if (bytes_sent <= 0) {
#if PRINT_ENABLED
                fprintf(stderr, "Failed to Send SubscribeCOV Request (%s)!\n",
                        strerror(errno));
#endif
            }
        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                    "Failed to Send SubscribeCOV Request "
                    "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}
