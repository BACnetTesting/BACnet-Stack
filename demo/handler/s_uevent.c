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
*********************************************************************/
//#include <stddef.h>
//#include <stdint.h>
//#include <errno.h>
#include "event.h"
#include "datalink.h"
//#include "client.h"
//#include "device.h"
#include "config.h"

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1)

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
    DLCB *dlcb, // todo3 - why is this created here?
    BACNET_EVENT_NOTIFICATION_DATA * data,
    BACNET_ROUTE * dest)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;
    // BACNET_PATH my_address;

    // datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_setup_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu( dlcb->Handler_Transmit_Buffer, &dest->bacnetPath.glAdr, NULL, &npdu_data);
    /* encode the APDU portion of the packet */
    len = uevent_notify_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], data);
    pdu_len += len;
    /* send the data */
    //bytes_sent = portParams->SendPdu(portParams, dest, &npdu_data, pdu_len);
    dlcb->bufSize = pdu_len;
    dest->portParams->SendPdu(dest->portParams, &dest->bacnetPath.localMac, &npdu_data, dlcb);
}

#endif  // (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1)
