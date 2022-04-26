/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2012 Steve Karg

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

//#include <stdint.h>     /* for standard integer types uint8_t etc. */
//#include <stdbool.h>    /* for the standard bool type. */
//#include "bacdef.h"
//#include "bacdcode.h"
#include "bacint.h"
//#include "bip.h"
#include "bvlc.h"
//#include "handlers.h"
#include "net.h"        /* custom per port */

#include "datalink.h"
#include "btaDebug.h"
#include "portthread.h"
#include "emm.h"
#include "bacaddr.h"
#include "ese.h"

/** @file bip.c  Configuration and Operations for BACnet/IP */

extern OS_RSEMA newIPsema ;
extern OS_MAILBOX   incomingIPqueue ;

extern uint32_t        bcastIpAddr ;

extern ROUTER_PORT *ipRouterPort ;
extern SYS_NV*            gSYS_nv;

/* returns network byte order */
uint32_t bip_get_addr(const PORT_SUPPORT *portParams)
{
    // return BIP_Address.s_addr;
    return portParams->bipParams.nwoLocal_addr;
}

#if 0
void bip_set_broadcast_addr(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
    // BIP_Broadcast_Address.s_addr = net_address;
    portParams->bipParams.nwoBroadcast_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_ipAddr(
    const PORT_SUPPORT *portParams)
{
    // return BIP_Broadcast_Address.s_addr;
    return portParams->bipParams.nwoBroadcast_addr;
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
    return portParams->bipParams.nwoPort;
}


    // copy data, everything in network order...
void bip_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *addr, const uint32_t nwoIpAddr, const uint16_t nwoPort)
{
#if ( BAC_DEBUG == 1)
    addr->signature = 'M' ;
#endif

    addr->bytes[0] = ((uint8_t *)&nwoIpAddr)[0];
    addr->bytes[1] = ((uint8_t *)&nwoIpAddr)[1];
    addr->bytes[2] = ((uint8_t *)&nwoIpAddr)[2];
    addr->bytes[3] = ((uint8_t *)&nwoIpAddr)[3];

    addr->bytes[4] = ((uint8_t *)&nwoPort)[0];
    addr->bytes[5] = ((uint8_t *)&nwoPort)[1];

    addr->len = 6 ;
}

void bip_mac_to_addr(
    struct in_addr *address,
    const uint8_t * mac)
{
    if (mac && address) {
        address->s_addr = ((u32_t) ((((uint32_t) mac[0]) << 24) & 0xff000000));
        address->s_addr |= ((u32_t) ((((uint32_t) mac[1]) << 16) & 0x00ff0000));
        address->s_addr |= ((u32_t) ((((uint32_t) mac[2]) << 8) & 0x0000ff00));
        address->s_addr |= ((u32_t) (((uint32_t) mac[3]) & 0x000000ff));
    }
}

#if 0
static void bip_addr_to_mac(
    uint8_t * mac,
    struct ip_addr *address)
{
    if (mac && address) {
        mac[0] = (uint8_t) (address->addr >> 24);
        mac[1] = (uint8_t) (address->addr >> 16);
        mac[2] = (uint8_t) (address->addr >> 8);
        mac[3] = (uint8_t) (address->addr);
    }
}
#endif

