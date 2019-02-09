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

//#include <stddef.h>
#include <stdint.h>
//#include <errno.h>
//#include <string.h>
//#include "config.h"
#include "bacaddr.h"
#include "bacdef.h"
//#include "bacdcode.h"
//#include "address.h"
//#if ( BACNET_CLIENT == 1 )
//#include "tsm.h"
//#endif
//#include "dcc.h"
#include "npdu.h"
//#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "iam.h"
//#include "txbuf.h"
///* some demo stuff needed */
//#include "handlers.h"
#include "client.h"

/** @file s_iam.c  Send an I-Am message. */

/** Send a I-Am request to a remote network for a specific device.
 * @param target_address [in] BACnet address of target router
 * @param device_id [in] Device Instance 0 - 4194303
 * @param max_apdu [in] Max APDU 0-65535
 * @param segmentation [in] #BACNET_SEGMENTATION enumeration
 * @param vendor_id [in] BACnet vendor ID 0-65535
 */
void Send_I_Am_To_Network(
    BACNET_ROUTE * dest,
    uint32_t device_id,
    uint16_t max_apdu,
    int segmentation,
    uint16_t vendor_id)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_NPCI_DATA npci_data;

    DLCB *dlcb = alloc_dlcb_application('r', dest);
    if (dlcb == NULL)
    {
        return;
    }

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest->bacnetPath.glAdr,
            NULL, &npci_data);
    /* encode the APDU portion of the packet */
    /* encode the APDU portion of the packet */
    len =
        iam_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            device_id, max_apdu, segmentation, vendor_id);

    pdu_len += len;
    dlcb->optr = pdu_len;
    dest->portParams->SendPdu(dlcb);
}

/** Encode an I Am message to be broadcast.
 * @param buffer [in,out] The buffer to use for building the message.
 * @param dest [out] The destination address information.
 * @param npci_data [out] The NPDU structure describing the message.
 * @return The length of the message in buffer[].
 */
int iam_encode_pdu(
    DLCB *dlcb,
    BACNET_GLOBAL_ADDRESS * dest,
    BACNET_NPCI_DATA * npci_data)
{
    int len;
    int pdu_len;

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], dest, NULL, npci_data);

    /* encode the APDU portion of the packet */
    len =
        iam_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], Device_Object_Instance_Number(),
            MAX_LPDU_IP, SEGMENTATION_NONE, Device_Vendor_Identifier());
    pdu_len += len;

    return pdu_len;
}


/** Broadcast an I Am message.
 * @ingroup DMDDB
 *
 * @param buffer [in] The buffer to use for building and sending the message.
 */

void Send_I_Am_Broadcast_Datalink(
    PORT_SUPPORT *datalink
)
{
    BACNET_ROUTE dest;
    BACNET_NPCI_DATA npci_data;

#if 0
    /* note: there is discussion in the BACnet committee
       that we should allow a device to reply with I-Am
       so that dynamic binding always work.  If the DCC
       initiator loses the MAC address and routing info,
       they can never re-enable DCC because they can't
       find the device with WhoIs/I-Am */
       /* are we are forbidden to send? */
    if (!dcc_communication_enabled())
        return 0;
#endif

    dest.portParams = datalink;
    bacnet_path_set_broadcast_global(&dest.bacnetPath);

    DLCB *dlcb = alloc_dlcb_application('u', &dest);
    if (dlcb == NULL)
    {
        return;
    }

    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);

    /* encode the data */
    dlcb->optr = iam_encode_pdu(dlcb, &dest.bacnetPath.glAdr, &npci_data);

    /* send data */
    datalink->SendPdu(dlcb);
}


/** Encode an I Am message to be unicast directly back to the src.
 *
 * @param buffer [in,out] The buffer to use for building the message.
 * @param src [in] The source address information. If the src address is not
 *                 given, the dest address will be a broadcast address.
 * @param dest [out] The destination address information.
 * @param npci_data [out] The NPDU structure describing the message.
 * @return The length of the message in buffer[].
 */

uint16_t iam_unicast_encode_pdu(
    DLCB * dlcb,
    BACNET_GLOBAL_ADDRESS * dest,
    BACNET_NPCI_DATA * npci_data)
{
    uint16_t npdu_len;
    uint16_t apdu_len;

    /* dest->net = 0; - no, must direct back to src->net to meet BTL tests */ // todo 2

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len = npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], dest, NULL, npci_data);

    /* encode the APDU portion of the packet */
    apdu_len =
        iam_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len], Device_Object_Instance_Number(),
            MAX_LPDU_IP, SEGMENTATION_NONE, Device_Vendor_Identifier());

    return npdu_len + apdu_len;
}


/** Send an I-Am message by unicasting directly back to the src.
 * @ingroup DMDDB
 * @note As of Addendum 135-2008q-1, unicast responses are allowed;
 *  in modern firewalled corporate networks, this may be the
 *  only type of response that will reach the source on
 *  another subnet (without using the BBMD).  <br>
 *  However, some BACnet routers may not correctly handle this message.
 *
 * @param buffer [in] The buffer to use for building and sending the message.
 * @param src [in] The source address information from service handler.
 */
void Send_I_Am_Unicast(
    BACNET_ROUTE *dest)
{
    int pdu_len;
    BACNET_NPCI_DATA npci_data;

#if 0
    /* note: there is discussion in the BACnet committee
       that we should allow a device to reply with I-Am
       so that dynamic binding always work.  If the DCC
       initiator loses the MAC address and routing info,
       they can never re-enable DCC because they can't
       find the device with WhoIs/I-Am */
    /* are we are forbidden to send? */
    if (!dcc_communication_enabled())
        return 0;
#endif

    DLCB *dlcb = alloc_dlcb_response('r', dest );     // todo2 where is the rxDetail?
    if (dlcb == NULL) return;

    /* encode the data */ // src is null, dest is the original source
    pdu_len = iam_unicast_encode_pdu(dlcb, &dest->bacnetPath.glAdr, &npci_data);

    /* send data */
    dlcb->optr = pdu_len;
    dest->portParams->SendPdu(dlcb);

}


// For routing, this is handled elsewhere. This function is only necessary for broadcasting
// on multiple datalink model only

extern PORT_SUPPORT *datalinkSupportHead;

void Send_I_Am_Broadcast(
    void
)
{
    for (PORT_SUPPORT *dl = datalinkSupportHead; dl != NULL; dl = (PORT_SUPPORT *)dl->llist.next) {
        Send_I_Am_Broadcast_Datalink(dl);
    }
}
