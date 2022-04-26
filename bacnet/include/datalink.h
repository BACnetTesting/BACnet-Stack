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

#include "configProj.h"

#if defined(BACDL_BIP) && BBMD_ENABLED
#include "bbmd.h"
#endif

// be careful not to replace this with #pragma once without changing the checks in bbmd.h
#ifndef DATALINK_H
#define DATALINK_H

#include "bacdef.h"
#include "bitsDebug.h"

void Init_Datalink_Thread( void )  ;

#if defined(BACDL_ETHERNET)
#include "ethernet.h"

#define datalink_init ethernet_init
#define datalink_send_pdu ethernet_send_pdu
#define datalink_receive ethernet_receive
#define datalink_cleanup ethernet_cleanup
#define datalink_get_broadcast_address ethernet_get_broadcast_address
#define datalink_get_my_address ethernet_get_my_address

#elif defined(BACDL_ARCNET)
#include "arcnet.h"

#define datalink_init arcnet_init
#define datalink_send_pdu arcnet_send_pdu
#define datalink_receive arcnet_receive
#define datalink_cleanup arcnet_cleanup
#define datalink_get_broadcast_address arcnet_get_broadcast_address
#define datalink_get_my_address arcnet_get_my_address

#elif defined(BACDL_MSTP)
#include "dlmstp.h"
#include "mstp.h"

#define MAX_APDU    MAX_APDU_MSTP
#define MAX_NPDU    MAX_NPDU_MSTP
#define MAX_MPDU    MAX_MPDU_MSTP
    
#define datalink_init dlmstp_init
#define datalink_send_pdu dlmstp_send_pdu
#define datalink_receive dlmstp_receive
#define datalink_cleanup dlmstp_cleanup
#define datalink_get_broadcast_address dlmstp_get_broadcast_address
#define datalink_get_my_address dlmstp_get_my_address

#elif defined(BACDL_BIP)
#include "bip.h"
#include "bvlc.h"

#define MAX_NPDU    MAX_NPDU_IP
// #define MAX_PDU     MAX_PDU_IP
#define MAX_MPDU    MAX_MPDU_IP

#define datalink_init bip_init
#if defined(BBMD_ENABLED) && BBMD_ENABLED
#define datalink_send_pdu bvlc_send_pdu
#define datalink_receive bvlc_receive
#else
#define datalink_send_pdu bip_send_pdu
#define datalink_receive bip_receive
#endif
#define datalink_cleanup bip_cleanup
#define datalink_get_broadcast_address bip_get_broadcast_address
#if ( BAC_ROUTING == 1 )
extern void routed_get_my_address(
    BACNET_ADDRESS * my_address);
#define datalink_get_my_address routed_get_my_address
#else
#define datalink_get_my_address bip_get_my_address
#endif

#elif defined(BACDL_BIP6)
#include "bip6.h"
#include "bvlc6.h"
#define datalink_init bip6_init
#define datalink_send_pdu bip6_send_pdu
#define datalink_receive bip6_receive
#define datalink_cleanup bip6_cleanup
#define datalink_get_broadcast_address bip6_get_broadcast_address
#define datalink_get_my_address bip6_get_my_address


#else /* Ie, BACDL_ALL */
#include "npdu.h"

#error
#define MAX_HEADER (8)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)


    int datalink_send_pdu(
        BACNET_ADDRESS * dest,
        BACNET_NPCI_DATA * npci_data,
        uint8_t * pdu,
        unsigned pdu_len);
    extern uint16_t datalink_receive(
        BACNET_ADDRESS * src,
        uint8_t * pdu,
        uint16_t max_pdu,
        unsigned timeout);
    extern void datalink_cleanup(
        void);
    extern void datalink_get_broadcast_address(
        BACNET_ADDRESS * dest);
    extern void datalink_get_my_address(
        BACNET_ADDRESS * my_address);
    extern void datalink_set_interface(
        char *ifname);
    extern void datalink_set(
        char *datalink_string);

#endif
/** @defgroup DataLink The BACnet Network (DataLink) Layer
 * <b>6 THE NETWORK LAYER </b><br>
 * The purpose of the BACnet network layer is to provide the means by which
 * messages can be relayed from one BACnet network to another, regardless of
 * the BACnet data link technology in use on that network. Whereas the data
 * link layer provides the capability to address messages to a single device
 * or broadcast them to all devices on the local network, the network layer
 * allows messages to be directed to a single remote device, broadcast on a
 * remote network, or broadcast globally to all devices on all networks.
 * A BACnet Device is uniquely located by a network number and a MAC address.
 *
 * Each client or server application must define exactly one of these
 * DataLink settings, which will control which parts of the code will be built:
 * - BACDL_ETHERNET -- for Clause 7 ISO 8802-3 ("Ethernet") LAN
 * - BACDL_ARCNET   -- for Clause 8 ARCNET LAN
 * - BACDL_MSTP     -- for Clause 9 MASTER-SLAVE/TOKEN PASSING (MS/TP) LAN
 * - BACDL_BIP      -- for ANNEX J - BACnet/IP
 * - BACDL_ALL      -- Unspecified for the build, so the transport can be
 *                     chosen at runtime from among these choices.
 * - Clause 10 POINT-TO-POINT (PTP) and Clause 11 EIA/CEA-709.1 ("LonTalk") LAN
 *   are not currently supported by this project.
 */
 /** @defgroup DLTemplates DataLink Template Functions
 * @ingroup DataLink
 * Most of the functions in this group are function templates which are assigned
 * to a specific DataLink network layer implementation either at compile time or
 * at runtime.
 */
 
 
//typedef struct _DLCB
//{
//#if ( BAC_DEBUG == 1 )
//    uint8_t signature ;
//#endif
//
//    bool                isDERresponse ;                      // is this packet due to an external DER (MSTP) or internal unsolicited (App) event?
//    uint16_t            bufMax;
//    uint16_t            optr;
//    uint8_t             *Handler_Transmit_Buffer;
//    
//    uint8_t     destMac;
//    /*
//    BACNET_MAC_ADDRESS  phyDest;
//    PORT_SUPPORT        *portParams;
//    */
//} DLCB ;
//
//#endif
//
// // now in bitsDatalink.h
//    // DLCB *alloc_dlcb_sys(char tag, bool isResponse, uint8_t destMac );
//    // void dlcb_free(DLCB *dlcb);
//
//#if ( BAC_DEBUG == 1 )
//    bool dlcb_check(DLCB *dlcb);
//#endif
//
//// renaming to allow tracing during debugging
//#define alloc_dlcb_response(tag, mac)        alloc_dlcb_sys(tag, true, mac)
//#define alloc_dlcb_new_message(tag, mac)     alloc_dlcb_sys(tag, false, mac)
//

#endif
