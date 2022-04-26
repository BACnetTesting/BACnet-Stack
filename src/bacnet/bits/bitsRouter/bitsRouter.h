/****************************************************************************************
 *
 *   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
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

#include <stdint.h>
#include "configProj.h"
#include "bacnet/bacaddr.h"
#include "bacnet/bits/util/multipleDatalink.h"

#if ( VIRTUALDEV > 0 )
#define PORTID_VIRT 101 
#endif

/* information for routing table */
typedef struct _dnet {
    //  uint8_t mac[MAX_MAC_LEN];
    //  uint8_t mac_len;
    BACNET_MAC_ADDRESS phyMac;
    uint16_t net;
    uint32_t timer;
    bool busy2; /* router busy or not */
    struct _dnet *next;
} DNET;

typedef struct _routing_table_entry {
    DNET configuredNet;		// The one and only configured NN for this port
    DNET *dnets2;			// the discovered NNs
} RT_ENTRY;

typedef struct _port ROUTER_PORT ;

struct _port {
    LLIST           llist;              // must be first
    uint8_t port_id;      /* different for every router port */
    RT_ENTRY        route_info;
    PORT_SUPPORT    *port_support;      // Cast this to PORT_SUPPORT *

    void(*forward_network_message) (ROUTER_PORT *port, RX_DETAILS *rxDetails, BACNET_NPCI_DATA *npci, BACNET_MAC_ADDRESS *mac,
        BACNET_GLOBAL_ADDRESS *dAdr, BACNET_GLOBAL_ADDRESS *sAdr, const DLCB *dlcb);

    LLIST_HDR datalinkDevicesVirtual;

};



typedef struct {
    BACNET_GLOBAL_ADDRESS       src_in_rxDetails;                // processed, or created for router layer
    BACNET_GLOBAL_ADDRESS       dest;
    BACNET_NPCI_DATA            npdu_data;

    ROUTER_PORT *sourceRouterPort;
    RX_DETAILS  *rxDetails;

    uint8_t     *bpdu;                              // payload of npdu -> apdu or network message
    uint16_t    bpdu_len;

} ROUTER_MSG;


typedef struct devObj_s DEVICE_OBJECT_DATA;

typedef struct _virtDevInfo {
    LLIST_HDR llist;

    DEVICE_OBJECT_DATA *pDev;   // This is only valid if the virtual device happens to be an Application Entity too (i.e. not a router).
                                // however, the virtual router case has not been handled yet, (it implies cascade of routers)
                                // so for now, this pointer should always be valid.

    BACNET_MAC_ADDRESS virtualMACaddr ;        // what all the fuss is about!

} VirtualDeviceInfo ;

extern ROUTER_PORT* applicationRouterPort;
extern DEVICE_OBJECT_DATA *routerApplicationEntity;

bool InitRouterportApp(int associatedNetworkNumber);
bool InitRouterport(const uint8_t portId, const BPT_TYPE type,  const char*adapter, const uint16_t networkNumber, const uint16_t ipPort ) ;
bool InitRouterportWithNAT(const uint8_t portId, BPT_TYPE type, const char *adapter, uint16_t networkNumber, uint16_t nwoPort, struct sockaddr_in *globalIPEP );
bool InitRouterportForeignDevice(const uint8_t portId, const char *adapter, uint16_t networkNumber, const char* remoteName, const uint16_t remoteIPport, uint16_t ttlSecs );
bool InitRouterportVirtual(const uint8_t portId, uint16_t networkNumber);

bool AlignApplicationWithPort(void);
void handle_npdu_router(ROUTER_MSG *rmsg);
bool IsDNETbusy(DNET *dnet);

uint16_t get_next_free_dnet(void);

void Virtual_Router_Init(
    uint32_t deviceInstance,
    const char *deviceName,
    const char *deviceDescription);

ROUTER_PORT *find_routerport_from_net(
    const uint16_t net,
    BACNET_MAC_ADDRESS * addr);

ROUTER_PORT* find_routerport_from_portID(
    const uint16_t portID);

/* add reachable network for specified router port */
void add_dnet(
    RT_ENTRY * route_info,
    uint16_t net,
    BACNET_MAC_ADDRESS * addr);

void remove_dnet(
    RT_ENTRY * route_info,
    uint16_t net
);

DNET *find_dnet(uint16_t net);
void InitDnet(DNET *dnet, uint16_t networkNumber);
void RoutingUtilInit(void);

void cleanup_dnets(
    DNET * dnets);

VirtualDeviceInfo* Device_Find_VirtualDevice(
    const uint32_t deviceInstance);

void Device_Remove_VirtualDevice(
    const uint32_t deviceInstance);

VirtualDeviceInfo* Create_Device_Virtual(
    const uint8_t portId, 
    const unsigned devInstance, 
    const char* devName, 
    const char* devDesc, 
    const unsigned vendorId, 
    const char* vendorName);

// in menuDiags.h  void ShowRouterports(void);
extern void update_network_number_cache(ROUTER_PORT* srcport, uint16_t net, BACNET_MAC_ADDRESS* routerMac);
