/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "apdu.h"
#include "npdu.h"
#include "reject.h"
#include "handlers.h"
#include "device.h"

/** @file noserv.c  Handles an unrecognized/unsupported service. */

/** Handler to be invoked when a Service request is received for which no
 *  handler has been defined.
 * @ingroup MISCHNDLR
 * This handler builds a Reject response packet, and sends it.
 *
 * @param service_request [in] The contents of the service request (unused).
 * @param service_len [in] The length of the service_request (unused).
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_unrecognized_service(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE *src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    int pdu_len = 0;
    // int bytes_sent = 0;
    BACNET_NPCI_DATA npci_data;
    // BACNET_GLOBAL_ADDRESS my_address;

    (void) service_request;
    (void) service_len;

	DLCB *dlcb = alloc_dlcb_response('q', src);
	if (dlcb == NULL)
	{
		return ;
	}

    /* encode the NPDU portion of the packet */
    // portParams->get_my_address(portParams, NULL);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], src, &my_address,
        &npci_data);
    /* encode the APDU portion of the packet */
    len =
        reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                           service_data->invoke_id, REJECT_REASON_UNRECOGNIZED_SERVICE);
    pdu_len += len;
    /* send the data */
        dlcb->optr = pdu_len;
    bytes_sent =
        datalink_send_pdu(src, &npci_data, dlcb );
    if (bytes_sent > 0) {
#if PRINT_ENABLED
        fprintf(stderr, "Sent Reject!\n");
#endif
    } else {
#if PRINT_ENABLED
        fprintf(stderr, "Failed to Send Reject (%s)!\n", strerror(errno));
#endif
    }
}
