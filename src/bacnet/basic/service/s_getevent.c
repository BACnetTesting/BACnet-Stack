/**************************************************************************
*
* Copyright (C) 2015 bowe
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

#include <stdint.h>
#include "configProj.h"

#if (BACNET_USE_EVENT_HANDLING == 1)

#include "bacdef.h"
#include "bacdcode.h"
#include "dcc.h"
#include "getevent.h"
#include "bacaddr.h"


// redefinition, see config.h #define PRINT_ENABLED 1

/** @file s_getevent.c  Send a GetEventInformation request. */

/** Send a GetEventInformation request to a remote network for a specific device, a range,
 * or any device.
 * @param target_address [in] BACnet address of target or broadcast
 */
uint8_t Send_GetEvent(
    // todo ekh 1 - two devs? resolve
    PORT_SUPPORT* datalink,
    DEVICE_OBJECT_DATA *sendingDev,
    // DEVICE_OBJECT_DATA *pDev,
    BACNET_PATH *dest,
    // DEVICE_OBJECT_DATA *sendingDev,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    //BACNET_PATH my_address;
    uint8_t invoke_id ;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    // other s_*.c files had dcc_communication_enabled() check here.

    DLCB *dlcb = alloc_dlcb_application('a', dest, datalink->max_lpdu );
    if (dlcb == NULL)
    {
        return 0;
    }

    /* is there a tsm available? */
    invoke_id = tsm_next_free_invokeID(sendingDev);
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);   // todo ekh 2 - see s_rp.c here, that file has true

        pdu_len =
            npdu_encode_pdu(
                dlcb->Handler_Transmit_Buffer, 
                &dlcb->bacnetPath.glAdr, 
                NULL,
                &npci_data);
        /* encode the APDU portion of the packet */
        len =
            getevent_encode_apdu(
                &dlcb->Handler_Transmit_Buffer[pdu_len],
                invoke_id,
                lastReceivedObjectIdentifier);

        pdu_len += len;

            // todo ekh 1 - this is quite different to s_rp.c - review
            dlcb->optr = (uint16_t) pdu_len; // todo2 review
            datalink->SendPdu(datalink, dlcb);

    }
    else {
        tsm_free_invoke_id(sending, invoke_id);
        invoke_id = 0;

    }
    return invoke_id;
}


#if 0
/** Send a global GetEventInformation request.
 */
uint8_t Send_GetEvent_Global(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *pDev )
{
    BACNET_PATH dest;

    if (!dcc_communication_enabled(pDev))
        return -1;

    bacnet_path_set_broadcast_global(&dest);
    //datalink_get_broadcast_address(&dest);

    return Send_GetEvent(pDev, &dest, NULL);
}
#endif // 0

#endif // #if (BACNET_USE_EVENT_HANDLING == 1)

