#if 0

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
#include "dcc.h"
#include "rpm.h"
/* some demo stuff needed */
#include "bacnet/basic/services.h"
#include "readrange.h"
#include "client.h"

/** @file s_readrange.c  Send a ReadRange request. */

/* returns invoke id of 0 if device is not bound or no tsm available */
uint8_t Send_ReadRange_Request(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *sendingDev,
    uint32_t device_id, /* destination device */
    BACNET_READ_RANGE_DATA * read_access_data)
{
    BACNET_PATH dest;
    //BACNET_PATH my_address;
    uint16_t max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    if (!dcc_communication_enabled(sendingDev))
        return 0;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status)
        invoke_id = tsm_next_free_invokeID(sendingDev);

    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest.adr, NULL,
            &npci_data);

        /* encode the APDU portion of the packet */
        len =
            rr_encode_apdu(&Handler_Transmit_Buffer[pdu_len], invoke_id,
            read_access_data);
        if (len <= 0) {
            return 0;
        }

        pdu_len += len;
        /* is it small enough for the the destination to receive?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(portParams, sendingDev, invoke_id, &dest.localMac,
                &npci_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);

            //bytes_sent =
            //    datalink_send _pdu(&dest, &npci_data,
            //    &Handler_Transmit_Buffer[0], pdu_len);

            portParams->SendPdu(portParams, sendingDev, &dest.localMac, &npci_data, &Handler_Transmit_Buffer[0],
                pdu_len);

        } else {
            tsm_free_invoke_id(sendingDev, invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send ReadRange Request (exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}

#endif
