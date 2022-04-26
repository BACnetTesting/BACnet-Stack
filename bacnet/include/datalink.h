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

#include "config.h"

#if defined(BACDL_BIP) && BBMD_ENABLED
#include "bbmd.h"
#endif

// be careful not to replace this with #pragma once without changing the checks in bbmd.h
#ifndef DATALINK_H

#include "bip.h"
#include "bbmd.h"
#include "bvlc.h"
#include "bbmd.h"
#include "dlmstp.h"

#define DATALINK_H

#include <stdint.h>
#include "bacdef.h"
#include "bitsDebug.h"
//#include "npdu.h"
#include "net.h"
#include "linklist.h"
//#include "ngwdevice.h"
#include "multipleDatalink.h"

// #include <vector>

class VirtualDeviceInfo;

// now in multipledatalink.h
#if 0
//typedef enum {
//    BPT_BIP = 0x11,
//    BPT_MSTP,
//#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
//    BPT_BBMD,
//    BPT_NAT,
//    BPT_FD,
//#endif
//#include "mstp.h"
//    BPT_VIRT,
//#define MAX_APDU    MAX_APDU_MSTP
//#define MAX_NPDU    MAX_NPDU_MSTP
//#define MAX_MPDU    MAX_MPDU_MSTP
//    
//    BPT_APP
//} BPT_TYPE;


//typedef struct {
//    int socket;
//    uint16_t nwoPort;                           // network order
//
//#define MAX_NPDU    MAX_NPDU_IP
//// #define MAX_PDU     MAX_PDU_IP
//#define MAX_MPDU    MAX_MPDU_IP
//    uint32_t nwoLocal_addr;                    // network order
//#if defined(BBMD_ENABLED) && BBMD_ENABLED
//    uint32_t nwoBroadcast_addr;                // network order
//#endif
//        // uint32_t netmask_not_used_todo3;
//    bool BVLC_NAT_Handling;                     // for NAT BBMD handling
//
//    struct sockaddr_in  fd_ipep;                  // For FD registration
//    uint16_t            fd_timetolive;
//    uint16_t            fd_timeRemaining;
//
//    struct sockaddr_in BVLC_Global_Address;
//} BIP_PARAMS;


//typedef struct {
//    uint32_t baudrate;
//    uint8_t databits;
//    uint8_t stopbits;
//    uint8_t max_master;
//    uint8_t max_frames;
//} MSTP_PARAMS;

typedef struct _PORT_SUPPORT PORT_SUPPORT;
// typedef struct _BACNET_ROUTE BACNET_ROUTE;

//typedef struct _DLCB
//{
//#if ( BAC_DEBUG == 1 )
//  uint8_t signature ;
//#endif
//
//    char                source ;                    // is this packet due to an external (MSTP) or internal (App) event
//    uint16_t            bufMax;
//    uint16_t            optr;
//    uint8_t             *Handler_Transmit_Buffer;
//
//    //BACNET_NPCI_DATA    npciData ;                  // this is only needed to carry expecting_reply for MSTP. Review... todo4
//    //BACNET_MAC_ADDRESS  phyDest;
//    //const PORT_SUPPORT *portParams;                 // this is the datalink port that the packet must go out on - does not make sense ... just checking todo1
//    // rolling up the above 3
//    bool            expectingReply;
//    BACNET_ROUTE    route;
//
//} DLCB ;


//struct _PORT_SUPPORT {
//    LLIST   llist;                  // awkward - need to maintain 2 lists, one for baseline ports, another for routerports.
//
//    uint8_t port_id;                /* different for every router port */
//    BPT_TYPE portType;
//    const char* ifName ;
//
//    // todo3 - we should point to these parameters rather than block out unused space (e.g. BDT, FDT for non BBMD ports)
//    union
//    {
//        BIP_PARAMS bipParams;
//        MSTP_PARAMS mstpParams;
//    } datalink;
//
//    uint8_t *txBuff;    // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).   
//    uint16_t max_lpdu ;
//
//    // BACNET_GLOBAL_ADDRESS      my_address;
//    // BACNET_GLOBAL_ADDRESS      broadcast_address;
//
//    // receive control block - this one is obviously for a BIP type device
//
//    // BACNET_GLOBAL_ADDRESS      src;
//
//    // todo3 all these should be in bip_params above..
//    // int                 socket;
//    BBMD_TABLE_ENTRY    BBMD_Table[MAX_BBMD_ENTRIES];
//    FD_TABLE_ENTRY      FD_Table[MAX_FD_ENTRIES];
//
//    void (*SendPdu) (
//        // const DEVICE_OBJECT_DATA    *pDev,                  // This is only here because the router for the virtual network needs to know the MAC address of the sending device.
//        const DLCB                  *dlcb);
//        //const PORT_SUPPORT          *portParams,
//        //const DEVICE_OBJECT_DATA    *pDev,                  // This is only here because the router for the virtual network needs to know the MAC address of the sending device.
//        //const BACNET_MAC_ADDRESS    *bacnetMac,
//        //const BACNET_NPCI_DATA      *npci_data,
//        //const DLCB                  *dlcb);
//
//    uint16_t(*ReceiveMPDU) (
//        PORT_SUPPORT *portParams,
//        BACNET_MAC_ADDRESS *src,
//        uint8_t *pdu,
//        uint16_t maxlen
//        );
//
//    //// returns true if more idle time needed
//    //bool(*IdleFunc) (
//    //    const _PORT_SUPPORT          *portParams);
//
//    // because we don't know if the following are going to be IP or MSTP... use function call
//
//    //void (*get_broadcast_address) (
//    //    const struct PORT_SUPPORT *portParams, 
//    //    BACNET_GLOBAL_ADDRESS *bcastAddr);
//
//    void (*get_MAC_address) (
//        const struct _PORT_SUPPORT *portParams,
//        BACNET_MAC_ADDRESS *my_address);
//
//    // this is the second link list to the devices (there is a global one for all devices, this is just for the devices on this datalink)
//    // 2018.01.28 Making this a pointer because this structure (PORT_SUPPORT) is not a class, and is initialized to zero, and a non-pointer is not C++ initialized.
//    // cr2341234134141
//    std::vector<VirtualDeviceInfo *> *datalinkDevices;
//
//} ; // PORT_SUPPORT ;

