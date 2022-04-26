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

//#include <stddef.h>
// #include <stdint.h>
//#include <errno.h>
//#include <string.h>
//#include "config.h"
//#include "bacdef.h"
//#include "bacdcode.h"
//#include "address.h"
//#if ( BACNET_CLIENT == 1 )
//#include "tsm.h"
//#endif
//#include "npdu.h"
//#include "apdu.h"
//#include "device.h"
#include "datalink.h"
#include "dcc.h"
#include "ihave.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"
#include "debug.h"
#include "bacaddr.h"


/** @file s_ihave.c  Send an I-Have (property) message. */

/** Broadcast an I Have message.
 * @ingroup DMDOB
 *
 * @param device_id [in] My device ID.
 * @param object_type [in] The BACNET_OBJECT_TYPE that I Have.
 * @param object_instance [in] The Object ID that I Have.
 * @param object_name [in] The Name of the Object I Have.
 */
void Send_I_Have(
    BACNET_ROUTE *dest2,
    uint32_t device_id,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_I_HAVE_DATA data;
    BACNET_NPCI_DATA npci_data;

    DLCB *dlcb = alloc_dlcb_response('s', dest2);
    if (dlcb == NULL) return;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return;

    /*The sending BACnet-user shall broadcast or unicast the I-Have unconfirmed request. If the I-Have is broadcast, this
    broadcast may be on the local network only, a remote network only, or globally on all networks at the discretion of the
    application. If the I-Have is being transmitted in response to a previously received Who-Has, then the I-Have shall be
    transmitted in such a manner that the BACnet-user that sent the Who-Has will receive the resulting I-Have.*/

    /* Who-Has is a global broadcast */
    // Make a local copy so we don't corrupt the original
    BACNET_PATH localPath;
    bacnet_path_copy(&localPath, &dest2->bacnetPath);
    bacnet_path_set_broadcast_global(&localPath);

    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &localPath.glAdr, NULL,
            &npci_data);

    /* encode the APDU portion of the packet */
    data.device_id.type = OBJECT_DEVICE;
    data.device_id.instance = device_id;
    data.object_id.type = object_type;
    data.object_id.instance = object_instance;
    characterstring_copy(&data.object_name, object_name);
    len = ihave_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], &data);
    pdu_len += len;

    /* send the data */
    dlcb->optr = pdu_len;
    dest2->portParams->SendPdu(dlcb);
}
