/**
 * @file
 * @author Daniel Blazevic <daniel.blazevic@gmail.com>
 * @date 2013
 * @brief Send Write Property Multiple request
 *
 * @section LICENSE
 *
 * Copyright (C) 2013 Daniel Blazevic <daniel.blazevic@gmail.com>
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
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/npdu.h"
#include "bacnet/apdu.h"
#include "bacnet/basic/object/device.h"
///#include "datalink.h"
#include "bacnet/dcc.h"
#include "bacnet/wpm.h"
/* some demo stuff needed */
#include "bacnet/basic/services.h"
// #include "sbuf.h"
// #include "client.h"

#if 0

/** @file s_wpm.c  Send Write Property Multiple request. */

/** Sends a Write Property Multiple request.
 * @param pdu [out] Buffer to build the outgoing message into
 * @param max_pdu [in] Length of the pdu buffer.
 * @param device_id [in] ID of the destination device
 * @param write_access_data [in] Ptr to structure with the linked list of
 *        objects and properties to be written.
 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
 */
uint8_t Send_Write_Property_Multiple_Request_Data(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *destDev,
    uint32_t device_id,
    BACNET_WRITE_ACCESS_DATA * write_access_data)
{
    BACNET_PATH dest;
    //BACNET_PATH my_address;
    uint16_t max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len ;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled(destDev)) {
        return 0;
    }
    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status) {
        invoke_id = tsm_next_free_invokeID(destDev);
    }
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
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
        /* encode the APDU portion of the packet */
        len =
            wpm_encode_apdu(&Handler_Transmit_Buffer[pdu_len], max_apdu,
                invoke_id,
                write_access_data);
        if (len <= 0) {
            return 0;
        }
        pdu_len += len;

        /* is it small enough for the the destination to receive?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((uint16_t)pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, dlcb);

            dlcb->route.portParams->SendPdu(dlcb);
                &Handler_Transmit_Buffer[0], pdu_len);
            if (bytes_sent <= 0)
                fprintf(stderr,
                fprintf(stderr, "Failed to Send WriteProperty Request (%s)!\n",
            }
        }
        else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
        }
    }

    return invoke_id;
}

#endif // 0