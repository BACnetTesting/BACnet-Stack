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
//#include <stdbool.h>    /* for the standard bool type. */
#include "stdint.h"     /* for standard integer types uint8_t etc. */

//#include "bacdcode.h"
#include "bacint.h"
// #include "bip.h"
//#include "bvlc.h"
// #include "net.h"        /* custom per port */
#include "datalink.h"
//#if PRINT_ENABLED
//#include <stdio.h>      /* for standard i/o, like printing */
//#endif
#include "btaDebug.h"
#include "bacaddr.h"

// static int BIP_Socket = -1;
/* port to use - stored in network byte order */
// uint16_t BIP_local_Port = 0;   
// uint16_t BIP_nat_Port = 0;   


// in_addr does not contain port information
// sockaddr_in does..
/* IP Address - stored in network byte order */
// static struct in_addr BIP_Address;

/* Broadcast Address - stored in network byte order */
// static struct in_addr BIP_Broadcast_Address;


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

#if defined(BBMD_ENABLED) && BBMD_ENABLED
void bip_set_broadcast_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
    portParams->datalink.bipParams.nwoBroadcast_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_ipAddr(
    const PORT_SUPPORT *portParams)
{
    return portParams->datalink.bipParams.nwoBroadcast_addr;
}
#endif


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

//void bip_addr_to_mac(
//    uint8_t * mac,
//    const struct in_addr *address)
//{
//    if (mac && address) {
//        mac[3] = (uint8_t)(address->s_addr >> 24);
//        mac[2] = (uint8_t)(address->s_addr >> 16);
//        mac[1] = (uint8_t)(address->s_addr >> 8);
//        mac[0] = (uint8_t)(address->s_addr);
//    }
//}

// all in network order
//void bip_ipAddr_from_mac(
//    struct in_addr *address,
//    const uint8_t * mac
//    )
//{
//    uint8_t *tad = (uint8_t *) address ;
//#ifdef BIG_ENDIAN
//    tad[0] = mac[0];
//    tad[1] = mac[1];
//    tad[2] = mac[2];
//    tad[3] = mac[3];
//#else
//    tad[0] = mac[3];
//    tad[1] = mac[2];
//    tad[2] = mac[1];
//    tad[3] = mac[0];
//#endif
//}

    
// copy data, everything in network order...
void bip_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *addr, const uint32_t nwoIpAddr, const uint16_t nwoPort)
{
#if ( BAC_DEBUG == 1 )
    addr->signature = 'M';
#endif

    addr->bytes[0] = ((uint8_t *)&nwoIpAddr)[0];
    addr->bytes[1] = ((uint8_t *)&nwoIpAddr)[1];
    addr->bytes[2] = ((uint8_t *)&nwoIpAddr)[2];
    addr->bytes[3] = ((uint8_t *)&nwoIpAddr)[3];

    addr->bytes[4] = ((uint8_t *)&nwoPort)[0];
    addr->bytes[5] = ((uint8_t *)&nwoPort)[1];

    addr->len = 6 ;
}

//void bip_ipAddr_port_to_bacnet_addr(BACNET_GLOBAL_ADDRESS *addr, const uint32_t nwoIpAddr, const uint16_t nwoPort)
//{
//    addr->mac[0] = ((uint8_t *)&nwoIpAddr)[0];
//    addr->mac[1] = ((uint8_t *)&nwoIpAddr)[1];
//    addr->mac[2] = ((uint8_t *)&nwoIpAddr)[2];
//    addr->mac[3] = ((uint8_t *)&nwoIpAddr)[3];
//
//    addr->mac[4] = ((uint8_t *)&nwoPort)[0];
//    addr->mac[5] = ((uint8_t *)&nwoPort)[1];
//
//    addr->localMac.len = 6;
//}

//void bip_ipAddr_port_from_bacnet_address(struct in_addr *ipAddr, uint16_t *port, const BACNET_GLOBAL_ADDRESS *addr )
//{
//#ifdef _DEBUG
//        if ( addr->localMac.len != 6 )
//        {
//            panic();
//            return ;
//        }
//#endif
//        bip_ipAddr_from_mac( ipAddr, addr->mac);
//        *port = (uint16_t)(addr->mac[4] << 8);
//        *port |= (uint16_t)(addr->mac[5]); 
//}


