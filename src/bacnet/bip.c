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

#include <stdio.h>
#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include "bacint.h"
//#include "datalink.h"
#include "eLib/util/btaDebug.h"
#include "bacnet/bacaddr.h"
#include "bacnet/bits/util/bitsIpUtil.h"
#include "eLib/util/eLibDebug.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "eLib/util/emm.h"
#include "bacnet/datalink/bvlc.h"
#include "bacnet/datalink/bip.h"
#include "bacnet/basic/bbmd/h_bbmd.h"
#include "osNet.h"

void bip_set_addr(
	PORT_SUPPORT *portParams,
    uint32_t net_address)
{       /* in network byte order */
    //BIP_Address.s_addr = net_address;
    portParams->datalink.bipParams.nwoLocal_addr = net_address ;
}


/* returns network byte order */
uint32_t bip_get_addr(const PORT_SUPPORT *portParams)
{
    return portParams->datalink.bipParams.nwoLocal_addr;
}

// #if defined(BBMD_ENABLED) && BBMD_ENABLED
void bip_set_broadcast_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
    portParams->datalink.bipParams.nwoBroadcast_addr = net_address;
}


void bip_set_netmask_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
    portParams->datalink.bipParams.nwoNetmask_addr = net_address;
}


void bip_set_subnet_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
    portParams->datalink.bipParams.nwoSubnet_addr = net_address;
}


/* returns network byte order */
uint32_t bip_get_broadcast_ipAddr(
    const PORT_SUPPORT *portParams)
{
    return portParams->datalink.bipParams.nwoBroadcast_addr;
}
// #endif


bool bip_isOnSubnet(const BIP_PARAMS *datalinkParams, uint32_t addr )
{
    return (datalinkParams->nwoSubnet_addr == (addr & datalinkParams->nwoNetmask_addr)) ;
}

    
//void bip_set_local_port(
//    uint16_t port)
//{
//    /* in network byte order */
//    BIP_local_Port = port;
//}

/* returns network byte order */
uint16_t bip_get_local_port(
    const PORT_SUPPORT *portParams )
{
    return portParams->datalink.bipParams.nwoPort;
}




static void bip_decode_bip_address(
    const uint8_t * bac_mac,
    struct in_addr *address,    /* in network format */
    uint16_t *port)             /* in network byte order */
{
        memcpy(&address->s_addr, &bac_mac[0], 4);
        memcpy(port, &bac_mac[4], 2);
}


void bip_send_npdu(
    PORT_SUPPORT* datalink,
    const DLCB *dlcb )
{
    struct sockaddr_in bip_dest;
    uint8_t *mtu ;
    int mtu_len = 0;
    int bytes_sent ;
    /* addr and port in network format */
    struct in_addr address;
    uint16_t port;

    mtu = (uint8_t*)emm_dmalloc('g', datalink->max_lpdu);
    if (!mtu)
    {
        dlcb_free(dlcb);
        panic();
        return;
    }

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;
    if ( dlcb->bacnetPath.localMac.len == 0) {
        /* broadcast */
        address.s_addr = datalink->datalink.bipParams.nwoBroadcast_addr; // BIP_Broadcast_Address.s_addr;
        // port = BIP_Port;
        port = datalink->datalink.bipParams.nwoPort;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
    else
    {
        bip_decode_bip_address(dlcb->bacnetPath.localMac.bytes, &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    }

    bip_dest.sin_addr.s_addr = address.s_addr;
    bip_dest.sin_port = port;
    memset(&(bip_dest.sin_zero), '\0', 8);      // todo 1 suspicious &
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t)(dlcb->optr + 4 /*inclusive */));
    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->optr);
    mtu_len += dlcb->optr;

