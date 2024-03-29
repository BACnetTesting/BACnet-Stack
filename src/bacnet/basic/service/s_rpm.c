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
//#include "datalink.h"
#include "dcc.h"
#include "rpm.h"
/* some demo stuff needed */
// #include "sbuf.h"

/** @file s_rpm.c  Send Read Property Multiple request. */

/** Sends a Read Property Multiple request.
 * @ingroup DSRPM
 *
 * @param pdu [out] Buffer to build the outgoing message into
 * @param max_pdu [in] Length of the pdu buffer.
 * @param device_id [in] ID of the destination device
 * @param read_access_data [in] Ptr to structure with the linked list of
 *        properties to be read.
 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
 */
#ifdef RPM_A

uint8_t Send_Read_Property_Multiple_Request(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *sendingDev,
    uint32_t device_id, /* destination device */
    BACNET_READ_ACCESS_DATA * read_access_data)
{
    BACNET_ROUTE dest;
    uint8_t invoke_id = 0;
    bool status;
    int len = 0;
    int pdu_len = 0;
//    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled(sendingDev))
        return 0;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);

    /* is there a tsm available? */
    if (status)
        invoke_id = tsm_next_free_invokeID(sendingDev);

    if (invoke_id) {

        DLCB *dlcb = alloc_dlcb_application('r', &dest);
        if (dlcb == NULL)
        {
            panic();
            return 0;   // todo, make a real flag here.
        }

        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(dlcb->Handler_Transmit_Buffer, &dlcb->route.bacnetPath.glAdr, NULL,
                &npci_data);
        /* encode the APDU portion of the packet */
        len =
            rpm_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], max_apdu - pdu_len,
                invoke_id,
                read_access_data);
        if (len <= 0) {
            return 0;
        }
        pdu_len += len;

        /* is it small enough for the destination to receive?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((uint16_t)pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(sendingDev, invoke_id, dlcb);

            dlcb->route.portParams->SendPdu(dlcb);
        }
        else {
            tsm_free_invoke_id(sendingDev, invoke_id);
            invoke_id = 0;
        }
    }

    return invoke_id;
}

#endif
