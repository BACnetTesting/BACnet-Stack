/**************************************************************************
 *
 * Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
 *********************************************************************/
#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
#include "bacnet/npdu.h"
#include "bacnet/bits/util/multipleDatalink.h"

// typedef struct _PORT_SUPPORT PORT_SUPPORT;

/* specific defines for Ethernet */
#define MAX_HEADER_ETHERNET (6+6+2+1+1+1)
// #define MAX_MPDU (MAX_HEADER+MAX_PDU)

#define MAX_LPDU_ETHERNET   1500    // LPDU is the largest packet that the datalink can send in a single transaction

#define MAX_NPDU_ETHERNET   (MAX_LPDU_ETHERNET-MAX_HEADER_ETHERNET)

#define MAX_APDU_ETHERNET   (MAX_NPDU_ETHERNET-MAX_NPCI)

// bacdef.h MAX_NPCI is (21) bytes, see http://www.bacnetwiki.com/wiki/index.php?title=NPCI

#define MAX_ETHERNET_MAC_LEN    6

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus_disable */

    BACNET_STACK_EXPORT
    bool ethernet_valid(
        void);

    BACNET_STACK_EXPORT
    void ethernet_cleanup(
        void);

    BACNET_STACK_EXPORT
    bool ethernet_init(
        PORT_SUPPORT *portParams,
        const char *interface_name);

/* function to send a packet out the 802.2 socket */
/* returns number of bytes sent on success, negative on failure */
    BACNET_STACK_EXPORT
    void ethernet_send_npdu(
        const PORT_SUPPORT* datalink,
        const DLCB *dlcb);

/* receives an 802.2 framed packet */
/* returns the number of octets in the PDU, or zero on failure */
    BACNET_STACK_EXPORT
    uint16_t ethernet_receive(
        PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *mac,
        uint8_t *npdu,
        uint16_t max_pdu);

    //BACNET_STACK_EXPORT
    //void ethernet_set_my_address(
    //    BACNET_GLOBAL_ADDRESS * my_address);

    BACNET_STACK_EXPORT
        void ethernet_get_MAC_address(
            const PORT_SUPPORT *portParams,
            BACNET_MAC_ADDRESS * my_address);

    //BACNET_STACK_EXPORT
    //void ethernet_get_broadcast_address(
    //    BACNET_PATH * dest); /* destination address */

    /* some functions from Linux driver */
    BACNET_STACK_EXPORT
    void ethernet_debug_address(
        const char *info,
        BACNET_GLOBAL_ADDRESS * dest);

#ifdef __cplusplus_disable
}
#endif /* __cplusplus_disable */

void RouterListenEthernet (
    void *pArgs);

#endif
