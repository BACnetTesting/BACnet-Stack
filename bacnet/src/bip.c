/****************************************************************************************

 Copyright (C) 2006 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
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

#include "bacint.h"
#include "bvlc.h"
#include "net.h"        /* custom per port */

//#if PRINT_ENABLED
//#include <stdio.h>      /* for standard i/o, like printing */
//#endif
#include "bacdef.h"
#include "btaDebug.h"

static int BIP_Socket = -1;
/* port to use - stored in network byte order */
static uint16_t BIP_Port = 0;   /* this will force initialization in demos */
/* IP Address - stored in network byte order */
static struct in_addr BIP_Address;
/* Broadcast Address - stored in network byte order */
static struct in_addr BIP_Broadcast_Address;

/** Setter for the BACnet/IP socket handle.
 *
 * @param sock_fd [in] Handle for the BACnet/IP socket.
 */
void bip_set_socket(
    int sock_fd)
{
    BIP_Socket = sock_fd;
}

/** Getter for the BACnet/IP socket handle.
 *
 * @return The handle to the BACnet/IP socket.
 */
int bip_socket(
    void)
{
    return BIP_Socket;
}

bool bip_valid(
    void)
{
    return (BIP_Socket != -1);
}

void bip_set_addr(
    uint32_t net_address)
{       /* in network byte order */
    BIP_Address.s_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_addr(
    void)
{
    return BIP_Address.s_addr;
}

void bip_set_broadcast_addr(
    uint32_t net_address)
{       /* in network byte order */
    BIP_Broadcast_Address.s_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_addr(
    void)
{
    return BIP_Broadcast_Address.s_addr;
}


void bip_set_port(
    uint16_t port)
{       /* in network byte order */
    BIP_Port = port;
}

/* returns network byte order */
uint16_t bip_get_port(
    void)
{
    return BIP_Port;
}

static int bip_decode_bip_address(
    BACNET_ADDRESS * bac_addr,
    struct in_addr *address,    /* in network format */
    uint16_t *port)             /* in network byte order */
{
    memcpy(&address->s_addr, &bac_addr->mac[0], 4);
    memcpy(port, &bac_addr->mac[4], 2);
    return 6;
}


/** Function to send a packet out the BACnet/IP socket (Annex J).
 * @ingroup DLBIP
 *
 * @param dest [in] Destination address (may encode an IP address and port #).
 * @param npci_data [in] The NPDU header (Network) information (not used).
 * @param pdu [in] Buffer of data to be sent - may be null (why?).
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */

int bip_send_pdu(
    BACNET_ADDRESS * dest,              /* destination address */
    BACNET_NPCI_DATA * npci_data,       /* network information */
    uint8_t * pdu,                      /* any data to be sent - may be null - todo 2 - why null? */
    unsigned pdu_len                    /* number of bytes of data */
    )
{
    struct sockaddr_in bip_dest;
    uint8_t mtu[MAX_MPDU_IP] = { 0 };
    int mtu_len = 0;
    int bytes_sent ;
    /* addr and port in network format */
    struct in_addr address;
    uint16_t port;

    (void) npci_data;
    /* assumes that the driver has already been initialized */
    if (BIP_Socket < 0) {
        return BIP_Socket;
    }

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;
    if ((dest->net == BACNET_BROADCAST_NETWORK) || (dest->mac_len == 0)) {
        /* broadcast */
        address.s_addr = BIP_Broadcast_Address.s_addr;
        port = BIP_Port;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    } else if ((dest->net > 0) && (dest->len == 0)) {
        /* network specific broadcast */
        if (dest->mac_len == 6) {
            bip_decode_bip_address(dest, &address, &port);
        } else {
            address.s_addr = BIP_Broadcast_Address.s_addr;
            port = BIP_Port;
        }
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    } else if (dest->mac_len == 6) {
        bip_decode_bip_address(dest, &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    } else {
        /* invalid address */
        return -1;
    }

    bip_dest.sin_addr.s_addr = address.s_addr;
    bip_dest.sin_port = port;
    memset(&(bip_dest.sin_zero), '\0', 8);      // todonext suspicious &
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t) (pdu_len + 4 /*inclusive */ ));
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += pdu_len;

#if 1
    // BTA diagnostic stuff only
    BACNET_MAC_ADDRESS dummySrcMAC = { 0 };
    BACNET_MAC_ADDRESS dummyDstMAC = { 0 };

    memcpy(dummyDstMAC.adr, &bip_dest.sin_addr, 4);
    memcpy(&dummyDstMAC.adr[4], &bip_dest.sin_port, 2);
    dummyDstMAC.len = 6;

    // and in this case, the dst address is ourselves

    uint32_t ourAddr = bip_get_addr();
    uint16_t ourPort = bip_get_port();

    memcpy(dummySrcMAC.adr, &ourAddr, 4);
    memcpy(&dummySrcMAC.adr[4], &ourPort, 2);
    dummySrcMAC.len = 6;

    // Set portID (first parameter to dummy 'port 1'
    SendBTApacketTx(1, &dummySrcMAC, &dummyDstMAC, mtu, mtu_len );
#endif

    /* Send the packet */
    bytes_sent =
        sendto(BIP_Socket, (char *) mtu, mtu_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    return bytes_sent;
}


/** Implementation of the receive() function for BACnet/IP; receives one
 * packet, verifies its BVLC header, and removes the BVLC header from
 * the PDU data before returning.
 *
 * @param src [out] Source of the packet - who should receive any response.
 * @param pdu [out] A buffer to hold the PDU portion of the received packet,
 *                  after the BVLC portion has been stripped off.
 * @param max_pdu [in] Size of the pdu[] buffer.
 * @param timeout [in] The number of milliseconds to wait for a packet.
 * @return The number of octets (remaining) in the PDU, or zero on failure.
 */
uint16_t bip_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * npdu,             /* returns the NPDU */
    uint16_t max_npdu,          /* amount of space available in the NPDU  */
    unsigned timeout)
{
    int received_bytes = 0;
    uint16_t npdu_len = 0;      /* return value */
    fd_set read_fds;
    int max ;
    struct timeval select_timeout;
    struct sockaddr_in sin = { 0 };
    socklen_t sin_len = sizeof(sin);
    uint16_t i = 0;
    BACNET_BVLC_FUNCTION function ;

    /* Make sure the socket is open */
    if (BIP_Socket < 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    if (timeout >= 1000) {
        select_timeout.tv_sec = timeout / 1000;
        select_timeout.tv_usec =
            1000 * (timeout - select_timeout.tv_sec * 1000);
    }
    else {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 1000 * timeout;
    }
    FD_ZERO(&read_fds);
    FD_SET(BIP_Socket, &read_fds);
    max = BIP_Socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {
        received_bytes =
            recvfrom(BIP_Socket, (char *) &npdu[0], max_npdu, 0,
            (struct sockaddr *) &sin, &sin_len);
    } 
    else {
        return 0;
    }
    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }

    /* no problem, just no bytes */
    if (received_bytes == 0) {
        return 0;
    }
    /* the signature of a BACnet/IP packet */
    if (npdu[0] != BVLL_TYPE_BACNET_IP) {
        return 0;
        }

    // ignore packets from ourself (broadcasts)
    if (sin.sin_addr.s_addr == bip_get_addr() && sin.sin_port == bip_get_port())
    {
        return 0;
    }

    if (bvlc_for_non_bbmd(&sin, npdu, received_bytes) > 0) {
        /* Handled, usually with a NACK. */
#if PRINT_ENABLED
        fprintf(stderr, "BIP: BVLC discarded!\n");
#endif
        return 0;
        }

#if 1
    // For BTA
    BACNET_MAC_ADDRESS dummySrcMAC = { 0 };
    BACNET_MAC_ADDRESS dummyDstMAC = { 0 };

    memcpy(dummySrcMAC.adr, &sin.sin_addr, 4);
    memcpy(&dummySrcMAC.adr[4], &sin.sin_port, 2);
    dummySrcMAC.len = 6;

    // and in this case, the dst address is ourselves

    uint32_t ourAddr = bip_get_addr();
    uint16_t ourPort = bip_get_port();

    memcpy(dummyDstMAC.adr, &ourAddr, 4);
    memcpy(&dummyDstMAC.adr[4], &ourPort, 2);
    dummyDstMAC.len = 6;

    // Set portID (first parameter to dummy 'port 1'
    SendBTApacketRx(1, &dummySrcMAC, &dummyDstMAC, npdu, received_bytes);
#endif

    function = bvlc_get_function_code();        /* aka, pdu[1] */
    if ((function == BVLC_ORIGINAL_UNICAST_NPDU) ||
        (function == BVLC_ORIGINAL_BROADCAST_NPDU)) {
        /* ignore messages from me */ // todo2 - this is redundant, see above
        if ((sin.sin_addr.s_addr == BIP_Address.s_addr) &&
            (sin.sin_port == BIP_Port)) {
            npdu_len = 0;
#if 0
            fprintf(stderr, "BIP: src is me. Discarded!\n");
#endif
        } else {
            /* data in src->mac[] is in network format */
            src->mac_len = 6;
            memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
            memcpy(&src->mac[4], &sin.sin_port, 2);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&npdu[2], &npdu_len);
            /* subtract off the BVLC header */
            npdu_len -= 4;
            if (npdu_len < max_npdu) {
#if 0
                fprintf(stderr, "BIP: NPDU[%hu]:", pdu_len);
#endif
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < npdu_len; i++) {
                    npdu[i] = npdu[4 + i];
                }
            }
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            else {
                    npdu_len = 0;
#if PRINT_ENABLED
                fprintf(stderr, "BIP: PDU too large. Discarded!.\n");
#endif
            }
        }
    } else if (function == BVLC_FORWARDED_NPDU) {
        
        // extract originating endpoint
        memcpy(&sin.sin_addr.s_addr, &npdu[4], 4);
        memcpy(&sin.sin_port, &npdu[8], 2);
        
        // ignore messages from self
        if ((sin.sin_addr.s_addr == BIP_Address.s_addr) &&
            (sin.sin_port == BIP_Port)) {
            /* ignore messages from me */
            npdu_len = 0;
        } 
        
        // valid originating endpoint?
        if(sin.sin_addr.s_addr == 0 || sin.sin_addr.s_addr == 0xffffffff)
        {
            dbTraffic(DBD_OOB_TRAFFIC,
                DB_ERROR,
                "BIP: Invalid originating address %u", sin.sin_addr.s_addr );
            return 0 ;
        }

        /* data in src->mac[] is in network format */
        src->mac_len = 6;
        memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
        memcpy(&src->mac[4], &sin.sin_port, 2);
        /* FIXME: check destination address */
        /* see if it is broadcast or for us */
        /* decode the length of the PDU - length is inclusive of BVLC */
        (void) decode_unsigned16(&npdu[2], &npdu_len);
        /* subtract off the BVLC header */
        npdu_len -= 10;
        if (npdu_len < max_npdu) {
            /* shift the buffer to return a valid PDU */
            for (i = 0; i < npdu_len; i++) {
                npdu[i] = npdu[4 + 6 + i];
            }
        }
        else {
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            npdu_len = 0;
        }
        
    }

    return npdu_len;
}


void bip_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    if (my_address) {
        my_address->mac_len = 6;
        memcpy(&my_address->mac[0], &BIP_Address.s_addr, 4);
        memcpy(&my_address->mac[4], &BIP_Port, 2);
        my_address->net = 0;    /* local only, no routing */
        my_address->len = 0;    /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            my_address->adr[i] = 0;
        }
    }
}


void bip_get_broadcast_address(
    BACNET_ADDRESS * dest       /* destination address */
    )
{
    int i = 0;  /* counter */

    dest->mac_len = 6;
    memcpy(&dest->mac[0], &BIP_Broadcast_Address.s_addr, 4);
    memcpy(&dest->mac[4], &BIP_Port, 2);
    dest->net = BACNET_BROADCAST_NETWORK;
    dest->len = 0;  /* no SLEN */
    for (i = 0; i < MAX_MAC_LEN; i++) {
        /* no SADR */
        dest->adr[i] = 0;
    }
}