static void bip_decode_bip_address(
    const uint8_t * bac_mac,
    struct in_addr *address,    /* in network format */
    uint16_t *port)             /* in network byte order */
{
        memcpy(&address->s_addr, &bac_mac[0], 4);
        memcpy(port, &bac_mac[4], 2);
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

//int bip_send_npdu(
//    const PORT_SUPPORT *portParams,
//    const BACNET_PATH * destPath,            /* destination address */
//    const BACNET_NPCI_DATA * npci_data,     /* network information */
//    const uint8_t * pdu,                    /* any data to be sent - may be null */
//    const uint16_t pdu_len)
//{       /* number of bytes of data */
//    // struct sockaddr_in bip_dest;
//    // uint8_t mtu[MAX_MPDU_ETHERNET] = { 0 };
//    // int mtu_len = 0;
//    // int bytes_sent = 0;
//    /* addr and port in host format */
//    // struct in_addr address;
//    // uint16_t port;
//    const BACNET_MAC_ADDRESS *targetMac;
//    
//    /* bip datalink doesn't need to know the npdu data */
//    // todo3 - this information is only required for MSTP (for DER), which is unfortunate because this object pollutes all other calls. Think about removing this..
//    (void) npci_data;
//    /* assumes that the driver has already been initialized */
//    if ( portParams->bipParams.socket < 0) {
//        panic();
//        return -1 ;
//    }
//
////    mtu[0] = BVLL_TYPE_BACNET_IP;
////    bip_dest.sin_family = AF_INET;
//
//    // fill in the Router MAC if this message is for another network
//
//    if ((destPath->glAdr.net == BACNET_BROADCAST_NETWORK) || ( destPath->localMac.len == 0)) {
//        /* broadcast */
//        // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
//        // // port = BIP_Port;
//        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
//        targetMac = NULL;   // broadcast
//    }
//    else if ((destPath->glAdr.net > 0) && (destPath->localMac.len == 0)) {
//        /* network specific (remote) broadcast */
//        if (destPath->localMac.len == 6) {
//            // bip_decode_bip_address(dest, &address, &port);
//            targetMac = &destPath->localMac;
//        }
//        else {
//            // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
//            // // port = BIP_Port;
//            targetMac = NULL;   // broadcast
//        }
//        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
//    }
//    else if (destPath->localMac.len == 6) {
//        // bip_decode_bip_address(dest, &address, &port);
//        // mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
//        targetMac = &destPath->localMac ;
//    }
//    else {
//        panicDesc("m0039: Cannot resolve local MAC from Path for IP");
//        return -1;
//    }
//
//    //bip_dest.sin_addr.s_addr = address.s_addr;
//    //bip_dest.sin_port = port;
//    //memset(&(bip_dest.sin_zero), '\0', 8);
//    //mtu_len = 2;
//    //mtu_len +=
//    //    encode_unsigned16(&mtu[mtu_len],
//    //    (uint16_t)(pdu_len + 4 /*inclusive */));
//    //memcpy(&mtu[mtu_len], pdu, pdu_len);
//    //mtu_len += pdu_len;
//    //// todonext - other dl_ip_send_local_only had this lock here. Do some research todonext8
//    //// LockTransaction ( mutexLon ) ;
//
//    return bip_send_pdu_local_only(portParams, targetMac, pdu, pdu_len);
//}


void bip_send_npdu(
    // const PORT_SUPPORT *portParams,
    // const DEVICE_OBJECT_DATA    *pDev,
    // const BACNET_MAC_ADDRESS *destMAC,      /* destination address, if null, then broadcast */
    // const BACNET_NPCI_DATA * npci_data,     /* network information */
    const DLCB *dlcb )
{
    struct sockaddr_in bip_dest;
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    int mtu_len = 0;
    int bytes_sent ;
    /* addr and port in host format */
    struct in_addr address;
    uint16_t port;

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;
    if ( dlcb->route.bacnetPath.localMac.len == 0) {
        /* broadcast */
        address.s_addr = dlcb->route.portParams->datalink.bipParams.nwoBroadcast_addr; // BIP_Broadcast_Address.s_addr;
        // port = BIP_Port;
        port = dlcb->route.portParams->datalink.bipParams.nwoPort;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
    else
    {
        bip_decode_bip_address(dlcb->route.bacnetPath.localMac.bytes, &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    }

    bip_dest.sin_addr.s_addr = address.s_addr;
    bip_dest.sin_port = port;
    memset(&(bip_dest.sin_zero), '\0', 8);      // todonext suspicious &
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t)(dlcb->optr + 4 /*inclusive */));
    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->optr);
    mtu_len += dlcb->optr;

    /* Send the packet */
    bytes_sent =
        sendto(dlcb->route.portParams->datalink.bipParams.socket, (char *)mtu, mtu_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    if (bytes_sent <= 0)
    {
        SendBTApanicMessage("m0025 - Could not send PDU");
        return; 
    }

    // just for BTA, if we just did a broadcast, fill in the IP b'cast address for clarity (len == 0 indicates b'cast)
    {
        BACNET_MAC_ADDRESS phySrc;
        BACNET_MAC_ADDRESS phyDest;

        //memcpy(&phySrc.adr, &portParams->bipParams.local_addr, 4);
        //memcpy(&phySrc.adr[4], &portParams->bipParams.nwoPort, 2);
        //phySrc.len = 6;
        bip_ipAddr_port_to_bacnet_mac(&phySrc, dlcb->route.portParams->datalink.bipParams.nwoLocal_addr, dlcb->route.portParams->datalink.bipParams.nwoPort);

        if (dlcb->route.bacnetPath.localMac.len != 0)
        {
            bacnet_mac_copy(&phyDest, &dlcb->route.bacnetPath.localMac );
            SendBTApacketTx(500 + dlcb->route.portParams->datalinkId, &phySrc, &phyDest, mtu, mtu_len);
        }
        else
        {
            // broadcast, make a dest for BTA
            BACNET_MAC_ADDRESS bcastMac;
            //memcpy(bcastMac.adr, &portParams->bipParams.local_addr, 4);
            //memcpy(&bcastMac.adr[4], &portParams->bipParams.nwoPort, 2);
            //bcastMac.len = 6;
            bip_ipAddr_port_to_bacnet_mac(&bcastMac, dlcb->route.portParams->datalink.bipParams.nwoBroadcast_addr, dlcb->route.portParams->datalink.bipParams.nwoPort);
            SendBTApacketTx( 600+ dlcb->route.portParams->datalinkId, &phySrc, &bcastMac, mtu, mtu_len);
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
 *                  after the BVLC portion has been stripped off.
 * @param max_pdu [in] Size of the pdu[] buffer.
 * @param timeout [in] The number of milliseconds to wait for a packet.
 * @return The number of octets (remaining) in the PDU, or zero on failure.
 */
uint16_t bip_receive(
    PORT_SUPPORT *portParams,
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
    if (portParams->datalink.bipParams.socket < 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 10000;

    FD_ZERO(&read_fds);
    FD_SET(portParams->datalink.bipParams.socket, &read_fds);
    max = portParams->datalink.bipParams.socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0)
        received_bytes =
        recvfrom(portParams->datalink.bipParams.socket, (char *)&pdu[0], max_pdu, 0,
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

#if defined ( _MSC_VER  )
    // todo2 - move this to Microsoft
    CheckLocalAddressKnown(portParams, &sin);
#endif

    // we do NOT want to process packets from ourselves
    if (sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr && sin.sin_port == portParams->datalink.bipParams.nwoPort)
    {
        return 0;
    }

    BACNET_MAC_ADDRESS smac, dmac;

    bip_ipAddr_port_to_bacnet_mac(&smac, sin.sin_addr.s_addr, sin.sin_port);
    bip_ipAddr_port_to_bacnet_mac(&dmac, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);

    SendBTApacketRx(portParams->datalinkId, &smac, &dmac, pdu, received_bytes);


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
        if ((sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr ) && // BIP_Address.s_addr) &&
            (sin.sin_port == portParams->datalink.bipParams.nwoPort )) {
            pdu_len = 0;
#if 0
            fprintf(stderr, "BIP: src is me. Discarded!\n");
#endif
        } else {
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
                // Do NOT use memcpy on overlapping memory!! (I found this to be an issue on Ubuntu!)
                memmove ( pdu, &pdu[4], pdu_len ) ;
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
        if ((sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr ) && // BIP_Address.s_addr) &&
            (sin.sin_port == portParams->datalink.bipParams.nwoPort)) {
            /* ignore messages from me */
            pdu_len = 0;
        } else {
            /* data in src->mac[] is in network format */
            bip_ipAddr_port_to_bacnet_mac(mac, sin.sin_addr.s_addr, sin.sin_port);
            /* FIXME: check destination address */
            /* see if it is broadcast or for us */
            /* decode the length of the PDU - length is inclusive of BVLC */
            (void) decode_unsigned16(&pdu[2], &pdu_len);
            /* subtract off the BVLC header */
            pdu_len -= 10;
            if (pdu_len < max_pdu) {
                /* shift the buffer to return a valid PDU */
                // Do NOT use memcpy on overlapping memory!! (I found this to be an issue on Ubuntu!)
            	memmove(pdu, &pdu[4 + 6 + i], pdu_len);
            } 
            else {
                /* ignore packets that are too large */
                /* clients should check my max-apdu first */
                pdu_len = 0;
            }
        }
    }

    return pdu_len;
}


void bip_get_MAC_address(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    bip_ipAddr_port_to_bacnet_mac(my_address, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);
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

