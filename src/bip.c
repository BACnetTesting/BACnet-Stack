/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

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
 -------------------------------------------
####COPYRIGHTEND####*/

#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacdcode.h"
#include "bacint.h"
#include "bip.h"
#include "bvlc.h"
#include "net.h"        /* custom per port */
#if PRINT_ENABLED
#include <stdio.h>      /* for standard i/o, like printing */
#endif
#include "CEDebug.h"
#include "btaDebug.h"
#include "datalink.h"
#include "bacaddr.h"
/** @file bip.c  Configuration and Operations for BACnet/IP */

// static int BIP_Socket = -1;
/* port to use - stored in network byte order */
// static uint16_t BIP_Port = 0;   /* this will force initialization in demos */

/* returns network byte order */
uint32_t bip_get_addr(const DLINK_SUPPORT *portParams)
{
    // return BIP_Address.s_addr;
    return portParams->bipParams.nwoLocal_addr;
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
void bip_set_broadcast_addr(
    DLINK_SUPPORT *portParams,
    uint32_t net_address)
{
    // BIP_Broadcast_Address.s_addr = net_address;
    portParams->bipParams.nwoBroadcast_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_ipAddr(
    const DLINK_SUPPORT *portParams)
{
    // return BIP_Broadcast_Address.s_addr;
    return portParams->bipParams.nwoBroadcast_addr;
}
#endif


void bip_set_local_port(
    DLINK_SUPPORT *portParams,
    uint16_t nwoPort)
{
    portParams->bipParams.nwoPort = nwoPort;
}

/* returns network byte order */
uint16_t bip_get_local_port(
    const DLINK_SUPPORT *portParams )
{
    return portParams->bipParams.nwoPort;
}


    // copy data, everything in network order...
void bip_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *addr, const uint32_t nwoIpAddr, const uint16_t nwoPort)
{
    addr->bytes[0] = ((uint8_t *)&nwoIpAddr)[0];
    addr->bytes[1] = ((uint8_t *)&nwoIpAddr)[1];
    addr->bytes[2] = ((uint8_t *)&nwoIpAddr)[2];
    addr->bytes[3] = ((uint8_t *)&nwoIpAddr)[3];

    addr->bytes[4] = ((uint8_t *)&nwoPort)[0];
    addr->bytes[5] = ((uint8_t *)&nwoPort)[1];

    addr->len = 6 ;
}


static void bip_decode_bip_address(
    const uint8_t * bac_mac,
    struct in_addr *address,    /* in network format */
    uint16_t * port)
{       /* in network format */
        memcpy(&address->s_addr, &bac_mac[0], 4);
        memcpy(port, &bac_mac[4], 2);
}


/** Function to send a packet out the BACnet/IP socket (Annex J).
 * @ingroup DLBIP
 *
 * @param dest [in] Destination address (may encode an IP address and port #).
 * @param npdu_data [in] The NPDU header (Network) information (not used).
 * @param pdu [in] Buffer of data to be sent - may be null (why?).
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
void bip_send_npdu(
    const DLINK_SUPPORT *portParams,
    const BACNET_MAC_ADDRESS *destMAC,      /* destination address, if null, then broadcast */
    const BACNET_NPDU_DATA * npdu_data,     /* network information */
    const DLCB *dlcb )
{       /* number of bytes of data */
    struct sockaddr_in bip_dest;
    uint8_t mtu[MAX_MPDU_IP] = { 0 };
    int mtu_len = 0;
    int bytes_sent ;
    /* addr and port in host format */
    struct in_addr address;
    uint16_t port;

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;

    if (destMAC == NULL)
    {
        /* broadcast */
        address.s_addr = portParams->bipParams.nwoBroadcast_addr; // BIP_Broadcast_Address.s_addr;
        // port = BIP_Port;
        port = portParams->bipParams.nwoPort;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
    else
    {
        bip_decode_bip_address(destMAC->bytes, &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    }

    bip_dest.sin_addr.s_addr = address.s_addr;
    bip_dest.sin_port = port;
    memset(&(bip_dest.sin_zero), '\0', 8);
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t)(dlcb->bufSize + 4 /*inclusive */));
    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->bufSize );
    mtu_len += dlcb->bufSize;

    /* Send the packet */
    bytes_sent =
        sendto(portParams->bipParams.socket, (char *)mtu, mtu_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    if (bytes_sent <= 0)
    {
        SendBTApanicMessage("mxxx - Could not send PDU");
        return; 
    }

    // just for BTA, if we just did a broadcast, fill in the IP b'cast address for clarity (len == 0 indicates b'cast)
    {
        BACNET_MAC_ADDRESS phySrc;
        BACNET_MAC_ADDRESS phyDest;

        //memcpy(&phySrc.adr, &portParams->bipParams.local_addr, 4);
        //memcpy(&phySrc.adr[4], &portParams->bipParams.nwoPort, 2);
        //phySrc.len = 6;
        bip_ipAddr_port_to_bacnet_mac(&phySrc, portParams->bipParams.nwoLocal_addr, portParams->bipParams.nwoPort);

        if (destMAC)
        {
            //memcpy(phyDest.adr, destMAC, 6);
            //phyDest.len = 6;
            // bacnet_mac_copy(&phyDest, destMAC);
            phyDest = *destMAC;
            SendBTApacketTx(500 + portParams->port_id2, &phySrc, &phyDest, mtu, mtu_len);
        }
        else
        {
            // broadcast, make a dest for BTA
            BACNET_MAC_ADDRESS bcastMac;
            //memcpy(bcastMac.adr, &portParams->bipParams.local_addr, 4);
            //memcpy(&bcastMac.adr[4], &portParams->bipParams.nwoPort, 2);
            //bcastMac.len = 6;
            bip_ipAddr_port_to_bacnet_mac(&bcastMac, portParams->bipParams.nwoBroadcast_addr, portParams->bipParams.nwoPort);
            SendBTApacketTx( 600+portParams->port_id2, &phySrc, &bcastMac, mtu, mtu_len);
        }
    }
	
	// todo ekh - free dlcb
}


