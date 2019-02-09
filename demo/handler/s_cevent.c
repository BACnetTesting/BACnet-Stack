/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
// #include "event.h"
#include "device.h"
#include "datalink.h"
#include "tsm.h"
#include "dcc.h"
#include "address.h"
/* some demo stuff needed */
#include "handlers.h"
#include "client.h"

#include "config.h"
#if ( BACNET_USE_EVENT_HANDLING == 1 )

/** @file s_cevent.c  Send a ConfirmedEventNotification Request. */

/** Sends an Confirmed Alarm/Event Notification.
 * @ingroup EVNOTFCN
 *
 * @param device_id [in] ID of the destination device
 * @param data [in] The information about the Event to be sent.
 * @return invoke id of outgoing message, or 0 if communication is disabled,
 *         or no tsm slot is available.
 */
uint8_t Send_CEvent_Notify(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *pDev,
    uint32_t device_id,
    BACNET_EVENT_NOTIFICATION_DATA * data)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_NPCI_DATA npci_data;
    BACNET_PATH dest;
    //BACNET_GLOBAL_ADDRESS my_address;
    unsigned max_apdu = 0;
    bool status = false;
    uint8_t invoke_id = 0;

    // BTC - todo 3
    // the Send_UEvent_Notify() function does not have this check. This is a BTC opportunity
    // Set up an event with a timer and in the meantime, set dcc off, and see if the event sneaks past!
    // 2018.01.30
    if (!dcc_communication_enabled(pDev)) {
        return 0;
    }

    // todo 2 - do this in the calling function (iff Send_CEvent_Notify (and uevent) not called from multiple locations that is..)
    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status) {
    invoke_id = tsm_next_free_invokeID_autoclear(pDev);
    } else {
        // todonext8 - there must be a better way...
        // printf("Target device is not bound, ignoring the Notify Event request\n\r");
    }

    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest->bacnetPath.glAdr, NULL, // &my_address,
                            &npci_data);
        /* encode the APDU portion of the packet */
        len =
            cevent_notify_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                                      invoke_id, data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            // dest.portParams = portParams;
            dlcb->optr = pdu_len;
            tsm_set_confirmed_unsegmented_transaction(dlcb, invoke_id, dest,
                &npci_data, (uint16_t) pdu_len);

            dest->portParams->SendPdu(dlcb);
        } else {
            tsm_free_invoke_id(pDev, invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                    "Failed to Send ConfirmedEventNotification Request "
                    "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}

#endif // #if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1)