#if 0
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
        sendto(datalink->datalink.bipParams.socket, (char *)mtu, mtu_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    if (bytes_sent <= 0)
    {
        dlcb_free(dlcb);
        SendBTApanicMessage("m0025 - Could not send PDU");
        emm_free(mtu);
        return; 
    }

    // just for BTA, if we just did a broadcast, fill in the IP b'cast address for clarity (len == 0 indicates b'cast)
    {
        BACNET_MAC_ADDRESS phySrc;
        BACNET_MAC_ADDRESS phyDest;

        //memcpy(&phySrc.adr, &portParams->bipParams.local_addr, 4);
        //memcpy(&phySrc.adr[4], &portParams->bipParams.nwoPort, 2);
        //phySrc.len = 6;
        bits_ipAddr_port_to_bacnet_mac(&phySrc, datalink->datalink.bipParams.nwoLocal_addr, datalink->datalink.bipParams.nwoPort);

        if (dlcb->bacnetPath.localMac.len != 0)
        {
            bacnet_mac_copy(&phyDest, &dlcb->bacnetPath.localMac );
            SendBTApacketTx(datalink->datalinkId, &phySrc, &phyDest, mtu, mtu_len);
        }
        else
        {
            // broadcast, make a dest for BTA
            BACNET_MAC_ADDRESS bcastMac;
            //memcpy(bcastMac.adr, &portParams->bipParams.local_addr, 4);
            //memcpy(&bcastMac.adr[4], &portParams->bipParams.nwoPort, 2);
            //bcastMac.len = 6;
            bits_ipAddr_port_to_bacnet_mac(&bcastMac, datalink->datalink.bipParams.nwoBroadcast_addr, datalink->datalink.bipParams.nwoPort);
            SendBTApacketTx( datalink->datalinkId, &phySrc, &bcastMac, mtu, mtu_len);
        }
    }
	
    emm_free(mtu);
    dlcb_free(dlcb);
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
    PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *mac,
    uint8_t *npdu,
    uint16_t max_pdu
    )
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
    if (portParams->datalink.bipParams.socket < 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 10000;

    FD_ZERO(&read_fds);
    FD_SET(portParams->datalink.bipParams.socket, &read_fds);
    max = (int) portParams->datalink.bipParams.socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {
        received_bytes =
        recvfrom(portParams->datalink.bipParams.socket, (char *) npdu, max_pdu, 0,
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

#if defined ( _MSC_VER  )
    CheckLocalAddressKnown(portParams, &sin);
#endif

    // we do NOT want to process packets from ourselves
    if (sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr && sin.sin_port == portParams->datalink.bipParams.nwoPort)
    {
        return 0;
    }
    
    // and we do not want to receive parameters from another subnet (for now)
    if(!bip_isOnSubnet(&portParams->datalink.bipParams, sin.sin_addr.s_addr))
    {
        char tbuf1[50];
        char tbuf2[50];
        
        dbMessage(DBD_OOB_TRAFFIC, DB_UNUSUAL_TRAFFIC, "Ignoring packet, from addr %s, but received on %s", IPAddr_ToString(tbuf1, &sin.sin_addr), IPAddr_ToString(tbuf2, (const struct in_addr *) &portParams->datalink.bipParams.nwoSubnet_addr));
        SendBTAhexdump("packet", npdu, received_bytes);
        
        return 0;
    }


    BACNET_MAC_ADDRESS smac, dmac;

    bits_ipAddr_port_to_bacnet_mac(&smac, sin.sin_addr.s_addr, sin.sin_port);
    bits_ipAddr_port_to_bacnet_mac(&dmac, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);

    SendBTApacketRx(portParams->datalinkId, &smac, &dmac, npdu, received_bytes);

    
    if (bvlc_for_non_bbmd2(portParams, &sin, npdu, received_bytes) > 0) {
        /* Handled, usually with a NACK. */
        dbMessage(DBD_OOB_TRAFFIC,
            DB_UNUSUAL_TRAFFIC,
            "BIP: BVLC discarded");
        return 0;
    }

    function = bvlc_get_function_code();        /* aka, pdu[1] */
    if ((function == BVLC_ORIGINAL_UNICAST_NPDU) ||
        (function == BVLC_ORIGINAL_BROADCAST_NPDU)) {
        /* ignore messages from me */
        if ((sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr ) &&
            (sin.sin_port == portParams->datalink.bipParams.nwoPort )) {
            npdu_len = 0;
#if 0
            fprintf(stderr, "BIP: src is me. Discarded!\n");
#endif
        } else {
            bits_ipAddr_port_to_bacnet_mac( mac, sin.sin_addr.s_addr, sin.sin_port);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&npdu[2], &npdu_len);
            /* subtract off the BVLC header */
            npdu_len -= 4;
            if (npdu_len < max_pdu) {
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
        if ((sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr ) && // BIP_Address.s_addr) &&
            (sin.sin_port == portParams->datalink.bipParams.nwoPort)) {
            /* ignore messages from me */
                return 0 ;
        } 
        
        // valid originating endpoint?
        if(sin.sin_addr.s_addr == 0 || sin.sin_addr.s_addr == 0xffffffff)
        {
            dbMessage(DBD_OOB_TRAFFIC,
                DB_ERROR,
                "BIP: Invalid originating address %u", sin.sin_addr.s_addr );
            return 0 ;
        }
        
        /* data in src->mac[] is in network format */
        bits_ipAddr_port_to_bacnet_mac(mac, sin.sin_addr.s_addr, sin.sin_port);
        /* FIXME: check destination address */
        /* see if it is broadcast or for us */
        /* decode the length of the PDU - length is inclusive of BVLC */
        (void) decode_unsigned16(&npdu[2], &npdu_len);
        /* subtract off the BVLC header */
        npdu_len -= 10;
        if (npdu_len < max_pdu) {
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


void bip_get_MAC_address(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    my_address->macType = MAC_TYPE_BIP;
    bits_ipAddr_port_to_bacnet_mac(my_address, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);
}


void bip_get_broadcast_BACnetMacAddress(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * dest)
{
    /* destination address */

        dest->len = 6;
        memcpy(&dest->bytes[0], &portParams->datalink.bipParams.nwoBroadcast_addr, 4) ; //BIP_Broadcast_Address.s_addr, 4);
        memcpy(&dest->bytes[4], &portParams->datalink.bipParams.nwoPort, 2);
}