//typedef struct _BACNET_ROUTE {
//    const PORT_SUPPORT  *portParams;
//    BACNET_PATH         bacnetPath;
//} BACNET_ROUTE ;
//
//typedef struct
//{
//    char                block_valid;
//
//    const PORT_SUPPORT  *portParams;
//
//    BACNET_PATH         srcPath;
//
//    uint8_t             *npdu;
//    uint16_t            npdu_len;
//
//} RX_DETAILS;


//typedef struct _DLCB
//{
//#if ( BAC_DEBUG == 1 )
//    uint8_t signature;
//#endif
//    bool                expectingReply;
//    char                source;                     // is this packet due to an external (MSTP) or internal (App) event
//    uint16_t            lpduMax;                    // This is NOT the MAX APDU !!! it is the maximum available buffer to build into MAX_LPDU)
//    uint16_t            optr;
//    uint8_t             *Handler_Transmit_Buffer;
//    BACNET_NPCI_DATA    npciData2 ;                      // this is only needed to carry expecting_reply for MSTP. Review... todo4
//    BACNET_ROUTE        route;                          // contains phyDest (route.bacnetPath.localMac), portParams
//} DLCB ;
#endif
#endif



#ifdef replaced_by_dlcb
typedef struct
{
    int                 block_valid;
    // PORT_SUPPORT        *portParams;
    BACNET_MAC_ADDRESS phyDest;             // this is the locally connected device's MAC address (IPaddress in IP case, 1 byte in MSTP case)
    // BACNET_MAC_ADDRESS *phySrc;
    uint8_t *pdu;
    uint16_t pdu_len;
} TX_DETAILS;
#endif

//void SendToMsgQueue(PORT_SUPPORT *portSupport, BACNET_GLOBAL_ADDRESS *dest, BACNET_NPCI_DATA *npci_data, uint8_t *buffer, uint16_t apdu_len);

void noop_get_broadcast_BACnetAddress(
    const PORT_SUPPORT *portParams,
    BACNET_GLOBAL_ADDRESS * dest);

// now in multipleDatalink.h
// bool dlcb_check(const DLCB *dlcb);
//DLCB *alloc_dlcb_sys(char typ, const BACNET_ROUTE *portParams );
//void dlcb_free(const DLCB *dlcb);
//DLCB *dlcb_clone_deep(const DLCB *dlcb);
//DLCB *dlcb_clone_with_copy_of_buffer(const DLCB *dlcb, const uint16_t len, const uint8_t *buffer);

// renaming to allow tracing during debugging
// in multipleDatalink.h
// #define alloc_dlcb_response(tag, route)     alloc_dlcb_sys(tag, (const BACNET_ROUTE *) route)
// #define alloc_dlcb_application(tag, route)  alloc_dlcb_sys(tag, route)
// #define alloc_dlcb_app2(tag, route)	        alloc_dlcb_sys(tag, route)


//void SendIAmRouter(void);
//void SendNetworkNumberIs(void);
//void SendWhoIsRouter(void);
//void dump_router_table(void);
//// void Send_I_Am_Broadcast_Router(void);
//// void set_local_broadcast_path(BACNET_PATH *dest);
//// void set_global_broadcast_path(BACNET_PATH *dest);
//void set_local_broadcast_mac(BACNET_MAC_ADDRESS *mac);
//void set_broadcast_addr_local(BACNET_GLOBAL_ADDRESS *dest);
//void set_broadcast_addr_global(BACNET_GLOBAL_ADDRESS *dest);
//
//char *PortSupportToString(PORT_SUPPORT *ps);
//void bacnet_route_copy(BACNET_ROUTE *rdest, const BACNET_ROUTE *rsrc);
//void bacnet_route_clear(BACNET_ROUTE *rdest);
//
// PORT_SUPPORT *datalink_initCommon2(const char *adapter, const BPT_TYPE rpt, const uint16_t maxLpdu );
//PORT_SUPPORT *InitDatalink(const char *adapter, const BPT_TYPE rpt, const uint16_t nwoPort);
//void datalink_cleanup(void);

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
 
 
// typedef struct _DLCB
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


// DLCB *alloc_dlcb_sys(char tag, bool isResponse, uint8_t destMac );
// void dlcb_free(DLCB *dlcb);

// #if ( BAC_DEBUG == 1 )
    // bool dlcb_check(DLCB *dlcb);
//#endif

// renaming to allow tracing during debugging
//#define alloc_dlcb_response(tag, mac)        alloc_dlcb_sys(tag, true, mac)
//#define alloc_dlcb_new_message(tag, mac)     alloc_dlcb_sys(tag, false, mac)

