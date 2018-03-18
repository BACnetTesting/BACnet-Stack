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

    Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

    This file contains changes made by BACnet Interoperability Testing
    Services, Inc. These changes are subject to the permissions,
    warranty terms and limitations above.
    For more information: info@bac-test.com
    For access to source code:  info@bac-test.com
            or      www.github.com/bacnettesting/bacnet-stack

####COPYRIGHTEND####
*********************************************************************/
#ifndef NPDU_H
#define NPDU_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacenum.h"

/** Hop count default is required by BTL to be maximum */
#ifndef HOP_COUNT_DEFAULT
#define HOP_COUNT_DEFAULT 255
#endif

/* an NPDU structure keeps the parameter stack to a minimum */
typedef struct BACNET_NPCI_DATA_t {
    uint8_t protocol_version;
    /* parts of the control octet: */
    bool data_expecting_reply;
    bool network_layer_message; /* false if APDU */
    BACNET_MESSAGE_PRIORITY priority;
    /* optional network message info */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type;   /* optional */
    uint16_t vendor_id; /* optional, if net message type is > 0x80 */
    uint8_t hop_count;
} BACNET_NPCI_DATA;

struct router_port_t;
/** The info[] string has no agreed-upon purpose, hence it is useless.
 * Keeping it short here. This size could be 0-255. */
#define ROUTER_PORT_INFO_LEN 2
/** Port Info structure used by Routers for their routing table. */
typedef struct router_port_t {
    uint16_t dnet;              /**< The DNET number that identifies this port. */
    uint8_t id;                 /**< Either 0 or some ill-defined, meaningless value. */
    uint8_t info[ROUTER_PORT_INFO_LEN];  /**< Info like 'modem dialing string' */
    uint8_t info_len;   /**< Length of info[]. */
    struct router_port_t *next;         /**< Point to next in linked list */
} BACNET_ROUTER_PORT;


    uint8_t npdu_encode_max_seg_max_apdu(
        int max_segs,
        int max_apdu);

    int npdu_encode_pdu(
        uint8_t * npdu,
        BACNET_ADDRESS * dest,
        BACNET_ADDRESS * src,
        BACNET_NPCI_DATA * npci_data);

    void npdu_setup_npci_data(
        BACNET_NPCI_DATA * npdu,
        bool data_expecting_reply,
        BACNET_MESSAGE_PRIORITY priority);

    void npdu_copy_data(
        BACNET_NPCI_DATA * dest,
        BACNET_NPCI_DATA * src);

    int npdu_decode(
        uint8_t * npdu,
        BACNET_ADDRESS * dest,
        BACNET_ADDRESS * src,
        BACNET_NPCI_DATA * npci_data);

#endif
