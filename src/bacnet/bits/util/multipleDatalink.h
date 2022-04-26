/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *   As a special exception, if other files instantiate templates or
 *   use macros or inline functions from this file, or you compile
 *   this file and link it with other works to produce a work based
 *   on this file, this file does not by itself cause the resulting
 *   work to be covered by the GNU General Public License. However
 *   the source code for this file must still be made available in
 *   accordance with section (3) of the GNU General Public License.
 *
 *   For more information : info@bac-test.com
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#pragma once

#include "configProj.h"

#include <stdint.h>

#if (BACDL_MSTP == 1)
#include "mstp.h"
#endif

#include "osLayer.h"
#include "bacnet/npdu.h"
// #include "osNet.h"
#include "bacnet/datalink/bvlc.h"
#include "eLib/util/linklist.h"
#include "eLib/util/llist.h"
#include "bacnet/bacdef.h"
#include "bacnet/bits/util/bitsIpUtil.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/bbmd/h_bbmd.h"
#include "bacnet/bits/util/multipleDatalink.h"

typedef struct Address_Cache_Entry  ADDRESS_CACHE_ENTRY ;
typedef struct _PORT_SUPPORT PORT_SUPPORT;

typedef struct _virtDevInfo VirtualDeviceInfo;

// 2018.09.17 PF_xxxx collided with Microsoft names, renaming BPT_xxxx
typedef enum {
#if (BACDL_BIP == 1)
    BPT_BIP,
#endif
#if (BACDL_MSTP == 1)
    BPT_MSTP,
#endif
#if (BACDL_ETHERNET == 1)
    BPT_Ethernet,
#endif
#if (BACDL_BBMD == 1)
    BPT_BBMD,
#endif
#if (BACDL_FD == 1)
    BPT_FD,
#endif
#if ( BITS_ROUTER_LAYER == 1)
    BPT_VIRT,
    BPT_APP
#endif
} BPT_TYPE;


typedef struct _BIP_PARAMS {
    SOCKET socket;
    uint16_t nwoPort;                           // network order
    uint32_t nwoLocal_addr;                     // network order

    uint32_t nwoBroadcast_addr;                 // network order
    uint32_t nwoSubnet_addr;
    uint32_t nwoNetmask_addr;

#if ( BACDL_BBMD == 1 )
    bool BVLC_NAT_Handling;                     // for NAT BBMD handling
    struct sockaddr_in BVLC_Global_Address;
#endif

#if ( BACDL_FD == 1 )
    struct sockaddr_in  fd_ipep;                // For FD registration
    uint16_t            fd_timetolive;
    uint16_t            fd_timeRemaining;
#endif

#if ( BACDL_BBMD == 1 )
    BBMD_TABLE_ENTRY *BBMD_Table;
    FD_TABLE_ENTRY *FD_Table;
#endif

} BIP_PARAMS;


#if ( BACDL_MSTP == 1 )
typedef struct {
    //uint32_t    baudrate;
    //uint8_t     databits;
    //uint8_t     stopbits;
    //uint8_t     max_master;
    //uint8_t     max_frames;
    // uint8_t     myAddress;
    mstp_port_struct mstpPort;
} MSTP_PARAMS;
#endif

// this will ultimately have all sorts of performance data for all sorts
// of datalinks



typedef struct _PORT_SUPPORT PORT_SUPPORT;
typedef struct _PORT_SUPPORT DATALINK_SUPPORT;
typedef struct _BACNET_ROUTE BACNET_ROUTE;
typedef struct _DLCB         DLCB;

struct _PORT_SUPPORT {
    LLIST   llist;                  // awkward - need to maintain 2 lists, one for baseline ports, another for routerports.

    uint8_t datalinkId;
    BPT_TYPE portType;
    const char* ifName ;            // hmmm, this name may be ephemeral in the calling function, may need to make a copy here! // todo 0

    union
    {
        BIP_PARAMS bipParams;
#if ( BACDL_MSTP == 1 )
        MSTP_PARAMS mstpParams;
#endif
    } datalink;

    void    *dlSpecific;

    uint8_t *txBuff;		// this is the buffer used by the datalink send routine to prepare the LPDU. (eg BVLC).
	uint16_t max_lpdu;		// Largest packet that the datalink can send, this is max available space for all BACnet PDUs
	uint16_t max_apdu;      // todo 0 - I don't think this is relevant when we blend in client side needs too - investigate
                            // apdu is not just a datalink parameter - it is also a device parameter, 
                            // which is to default to the pDev datalink, but then update as more and more information about 
                            // the remote bacnet device is discovered

    void (*SendPdu) (
        PORT_SUPPORT*   datalink,       // no longer const, because e.g. stats, queues can be modifiable
        const DLCB*     dlcb );

    uint16_t(*ReceiveMPDU) (
        PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *src,
        uint8_t *pdu,
        uint16_t maxlen
        );

    bool (*isActive) (
        PORT_SUPPORT *portParams
        );

