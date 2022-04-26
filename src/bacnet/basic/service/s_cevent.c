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
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/dcc.h"
#include "bacnet/basic/binding/address.h"
/* some demo stuff needed */
#include "bacnet/basic/services.h"

#include "configProj.h"

#if (BACNET_USE_EVENT_HANDLING == 1)

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
	DLCB *dlcb,
    DEVICE_OBJECT_DATA *pDev,
    BACNET_EVENT_NOTIFICATION_DATA * data,
    BACNET_ROUTE * dest )
{
    int len = 0;
    BACNET_NPCI_DATA npci_data;
    unsigned max_apdu = dest->portParams->max_lpdu ;
    uint8_t invoke_id ;

    if (!dcc_communication_enabled(pDev)) {
        return 0;
    }

    invoke_id = tsm_next_free_invokeID_autoclear(pDev);

    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        // datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        dlcb->optr =
            npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest->bacnetPath.glAdr, NULL, // &my_address,
                            &npci_data);
        /* encode the APDU portion of the packet */
        len =
            cevent_notify_encode_apdu(&dlcb->Handler_Transmit_Buffer[dlcb->optr],
                                      invoke_id, data);
        dlcb->optr += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned)dlcb->optr < max_apdu) {
            // dest.portParams = portParams;
            tsm_set_confirmed_unsegmented_transaction(pDev, invoke_id, dlcb);

            dest->portParams->SendPdu(dest->portParams, dlcb);
        } else {
            tsm_free_invoke_id(pDev, invoke_id);
            dlcb_free ( dlcb ) ;
            invoke_id = 0;
            dbMessage(DBD_Intrinsic, DB_ERROR,
                    "Failed to Send ConfirmedEventNotification Request (exceeds destination maximum APDU)!\n");
        }
    }

    return invoke_id;
}

#endif // #if (BACNET_USE_EVENT_HANDLING == 1)

