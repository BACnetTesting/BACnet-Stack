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

//#include <stddef.h>
#include <stdint.h>
//#include <errno.h>
//#include <string.h>
#include "config.h"
//#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
//#include "address.h"
#include "tsm.h"
//#include "npdu.h"
//#include "apdu.h"
//#include "device.h"
#include "datalink.h"
#include "dcc.h"
#include "getevent.h"
//#include "bacenum.h"
///* some demo stuff needed */
//#include "handlers.h"
#include "txbuf.h"
#include "client.h"

#if (BACNET_USE_EVENT_HANDLING == 1)

// redefinition, see config.h #define PRINT_ENABLED 1

/** @file s_getevent.c  Send a GetEventInformation request. */

/** Send a GetEventInformation request to a remote network for a specific device, a range,
 * or any device.
 * @param target_address [in] BACnet address of target or broadcast
 */
uint8_t Send_GetEvent(
        BACNET_ADDRESS * target_address,
        BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    uint8_t invoke_id = 0;
    BACNET_NPCI_DATA npci_data;
    BACNET_ADDRESS my_address;

    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], target_address,
        &my_address, &npci_data);

    invoke_id = tsm_next_free_invokeID();
    if (invoke_id) {
        /* encode the APDU portion of the packet */
        len =
            getevent_encode_apdu(&Handler_Transmit_Buffer[pdu_len], invoke_id, lastReceivedObjectIdentifier);
        pdu_len += len;
        bytes_sent =
            datalink_send_pdu(target_address, &npci_data,
            &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
        if (bytes_sent <= 0)
            fprintf(stderr, "Failed to Send GetEventInformation Request (%s)!\n",
                strerror(errno));
#endif
    } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send GetEventInformation Request "
                "(exceeds destination maximum APDU)!\n");
#endif
    }
    return invoke_id;
}

/** Send a global GetEventInformation request.
 */
uint8_t Send_GetEvent_Global( void )
{
    BACNET_ADDRESS dest;

    if (!dcc_communication_enabled())
        return -1;

    datalink_get_broadcast_address(&dest);

    return Send_GetEvent(&dest, NULL);
}

#endif // #if (BACNET_USE_EVENT_HANDLING == 1)

