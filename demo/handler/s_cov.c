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
*********************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "config.h"

#if ( BACNET_SVC_COV_B == 1 )

#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "dcc.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "cov.h"
/* some demo stuff needed */
#include "handlers.h"
#include "client.h"
#include "bacaddr.h"

/** @file s_cov.c  Send a Change of Value (COV) update or a Subscribe COV request. */

/** Encodes an Unconfirmed COV Notification.
 * @ingroup DSCOV
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param buffer_len [in] Number of bytes in the buffer
 * @param dest [in] Destination address
 * @param npdu_data [in] Network Layer information
 * @param cov_data [in]  The COV update information to be encoded.
 * @return Size of the message sent (bytes), or a negative value on error.
 */
int ucov_notify_encode_pdu(
	DLCB *dlcb, // todo3 why is this passed to us?
    BACNET_PATH * dest,
    BACNET_NPDU_DATA * npdu_data,
    BACNET_COV_DATA * cov_data)
{
    int len = 0;
    int pdu_len = 0;
    // BACNET_PATH my_address;
    // datalink_get_my_address(&my_address);

    /* unconfirmed is a broadcast */
    // datalink_get_broadcast_address(dest);
    bacnet_path_set_broadcast_global(dest);
    /* encode the NPDU portion of the packet */
    npdu_setup_npdu_data(npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(dlcb->Handler_Transmit_Buffer, &dest->glAdr, NULL, npdu_data);

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
// todo 3 - why can we comment this out? are we not using it?
int Send_UCOV_Notify(
    PORT_SUPPORT *portParams,
    DLCB *dlcb,
    BACNET_COV_DATA * cov_data)
{
    int pdu_len = 0;
    BACNET_PATH dest;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;

    pdu_len = ucov_notify_encode_pdu(dlcb, &dest, &npdu_data, cov_data);
    dlcb->bufSize = pdu_len;
    bytes_sent = portParams->SendPdu(portParams, &dest.localMac, &npdu_data, dlcb);

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
    DLCB *dlcb,
    uint32_t device_id,
    BACNET_SUBSCRIBE_COV_DATA * cov_data)
{
    BACNET_PATH dest;
    // BACNET_PATH my_address;
    unsigned max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
#if PRINT_ENABLED
    int bytes_sent ;
#endif
    BACNET_NPDU_DATA npdu_data;

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
        // datalink_get_my_address(&my_address);
        npdu_setup_npdu_data(&npdu_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest, NULL, // &my_address,
            &npdu_data);
        /* encode the APDU portion of the packet */
        len =
            cov_subscribe_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            dlcb->used - pdu_len, invoke_id, cov_data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            dest->portParams = portParams;
            tsm_set_confirmed_unsegmented_transaction(dlcb, invoke_id, &dest,
                &npdu_data, (uint16_t) pdu_len);
#if PRINT_ENABLED
            bytes_sent =
#endif
                datalink_send_pdu(dlcb, &dest, &npdu_data, pdu_len);
#if PRINT_ENABLED
            if (bytes_sent <= 0)
                fprintf(stderr, "Failed to Send SubscribeCOV Request (%s)!\n",
                    strerror(errno));
#endif
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
#endif // 0  client side

#endif // ( BACNET_SVC_COV_B == 1 )

