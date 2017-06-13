/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/

//#include <stddef.h>
//#include <stdint.h>
//#include <errno.h>
//#include <string.h>
//#include "config.h"
//#include "bacdef.h"
//#include "bacdcode.h"
//#include "address.h"
//#include "tsm.h"
//#include "npdu.h"
//#include "apdu.h"
//#include "device.h"
#include "datalink.h"
//#include "dcc.h"
#include "ptransfer.h"
///* some demo stuff needed */
//#include "handlers.h"
//#include "client.h"

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1)

/** @file s_upt.c  Send an Unconfirmed Private Transfer request. */

// todo2 - who uses this (seems to me to be added pretty late in the game 2017.03.09 )
void Send_UnconfirmedPrivateTransfer(
	DLCB *dlcb,
    BACNET_PATH * dest,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    int pdu_len = 0;
    // int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_PATH my_address;

    if (!dcc_communication_enabled())
        return;

#if 0
    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_setup_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], dest, &my_address,
        &npdu_data);

    /* encode the APDU portion of the packet */
    len =
        uptransfer_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
        private_data);
    pdu_len += len;
        datalink_send_pdu(dlcb, dest, &npdu_data, 
        pdu_len);

#endif
}

#endif // #if ( BACNET_SVC_PRIVATE_TRANSFER )

