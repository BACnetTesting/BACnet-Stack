/**
 * @file
 * @author Daniel Blazevic <daniel.blazevic@gmail.com>
 * @date 2014
 * @brief Get Alarm Summary Request
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
****************************************************************************************
 *
 * @section DESCRIPTION
 *
 * The Get Alarm Summary Request is used by a client BACnet-user to
 * obtain a summary of "active alarms." The term "active alarm" refers to
 * BACnet standard objects that have an Event_State property whose value is
 * not equal to NORMAL and a Notify_Type property whose value is ALARM.
 */
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "handlers.h"
#include "client.h"
#include "get_alarm_sum.h"

uint8_t Send_Get_Alarm_Summary_Address(
    BACNET_ADDRESS *dest,
    uint16_t max_apdu)
{
    int len = 0;
    int pdu_len = 0;
    uint8_t invoke_id = 0;
    BACNET_NPCI_DATA npci_data;
    BACNET_ADDRESS my_address;
#if PRINT_ENABLED
    int bytes_sent = 0;
#endif

    /* is there a tsm available? */
    invoke_id = tsm_next_free_invokeID();
    if (invoke_id) {
        datalink_get_my_address(&my_address);
        /* encode the NPDU portion of the packet */
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);

        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], dest,
                            &my_address, &npci_data);
        /* encode the APDU portion of the packet */
        len = get_alarm_summary_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                                            invoke_id);

        pdu_len += len;
        if ((uint16_t) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, dest,
                &npci_data, &Handler_Transmit_Buffer[0],
                (uint16_t) pdu_len);
#if PRINT_ENABLED
            bytes_sent =
#endif
                datalink_send_pdu(dest, &npci_data,
                &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
            if (bytes_sent <= 0)
                fprintf(stderr,
                "Failed to Send Get Alarm Summary Request (%s)!\n",
                    strerror(errno));
#endif
        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send Get Alarm Summary Request "
                "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}

uint8_t Send_Get_Alarm_Summary(
    uint32_t device_id)
{
    BACNET_ADDRESS dest;
    uint16_t max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    if (status) {
        invoke_id = Send_Get_Alarm_Summary_Address(
                        &dest, max_apdu);
    } else {
        invoke_id = 0 ;
        // todo 5 - surely we can do better than this?
    }

    return invoke_id;
}