/** Implementation of the receive() function for BACnet/IP; receives one
 * packet, verifies its BVLC header, and removes the BVLC header from
 * the PDU data before returning.
 *
 * @param src [out] Source of the packet - who should receive any response.
 * @param pdu [out] A buffer to hold the PDU portion of the received packet,
 * 					after the BVLC portion has been stripped off.
 * @param max_pdu [in] Size of the pdu[] buffer.
 * @param timeout [in] The number of milliseconds to wait for a packet.
 * @return The number of octets (remaining) in the PDU, or zero on failure.
 */
uint16_t bip_receive(
    DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *mac,
    uint8_t *pdu,
    uint16_t max_pdu
    )
{
    int received_bytes = 0;
    uint16_t pdu_len = 0;       /* return value */
    fd_set read_fds;
    int max ;
    struct timeval select_timeout;
    struct sockaddr_in sin = { 0 };
    socklen_t sin_len = sizeof(sin);
    uint16_t i = 0;
    int function ;

    /* Make sure the socket is open */
    if (portParams->bipParams.socket < 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 10000;

    FD_ZERO(&read_fds);
    FD_SET(portParams->bipParams.socket, &read_fds);
    max = portParams->bipParams.socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0)
        received_bytes =
        recvfrom(portParams->bipParams.socket, (char *)&pdu[0], max_pdu, 0,
            (struct sockaddr *) &sin, &sin_len);
    else
        return 0;

    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }

    /* no problem, just no bytes */
    if (received_bytes == 0)
        return 0;

    // we do NOT want to process packets from ourselves
    if (sin.sin_addr.s_addr == portParams->bipParams.nwoLocal_addr && sin.sin_port == portParams->bipParams.nwoPort)
    {
        return 0;
    }

    BACNET_MAC_ADDRESS smac, dmac;

    bip_ipAddr_port_to_bacnet_mac(&smac, sin.sin_addr.s_addr, sin.sin_port);
    bip_ipAddr_port_to_bacnet_mac(&dmac, portParams->bipParams.nwoLocal_addr, portParams->bipParams.nwoPort);

    SendBTApacketRx(portParams->port_id2, &smac, &dmac, pdu, received_bytes);


    /* the signature of a BACnet/IP packet */
    if (pdu[0] != BVLL_TYPE_BACNET_IP)
        return 0;

    if (bvlc_for_non_bbmd(portParams, &sin, pdu, received_bytes) > 0) {
        /* Handled, usually with a NACK. */
#if PRINT_ENABLED
        fprintf(stderr, "BIP: BVLC discarded!\n");
#endif
        return 0;
    }

    function = bvlc_get_function_code();        /* aka, pdu[1] */
    if ((function == BVLC_ORIGINAL_UNICAST_NPDU) ||
        (function == BVLC_ORIGINAL_BROADCAST_NPDU)) {
        /* ignore messages from me */ // todo2 - this is redundant, see above
        if ((sin.sin_addr.s_addr == portParams->bipParams.nwoLocal_addr ) && // BIP_Address.s_addr) &&
            (sin.sin_port == portParams->bipParams.nwoPort )) {
            pdu_len = 0;
#if 0
            fprintf(stderr, "BIP: src is me. Discarded!\n");
#endif
        } else {
            /* data in src->mac[] is in network format */
            //portParams->src.mac_len = 6;
            //memcpy(&portParams->src.mac[0], &sin.sin_addr.s_addr, 4);
            //memcpy(&portParams->src.mac[4], &sin.sin_port, 2);
            bip_ipAddr_port_to_bacnet_mac( mac, sin.sin_addr.s_addr, sin.sin_port);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&pdu[2], &pdu_len);
            /* subtract off the BVLC header */
            pdu_len -= 4;
            if (pdu_len < max_pdu) {
#if 0
                fprintf(stderr, "BIP: NPDU[%hu]:", pdu_len);
#endif
                /* shift the buffer to return a valid PDU */
                memcpy ( pdu, &pdu[4], pdu_len ) ;
#if 0
                fprintf(stderr, "\n");
#endif
            }
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            else {
                pdu_len = 0;
#if PRINT_ENABLED
                fprintf(stderr, "BIP: PDU too large. Discarded!.\n");
#endif
            }
        }
    } else if (function == BVLC_FORWARDED_NPDU) {
        memcpy(&sin.sin_addr.s_addr, &pdu[4], 4);
        memcpy(&sin.sin_port, &pdu[8], 2);
        if ((sin.sin_addr.s_addr == portParams->bipParams.nwoLocal_addr ) && // BIP_Address.s_addr) &&
            (sin.sin_port == portParams->bipParams.nwoPort)) {
            /* ignore messages from me */
            pdu_len = 0;
        } else {
            /* data in src->mac[] is in network format */
            //portParams->src.mac_len = 6;
            //memcpy(&portParams->src.mac[0], &sin.sin_addr.s_addr, 4);
            //memcpy(&portParams->src.mac[4], &sin.sin_port, 2);
            bip_ipAddr_port_to_bacnet_mac(mac, sin.sin_addr.s_addr, sin.sin_port);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&pdu[2], &pdu_len);
            /* subtract off the BVLC header */
            pdu_len -= 10;
            if (pdu_len < max_pdu) {
                /* shift the buffer to return a valid PDU */
                memcpy(pdu, &pdu[4 + 6 + i], pdu_len);
            } else {
                /* ignore packets that are too large */
                /* clients should check my max-apdu first */
                pdu_len = 0;
            }
        }
    }

    return pdu_len;
}


void bip_get_MAC_address(
    const DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    bip_ipAddr_port_to_bacnet_mac(my_address, portParams->bipParams.nwoLocal_addr, portParams->bipParams.nwoPort);
}


void bip_get_broadcast_BACnetMacAddress(
    const DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * dest)
{
    /* destination address */
//     int i = 0;  /* counter */

        dest->len = 6;
        memcpy(&dest->bytes[0], &portParams->bipParams.nwoBroadcast_addr, 4) ; //BIP_Broadcast_Address.s_addr, 4);
        memcpy(&dest->bytes[4], &portParams->bipParams.nwoPort, 2);
}


