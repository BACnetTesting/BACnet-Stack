/**************************************************************************
 *
 * Copyright (C) 2016 Steve Karg <skarg@users.sourceforge.net>
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
#include "bacnet/abort.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/dcc.h"
#include "bacnet/npdu.h"
#include "bacnet/apdu.h"
#include "bacnet/basic/object/device.h"
//#include "datalink.h"
#include "bacnet/iam.h"
/* some demo stuff needed */
#include "bacnet/basic/services.h"
// #include "client.h"


/** Encodes an Abort message
 * @param buffer The buffer to build the message for sending.
 * @param dest - Destination address to send the message
 * @param src - Source address from which the message originates
 * @param npci_data - buffer to hold NPDU data encoded
 * @param invoke_id - use to match up a reply
 * @param reason - #BACNET_ABORT_REASON enumeration
 * @param server - true or false
 *
 * @return Size of the message sent (bytes), or a negative value on error.
 */
int abort_encode_pdu(
    uint8_t * buffer,
    BACNET_GLOBAL_ADDRESS * dest,
    BACNET_GLOBAL_ADDRESS * src,
    BACNET_NPCI_DATA * npci_data,
    uint8_t invoke_id,
    BACNET_ABORT_REASON reason,
    bool server)
{
    int len = 0;
    int pdu_len = 0;

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&buffer[0], dest, src, npci_data);

    /* encode the APDU portion of the packet */
    len = abort_encode_apdu(&buffer[pdu_len], invoke_id, reason, server);
    pdu_len += len;

    return pdu_len;
}

/** Sends an Abort message
 * @param buffer The buffer to build the message for sending.
 * @param dest - Destination address to send the message
 * @param invoke_id - use to match up a reply
 * @param reason - #BACNET_ABORT_REASON enumeration
 * @param server - true or false
 *
 * @return Size of the message sent (bytes), or a negative value on error.
 */
#if 0
void Send_Abort_To_Network(
    BACNET_ROUTE *dest,
    DEVICE_OBJECT_DATA *sendingDev,
    uint8_t * buffer,
    uint8_t invoke_id,
    BACNET_ABORT_REASON reason,
    bool server)
{
    int pdu_len = 0;
    // BACNET_PATH src;
    int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;

    //datalink_get_my_address(&src);
    pdu_len = abort_encode_pdu(buffer, &dest->bacnetPath->adr, NULL, &npci_data,
        invoke_id, reason, server);

    //bytes_sent = datalink_send _pdu(portParams, dest, &npci_data, &buffer[0], pdu_len);

    dest->portParams->SendPdu(dlcb);

    // return bytes_sent;
}
#endif