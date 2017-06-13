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
*   This file contains changes made by ConnectEx, Inc. If published,
*   these changes are subject to the permissions, warranty
*   terms and limitations above. If not published, then these terms
*   apply to ConnectEx, Inc's customers to whom the code has
*   been supplied. For more details info@connect-ex.com
*   Where appropriate, the changes are Copyright (C) 2014-2017 ConnectEx, Inc.
*
*********************************************************************/

#ifndef DATALINK_H
#define DATALINK_H

#include "config.h"
#include "bacdef.h"
#include "net.h"
#include "linklist.h"
#include "npdu.h"
// #include "bvlc.h"
#include "bbmd.h"

typedef enum {
    DL_BIP,
    DL_MSTP,
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    DL_BBMD,
    DL_FD,
#endif
    DL_APP
} DL_TYPE;


typedef struct {
        int socket;
        uint16_t nwoPort;                           // network order
        uint32_t nwoLocal_addr ;                    // network order
#if defined(BBMD_ENABLED) && BBMD_ENABLED
        uint32_t nwoBroadcast_addr ;                // network order
#endif
        // uint32_t netmask_not_used_todo3;
        bool BVLC_NAT_Handling;                     // for NAT BBMD handling

        struct sockaddr_in  fd_ipep;                  // For FD registration
        uint16_t            fd_timetolive;            
        uint16_t            fd_timeRemaining;

        struct sockaddr_in BVLC_Global_Address;
    } BIP_PARAMS ;


typedef struct {
        uint32_t baudrate;
        uint8_t databits;
        uint8_t stopbits;
        uint8_t max_master;
        uint8_t max_frames;
    } MSTP_PARAMS ;

typedef struct _PORT_SUPPORT DLINK_SUPPORT;

typedef struct
{
#if ( BAC_DEBUG == 1 )
  uint8_t signature ;
#endif

    char                source ;                    // is this packet due to an external (MSTP) or internal (App) event
    uint16_t            bufSize;
    uint8_t             *Handler_Transmit_Buffer;
    BACNET_NPDU_DATA    npciData ;                  // this is only needed to carry expecting_reply for MSTP. Review... todo4
    BACNET_MAC_ADDRESS  phyDest;
    const DLINK_SUPPORT *portParams;
} DLCB ;


struct _PORT_SUPPORT {
    LLIST   llist;         
    uint8_t port_id2;      /* different for every router port */
    DL_TYPE portType;

    union
    {
        BIP_PARAMS bipParams;
        MSTP_PARAMS mstpParams;
    }  ;

    uint8_t *txBuff;    // this is the buffer used by the datalink send routing to prepare the MMPDU. (eg BVLC).   
    uint16_t max_apdu;  // note, this is smaller than the required buffer size !

#if ( BBMD_ENABLED == 1 )
    // todo3 all these should be in bip_params above..
    BBMD_TABLE_ENTRY    BBMD_Table[MAX_BBMD_ENTRIES];
    FD_TABLE_ENTRY      FD_Table[MAX_FD_ENTRIES];
#endif

    void (*SendPdu) (
        const DLINK_SUPPORT          *portParams,
        const BACNET_MAC_ADDRESS    *bacnetMac,
        const BACNET_NPDU_DATA      *npdu_data,
        const DLCB                  *dlcb);

    uint16_t (*ReceiveMPDU) ( 
        DLINK_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *src,
        uint8_t *pdu, 
        uint16_t maxlen
        );

    void (*get_MAC_address) (
        const struct _PORT_SUPPORT *portParams,
        BACNET_MAC_ADDRESS *my_address);

} ; // PORT_SUPPORT ;

typedef struct {
    const DLINK_SUPPORT  *portParams;
    BACNET_PATH         bacnetPath;
} BACNET_ROUTE;

typedef struct
{
    char                  block_valid;

    const DLINK_SUPPORT        *portParams;

    BACNET_PATH         srcPath ;

    uint8_t             *rxBuf ;
    uint16_t            len ;

} RX_DETAILS ;




DLCB *alloc_dlcb_sys(char typ, const DLINK_SUPPORT *portParams );
void dlcb_free(const DLCB *dlcb);
DLCB *dlcb_deep_clone(const DLCB *dlcb);

// renaming to allow tracing during debugging
#define alloc_dlcb_response(tag, portParams)        alloc_dlcb_sys(tag, portParams)
#define alloc_dlcb_application(tag, portParams)     alloc_dlcb_sys(tag, portParams)
#define alloc_dlcb_app2(tag, portParams)	        alloc_dlcb_sys(tag, portParams)

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

char *PortSupportToString( const DLINK_SUPPORT *ps);
void bacnet_route_copy(BACNET_ROUTE *rdest, const BACNET_ROUTE *rsrc);
void bacnet_route_clear(BACNET_ROUTE *rdest);

DLINK_SUPPORT *InitDatalink(DL_TYPE rpt, uint16_t nwoPort);
void datalink_cleanup(void);

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
                                                                                                                                                                                              *//** @defgroup DLTemplates DataLink Template Functions
 * @ingroup DataLink
 * Most of the functions in this group are function templates which are assigned
 * to a specific DataLink network layer implementation either at compile time or
 * at runtime.
 */
#endif
