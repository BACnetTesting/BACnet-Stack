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
#ifndef BIP_H
#define BIP_H

//#include <stdbool.h>
//#include <stdint.h>
//#include <stddef.h>
//#include "bacdef.h"
//#include "npdu.h"
//#include "net.h"
#include "datalink.h"

/* specific defines for BACnet/IP over Ethernet */
#define MAX_PDU_IP 1497         // resolve - there is no such thing as a PDU. NPDU, APDU, MPDU yes... but.. 
#define MAX_HEADER_IP (1 + 1 + 2)
#define MAX_MPDU_IP (MAX_HEADER_IP+MAX_PDU_IP)

#define MAX_MPDU_ETHERNET   1497

#define BVLL_TYPE_BACNET_IP (0x81)

#if defined(BIP_DEBUG)
extern bool BIP_Debug;
#endif

/* note: define init, set_interface, and cleanup in your port */
/* on Linux, ifname is eth0, ath0, arc0, and others.
   on Windows, ifname is the dotted ip address of the interface */
bool bip_init(
    DLINK_SUPPORT *portParams,
    char *ifname);

void bip_set_interface(
    DLINK_SUPPORT *portParams, 
    const char *ifname);

//void bip_cleanup(
//    );

/* common BACnet/IP functions */

// following moved to PORT_SUPPORT
//void bip_set_socket(
//    int sock_fd);
//int bip_socket(
//    void);
//bool bip_valid(
//    void);

// EKH: this original name (bip_get_broadcast_address) was very close to (bip_get_broadcast_addr)
// Renaming bip_get_broadcast_address to bip_get_broadcast_BACnetAddress
void bip_get_broadcast_BACnetMacAddress(
    const DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *dest);

void bip_get_MAC_address(
    const DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address);

    /* function to send a packet out the BACnet/IP socket */
    /* returns zero on success, non-zero on failure */
void bip_send_npdu(
    const DLINK_SUPPORT          *portParams,
    const BACNET_MAC_ADDRESS    *destMac,           /* destination address */
    const BACNET_NPDU_DATA      *npdu_data,         /* network information */
    const DLCB *dlcb);

    /* receives a BACnet/IP packet */
    /* returns the number of octets in the PDU, or zero on failure */
uint16_t bip_receive(
    DLINK_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *mac, 
    uint8_t *pdu,
    uint16_t max_pdu);

/* use network byte order for setting */
void bip_set_local_port(
    DLINK_SUPPORT *portParams,
    uint16_t port);

void bip_set_nat_port(
    uint16_t port);
    
bool bip_port_changed(void);
    
// replaced by portParams
///* returns network byte order */
uint16_t bip_get_local_port(
    const DLINK_SUPPORT *portParams ) ;

///* use network byte order for setting */
void bip_set_addr(
    DLINK_SUPPORT *portParams,
    uint32_t net_address);

/* returns network byte order */
uint32_t bip_get_addr(const DLINK_SUPPORT *portParams);

/* use network byte order for setting */
void bip_set_broadcast_addr(
    DLINK_SUPPORT *portParams,
    uint32_t net_address);

void set_broadcast_address(
    DLINK_SUPPORT *portParams,
    uint32_t hostAddress);

    /* returns network byte order */
uint32_t bip_get_broadcast_ipAddr(
    const DLINK_SUPPORT *portParams
    );

//void bip_ipAddr_port_from_bacnet_address(struct in_addr *ipAddr, uint16_t *port, const BACNET_GLOBAL_ADDRESS *addr );
void bip_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *mac, uint32_t nwoIpAddr, uint16_t nwoPort);
void bip_ipAddr_port_to_bacnet_addr(BACNET_GLOBAL_ADDRESS *addr, uint32_t nwoIpAddr, uint16_t nwoPort);

    /* gets an IP address by name, where name can be a
       string that is an IP address in dotted form, or
       a name that is a domain name
       returns 0 if not found, or
       an IP address in network byte order */
    long bip_getaddrbyname(
        const char *host_name);


char *winsock_error_code_text(
    int code);


/** @defgroup DLBIP BACnet/IP DataLink Network Layer
 * @ingroup DataLink
 * Implementation of the Network Layer using BACnet/IP as the transport, as
 * described in Annex J.
 * The functions described here fulfill the roles defined generically at the
 * DataLink level by serving as the implementation of the function templates.
 *
 */
#endif
