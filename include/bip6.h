/**
* @file
* @author Steve Karg
* @date 2015
* @defgroup DLBIP6 BACnet/IPv6 DataLink Network Layer
* @ingroup DataLink
*
* Implementation of the Network Layer using BACnet/IPv6 as the transport, as
* described in Annex J.
* The functions described here fulfill the roles defined generically at the
* DataLink level by serving as the implementation of the function templates.



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
*/
#ifndef BIP6_H
#define BIP6_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"
#include "bvlc6.h"

/* specific defines for BACnet/IP over Ethernet */
#define BIP6_HEADER_MAX (1 + 1 + 2)
#define BIP6_MPDU_MAX (BIP6_HEADER_MAX+MAX_PDU)
/* for legacy demo applications */
#define MAX_MPDU BIP6_MPDU_MAX


    /* 6 datalink functions used by demo handlers and applications:
       init, send, receive, cleanup, unicast/broadcast address.
       Note: the addresses used here are VMAC addresses. */
    bool bip6_init(
        char *ifname);
    void bip6_cleanup(
        void);
    void bip6_get_broadcast_address(
        BACNET_ADDRESS * my_address);
    void bip6_get_my_address(
        BACNET_ADDRESS * my_address);
    int bip6_send_pdu(
        BACNET_ADDRESS * dest,
        BACNET_NPCI_DATA * npci_data,
        uint8_t * pdu,
        unsigned pdu_len);
    uint16_t bip6_receive(
        BACNET_ADDRESS * src,
        uint8_t * pdu,
        uint16_t max_pdu,
        unsigned timeout);

    /* functions that are custom per port */
    void bip6_set_interface(
        const char *ifname);

    bool bip6_set_addr(
        BACNET_IP6_ADDRESS *addr);
    bool bip6_get_addr(
        BACNET_IP6_ADDRESS *addr);

    void bip6_set_port(
        uint16_t port);
    uint16_t bip6_get_port(
        void);

    bool bip6_set_broadcast_addr(
        BACNET_IP6_ADDRESS *addr);
    /* returns network byte order */
    bool bip6_get_broadcast_addr(
        BACNET_IP6_ADDRESS *addr);

    int bip6_send_mpdu(
        BACNET_IP6_ADDRESS *addr,
        uint8_t * mtu,
        uint16_t mtu_len);


#endif
