/**************************************************************************
 *
 * Copyright (C) 2008 John Minack
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

#include "bacnet/event.h"
#include "bacnet/basic/object/device.h"

#if (BACNET_USE_EVENT_HANDLING == 1)

/** @file s_uevent.c  Send an Unconfirmed Event Notification. */

/** Sends an Unconfirmed Alarm/Alert/Event Notification.
 * @ingroup EVNOTFCN
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param data [in] The information about the Event to be sent.
 * @param dest [in] The destination address information (may be a broadcast).
 * @return Size of the message sent (bytes), or a negative value on error.
 */
void Send_UEvent_Notify(
    BACNET_ROUTE *dest,
    BACNET_EVENT_NOTIFICATION_DATA * data
    )
{
    int len ;
    int pdu_len ;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;


    DLCB *dlcb = alloc_dlcb_application('a', dest);
    if (dlcb == NULL)
    {
        return ;
    }

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(
                dlcb->Handler_Transmit_Buffer, 
                &dlcb->route.bacnetPath.glAdr, 
                NULL,
                &npci_data);
            
        /* encode the APDU portion of the packet */
    len =
        uevent_notify_encode_apdu(
            &dlcb->Handler_Transmit_Buffer[pdu_len],
            data);
            
    pdu_len += len;
    /* send the data */
    //bytes_sent = portParams->SendPdu(portParams, dest, &npci_data, pdu_len);
    dlcb->optr = pdu_len;
    dlcb->route.portParams->SendPdu(dlcb);
}

#endif  // (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1)

