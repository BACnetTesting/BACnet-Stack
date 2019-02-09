/**************************************************************************

    Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.

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

    For more information : info@bac-test.com
        For access to source code : info@bac-test.com
            or www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#pragma once

#include <stdint.h>
#include "npdu.h"
#include "net.h"
#include "bbmd.h"
#include "linklist.h"
#include "llist.h"
#include "bacdef.h"

// BACnet Port Type
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


typedef struct {
    int socket;
    uint16_t nwoPort;                           // network order
    uint32_t nwoLocal_addr;                    // network order
    
#if defined(BBMD_ENABLED) && BBMD_ENABLED
    uint32_t nwoBroadcast_addr;                // network order
#endif

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

    LLIST_HDR   outputQueue ;

    int(*SendPdu) (
// not at all sure about this        PORT_SUPPORT                *portParams,
// not at all sure about this        const BACNET_MAC_ADDRESS    *bacnetMac,
// not at all sure about this        const BACNET_NPCI_DATA      *npdu_data,
        DLCB                  *dlcb);
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

} ;

typedef struct _BACNET_ROUTE {
    PORT_SUPPORT        *portParams;
    BACNET_PATH         bacnetPath;
} BACNET_ROUTE;

typedef struct
{
    char                block_valid;

    PORT_SUPPORT        *portParams;

    BACNET_PATH         srcPath;

    uint8_t             *npdu;
    uint16_t            npdu_len;

} RX_DETAILS;


typedef struct _DLCB
{
#if ( BAC_DEBUG == 1 )
    uint8_t signature;
#endif
    bool                expectingReply;
    char                source;                     // is this packet due to an external (MSTP) or internal (App) event
    uint16_t            lpduMax;                    // This is NOT the MAX APDU !!! it is the maximum available buffer to build into MAX_LPDU)
    uint16_t            optr;
    uint8_t             *Handler_Transmit_Buffer;
    BACNET_NPCI_DATA    npciData2 ;                      // this is only needed to carry expecting_reply for MSTP. Review... todo4
    BACNET_ROUTE        route;                          // contains phyDest (route.bacnetPath.localMac), portParams
} DLCB ;

DLCB *alloc_dlcb_sys(char typ, bool isResponse, const BACNET_ROUTE *route );
void dlcb_free(DLCB *dlcb);
DLCB *dlcb_clone_deep(const DLCB *dlcb);
DLCB *dlcb_clone_with_copy_of_buffer(DLCB *dlcb, const uint16_t len, const uint8_t *buffer );

#if ( BAC_DEBUG == 1 )
bool dlcb_check ( const DLCB *dlcb ) ;
#endif

// duplicat defs, AND be aware that portParams replaced using route
// renaming to allow tracing during debugging
//#define alloc_dlcb_response(tag, portParams)        alloc_dlcb_sys(tag, true, portParams)
//#define alloc_dlcb_application(tag, portParams)     alloc_dlcb_sys(tag, false, portParams)
#define alloc_dlcb_response(tag, route)     alloc_dlcb_sys(tag, true,  route)
#define alloc_dlcb_application(tag, route)  alloc_dlcb_sys(tag, false, route)
#define alloc_dlcb_app2(tag, route)	        alloc_dlcb_sys(tag, false, route)

void SendIAmRouter(void);
void SendNetworkNumberIs(void);
void SendWhoIsRouter(void);
void dump_router_table(void);
void Send_I_Am_Broadcast(void);
void set_local_broadcast_path(BACNET_PATH *dest);
void set_global_broadcast_path(BACNET_PATH *dest);
void set_local_broadcast_mac(BACNET_MAC_ADDRESS *mac);
void set_broadcast_addr_local(BACNET_GLOBAL_ADDRESS *dest);
void set_broadcast_addr_global(BACNET_GLOBAL_ADDRESS *dest);

char *PortSupportToString( const PORT_SUPPORT *ps);
void bacnet_route_copy(BACNET_ROUTE *rdest, const BACNET_ROUTE *rsrc);
void bacnet_route_clear(BACNET_ROUTE *rdest);