static int bip_decode_bip_address(
    const BACNET_MAC_ADDRESS * macAddr,
    struct in_addr *nwoAddress,       /* in network format */
    uint16_t * nwoPort)               /* in network format */
{
#if ( BAC_BIG_ENDIAN == 1 )
#error
    bip_mac_to_addr(nwoAddress, &macAddr->bytes[0]);
    memcpy(nwoPort, &macAddr->bytes[4], 2);
#else
    ((u8_t *)&nwoAddress->s_addr)[0] = macAddr->bytes[0] ;
    ((u8_t *)&nwoAddress->s_addr)[1] = macAddr->bytes[1] ;
    ((u8_t *)&nwoAddress->s_addr)[2] = macAddr->bytes[2] ;
    ((u8_t *)&nwoAddress->s_addr)[3] = macAddr->bytes[3] ;

    ((u8_t *)nwoPort)[0] = macAddr->bytes[4] ;
    ((u8_t *)nwoPort)[1] = macAddr->bytes[5] ;
#endif
    return 6 ;
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
int bip_send_npdu(
    const PORT_SUPPORT *portParams,
    const BACNET_MAC_ADDRESS *destMAC,      /* destination address, if null, then broadcast */
    const BACNET_NPDU_DATA * npdu_data,     /* network information */
    DLCB *dlcb )
{
#ifdef DEBUG
    if ( ! dlcb_check ( dlcb ) ) return 0 ;
#endif

    uint8_t mtu[1500] ;   // todo2 get this off the stack.
    int mtu_len ;
    /* addr and port in host format */
    struct sockaddr_in servaddr;

    // uint16_t port ;
    // uint16_t length = dlcb->bufSize + 4;
    // err_t status = ERR_OK;

    (void) npdu_data;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    mtu[0] = BVLL_TYPE_BACNET_IP;
    if ( destMAC->len == 0 ) {
        /* broadcast */
        servaddr.sin_addr.s_addr = bcastIpAddr ; //  ntohl( bcastIpAddr ) ;
        servaddr.sin_port = ipRouterPort->port_support->bipParams.nwoPort ;
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    } else if (destMAC->len == 6) {
        /* unicast */
        bip_decode_bip_address(destMAC, &servaddr.sin_addr, &servaddr.sin_port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    } else {
        /* invalid address */
        ese_enqueue_once ( ese013_0D_IP_err ) ;
        dlcb_free ( dlcb ) ;
        return 0;
    }
    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t) ( dlcb->optr + 4 /*inclusive */ ));
    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->optr );
    mtu_len += dlcb->optr;

    dlcb_free ( dlcb ) ;

    /* Send the packet */

    // LWIP requires port to be in host order (and we store in network order, so swap about)
    // LWIP requires addresses to host order too, and we manipulate in network order, so swap about too

    // servaddr.sin_port = ntohs ( servaddr.sin_port ) ; // already there
    // *((uint32_t *)&servaddr.sin_addr) = ntohl (*((uint32_t *)&servaddr.sin_addr)) ;

    BACNET_MAC_ADDRESS mac ;
    bacnet_mac_clear ( &mac ) ;
    portParams->get_MAC_address( portParams, &mac) ;
    SendBTApacketTx ( 1, &mac, destMAC, mtu, mtu_len ) ;

    int len = sendto(portParams->bipParams.socket, mtu, mtu_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) ;
    if (  len <= 0 )
    {
      ese_enqueue_once ( ese011_0B_IP_failed_to_send ) ;
      return 0;
    }

    return mtu_len;
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
  u32_t fromlen = sizeof (struct sockaddr) ;
  struct sockaddr_in fromIPEP ;

  int rlen = lwip_recvfrom ( portParams->bipParams.socket, pdu, max_pdu, 0, (struct sockaddr *) &fromIPEP, &fromlen) ;
  if ( rlen < 6 )   // 6 is safe for non-bbmd devices.
  {
    ese_enqueue ( ese014_0E_Non_BACnetIP_pkt_recd ) ;
    return 0 ;
  }
  

  /* the signature of a BACnet/IP packet */
  if (  pdu[0] != BVLL_TYPE_BACNET_IP) {
    ese_enqueue ( ese014_0E_Non_BACnetIP_pkt_recd ) ;
    return 0 ;
  }

  bip_ipAddr_port_to_bacnet_mac( mac, fromIPEP.sin_addr.s_addr, fromIPEP.sin_port ) ;

  if ( pdu[1] == BVLC_ORIGINAL_UNICAST_NPDU ||
      pdu[1] == BVLC_ORIGINAL_BROADCAST_NPDU )
  {
      if ( rlen < 4 )
      {
          SendBTAhexdump("Panic: Bad Packet 1", pdu, rlen);
          SendBTAhexdump ( "srcPhy", mac, sizeof ( BACNET_MAC_ADDRESS ));
          return 0 ;
      }
  }
  else if ( pdu[1] == BVLC_FORWARDED_NPDU )
  {
      if ( rlen < 4 )
      {
          SendBTAhexdump("Panic: Bad Packet 2", pdu, rlen);
          SendBTAhexdump ( "srcPhy", mac, sizeof ( BACNET_MAC_ADDRESS ));
          return 0 ;
      }
  }
  else
  {
    // todo 1, we should be sending a NAK.
    SendBTAhexdump("Panic: Bad Packet 3", pdu, rlen);
    SendBTAhexdump ( "srcPhy", mac, sizeof ( BACNET_MAC_ADDRESS ));
    ese_enqueue ( ese014_0E_Non_BACnetIP_pkt_recd ) ;
    return 0 ;
  }

  return (uint16_t) rlen ;
}


void bip_get_MAC_address(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    bip_ipAddr_port_to_bacnet_mac(my_address, portParams->bipParams.nwoLocal_addr, portParams->bipParams.nwoPort);
}

#if 0
void bip_get_broadcast_BACnetMacAddress(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * dest)
{
    /* destination address */
//     int i = 0;  /* counter */

        dest->len = 6;
        memcpy(&dest->bytes[0], &portParams->bipParams.nwoBroadcast_addr, 4) ; //BIP_Broadcast_Address.s_addr, 4);
        memcpy(&dest->bytes[4], &portParams->bipParams.nwoPort, 2);
}
#endif

/** Initialize the BACnet/IP services at the given interface.
 * @ingroup DLBIP
 * -# Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 * -# Opens a UDP socket
 * -# Configures the socket for sending and receiving
 * -# Configures the socket so it can send broadcasts
 * -# Binds the socket to the local IP address at the specified port for
 *    BACnet/IP (by default, 0xBAC0 = 47808).
 *
 * @note For Windows, ifname is the dotted ip address of the interface.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "eth0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */

extern bool bipInitialized ;

bool bip_init(
    PORT_SUPPORT *portParams,
    const char *ifname)
{
  int sockopt = 1 ;
  (void) ifname;

    // SendBTAmessage("bip_init called......");
    // todo 2 0 we need to figure out how to establish our own address to avoid reflections
    // BIP_Address = portParams->bipParams.nwoLocal_addr ;

  int sockfd = lwip_socket(PF_INET, SOCK_DGRAM, 0);
  sockfd = lwip_socket(PF_INET, SOCK_DGRAM, 0);
  if ( sockfd < 0 )
  {
    ese_enqueue( ese016_10_Could_Not_Init_IP ) ;
    return false ;
  }

  if(lwip_setsockopt( sockfd,
                  SOL_SOCKET,
                  SO_BROADCAST,
                  &sockopt,
                  sizeof(int)) == -1)
    {
    close(sockfd);
    ese_enqueue( ese016_10_Could_Not_Init_IP ) ;
    return false ;
    }

  struct sockaddr_in self;
  memset(&self, 0, sizeof(self));
  self.sin_family = AF_INET;
  self.sin_port = portParams->bipParams.nwoPort ;
  self.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to port
  if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self))  < 0 )
  {
    close(sockfd);
    ese_enqueue( ese016_10_Could_Not_Init_IP ) ;
    return false ;
  }

  portParams->bipParams.socket = sockfd ;

  bipInitialized = true ;
  return true;
}
