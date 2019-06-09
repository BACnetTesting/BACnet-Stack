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

#ifndef BIP_H
#define BIP_H

#include "npdu.h"

/* specific defines for BACnet/IP over Ethernet */

/*
12.11.18 Max_APDU_Length_Accepted

This property, of type Unsigned, is the maximum number of octets that may be contained in a single, indivisible application
layer protocol data unit. The value of this property shall be greater than or equal to 50. The value of this property is also
constrained by the underlying data link technology and shall be less than or equal to the largest APDU_Length of the enabled
Network Port objects used to represent the underlying data links. See Clauses 6 through 11, Annex J, Annex O, and Annex U.
If the value of this property is not encodable in the 'Max APDU Length Accepted' parameter of a ConfirmedRequest-PDU,
then the value encoded shall be the highest encodable value less than the value of this property. In such cases, a responding
device may ignore the encoded value in favor of the value of this property, if it is known.
*/

/* 
    The most common Ethernet packet frame can carry 1500 bytes of payload.. see https://en.wikipedia.org/wiki/Ethernet_frame#Ethernet_II
    UDP packets have to be packed within this.. so 1500 less IP header (20 bytes), less UDP header (8 bytes) 
            https://en.wikipedia.org/wiki/User_Datagram_Protocol#Packet_structure
            https://notes.shichao.io/tcpv1/ch10/
    leaves LPDU for BACnet/IP of 1472. A packet any longer than this will result in fragmented packets, so we would like to draw the line here
    However. BACnet specifies a possible MAX_LPDU_IP of 1476, and many people use it (fragmented), so we need to accomodate these reassembled
    packets. Lets just shame those users who go to the limit, allow it, and for ourselves just set a lower threshold...
*/

#define MAX_APDU_IP     1476

// bacdef.h MAX_NPCI is (21) bytes, see http://www.bacnetwiki.com/wiki/index.php?title=NPCI

#define MAX_HEADER_IP   (1 + 1 + 2)

#define MAX_NPDU_IP     (MAX_NPCI+MAX_APDU_IP)

#define MAX_MPDU_IP     (MAX_HEADER_IP+MAX_NPDU_IP) // todo 1 change to MAX_LPDU_IP

// MAX_MPDU_IP contains the UDP header, which we have no control over. 

/*
    See my calculations: https://docs.google.com/spreadsheets/d/1lHet3Dd_Qm6tAHOXjy0zL1QZMZQ-PiYPFlP9TjMofjg/edit#gid=0

    Note that for BACnet/IP the LPDU calculation is 1500-20-8-4-21 = 1447.

    But MAX_APDU here was defined using BACnet/Ethernet
    So we are stuck with the original BACnet Committee of 1500-3-21 = 1476 for BACnet/IP.
    
    If packets are build using a max APDU 1476 bytes, they WILL fragment over IPv4 UDP, and 
    according to Maximum Transmission Unit (MTU) of the underlying layer, (looking at you IPv6 at 1280) 
    possibly sooner.

    https://en.wikipedia.org/wiki/Maximum_transmission_unit
    https://blog.apnic.net/2016/05/19/fragmenting-ipv6/
*/

#define BVLL_TYPE_BACNET_IP (0x81)

#if defined(BIP_DEBUG)
extern bool BIP_Debug;
#endif

/* note: define init, set_interface, and cleanup in your port */
/* on Linux, ifname is eth0, ath0, arc0, and others.
   on Windows, ifname is the dotted ip address of the interface */
bool bip_init(
    const char *ifname);

void bip_set_interface(
    const char *ifname);

void bip_cleanup(
    void);

/* common BACnet/IP functions */
void bip_set_socket(
    int sock_fd);
int bip_socket(
    void);
bool bip_valid(
    void);
void bip_get_broadcast_address(
    BACNET_ADDRESS * dest); /* destination address */
void bip_get_my_address(
    BACNET_ADDRESS * my_address);

/* function to send a packet out the BACnet/IP socket */
/* returns zero on success, non-zero on failure */
int bip_send_pdu(
    BACNET_ADDRESS * dest,  /* destination address */
    BACNET_NPCI_DATA * npci_data,   /* network information */
    uint8_t * pdu,  /* any data to be sent - may be null */
    unsigned pdu_len);      /* number of bytes of data */

/* receives a BACnet/IP packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t bip_receive(
    BACNET_ADDRESS * src,   /* source address */
    uint8_t * pdu,  /* PDU data */
    uint16_t max_pdu,       /* amount of space available in the PDU  */
    unsigned timeout);      /* milliseconds to wait for a packet */

/* use network byte order for setting */
void bip_set_port(
    uint16_t port);

bool bip_port_changed(void);

/* returns network byte order */
uint16_t bip_get_port(
    void);

/* use network byte order for setting */
void bip_set_addr(
    uint32_t net_address);

/* returns network byte order */
uint32_t bip_get_addr(
    void);

/* use network byte order for setting */
void bip_set_broadcast_addr(
    uint32_t net_address);

/* returns network byte order */
uint32_t bip_get_broadcast_addr(
    void);

/* gets an IP address by name, where name can be a
   string that is an IP address in dotted form, or
   a name that is a domain name
   returns 0 if not found, or
   an IP address in network byte order */
long bip_getaddrbyname(
    const char *host_name);


/** @defgroup DLBIP BACnet/IP DataLink Network Layer
 * @ingroup DataLink
 * Implementation of the Network Layer using BACnet/IP as the transport, as
 * described in Annex J.
 * The functions described here fulfill the roles defined generically at the
 * DataLink level by serving as the implementation of the function templates.
 *
 */
#endif