    void (*get_MAC_address) (
        const struct _PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *my_address);
    
    void(*Datalink_Maintenance)(
        PORT_SUPPORT *portParams);

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1)
    // todo 3 - use dynamic allocation for this table... - either linked list, or realloc table
    ADDRESS_CACHE_ENTRY     *Address_Cache ;
#endif

    BACNET_MAC_ADDRESS *localMAC;
} ;


typedef struct _BACNET_ROUTE {
    PORT_SUPPORT    *portParams;
    BACNET_PATH     bacnetPath;
} BACNET_ROUTE;

typedef struct
{
    PORT_SUPPORT *portParams;

    BACNET_PATH         srcPath;

    uint8_t             *npdu;
    uint16_t            npdu_len;

} RX_DETAILS;


typedef struct _DLCB
{
    LLIST_LB            *next;                      // Must be first. This is included here because most DLCBs end up on a queue (MS/TP and FT).

#if ( BAC_DEBUG == 1 )
    uint8_t signature;
#endif
    bool                expectingReply;
    bool                isBroadcastWhoIs ;
    char                source;                     // is this packet due to an external (MSTP) or internal (App) event
    uint16_t            lpduMax;                    // This is NOT the MAX APDU !!! it is the maximum available buffer to build into)
    uint16_t            optr;
    uint8_t             *Handler_Transmit_Buffer;
    BACNET_PATH         bacnetPath;                 // Especially in the router case, we do not want to carry port infomation around in the DLCB, which is going to be cloned like crazy during e.g. broadcasts
} DLCB ;


#ifdef replaced_by_dlcb
typedef struct
{
    int                 block_valid;
    BACNET_MAC_ADDRESS phyDest;             // this is the locally connected device's MAC address (IPaddress in IP case, 1 byte in MSTP case)
    uint8_t *pdu;
    uint16_t pdu_len;
} TX_DETAILS;
#endif

void noop_get_broadcast_BACnetAddress(
    const PORT_SUPPORT *portParams,
    BACNET_GLOBAL_ADDRESS * dest);

#if ( BAC_DEBUG == 1 )
bool dlcb_check(const DLCB *dlcb);
#endif

DLCB *dlcb_sys_alloc(char typ, bool isAresponse, const BACNET_PATH *portParams, const uint16_t size );
void dlcb_free(const DLCB *dlcb);
DLCB *dlcb_clone_deep(const DLCB *dlcb);
DLCB *dlcb_clone_with_copy_of_buffer(const DLCB *dlcb, const uint16_t len, const uint8_t *buffer);
DLCB *dlcb_alloc(char type, uint16_t bufSize);
DLCB *dlcb_alloc_with_buffer_clone(uint16_t len, uint8_t *buffer);

// renaming to allow tracing during debugging
#define alloc_dlcb_response(tag, bacnetPath, size)      dlcb_sys_alloc(tag, true,  bacnetPath, size)
#define alloc_dlcb_application(tag, bacnetPath, size)   dlcb_sys_alloc(tag, false, bacnetPath, size)
#define alloc_dlcb_app2(tag, bacnetPath, size)	        dlcb_sys_alloc(tag, false, bacnetPath, size)


void set_local_broadcast_mac(BACNET_MAC_ADDRESS *mac);

char *PortSupportToString( const PORT_SUPPORT *ps);
void bacnet_route_copy(BACNET_ROUTE *rdest, const BACNET_ROUTE *rsrc);
void bacnet_route_clear(BACNET_ROUTE *rdest);
// void datalink_idle(void);
void datalink_cleanup(void);

PORT_SUPPORT *datalink_initCommon(
	const char *adapter,
	const BPT_TYPE rpt);

void datalink_destroyCommon(PORT_SUPPORT *datalink);

PORT_SUPPORT *Init_Datalink_IP(const char *ifName, const uint16_t localIPport);
PORT_SUPPORT *Init_Datalink_BBMD(const char *ifName, const uint16_t localIPport);
PORT_SUPPORT *Init_Datalink_FD(const char *ifName, const char *remoteName, const uint16_t remoteIPport );
PORT_SUPPORT *Init_Datalink_MSTP(const char *ifName, const uint16_t stationNo);
PORT_SUPPORT *Init_Datalink_Ethernet(const char *ifName);

#if ( BITS_ROUTER_LAYER == 0 )
    // we only use this init for non-routing applications
PORT_SUPPORT *InitDatalink(
    const BPT_TYPE rpt, 
    const char *adapter,
    const uint16_t ipPort);
#endif

void bits_set_port_params(
	PORT_SUPPORT *portParams,
    IP_ADDR_PARAMS *portIpParams);

bool bits_Datalink_isActive_IP(PORT_SUPPORT *portParams);
bool bits_Datalink_isActive_NoOp(PORT_SUPPORT *portParams);
void IP_Datalink_Watch_IP_Address(PORT_SUPPORT *datalink);
void Datalink_Initialize_Thread(PORT_SUPPORT *datalink);



