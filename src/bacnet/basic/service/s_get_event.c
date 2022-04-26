/**
 * @file
 * @author Daniel Blazevic <daniel.blazevic@gmail.com>
 * @date 2014
 * @brief Get Event Request
 *
 * @section LICENSE
 *
 * Copyright (C) 2014 Daniel Blazevic <daniel.blazevic@gmail.com>
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
#include "configProj.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "bacnet/basic/tsm/tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
//#include "datalink.h"
#include "bacnet/basic/services.h"
#include "client.h"
#include "getevent.h"

// todo 3 - get a lint tool and start linting sources....

#if (INTRINSIC_REPORTING_B == 1)

uint8_t Send_Get_Event_Information_Address(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE *dest,
    uint16_t max_apdu,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    int len = 0;
    int pdu_len = 0;
    uint8_t invoke_id = 0;
    BACNET_NPCI_DATA npci_data;

    // todo ekh 4 - other files have dcc_communication_enabled() here.

    DLCB *dlcb = alloc_dlcb_application('a', dest);
    if (dlcb == NULL)
    {
        return 0;
    }

    /* is there a tsm available? */
    invoke_id = tsm_next_free_invokeID(pDev);
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(
                dlcb->Handler_Transmit_Buffer, 
                &dlcb->bacnetPath.glAdr, 
                NULL,
                &npci_data);

        /* encode the APDU portion of the packet */
        // todo 2 - need to pass max_apdu downwards... 
        len =
            getevent_encode_apdu(
                &dlcb->Handler_Transmit_Buffer[pdu_len],
                invoke_id,
                lastReceivedObjectIdentifier);

        pdu_len += len;
        if ((uint16_t) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(pDev, invoke_id, dlcb);
            dlcb->optr = (uint16_t) pdu_len; // todo2 review
            dlcb->portParams->SendPdu(dlcb);

        }
        else {
            // dlcb_free(dlcb); // dlcb free done below // todo2 review
            tsm_free_invoke_id(pDev, invoke_id);
            invoke_id = 0;
            dbMessage(DBD_ALL, DB_ERROR,
                "Failed to Send Get Event Information Request "
                "(exceeds destination maximum APDU)!");
        }
    }
    else
    {
        dlcb_free(dlcb); // todo1 - check wp, wpm for these frees too! // todo2 review
    }

    return invoke_id;
}

uint8_t Send_Get_Event_Information(
	DEVICE_OBJECT_DATA* pDev,
	uint32_t device_id,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    BACNET_PATH dest;
    uint16_t max_apdu ;
    uint8_t invoke_id ;
    bool status ;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    if (status) {
        invoke_id = Send_Get_Event_Information_Address(
						pDev,
                        &dest, max_apdu, lastReceivedObjectIdentifier);
    } else {
        // todo 5 - surely we can do better than this
        invoke_id = 0 ;
        // and what about max_apdu (etc) ?
    }

    return invoke_id;
}

#endif
