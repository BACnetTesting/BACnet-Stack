/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include "dcc.h"
#include "bacnet/basic/tsm/tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
//#include "datalink.h"
#include "arf.h"
/* some demo stuff needed */
#include "bacnet/basic/services.h"
#include "client.h"

/** @file s_arfs.c  Send part of an Atomic Read File Stream. */

#if 0

uint8_t Send_Atomic_Read_File_Stream(
    BACNET_ROUTE *dest,
    DEVICE_OBJECT_DATA *sendingDev,
    uint32_t device_id,
    uint32_t file_instance,
    int fileStartPosition,
    unsigned requestedOctetCount)
{
    BACNET_PATH dest;
    //BACNET_PATH my_address;
    BACNET_NPCI_DATA npci_data;
    uint16_t max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_ATOMIC_READ_FILE_DATA data;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return 0;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status)
        invoke_id = tsm_next_free_invokeID();
    if (invoke_id) {
        /* load the data for the encoding */
        data.object_type = OBJECT_FILE;
        data.object_instance = file_instance;
        data.access = FILE_STREAM_ACCESS;
        data.type.stream.fileStartPosition = fileStartPosition;
        data.type.stream.requestedOctetCount = requestedOctetCount;
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, NULL,
            &npci_data);
        len =
            arf_encode_apdu(&Handler_Transmit_Buffer[pdu_len], invoke_id,
            &data);
        pdu_len += len;
        /* will the APDU fit the target device?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            dlcb->optr = pdu_len ;
            tsm_set_confirmed_unsegmented_transaction(invoke_id, &dest,
                &npci_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);

            //bytes_sent =
            //    datalink_send_pdu(&dest, &npci_data,
            //    &Handler_Transmit_Buffer[0], pdu_len);

            dest->portParams->SendPdu(dest->portParams, sendingDev, &dest->bacnetPath->localMac, &npci_data, &Handler_Transmit_Buffer[0],
                pdu_len);

        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send AtomicReadFile Request "
                "(payload exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}

#endif
