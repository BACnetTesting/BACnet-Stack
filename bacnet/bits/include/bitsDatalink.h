/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.

    This program is free software : you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.If not, see <http://www.gnu.org/licenses/>.

    As a special exception, if other files instantiate templates or
    use macros or inline functions from this file, or you compile
    this file and link it with other works to produce a work based
    on this file, this file does not by itself cause the resulting
    work to be covered by the GNU General Public License. However
    the source code for this file must still be made available in
    accordance with section (3) of the GNU General Public License.

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

#pragma once

#include <stdint.h>
#include "bbmd.h"
#include "bacdef.h"
#include "npdu.h"
#include "net.h"
#include "bbmd.h"
#include "linklist.h"
#include "llist.h"
#include "bacdef.h"
#include <vector>

class VirtualDeviceInfo;

// 2018.09.17 PF_xxxx collided with Microsoft names, renaming BPT_xxxx
typedef enum {
    BPT_BIP = 0x60,
    BPT_MSTP,
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    BPT_BBMD,
    BPT_NAT,
    BPT_FD,
#endif
    BPT_VIRT,
    BPT_APP
} BPT_TYPE;


typedef struct _BIP_PARAMS {
    int socket;
    uint16_t nwoPort;                           // network order
    uint32_t nwoLocal_addr;                    // network order
    
// #if defined(BBMD_ENABLED) && BBMD_ENABLED
    uint32_t nwoBroadcast_addr;                // network order
    uint32_t nwoSubnet_addr;
    uint32_t nwoNetmask_addr;
    // #endif

    bool BVLC_NAT_Handling;                     // for NAT BBMD handling

    struct sockaddr_in  fd_ipep;                // For FD registration
    uint16_t            fd_timetolive;
    uint16_t            fd_timeRemaining;

    struct sockaddr_in BVLC_Global_Address;
} BIP_PARAMS;


typedef struct {
    uint32_t baudrate;
    uint8_t databits;
    uint8_t stopbits;
    uint8_t max_master;
    uint8_t max_frames;
} MSTP_PARAMS;

typedef struct _PORT_SUPPORT PORT_SUPPORT;
typedef struct _BACNET_ROUTE BACNET_ROUTE;
typedef struct _DLCB         DLCB;

struct _PORT_SUPPORT {
    LLIST   llist;          // awkward - need to maintain 2 lists, one for baseline ports, another for routerports.

    // datalinks do not have portIds uint8_t port_id;                /* different for every router port */
    uint8_t datalinkId;
    BPT_TYPE portType;
    const char* ifName ;

    // todo3 - we should point to these parameters rather than block out unused space (e.g. BDT, FDT for non BBMD ports)
    union
    {
        BIP_PARAMS bipParams;
        MSTP_PARAMS mstpParams;
    } datalink;

    uint8_t *txBuff;    // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).
    uint16_t max_lpdu ;

    // todo3 all these should be in bip_params above..
    BBMD_TABLE_ENTRY    BBMD_Table[MAX_BBMD_ENTRIES];
    FD_TABLE_ENTRY      FD_Table[MAX_FD_ENTRIES];

    void (*SendPdu) (
        // const DEVICE_OBJECT_DATA    *pDev,                  // This is only here because the router for the virtual network needs to know the MAC address of the sending device.
        const DLCB                  *dlcb);
    // uint8_t *pdu,
    // const uint16_t pdu_len);

    uint16_t(*ReceiveMPDU) (
        PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *src,
        uint8_t *pdu,
        uint16_t maxlen
        );

    void (*get_MAC_address) (
        const struct _PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *my_address);

    // this is the second link list to the devices (there is a global one for all devices, this is just for the devices on this datalink)
    // 2018.01.28 Making this a pointer because this structure (PORT_SUPPORT) is not a class, and is initialized to zero, and a non-pointer is not C++ initialized.
    // cr2341234134141
    std::vector<VirtualDeviceInfo *> *datalinkDevices;

} ;

typedef struct
{
    // no one uses this char                block_valid;

    PORT_SUPPORT        *portParams;

    // BACNET_PATH         srcPath;

    uint8_t             *npdu;
    uint16_t            npdu_len;

} RX_DETAILS;


typedef struct _DLCB
{
#if ( BAC_DEBUG == 1 )
    uint8_t signature;
#endif
    bool                isDERresponse ;                      // is this packet due to an external DER (MSTP) or internal unsolicited (App) event?
    bool                expectingReply;
    char                source;                             // is this packet due to an external (MSTP) or internal (App) event
    uint16_t            lpduMax;                            // This is NOT the MAX APDU !!! it is the maximum available buffer to build into MAX_LPDU)
    uint16_t            optr;
    uint8_t             *Handler_Transmit_Buffer;
    BACNET_NPCI_DATA    npciData2 ;                         // this is only needed to carry expecting_reply for MSTP. Review... todo4
    BACNET_MAC_ADDRESS  phyDest;
    PORT_SUPPORT        *portParams;
} DLCB ;


DLCB *alloc_dlcb_sys(char typ, bool isResponse, PORT_SUPPORT *portParams );
DLCB *dlcb_clone_deep(const DLCB *dlcb);
bool dlcb_check ( DLCB *dlcb ) ;
// DLCB *dlcb_clone_with_copy_of_buffer(const DLCB *dlcb, const uint16_t len, const uint8_t *buffer);

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


// renaming to allow tracing during debugging
#define alloc_dlcb_response(tag, route)     alloc_dlcb_sys(tag, true,  route)
#define alloc_dlcb_application(tag, route)  alloc_dlcb_sys(tag, false, route)
#define alloc_dlcb_app2(tag, route)	        alloc_dlcb_sys(tag, false, route)


//void set_local_broadcast_mac(BACNET_MAC_ADDRESS *mac);
//void set_broadcast_addr_local(BACNET_GLOBAL_ADDRESS *dest);
//void set_broadcast_addr_global(BACNET_GLOBAL_ADDRESS *dest);

char *PortSupportToString( const PORT_SUPPORT *ps);
void bacnet_route_copy(BACNET_ROUTE *rdest, const BACNET_ROUTE *rsrc);
void bacnet_route_clear(BACNET_ROUTE *rdest);
void bitsDatalink_cleanup(void);
void bitsDatalink_tick(void); 
PORT_SUPPORT *datalink_initCommon2(const char *adapter, const BPT_TYPE rpt, const uint16_t maxLPDU);

PORT_SUPPORT *Init_Datalink_IP(const char *ifName, const uint16_t localIPport);
PORT_SUPPORT *Init_Datalink_BBMD(const char *ifName, const uint16_t localIPport);
PORT_SUPPORT *Init_Datalink_FD(const char *ifName, const uint16_t localIPport, const char *remoteName, const uint16_t remoteIPport );
