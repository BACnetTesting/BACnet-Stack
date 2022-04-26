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
 *   For more information : info@bac-test.com
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#include "configProj.h"

#if ( BITS_ROUTER_LAYER == 1)

#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "eLib/util/emm.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "eLib/util/btaDebug.h"
#include "eLib/util/logging.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "osLayer.h"
#include "bacnet/basic/object/device.h"
#include "eLib/util/timerCommon.h"
#include "bacnet/basic/object/netport.h"
#include "bacnet/basic/service/h_apdu.h"
#include "eLib/util/eLibUtil.h"
#include "bacnet/datalink/ethernet.h"

void create_router_flow_control_msg(BACNET_NETWORK_MESSAGE_TYPE mt,
    ROUTER_PORT* srcport);

static void create_i_am_router_network_message(ROUTER_PORT* port,
    uint16_t* networkNumber);

static void create_router_table_ack(ROUTER_PORT* destport,
    BACNET_MAC_ADDRESS* phyMac, BACNET_GLOBAL_ADDRESS* destaddr,
    void* val);

static void create_network_number_is_network_message(ROUTER_PORT* port);

static void init_network_npci(BACNET_NPCI_DATA* npci_data,
    bool data_expecting_reply);

static void SendToPort(const ROUTER_PORT* routerPort, DLCB* dlcb);

static void virtual_send_npdu(
    PORT_SUPPORT *datalink,
    const DLCB *dlcb);

PORT_SUPPORT* applicationDatalink;
PORT_SUPPORT* virtualDatalink;

extern bool bacnetShuttingDown;

DEVICE_OBJECT_DATA *routerApplicationEntity;

#define MAX_TXBUF       (1497-8)

uint16_t    search_nn;
const DLCB* search_dlcb;
uint32_t                search_start_time;
const ROUTER_PORT       *search_src_port;
BACNET_GLOBAL_ADDRESS   search_src_address;
BACNET_GLOBAL_ADDRESS   search_dest_address;
BACNET_NPCI_DATA        search_npdu_data;

ROUTER_PORT* headRouterPort;

ROUTER_PORT *associatedRouterPort;
ROUTER_PORT *applicationRouterPort;

// todo 2 - this is a debug counter, surround with ifdef BAC_DEBUG
extern int emmCount;

bits_mutex_extern(bacnetStackMutex);


void create_reject(const ROUTER_PORT* port, const BACNET_GLOBAL_ADDRESS* dest,
    BACNET_NETWORK_REJECT_REASONS rejectReason, uint16_t networkNumber) {
    int16_t buff_len;
    DLCB* outgoingMsgData = dlcb_alloc('H', MAX_LPDU_IP);
    BACNET_NPCI_DATA npci_data;

    if (!outgoingMsgData) {
        panicDesc("m0014");
        return;
    }
    init_network_npci(&npci_data, false);
    outgoingMsgData->bacnetPath.localMac.len = 0; // this will tell send routine we want to broadcast // todo 4 - use method():  setbroadcast mac?

    // and now to encode
    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer, NULL,
        NULL, &npci_data);

    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)rejectReason);
    // note, networkNumber always expected, even though it is sometimes not significant (e.g. reject 'cos unknown message type)
    buff_len += encode_unsigned16(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len, networkNumber);
    outgoingMsgData->optr = buff_len;
    SendToPort(port, outgoingMsgData);
}

bool IsDNETbusy(DNET* dnet) {
    if (dnet->busy2) {
        // actively timing, but we may have timed out...
        if (bits_sysTimer_elapsed_milliseconds(dnet->timer) > 30000) {
            dnet->busy2 = false;
        }
    }
    return dnet->busy2;
}


bool IsNetworkBusy(uint16_t networkNumber) {
    DNET *dnet = find_dnet(networkNumber);
    if (dnet == NULL) {
        return false;
    }
    return IsDNETbusy(dnet);
}


void create_who_is_router_network_message(
    const ROUTER_PORT *port,
    const uint16_t *networkNumber)
{
    int16_t buff_len;
    BACNET_NPCI_DATA npci_data;
    //BACNET_GLOBAL_ADDRESS dest;

    if (port->port_support->portType == BPT_APP
        || port->port_support->portType == BPT_VIRT) {
        return;
    }

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('R', MAX_LPDU_IP);
    if (!outgoingMsgData) {
        panicDesc("m0822");
        return;
    }

    init_network_npci(&npci_data, false);

    outgoingMsgData->bacnetPath.localMac.len = 0; // this will tell send routine we want to broadcast // todo 4 - setbroadcast()?

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer, NULL,
        NULL, &npci_data);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK);

    if (networkNumber != NULL) {
        // this means we are responding to a specific request
        buff_len += encode_unsigned16(
            outgoingMsgData->Handler_Transmit_Buffer + buff_len,
            *networkNumber);
    }

    dbMessage(DBD_RouterOperations,
        DB_UNUSUAL_TRAFFIC,
        "Sending Who-Is-Router to send to portId %d, IPport %d,  NN %d",
        port->port_id,
        ntohs(port->port_support->datalink.bipParams.nwoPort),
        port->route_info.configuredNet.net);

    outgoingMsgData->optr = buff_len;
    SendToPort(port, outgoingMsgData);

}


void SendToPort(const ROUTER_PORT* routerPort, DLCB* dlcb) {
    routerPort->port_support->SendPdu( routerPort->port_support, dlcb);
}


// forward a network message, insert DADR if required
static void router_forward_network_message(ROUTER_PORT* port,
    RX_DETAILS* rxDetails, BACNET_NPCI_DATA* incoming_npdu_data,
    BACNET_MAC_ADDRESS* localDest, BACNET_GLOBAL_ADDRESS* npciDest,
    BACNET_GLOBAL_ADDRESS* origSrc, const DLCB* dlcb) {
    int16_t buff_len;

#if ( BAC_DEBUG == 1)
    if (dlcb == NULL) {
        panic();
        return;
    }
#endif

    // Make a local copy of the src Address, we may be overwriting it with a zero later, and
    // we don't want to break all the other distributions that use the original source
    BACNET_GLOBAL_ADDRESS localSrc;
    bacnet_address_copy(&localSrc, origSrc);

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('R', MAX_LPDU_IP);
    if (!outgoingMsgData) {
        dlcb_free(dlcb);
        panic();
        return;
    }

    bacnet_mac_copy(&outgoingMsgData->bacnetPath.localMac, localDest);

    outgoingMsgData->expectingReply = incoming_npdu_data->data_expecting_reply;

    // Not in spec: If our src net is the same as our connected net, remove it (this happens when local app sends a packet, the src is preloaded in case
    // the destination is the other side of the router
    // but this makes our router to appear to fail (which imho is better than crashing the whole network). Need to make a HIGHLY visible warning
    // 
    if (localSrc.net == port->route_info.configuredNet.net) {
        localSrc.net = 0;
    }

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer,
        npciDest, &localSrc, incoming_npdu_data);

    memcpy(&outgoingMsgData->Handler_Transmit_Buffer[buff_len],
        dlcb->Handler_Transmit_Buffer, dlcb->optr);
    outgoingMsgData->optr = buff_len + dlcb->optr;

    dlcb_free(dlcb);

    SendToPort(port, outgoingMsgData);
}


// distributes network pdu to all ports EXCEPT the referred one, if it is indeed referred
static void distribute_local_broadcast_message(
    ROUTER_PORT* exceptThisPort,
    RX_DETAILS* rxDetails,
    BACNET_GLOBAL_ADDRESS* dest,
    BACNET_GLOBAL_ADDRESS* src,
    BACNET_NPCI_DATA* npci_data,
    const DLCB* dlcb)
{
    ROUTER_PORT* destPort = headRouterPort;
    BACNET_MAC_ADDRESS destMac;

    // make sure destMac indicates broadcast (clearing does this).
    bacnet_mac_clear(&destMac);

    while (destPort != NULL) {
        if (exceptThisPort != NULL && destPort == exceptThisPort) {
            // not interested in sending out this port
            destPort = (ROUTER_PORT*)destPort->llist.next;
            continue;
        }

        DLCB* newDLCB = dlcb_clone_deep(dlcb);
        if (newDLCB != NULL) {
            destPort->forward_network_message(destPort, rxDetails, npci_data,
                &destMac, dest, src, newDLCB);
        }
        else {
            dlcb_free(dlcb);
            return;
        }

        destPort = (ROUTER_PORT *)destPort->llist.next;
    }
    dlcb_free(dlcb);
}


static void ProcessLocalApplicationMessage(
    PORT_SUPPORT* portParams,
    RX_DETAILS* rxDetails, 
    BACNET_NPCI_DATA* npci_data,
    BACNET_GLOBAL_ADDRESS* dest, 
    BACNET_GLOBAL_ADDRESS* src,
    const DLCB* dlcb) 
{
    BACNET_ROUTE srcRoute;

    bacnet_mac_copy(&srcRoute.bacnetPath.localMac, &rxDetails->srcPath.localMac);
    srcRoute.bacnetPath.glAdr = *src;
    srcRoute.portParams = portParams;

    // BUT we are not going to answer to the originating port, but the virtual port that feeds the router layer...
    apdu_handler( routerApplicationEntity,
        &srcRoute,
        dlcb->Handler_Transmit_Buffer,
        dlcb->optr);

    dlcb_free(dlcb);
}


static void forward_router_message_to_application(ROUTER_PORT* port,
    RX_DETAILS* rxDetails, BACNET_NPCI_DATA* incoming_npdu_data,
    BACNET_MAC_ADDRESS* localDest, BACNET_GLOBAL_ADDRESS* npciDest,
    BACNET_GLOBAL_ADDRESS* src, const DLCB* dlcb) {
#if 0 
    SendBTAmessage("Processing Local Application Tx/Rx");
    count++;
    if (count == 2) {
        count++;
    }
#endif

    ProcessLocalApplicationMessage(port->port_support, rxDetails,
        incoming_npdu_data, npciDest, src, dlcb);
}

static void VirtualLayerProcessNPDU(
    ROUTER_PORT* routerPort,
    RX_DETAILS* rxDetails,
    BACNET_NPCI_DATA* incoming_npdu_data,
    BACNET_MAC_ADDRESS* localDest,
    BACNET_GLOBAL_ADDRESS* dest,
    BACNET_GLOBAL_ADDRESS* src,
    const DLCB* dlcb) {
    BACNET_ROUTE srcRoute;

    bacnet_address_copy(&srcRoute.bacnetPath.glAdr, src);
    bacnet_mac_copy(&srcRoute.bacnetPath.localMac,
        &rxDetails->srcPath.localMac);
    srcRoute.portParams = routerPort->port_support;

    // Is this a broadcast? It could be a global or a remote broadcast, but we have already checked for Network Number match for Remote,
    // and for 0xffff for Global, now what we have left to test is only the local MAC for broadcast.
    if (localDest->len == 0 ) {
        for (VirtualDeviceInfo* vDev = (VirtualDeviceInfo*)ll_First(&routerPort->datalinkDevicesVirtual);
            vDev != NULL;
            vDev = (VirtualDeviceInfo*)ll_Next(&routerPort->datalinkDevicesVirtual, vDev)) {
            // only process virtual devices
            DEVICE_OBJECT_DATA* pDev = vDev->pDev;
            if (pDev->datalink->portType != BPT_VIRT) continue;

            // acquire mac address here for eventual response
            pDev->datalink->localMAC = &((VirtualDeviceInfo *)pDev->userData)->virtualMACaddr;
            apdu_handler(pDev, &srcRoute, dlcb->Handler_Transmit_Buffer, dlcb->optr);
        }
    }
    else {
        // not so, need to find the matching MAC address
        for (VirtualDeviceInfo* vDev = (VirtualDeviceInfo*)ll_First(&routerPort->datalinkDevicesVirtual);
            vDev != NULL;
            vDev = (VirtualDeviceInfo*)ll_Next(&routerPort->datalinkDevicesVirtual, vDev)) {
            // only process virtual devices
            // done in for ... loop VirtualDeviceInfo *vDev = *iDev;
            DEVICE_OBJECT_DATA* pDev = vDev->pDev;
            if (pDev->datalink->portType != BPT_VIRT) continue;

            if (bacnet_mac_same(localDest, &vDev->virtualMACaddr)) {
                // acquire mac address here for eventual response
                pDev->datalink->localMAC = &((VirtualDeviceInfo *)pDev->userData)->virtualMACaddr;
                apdu_handler(pDev, &srcRoute, dlcb->Handler_Transmit_Buffer, dlcb->optr);
            }
        }
    }
    dlcb_free(dlcb);
}


static void LocalBcastMsgToFarPorts(ROUTER_PORT* nearPort,
    BACNET_NPCI_DATA* npci_data, uint8_t* apdu, int apdu_len) {
    uint16_t buff_len;

    ROUTER_PORT* destport = headRouterPort;
    while (destport != NULL) {
        if (destport != nearPort) {
            /* allocate message structure */
            DLCB* outgoingMsgData = dlcb_alloc('H', MAX_LPDU_IP); // todo 2 optimize by using datalink's max buffer size
            if (!outgoingMsgData) {
                // and of course we hope there is enough memory for dbMessage... but that function will deal with starvation too..
                dbMessage ( DBD_ALL, DB_ALWAYS, "m0023 - Error: Could not allocate memory");
                return;
            }

            set_local_broadcast_mac(
                &outgoingMsgData->bacnetPath.localMac);

            buff_len = npdu_encode_pdu(
                outgoingMsgData->Handler_Transmit_Buffer, NULL,
                NULL, npci_data);

            memcpy(&outgoingMsgData->Handler_Transmit_Buffer[buff_len],
                apdu, apdu_len);  // todo 0 make safe - this actually applies all over the place, and an audit and big update is probably the best approach
            buff_len += apdu_len;
            outgoingMsgData->optr = buff_len;

            SendToPort(destport, outgoingMsgData);
        }
        destport = (ROUTER_PORT*)destport->llist.next;
    }
}

void update_network_number_cache(ROUTER_PORT* srcport, uint16_t net,
    BACNET_MAC_ADDRESS* routerMac) {

#if ( BAC_DEBUG == 1 )
    if (!bacnet_mac_check(routerMac)) {
        panic();
        return;
    }
#endif

    if (net == 0 || net == 0xffffu) {
        // these are not allowed and we ignore them. Transmitting router is making a mistake.
        dbMessage(DBD_RouterOperations, DB_ERROR,
            "Some other router tried to send illegal Network Number %x to us, we are going to ignore it - 2");
        return;
    }

    // cycle through all the ports
    ROUTER_PORT* port = headRouterPort;
    while (port != NULL) {
        if (port->route_info.configuredNet.net == net) {
            // bah, one of our configured ports seems like a duplicate. I used to remove this devices configuration, but it is a site issue, and 'hiding' the
            // 'bad' configuration does not help troubleshooting. Additionally, this often came about from receiving broadcast I-Am-Router from 'self'. Although
            // there is meant to be a filter for that, I don't feel it is bulletproof at this point. So removing the cancellation, and modifying the message.
            // port->route_info.configuredNet.net = 0;
            dbMessage(DBD_RouterOperations, DB_ERROR,
                "Port %d has a duplicate Network Number %d, received from %s Site configuration incorrect.",
                port->port_id,
                net,
                BACnetMacAddrToString(routerMac));
        }
        else {
            if (port == srcport) {
                add_dnet(&srcport->route_info, net, routerMac); /* and update routing table */
            }
            else {
                remove_dnet(&port->route_info, net);
            }
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
}

void ProcessLocalNetworkLayerMessage(ROUTER_PORT* srcport,
    BACNET_NPCI_DATA* npci_data, BACNET_GLOBAL_ADDRESS* dest,
    BACNET_GLOBAL_ADDRESS* src, const ROUTER_MSG* rtmsg) {
    uint16_t net;
    BACNET_MAC_ADDRESS txAddress;
    ROUTER_PORT* destport;

    // resolve respond-to Address ( we know we are trying to repond to who we received it from ). Two cases
    //      i) via a router
    //      ii) direct

    if (src->net) {
        // arrived via router, case i
        bacnet_mac_copy(&txAddress, &src->mac);
    }
    else {
        // case ii
        bacnet_mac_copy(&txAddress,
            &rtmsg->rxDetails->srcPath.localMac);
    }

    int len = 1;
    BACNET_NETWORK_MESSAGE_TYPE network_message_type =
        (BACNET_NETWORK_MESSAGE_TYPE)rtmsg->bpdu[0];
    /* Message Type field contains a value in the range 0x80 - 0xFF, */
    /* then a Vendor ID field shall be present */
    if (network_message_type >= 0x80) {
        len += decode_unsigned16(&rtmsg->bpdu[len],
            &npci_data->vendor_id);
    }

    switch (network_message_type) {
    case NETWORK_MESSAGE_WHAT_IS_NETWORK_NUMBER:

        dbMessage(DBD_RouterOperations,
            DB_UNUSUAL_TRAFFIC,
            "m0092: Received What-Is-Network-Number message");
        if (dest->net != 0) {
            dbMessage(DBD_RouterOperations,
                DB_UNUSUAL_TRAFFIC,
                "What-Is-Network-Number cannot be directed to remote network. Ignoring.\n");
            return;
        }
        if (src->net != 0) {
            dbMessage(DBD_RouterOperations,
                DB_UNUSUAL_TRAFFIC,
                "What-Is-Network-Number must not be routed, ignoring.\n");
            return;
        }
        // get this far, do a local b'cast back regarding the network number.
        create_network_number_is_network_message(srcport);
        break;

    case NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK:

        dbMessage(DBD_RouterOperations, DB_UNUSUAL_TRAFFIC, "Received Who-Is-Router-To-Network message");

        if (dest->net == BACNET_BROADCAST_NETWORK) {
            dbMessage(DBD_RouterOperations,
                DB_UNUSUAL_TRAFFIC,
                "Someone is doing a global b'cast of Who-Is-Router - not good policy");
        }

        if (len < rtmsg->bpdu_len)
        {
            /* if NET specified */
            len += decode_unsigned16(&rtmsg->bpdu[len], &net);

            if (net == 0 || net == BACNET_BROADCAST_NETWORK) {
                dbMessage(DBD_RouterOperations,
                    DB_UNUSUAL_TRAFFIC,
                    "Illegal Network Number requested, ignoring");
                return;
            }

            // check if we are looking for a directly connected number (superfluous maybe).
            if (srcport->route_info.configuredNet.net == net) {
                dbMessage(DBD_RouterOperations,
                    DB_UNUSUAL_TRAFFIC,
                    "Message discarded: NET directly connected");
                return;
            }

            dbMessage(DBD_RouterOperations,
                DB_UNUSUAL_TRAFFIC,
                "Looking for routerport to network %d\n",
                net);

            destport = find_routerport_from_net(net, NULL); /* see if NET can be reached */
            if (destport) {
                if (destport != srcport) {
                    /* if TRUE send reply */
                    dbMessage(DBD_RouterOperations,
                        DB_UNUSUAL_TRAFFIC,
                        "Sending I-Am-Router-To-Network %d message",
                        net);
                    create_i_am_router_network_message(srcport, &net);
                    return;
                }
                else {
                    // We have just been asked who is router to net for the net associated with the incoming port. Obviously we dont want
                    // to respond to this!
                    dbMessage(DBD_RouterOperations,
                        DB_NOTE,
                        "Asking for Network Number attached to incoming port. Bad situation, ignoring message.");
                    return;
                }
            }

            // if the above search does not find the appropriate network, then..
            dbMessage(DBD_RouterOperations, DB_UNUSUAL_TRAFFIC, "Initiating net search %d", net);

            // From the spec:
            // If the network number is not found in the routing table, the router shall attempt to discover the next router on the path to the
            // indicated  destination  network  by  generating  a  Who - Is - Router - To - Network  message  containing  the  specified  destination
            // network number and broadcasting it out all its ports other than the one from which the Who - Is - Router - To - Network message
            destport = headRouterPort;
            while (destport != NULL) {
                if (destport != srcport) {
                    dbMessage(DBD_RouterOperations,
                        DB_UNUSUAL_TRAFFIC,
                        "Forwarding who-is out of portID %d",
                        destport->port_id);
                    create_who_is_router_network_message(destport,
                        &net);
                }
                destport = (ROUTER_PORT*)destport->llist.next;
            }
        }
        else {
            /* if NET is omitted */
            dbMessage(DBD_RouterOperations,
                DB_UNUSUAL_TRAFFIC,
                "Sending I-Am-Router-To-Network message");
            create_i_am_router_network_message(srcport, NULL);
        }
        break;

    case NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK:
        // these should NEVER be broadcast, and thus should never reach this logic (ProcessLocalNetworkLayerMessage).
        // If they do, we do some defensive stuff, like raise flags, remove networks (to avoid loops)...
    {
        // NOTE: I-Could-Be-Router messages can only be directed!
        dbMessage(DBD_RouterOperations,
            DB_UNUSUAL_TRAFFIC,
            "Received Broadcast/Locally addressed I-Could-Be-Router-To-Network message - this is wrong!");

        // defensively check our tables
        len += decode_unsigned16(&rtmsg->bpdu[len], &net); /* decode received NET values */
        if (net == 0 || net == 0xffff) {
            // these are not allowed and we ignore them. Transmitting router is making a mistake.
            dbMessage(DBD_RouterOperations,
                DB_ERROR,
                "Some other router tried to send illegal Network Number %x to us, we are going to ignore it - 1");
            break;
        }

        // cycle through all the ports
        ROUTER_PORT* port = headRouterPort;
        while (port != NULL) {
            if (port->route_info.configuredNet.net == net) {
                // todo 2 - what does BTL have to say about this?
                // bah, one of our configured ports needs to lose it's configured network number. This is necessary for avoiding looping routes, but indicates
                // a site configuration error
                port->route_info.configuredNet.net = 0;
                dbMessage(DBD_RouterOperations,
                    DB_ERROR,
                    "Port %d just lost its configured network %d, site configuration incorrect.", port->port_id, net);
            }
            remove_dnet(&port->route_info, net);
            port = (ROUTER_PORT*)port->llist.next;
        }
        break;
    }

    case NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK: {
        int net_count = (rtmsg->bpdu_len - len) / 2;
        int i;
        dbMessage(DBD_RouterOperations, DB_UNUSUAL_TRAFFIC, "Received I-Am-Router-To-Network message");
        for (i = 0; i < net_count; i++) {
            decode_unsigned16(&rtmsg->bpdu[len + 2 * i], &net); /* decode received NET values */

            update_network_number_cache(srcport, net,
                &rtmsg->rxDetails->srcPath.localMac);

            // We may be looking for this router (sent a prior who-is-router to network...)
            // if this I-Am-Router is from a Network that we are trying to discover, it is time to forward the postponed message...
            // make a record of the details.
            if (search_nn == net) {
                // forward the message, cancel the pending reject
                search_nn = 0;

                const DLCB* dlcb = dlcb_alloc_with_buffer_clone(
                    rtmsg->bpdu_len, rtmsg->bpdu);
                if (dlcb != NULL) {

                    search_src_port->forward_network_message(srcport,
                        rtmsg->rxDetails, // incomingMsg,
                        &search_npdu_data,
                        &rtmsg->rxDetails->srcPath.localMac, // &incomingMsg->srcPath.localMac,
                        &search_dest_address, &search_src_address,
                        dlcb);
                    //search_nsdu,
                    //search_nsdu_len);

                    dlcb_free(search_dlcb);
                }
                else {
                    panic();
                }
            }
        }

        // From the spec:
        // Whether the router table
        // was  updated  or  not, the  router  shall  then  generate  an  I_Am_Router_To_Network  message  for  all  the  network  numbers
        // contained in the received I_Am_Router_To_Network message and broadcast the new message, using the local broadcast MAC
        // address, out all ports other than the one from which the previous message was received.

        destport = headRouterPort;
        while (destport != NULL) {
            if (destport != srcport) {
                uint16_t val16;
                // todo2 - why do we keep making this switch? surely lower layer (SendToPort) can make the final decision?
                switch (destport->port_support->portType) {
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
                case BPT_BBMD:
#endif
#if (BACDL_FD == 1)
                case BPT_FD:
#endif
                case BPT_BIP:
#if (BACDL_MSTP == 1)
                case BPT_MSTP:
#endif
                    decode_unsigned16(&rtmsg->bpdu[len], &val16);
                    create_i_am_router_network_message(destport,
                        &val16);
                    break;
                case BPT_VIRT:
                case BPT_APP:
                    break;
                default:
                    panic();
                    break;
                }
            }
            destport = (ROUTER_PORT*)destport->llist.next;
        }
        break;
    }

    case NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK:
        // Our router ignores this message
        break;

    case NETWORK_MESSAGE_INIT_RT_TABLE:

        dbMessage(DBD_RouterOperations, DB_UNUSUAL_TRAFFIC, "Received Initialize-Routing-Table message");

        if (rtmsg->bpdu[len] > 0) {
            int net_count = rtmsg->bpdu[len];
            while (net_count--) {
                int i = 1;
                uint8_t portID;
                ROUTER_PORT* port;

                decode_unsigned16(&rtmsg->bpdu[len + i], &net); /* decode received NET values */
                portID = rtmsg->bpdu[len + i + 2];

                if (net == 0 || net == 0xffff) {
                    dbMessage(DBD_RouterOperations, DB_ERROR, "m0015 - Illegal NN in message");
                    continue;
                }

                if (portID == 0) {
                    // Need to remove the network, no discussion..
                    port = headRouterPort;
                    while (port != NULL) {
                        if (port->route_info.configuredNet.net == net) {
                            port->route_info.configuredNet.net = 0;
                            dbMessage(DBD_RouterOperations,
                                DB_ERROR,
                                "Port %d just been instructed to remove configured network %d.", port->port_id, net);
                        }
                        else {
                            remove_dnet(&port->route_info, net);
                        }
                        port = (ROUTER_PORT*)port->llist.next;
                    }
                }
                else {
                    // need to add the new DNET
                    port = headRouterPort;
                    while (port != NULL) {
                        if (port->port_id == portID) {
                            if (port->route_info.configuredNet.net
                                != net) {
                                add_dnet(&port->route_info, net,
                                    &rtmsg->rxDetails->srcPath.localMac); /* and update routing table */
                            }
                        }
                        else {
                            // some other port - make sure a duplicate does not exist
                            if (port->route_info.configuredNet.net == net) {
                                // no choice but to zero that
                                port->route_info.configuredNet.net = 0;
                                dbMessage(DBD_RouterOperations,
                                    DB_ERROR,
                                    "Port %d just been forced to zero its configured network %d.", port->port_id, net);
                            }
                            // and check it is not on the DNETS too
                            remove_dnet(&port->route_info, net);
                        }
                        port = (ROUTER_PORT*)port->llist.next;
                    }
                }

                if (rtmsg->bpdu[len + i + 3] > 0) /* find next NET value */
                {
                    i = i + rtmsg->bpdu[len + i + 3] + 4;
                }
                else {
                    i = i + 4;
                }
            }
            create_router_table_ack(srcport, &txAddress, src, NULL);
        }
        else {
            // need to return the complete table
            create_router_table_ack(srcport, &txAddress, src,
                &rtmsg->bpdu[len]);
        }
        break;

    case NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
        dbMessage(DBD_RouterOperations,
            DB_UNUSUAL_TRAFFIC,
            "Received Initialize-Routing-Table-Ack message");
        if (rtmsg->bpdu[len] > 0) {
            int net_count = rtmsg->bpdu[len];
            while (net_count--) {
                int i = 1;
                decode_unsigned16(&rtmsg->bpdu[len + i], &net); /* decode received NET values */

                add_dnet(&srcport->route_info, net, &txAddress); /* and update routing table */
                if (rtmsg->bpdu[len + i + 3] > 0) /* find next NET value */
                {
                    i = rtmsg->bpdu[len + i + 3] + 4;
                }
                else {
                    i = i + 4;
                }
            }
        }
        break;

    case NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK:
    case NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK:
        // next, forward this message to others
        if ((rtmsg->bpdu_len - len) == 0) {
            // the downstream router is lazy, and has not supplied any network numbers.
            // we need to generate the list of NNs that are busy so we don't end up shutting down the whole network
            create_router_flow_control_msg(network_message_type,
                srcport);
        }
        else {
            uint16_t len2 = 0;
            DNET* tdnet;
            while (len2 < (rtmsg->bpdu_len - len)) {
                uint16_t nn;
                decode_unsigned16(&rtmsg->bpdu[len + len2], &nn);
                tdnet = find_dnet(nn);
                if (tdnet != NULL) {
                    if (network_message_type
                        == NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK) {
                        tdnet->busy2 = true;
                        tdnet->timer = bits_sysTimer_get_milliseconds();
                    }
                    else {
                        // router is now available to network.
                        tdnet->busy2 = false;
                    }
                }
                len2 += 2;
            }
            // And forward the (whole) message, including the already processed byte.
            LocalBcastMsgToFarPorts(srcport, npci_data,
                &rtmsg->bpdu[len - 1], rtmsg->bpdu_len - len + 1);
        }
        break;

    case NETWORK_MESSAGE_INVALID:
    case NETWORK_MESSAGE_ESTABLISH_CONNECTION_TO_NETWORK:
    case NETWORK_MESSAGE_DISCONNECT_CONNECTION_TO_NETWORK:
        dbMessage(DBD_RouterOperations, DB_NOTE, "m0004: Message unsupported");
        /* Don't know what to do with these messages */
        break;

    case NETWORK_MESSAGE_NETWORK_NUMBER_IS:
        // we have just been 'told' what the Network Number for this datalink is. Let us see if we agree...
    {
        uint16_t nn;
        decode_unsigned16(&rtmsg->bpdu[len], &nn);
        if (srcport->route_info.configuredNet.net != nn) {
            dbMessage(
                DBD_Config,
                DB_UNEXPECTED_ERROR,
                "   Received Network Number [%d] conflicts with our configured Network Number [%d] on Router Port [%d].",
                nn,
                srcport->route_info.configuredNet.net,
                srcport->port_id);

            /* what to do about this:
            Option 1:   Replace our configured network number with that sent to us by another device. Who is to say which device is correct? If we were to do this, then
                        the operational NN for our router will not match what our UI shows.
            Option 2: we ignore the fact that someone else has a misconfigured NN. Depend on external tools to identify the conflict.

            I choose option 2.
            */
        }
    }
    break;

    default:
        dbMessage(DBD_RouterOperations, DB_NOTE, "m0095: Network Message [%d] unsupported", network_message_type);
        // if unknown network layer messages are global broadcasts, we have already forwarded, and we don't reject
        if (dest->net != BACNET_BROADCAST_NETWORK) {
            // we don't know what to do with them
            create_reject(srcport, src,
                NETWORK_REJECT_UNKNOWN_MESSAGE_TYPE, 0);
            dbMessage(DBD_RouterOperations, DB_NOTE, "   Sending a Reject as a result.");
        }
        else {
            dbMessage(DBD_RouterOperations, DB_NOTE, "   Cannot send a Reject as original was a broadcast.");
        }
        break;
    }
}

void distribute_npdu(ROUTER_PORT* srcport, RX_DETAILS* rxDetails,
    BACNET_GLOBAL_ADDRESS* dest, BACNET_GLOBAL_ADDRESS* src,
    BACNET_NPCI_DATA* npci_data, const DLCB* dlcb);
// extern uint8_t This_Station;  // MSTP MAC address

static void Application_get_MAC_address(
    const PORT_SUPPORT* portParams,
    BACNET_MAC_ADDRESS* my_address) {
    (void)portParams;

    associatedRouterPort->port_support->get_MAC_address(associatedRouterPort->port_support, my_address);
}


static void Virtual_get_MAC_address(
    const PORT_SUPPORT* portParams,			// need this parameter so this function's signature matches function pointer
    BACNET_MAC_ADDRESS* my_address) 
{
    (void)portParams;
    bacnet_mac_set_uint16(my_address, 0);
}


static void Application_Send_Npdu(
    PORT_SUPPORT *datalink,
    const DLCB* dlcb) 
{
    BACNET_NPCI_DATA tnpci;
    BACNET_GLOBAL_ADDRESS src, dest;
    RX_DETAILS  rxDetails;

    Application_get_MAC_address(datalink,
        &rxDetails.srcPath.localMac);

    // pity we have to decode NPCI again... after having just constructed it - clean up in future - todo 4
    int bpdu_offset = npci_decode(dlcb->Handler_Transmit_Buffer, &dest,
        &src, &tnpci);

    // and embellish our src for the other side too (note, later it may be stripped off again due to merged nets...)
    src.net = applicationRouterPort->route_info.configuredNet.net;

    applicationRouterPort->port_support->get_MAC_address(
        associatedRouterPort->port_support, &src.mac);

    int sendLen = dlcb->optr - bpdu_offset;
    DLCB* newDlcb = dlcb_clone_with_copy_of_buffer(dlcb, sendLen,
        &dlcb->Handler_Transmit_Buffer[bpdu_offset]);
    if (newDlcb) {
        distribute_npdu(applicationRouterPort, &rxDetails, &dest, &src,
            &tnpci, newDlcb);
    }
    dlcb_free(dlcb);
}


static void virtual_send_npdu(
    PORT_SUPPORT *datalink,
    const DLCB* dlcb) 
{
    ROUTER_PORT* sourcePort = headRouterPort;
    BACNET_NPCI_DATA tnpci;
    BACNET_GLOBAL_ADDRESS src, dest;
    RX_DETAILS rxDetails;

    // the virtual layer does not know the sourceRouterPort, we have to look it up..
    while (sourcePort != NULL) {
        if (sourcePort->port_support == datalink )
            break;
        sourcePort = (ROUTER_PORT*)sourcePort->llist.next;
    }
    if (sourcePort == NULL) {
        dlcb_free(dlcb);
        panic();
        return;
    }

    // pity we have to decode NPCI again... after having just constructed it - clean up in future - todo 4
    int bpdu_offset = npci_decode(dlcb->Handler_Transmit_Buffer, &dest,
        &src, &tnpci);

    // and embellish our src for the other side too (note, later it may be stripped off again due to merged nets...)
    src.net = sourcePort->route_info.configuredNet.net;

    bacnet_mac_copy(&src.mac, datalink->localMAC );
    bacnet_mac_copy(&rxDetails.srcPath.localMac, datalink->localMAC);

    // todo 4 - perhaps we should just shift the buffer around rather than creating a new one with offset, and discarding the old one.
    DLCB* ndlcb = dlcb_clone_with_copy_of_buffer(dlcb,
        dlcb->optr - bpdu_offset,
        &dlcb->Handler_Transmit_Buffer[bpdu_offset]);
    dlcb_free(dlcb);
    if (ndlcb == NULL) {
        panic();
        return;
    }

    distribute_npdu(sourcePort, &rxDetails, &dest, &src, &tnpci, ndlcb);
}


void handle_npdu_router(ROUTER_MSG* rtmsg) {
    BACNET_NPCI_DATA npci_data = rtmsg->npdu_data;
    ROUTER_PORT* srcport;
    BACNET_GLOBAL_ADDRESS src = rtmsg->rxDetails->srcPath.glAdr; // rtmsg->src;
    BACNET_GLOBAL_ADDRESS dest = rtmsg->dest;

    srcport = rtmsg->sourceRouterPort; 

    if (npci_data.network_layer_message)
    {
        // Not for the application layer, but perhaps for the local network layer

        if (dest.net == 0 ||                                // local bcast or directed
            dest.net == BACNET_BROADCAST_NETWORK)           // global bcast
        {
            // processing local network layer message may generate local broadcasts on other ports, but these are independent of this logic
            dbMessage(DBD_RouterOperations, DB_DEBUG, "Processing Network Layer Message");
            ProcessLocalNetworkLayerMessage(srcport, &npci_data, &dest,
                &src, rtmsg); // ->bpdu, apdu_len, incomingMsg);
        }

        // does this message need to be relayed
        if (dest.net == 0) {
            // no
            return;
        }
    }

    // did the message come from a directly connected network (sadr == 0)
    // If so, add the sadr before doing anything else, since we will be forwarding/distributing from this point on.

#if ( BAC_DEBUG == 1 )
    bacnet_mac_check(&rtmsg->rxDetails->srcPath.localMac);
#endif

    if (src.net == 0) {
        // add source to NPCI for remote device..
        src.net = srcport->route_info.configuredNet.net;
        //src.len = incomingMsg->phySrc->len;
        //memcpy(src.adr, incomingMsg->phySrc->adr, src.len);
        bacnet_mac_copy(&src.mac, &rtmsg->rxDetails->srcPath.localMac); // incomingMsg->src.localMac);
    }
    else {
        // packet is from a remote node, so use the encoded SADR
        bacnet_address_copy(&src, &rtmsg->rxDetails->srcPath.glAdr); // src);
        // _and_ we make sure we keep a record of where we can find this network number!
        update_network_number_cache(srcport, src.net,
            &rtmsg->rxDetails->srcPath.localMac);
    }

    // Time to check the hop count
    if (npci_data.hop_count != 0) {
        npci_data.hop_count--;
        if (npci_data.hop_count == 0) {
            dbMessage(DBD_RouterOperations, DB_ERROR,
                "Hop count reached zero. Broadcast storm certain. Loop in network configuration somewhere.");
            return;
        }
    }

    // distribute broadcasts, forward unicasts, etc.
    DLCB* newDLCB = dlcb_alloc_with_buffer_clone(
        rtmsg->bpdu_len,
        rtmsg->bpdu);

    if (newDLCB) {

        dbMessage(DBD_RouterOperations, DB_DEBUG, "Distributing copies of Network Layer message");

        distribute_npdu(
            srcport, 
            rtmsg->rxDetails, 
            &dest, 
            &src,
            &npci_data, newDLCB);
    }
}


void StartSearchForRouter(
    const ROUTER_PORT* srcport,
    const BACNET_GLOBAL_ADDRESS* dest,
    const BACNET_GLOBAL_ADDRESS* src, BACNET_NPCI_DATA* npci_data,
    const DLCB* dlcb, const uint16_t networkNumber) 
{
    ROUTER_PORT* frp = headRouterPort;
    // If we get here, we need to discover the router to the unknown network and if one is found, forward this packet (later).
    // if none is found, then after a timeout, we need to send a reject message back to the client...
    // start by sending who-is-router to all other networks
    while (frp != NULL) {
        if (frp != srcport) {
            // todo3 - this is reached too often. We are not making a note of the network numbers on incoming PDUs, and caching
            // some information which would help us. Shortcut the process. Make the improvement!
            dbMessage(DBD_RouterOperations, DB_ALWAYS,
                "Attempt to forward search for unknown net %d out portID %d",
                dest->net, frp->port_id);
            create_who_is_router_network_message(frp, &networkNumber);
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }

    if (search_nn) {
        dlcb_free(search_dlcb);
    }

    search_nn = networkNumber;
    search_start_time = bits_sysTimer_get_milliseconds();
    search_src_port = srcport;
    memcpy(&search_src_address, src, sizeof(BACNET_GLOBAL_ADDRESS));
    memcpy(&search_dest_address, dest, sizeof(BACNET_GLOBAL_ADDRESS));
    memcpy(&search_npdu_data, npci_data, sizeof(BACNET_NPCI_DATA));
    search_dlcb = dlcb;

    dbMessage(DBD_RouterOperations,
        DB_UNUSUAL_TRAFFIC,
        "Todo, resend message if search for destination router successful");
}


void forward_directed_router_message(
    const ROUTER_PORT* srcport,
    const BACNET_GLOBAL_ADDRESS* dest,
    const BACNET_GLOBAL_ADDRESS* src,
    BACNET_NPCI_DATA* npci_data,
    const DLCB* dlcb) {
    BACNET_MAC_ADDRESS destPhyMac;
    BACNET_GLOBAL_ADDRESS tSadr, tDadr;

    ROUTER_PORT* destport = find_routerport_from_net(dest->net,
        &destPhyMac);
    if (destport == NULL) {
        // perhaps the Network is busy?
        if (IsNetworkBusy(dest->net)) {
            panic();
            create_reject(srcport, src, NETWORK_REJECT_ROUTER_BUSY,
                dest->net);
            // dump the packet - todo2 - consider queuing
            dlcb_free(dlcb);
            return;
        }

        // we don't know the route to that Network Number. Set up a discovery.
        StartSearchForRouter(srcport, dest, src, npci_data, dlcb,
            dest->net);
        dlcb_free(dlcb);
        return;
    }

    if (destport->route_info.configuredNet.net == dest->net) {
        // directly coupled, destPhyMac populated from dest
        bacnet_mac_copy(&destPhyMac, &dest->mac);
        // remove DADR
        bacnet_address_clear(&tDadr);
    }
    else {
        // other side of a router, the destPhyMac will be populated by find_routerport_from_net()
        bacnet_address_copy(&tDadr, dest);
    }

    if (destport->route_info.configuredNet.net == src->net) {
        // this is an entangled (i.e. local) port, remove SADR
        bacnet_address_clear(&tSadr);
    }
    else {
        bacnet_address_copy(&tSadr, src);
    }

    RX_DETAILS rxDetails;
    memset(&rxDetails, 0, sizeof(RX_DETAILS));
    bacnet_path_clear(&rxDetails.srcPath);
    //                                                                  npci       mac          dadr    sadr
    destport->forward_network_message(destport, &rxDetails, npci_data,
        &destPhyMac, &tDadr, &tSadr, dlcb);
}

// this will either deliver the (single) npdu, else broadcast copies of it
void distribute_npdu(ROUTER_PORT* srcport, RX_DETAILS* rxDetails,
    BACNET_GLOBAL_ADDRESS* dest, BACNET_GLOBAL_ADDRESS* src,
    BACNET_NPCI_DATA* npci_data, const DLCB* dlcb) {
    DLCB* newDlcb;

#if ( BAC_DEBUG == 1 )
    if (!dlcb_check(dlcb))
        return;
    bool ok = true;
    ok &= bacnet_mac_check(&rxDetails->srcPath.localMac);
    ok &= bacnet_mac_check(&src->mac);
    ok &= bacnet_mac_check(&dest->mac);
    if (!ok) {
        dlcb_free(dlcb);
        return;
    }

    // some sanity checks, remove at appropriate time
    if (srcport->route_info.configuredNet.net == 0) {
        panic();
    }
#endif

    // According to my notes 2017.05.28
    if (srcport == associatedRouterPort) {
        switch (dest->net) {

        case 0:
            // this can only be a MSTP local broadcast or unicast TO THIS STATION (else net would be populated). Simply pass packet on to application (only).
            applicationRouterPort->forward_network_message(
                applicationRouterPort, rxDetails, npci_data, NULL,
                dest, src, dlcb);
            return;

        case 0xFFFF:
            // global broadcast has arrived, on MSTP port. Need to copy to app channel, and also broadcast to all others.
            // SADR must be removed for APP, but inserted for others.
            // DADR remains 'global bcast' for all
            newDlcb = dlcb_clone_deep(dlcb);
            if (newDlcb == NULL) {
                dlcb_free(dlcb);
                return;
            }

            //                                                                               npci       mac   dadr  sadr
            applicationRouterPort->forward_network_message(
                applicationRouterPort, rxDetails, npci_data, NULL,
                dest, src, newDlcb);
            distribute_local_broadcast_message(srcport, rxDetails, dest,
                src, npci_data, dlcb);
            return;

        default:
            // Directed response, but since there is a NN, it cannot by definition, be addressed to the entangled application port. Distribute right away to the other ports..
            forward_directed_router_message(srcport, dest, src,
                npci_data, dlcb);
            return;
        }
    }
    else if (srcport == applicationRouterPort) {
        switch (dest->net) {
        case 0:
            //  I suspect this is actually OK if user sets physical net number to 0
            dbMessage_Hourly(DBD_RouterOperations, DB_ERROR, "Attempt to repond to a device on Network Number 0. Our configuration has been destroyed by another Router on the BACnet Network");
            return;

        case 0xFFFF:
            // distribute NPDU to all routerports with no exception for srcport (app), it is not in the list.
            distribute_local_broadcast_message(NULL, rxDetails, dest,
                        src, npci_data, dlcb);
            return;

        default:
            // todo 1 - this is wrong in my notes. How come it works? Fix anyway.. (EKH will check notes next time he goes to the office) cr1363
            forward_directed_router_message(srcport, dest, src, npci_data, dlcb);
            return;
        }
    }
    else {
        // message arrives from 'other' port (not application, not associated)
        switch (dest->net) {

        case 0:
            // We can get here if there is a local broadcast, of an application message, from an attached network... 
            // but one where the Application Entity is not attached to the port... so we quietly drop the message.
            dlcb_free(dlcb);
            break;

        case 0xFFFF:
            // global broadcast has arrived, on MSTP or IP port. Need to copy to app channel, and also broadcast to all others.
            // SADR must be removed for APP, but inserted for others.
            // DADR remains 'global bcast' for all
            newDlcb = dlcb_clone_deep(dlcb);
            if (newDlcb == NULL) {
                dlcb_free(dlcb);
                return;
            }

            applicationRouterPort->forward_network_message(
                applicationRouterPort,
                rxDetails,
                npci_data,          // ncpi
                NULL,               // MAC
                dest,               // Dadr
                src,                // Sadr
                newDlcb);
            distribute_local_broadcast_message(srcport, rxDetails, dest,
                src, npci_data, dlcb);
            return;

        default:
            // the directed message from the other port needs to be forwarded. Before doing so though, check to make sure that the message is not aimed at (solely) the app
            if (dest->net == applicationRouterPort->route_info.configuredNet.net) {
                // aimed at our applications net (as well as the associcated net). Check to see if it is aimed at our application thought..
                BACNET_MAC_ADDRESS ourMac;
                applicationRouterPort->port_support->get_MAC_address(
                    applicationRouterPort->port_support, &ourMac);
                if (bacnet_mac_same(&dest->mac, &ourMac)) {
                    // yes, this message dest address is pointing to our MAC address
                    // if so, do an application call, and exit
                    applicationRouterPort->forward_network_message(
                        applicationRouterPort, rxDetails, npci_data,
                        NULL, dest, src, dlcb);
                    return;
                }
                // well, it is not for our application, could it be a remote broadcast?
                if (dest->mac.len == 0) {
                    // then it is for our app _and_ all the others.
                    newDlcb = dlcb_clone_deep(dlcb);
                    if (newDlcb == NULL) {
                        dlcb_free(dlcb);
                        return;
                    }
                    applicationRouterPort->forward_network_message(
                        applicationRouterPort, rxDetails, npci_data,
                        NULL, dest, src, newDlcb);
                    // but don't return, the broadcast out the associated port will happen next.. (the dest is a b'cast)
                }
            }
            forward_directed_router_message(srcport, dest, src,
                npci_data, dlcb);
            return;
        }
    }
}


void network_layer_check_timers(void) {
    // note. Internal Processing Fail time of 0.5 is given as an example in testing documents... so let us try and comply
    if (search_nn != 0 && bits_sysTimer_elapsed_milliseconds(search_start_time) > 400) {
        // we never did find that router, send a reject message
        create_reject(search_src_port, &search_src_address,
            NETWORK_REJECT_NO_ROUTE, search_nn);
        search_nn = 0;
        dlcb_free(search_dlcb);
        dbMessage(DBD_RouterOperations, DB_NOTE, "Dumping NN search details");
    }
}

void create_router_flow_control_msg(BACNET_NETWORK_MESSAGE_TYPE mt,
    ROUTER_PORT* srcport) {
    ROUTER_PORT* port = headRouterPort;
    BACNET_NPCI_DATA npci_data;
    // BACNET_ADDRESS dest;
    DNET* dnet;
    uint8_t* buffer;
    uint8_t mlen = 0;
    bool busyflag;

    // build the list of network numbers subject to flow control

    busyflag =
        (mt == NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK) ?
        true : false;

    buffer = (uint8_t*)malloc(100); // todo 3 use our emm_malloc() 
    if (buffer == NULL)
        return;

    init_network_npci(&npci_data, false);

    mlen += encode_unsigned8(&buffer[mlen], (uint8_t)mt);

    // find the networkNumber list for the port we are busy/not busy with.
    while (port != NULL) {
        if (port == srcport) {
            port->route_info.configuredNet.busy2 = busyflag;
            // add the network numbers to the list
            mlen += encode_signed16(&buffer[mlen],
                port->route_info.configuredNet.net);
            dnet = port->route_info.dnets2;
            while (dnet != NULL) {
                mlen += encode_signed16(&buffer[mlen], dnet->net);
                dnet->busy2 = busyflag;
                if (busyflag)
                    dnet->timer = bits_sysTimer_get_milliseconds();
                dnet = dnet->next;
            }
        }
        port = (ROUTER_PORT*)port->llist.next;
    }

    // now send this list out of the other ports
    LocalBcastMsgToFarPorts(srcport, &npci_data, buffer, mlen);

    free(buffer);
}

void create_i_am_router_network_message(ROUTER_PORT* port,
    uint16_t* networkNumber) {
    int16_t buff_len;
    BACNET_NPCI_DATA npci_data;
    // BACNET_GLOBAL_ADDRESS dest;

    if (port == NULL) {
        panic();
        return;
    }

    dbMessage(DBD_RouterOperations,
        DB_UNUSUAL_TRAFFIC,
        "Encoding I-Am-Router to send to portId %d, IPport %d,  NN %d",
        port->port_id,
        ntohs(port->port_support->datalink.bipParams.nwoPort),
        port->route_info.configuredNet.net);

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('A', port->port_support->max_lpdu);
    if (!outgoingMsgData) {
        panictodo(3); // cleanup all the diff reporting functions (normalize them all)
        panicDesc("m0104 - Error: Could not allocate memory");
        return;
    }

    init_network_npci(&npci_data, false);

    set_local_broadcast_mac(
        &outgoingMsgData->bacnetPath.localMac); // phyDest;

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer,
        NULL, NULL, &npci_data);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK);

    if (networkNumber != NULL) {
        // this means we are responding to a specific Network Number
        buff_len += encode_unsigned16(
            outgoingMsgData->Handler_Transmit_Buffer + buff_len,
            *networkNumber);
    }
    else {
        // build I-Am-Router message to all available OTHER networks..
        ROUTER_PORT* otherRouterPort = headRouterPort;
        DNET* dnet;
        bool foundNetNumbs = false;

        while (otherRouterPort != NULL) {
            if (otherRouterPort != port
                && otherRouterPort->port_support->portType
                != BPT_APP) // app always latches onto one of the other ports...
            {
                // perhaps the port has been decommissioned, check for that
                if (otherRouterPort->route_info.configuredNet.net
                    != 0) {
                    dbMessage(DBD_RouterOperations,
                        DB_UNUSUAL_TRAFFIC,
                        "Encoding configured dnet %d",
                        otherRouterPort->route_info.configuredNet.net);
                    buff_len +=
                        encode_unsigned16(
                            outgoingMsgData->Handler_Transmit_Buffer
                            + buff_len,
                            otherRouterPort->route_info.configuredNet.net);
                    foundNetNumbs = true;
                }

                dnet = otherRouterPort->route_info.dnets2;
                while (dnet != NULL) {
                    if (buff_len > port->port_support->max_lpdu) {
                        // no more space for our APDU
                        dbMessage(DBD_RouterOperations, DB_ERROR,
                            "Forwarded LPDU too large for destination datalink");
                        break;
                    }
                    buff_len += encode_unsigned16(
                        outgoingMsgData->Handler_Transmit_Buffer
                        + buff_len, dnet->net);
                    dbMessage(DBD_RouterOperations, DB_NORMAL_TRAFFIC, "Encoding discovered dnet %d",
                        dnet->net);
                    foundNetNumbs = true;
                    dnet = dnet->next;
                }
            }
            otherRouterPort =
                (ROUTER_PORT*)otherRouterPort->llist.next;
        }

        // if there are no network numbers to send with the I-Am-Router-To-Network, we should abandon..
        if (!foundNetNumbs) {
            dlcb_free(outgoingMsgData);
            dbMessage(DBD_RouterOperations, DB_ERROR,
                "No Network Numbers to send in the I-Am-Router-To-Network, abandoning attempt.");
            return;
        }
    }

    outgoingMsgData->optr = buff_len;
    SendToPort(port, outgoingMsgData);
}

static void create_network_number_is_network_message(ROUTER_PORT* port) {
    int16_t buff_len;
    BACNET_NPCI_DATA npci_data;
    // BACNET_GLOBAL_ADDRESS dest;

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('G',
        port->port_support->max_lpdu);

    if (!outgoingMsgData) {
        panicDesc("m0111");
        //        dbMessage(DB_NOTE, "m0024 - Error: Could not allocate memory");
        return;
    }

    if (port == NULL)
        return;

    init_network_npci(&npci_data, false);

    set_local_broadcast_mac(
        &outgoingMsgData->bacnetPath.localMac);
    //     outgoingMsgData->phyDest.len = 0;   // this will tell send routine we want to broadcast

    //     port->port_support->get_broadcast_address(port->port_support, &dest);

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer,
        NULL, NULL, &npci_data);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_NETWORK_NUMBER_IS);

    // build I-Am-Router message to all available OTHER networks..

    // perhaps the port has been decommissioned, check for that
    if (port->route_info.configuredNet.net == 0) {
        dlcb_free(outgoingMsgData);
        dbMessage(DBD_RouterOperations, DB_ERROR,
            "No Network Numbers to send in the I-Am-Router-To-Network, abandoning attempt.");
        return;
    }

    dbMessage(DBD_RouterOperations, DB_NORMAL_TRAFFIC,
        "Encoding 'Network-Number-Is' for portId %d, IPport %d, configured DNET %d",
        port->port_id,
        ntohs(port->port_support->datalink.bipParams.nwoPort),
        port->route_info.configuredNet.net);
    buff_len += encode_unsigned16(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        port->route_info.configuredNet.net);

    outgoingMsgData->Handler_Transmit_Buffer[buff_len++] = 0x01;

    outgoingMsgData->optr = buff_len;

    SendToPort(port, outgoingMsgData);
}

void create_i_could_be_router_network_message(ROUTER_PORT* port,
    uint8_t* pdu, int len) {
    int16_t buff_len;
    BACNET_NPCI_DATA npci_data;
    BACNET_GLOBAL_ADDRESS dest;

    int tcount = emmCount;

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('E',
        MAX_LPDU_IP);

    if (!outgoingMsgData) {
        panicDesc("m0121");
        dbMessage(DBD_RouterOperations, DB_ERROR, "m0006 - Error: Could not allocate memory");
        return;
    }

    if (port == NULL)
        return;

    init_network_npci(&npci_data, false);

    outgoingMsgData->bacnetPath.localMac.len = 0; // this will tell send routine we want to broadcast // todo 4 use function()

    set_local_broadcast_mac(
        &outgoingMsgData->bacnetPath.localMac);

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer,
        &dest, NULL, &npci_data);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK);

    memcpy(&outgoingMsgData->Handler_Transmit_Buffer[buff_len], pdu,
        len);
    buff_len += len;

    outgoingMsgData->optr = buff_len;
    SendToPort(port, outgoingMsgData);
    if (tcount != emmCount)
        panic();
}

void create_router_table_ack(ROUTER_PORT* destport,
    BACNET_MAC_ADDRESS* phyMac, BACNET_GLOBAL_ADDRESS* destaddr,
    void* val) {
    int16_t buff_len;
    BACNET_NPCI_DATA npci_data;

    /* allocate message structure */
    DLCB* outgoingMsgData = dlcb_alloc('F',
        MAX_LPDU_IP);
    if (!outgoingMsgData) {
        panic();
        return;
    }

    init_network_npci(&npci_data, false);

    memcpy(&outgoingMsgData->bacnetPath.localMac, phyMac,
        sizeof(BACNET_MAC_ADDRESS));

    buff_len = npdu_encode_pdu(outgoingMsgData->Handler_Transmit_Buffer,
        destaddr, NULL, &npci_data);
    buff_len += encode_unsigned8(
        outgoingMsgData->Handler_Transmit_Buffer + buff_len,
        (uint8_t)NETWORK_MESSAGE_INIT_RT_TABLE_ACK);

    if (val != NULL) {
        // want the whole table
        uint8_t port_count = 0;
        uint8_t* savePortCountPosition =
            &outgoingMsgData->Handler_Transmit_Buffer[buff_len++];

        ROUTER_PORT* port = headRouterPort;
        while (port != NULL) {
            if (port->route_info.configuredNet.net) {
                if (port->port_support->portType != BPT_APP) // App port is always bound to another of the same NN.. skip it.
                {
                    buff_len += encode_unsigned16(
                        outgoingMsgData->Handler_Transmit_Buffer
                        + buff_len,
                        port->route_info.configuredNet.net);
                    outgoingMsgData->Handler_Transmit_Buffer[buff_len++] =
                        port->port_id;
                    outgoingMsgData->Handler_Transmit_Buffer[buff_len++] =
                        0;
                    port_count++;

                    // don't forget the discovered networks
                    // 2015.01.04 - Update
                    // actually, we DONT want the discovered networks. This issue is currently under discussion between EKH and BTL.
                    // 2017.01.05 - EKH, reversing the above. Careful reading of spec shows we do need the whole table, including ephemeral connections, although it is not explicit.
                    // It _is_ clear that the 'routing table' contains ephemerals, and practically, how else are we to discover all networks?
                    // 2017.01.11 - EKH, Actually, we only report the connected DNETS
                    // http://www.bacnet.org/Interpretations/IC%20135-2008-13.pdf

                    //if (port->route_info.dnets2 != NULL)
                    //{
                    //    DNET *dnet = port->route_info.dnets2;
                    //    while (dnet != NULL)
                    //    {
                    //        buff_len += encode_unsigned16(outgoingMsgData->pdu + buff_len, dnet->net);
                    //        outgoingMsgData->pdu[buff_len++] = port->port_support->port_id2;
                    //        outgoingMsgData->pdu[buff_len++] = 0;
                    //        port_count++;
                    //        dnet = dnet->next;
                    //    }
                    //}

                }
            }
            else {
                // when would this ever be 0?
                dbMessage(DBD_Config, DB_ERROR, "Router Port %d configuredNet is zero", port->port_id);
            }
            port = (ROUTER_PORT*)port->llist.next;
        }
        *savePortCountPosition = port_count;
    }
    else {
        // want just an ack... ie. len=0
        outgoingMsgData->Handler_Transmit_Buffer[buff_len++] =
            (uint8_t)0;
    }
    outgoingMsgData->optr = buff_len;
    SendToPort(destport, outgoingMsgData);
}


void init_application_npdu(BACNET_NPCI_DATA* npci_data,
    bool data_expecting_reply) {
    init_common_npci(npci_data, data_expecting_reply);
    npci_data->network_layer_message = false;
}

void init_network_npci(BACNET_NPCI_DATA* npci_data,
    bool data_expecting_reply) {
    init_common_npci(npci_data, data_expecting_reply);
    npci_data->network_layer_message = true;
}


static void RouterListen(void* pArgs) {
    ROUTER_PORT* rp = (ROUTER_PORT*)pArgs;

    while (!bacnetShuttingDown) {
        uint8_t rxBuf[MAX_LPDU_IP];
        RX_DETAILS rxDetails;

        // Is our datalink alive?
        if (rp->port_support->isActive)
        {
            if (!rp->port_support->isActive(rp->port_support))
            {
                // no, so wait a while and try thereafter.
                msSleep(1000);
                continue;
            }
        }

        // Just a reminder, much activity can go on in ReceiveMPDU, e.g. BBMD handling, that results in this function returning
        // a length of 0, perfectly normal operation.
        // Also, virtual routing...

        uint16_t pdu_len = rp->port_support->ReceiveMPDU(
            rp->port_support, &rxDetails.srcPath.localMac, rxBuf,
            MAX_LPDU_IP);

        if (pdu_len > 0) {
            ROUTER_MSG rtmsg;

            rxDetails.npdu = rxBuf;
            rxDetails.npdu_len = pdu_len;
            // no one uses this - rxDetails.block_valid = 'R';
            rxDetails.portParams = rp->port_support; // more duplication..

            rtmsg.rxDetails = &rxDetails;
            rtmsg.sourceRouterPort = rp;

            bacnet_address_clear(&rxDetails.srcPath.glAdr);
            int bpdu_offset = npci_decode(rxBuf, &rtmsg.dest,
                &rxDetails.srcPath.glAdr, &rtmsg.npdu_data);

            // the shortest NSDU is 1 byte
            // http://www.bacnetwiki.com/wiki/index.php?title=NSDU
            if (pdu_len < bpdu_offset + 1) {
                // bad incoming packet. todo 2 Log a statistic?
                continue;
            }

            rtmsg.bpdu_len = pdu_len - bpdu_offset;

            rtmsg.bpdu = &rxBuf[bpdu_offset];

            bits_mutex_lock(bacnetStackMutex);
            handle_npdu_router(&rtmsg);
            bits_mutex_unlock(bacnetStackMutex);
        }

        msSleep(2);
    }
}

typedef void(*ThreadFunction) (void*);

void Init_Router_Thread(ThreadFunction router_thread, ROUTER_PORT* port) {

    bitsCreateThread(router_thread, port);

}

// todo 3 - there is a duplication of effort here, investigate cr38572034572057
static ROUTER_PORT* RouterPortFindByNetworkNumber(BPT_TYPE portType, uint16_t networkNumber) {
    ROUTER_PORT* frp = headRouterPort;
    // portType is required to allow "associated" RouterPorts
    while (frp != NULL) {
        if (frp->route_info.configuredNet.net == networkNumber &&
            frp->port_support->portType == portType) {
            return frp;
        }
        frp = (ROUTER_PORT*)frp->llist.next;
    }
    return NULL;
}


#if ( BITS_ROUTER_LAYER == 1 )

// set up our port control blocks
static ROUTER_PORT *SysInitRouterport(
    const uint8_t portId,
    const BPT_TYPE rpt,
    const char* ifName,
    const uint16_t networkNumber,
    const uint16_t ipPort)
{

    // Do not duplicate
    ROUTER_PORT* existing = RouterPortFindByNetworkNumber(rpt, networkNumber);
    if (existing != NULL) {
        panicstr("Duplicate type/network number found, check your configuration!");
        return existing;
    }

    ROUTER_PORT* rp = (ROUTER_PORT*)emm_scalloc('a', sizeof(ROUTER_PORT));
    if (rp == NULL)
        return NULL;

    rp->route_info.dnets2 = NULL;
    InitDnet(&rp->route_info.configuredNet, networkNumber);

    switch (rpt) {
    default:
        panic();
        emm_free(rp);
        return NULL;

#if (BACDL_FD == 1)
    case BPT_FD:
        // because FD needs additional parameters, it cannot be initialized by this general function
        rp->forward_network_message = router_forward_network_message;
        break;
#endif

#if (BACDL_ETHERNET == 1)
    case BPT_Ethernet:
        rp->forward_network_message = router_forward_network_message;
        rp->port_support = Init_Datalink_Ethernet(ifName);
        if (rp->port_support)
        {
            Init_Router_Thread(RouterListen, rp);
        }
        break;
#endif

    case BPT_BIP:
        rp->forward_network_message = router_forward_network_message;
        rp->port_support = Init_Datalink_IP(ifName, ipPort);
        if (rp->port_support ) {
            Init_Router_Thread(RouterListen, rp);
        }
        break;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case BPT_BBMD:
        rp->forward_network_message = router_forward_network_message;
        rp->port_support = Init_Datalink_BBMD(ifName, ipPort);
        if (rp->port_support) {
            Init_Router_Thread(RouterListen, rp);
        }
        break;
#endif

#if (BACDL_MSTP == 1)
    case BPT_MSTP:
        rp->forward_network_message = router_forward_network_message;
        rp->port_support = Init_Datalink_MSTP(ifName, ipPort);
        if (rp->port_support)
        {
            Init_Router_Thread(RouterListen, rp);
        }
        break;
#endif

    case BPT_APP:
        rp->forward_network_message = forward_router_message_to_application;
        // todo 3 - this only works if we associate the app routerPort with an IP datalink. Resolve what happens if
        // e.g. we associate with a MS/TP datalink
        rp->port_support = datalink_initCommon(ifName, rpt );
        rp->port_support->max_apdu = MAX_APDU_IP;
        rp->port_support->max_lpdu = MAX_LPDU_IP;
        if (rp->port_support) {
            rp->port_support->get_MAC_address = Application_get_MAC_address;
            rp->port_support->SendPdu = Application_Send_Npdu;
        }
        break;

#if ( VIRTUALDEV == 1 )
    case BPT_VIRT:
        rp->forward_network_message = VirtualLayerProcessNPDU;
        rp->port_support = datalink_initCommon(ifName, rpt);
        rp->port_support->max_apdu = MAX_APDU_IP;
        rp->port_support->max_lpdu = MAX_LPDU_IP;
        ll_Init(&rp->datalinkDevicesVirtual, MX_VIRTUAL_DEVICES);
        if (rp->port_support) {
            rp->port_support->get_MAC_address = Virtual_get_MAC_address;
            rp->port_support->SendPdu = virtual_send_npdu;
        }
        break;
#endif

    }

#if (BACDL_FD == 1)
    // FD (Foreign Device is a special case, gets initialized after this function call
    if ( rpt != BPT_FD && rp->port_support == NULL) {
        emm_free(rp);
        dbMessage(DBD_ALL, DB_ERROR, "Router Port [%s] not initialized", ifName);
        return NULL;
    }
#endif

    // the Application Port is not part of the routerport link list
    if (rp->port_support)
    {
        LinkListAppend((void**)&headRouterPort, rp); // Router port structure

        rp->port_id = portId;

        return rp;
    }
    else
    {
        emm_free(rp);
        return NULL;
    }
}
#endif


#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1) && (BITS_ROUTER_LAYER == 1)
bool InitRouterportWithNAT(
    const uint8_t portId,
    BPT_TYPE rpt,
    const char* adapter,
    uint16_t networkNumber,
    uint16_t ipPort,
    struct sockaddr_in* globalIPEP) {
    ROUTER_PORT* rp = SysInitRouterport(
        portId,
        rpt,
        adapter,
        networkNumber,
        ipPort);
    if (rp == NULL) {
        return false;
    }
    rp->port_support->datalink.bipParams.BVLC_NAT_Handling = true;
    rp->port_support->datalink.bipParams.BVLC_Global_Address =
        *globalIPEP;
    return true;
}
#endif

#if (BACDL_FD == 1)
bool InitRouterportForeignDevice(
    const uint8_t portId,
    const char* adapter,
    uint16_t networkNumber,                 // this network number had better match the NN of the BBMD network we are registering with.
                                            // todo - we should ask 'what is network number'and use it/confirm it
    const char* remoteName,
    const uint16_t remoteIPport,
    uint16_t ttl)
{
    ROUTER_PORT* rp = SysInitRouterport(
        portId,
        BPT_FD,
        adapter,
        networkNumber,
        0   // bind to any local port
        );
    if (rp == NULL) {
        return false;
    }

    rp->port_support = Init_Datalink_FD( adapter, remoteName, remoteIPport);
    rp->port_support->datalink.bipParams.fd_timetolive = ttl;

    if (rp->port_support) {
        Init_Router_Thread(RouterListenIP, rp);
    }
    return true;
}
#endif


#if ( BITS_ROUTER_LAYER == 1)
bool InitRouterport(
    const uint8_t portId,
    const BPT_TYPE rpt,
    const char* ifaceName,
    const uint16_t networkNumber,
    const uint16_t hoPort) {

    ROUTER_PORT* rp = SysInitRouterport(
        portId,
        rpt,
        ifaceName,
        networkNumber,
        hoPort);

    if (rp == NULL)
        return false;

    return true;
}


#if (BACDL_MSTP == 1)
ROUTER_PORT* InitRouterportMSTP(const uint8_t portId, uint16_t networkNumber) {
    return SysInitRouterport(portId, BPT_MSTP, "MSTP", networkNumber, 0);
}
#endif

bool AlignApplicationWithPort(void) {
    uint16_t networkNumberOfRouterPort;

    // First find the application port, record its NN, and then look for another port with that NN, and 'align'

    ROUTER_PORT* portToAlignWith = headRouterPort;

    if (applicationRouterPort == NULL) {
        // the system will crash soon, let's try report this fault regardless...
        panic();
        return false;
    }
    networkNumberOfRouterPort = applicationRouterPort->route_info.configuredNet.net;

    while (portToAlignWith != NULL) {
        if (portToAlignWith->port_support->portType != BPT_APP
            && portToAlignWith->route_info.configuredNet.net
            == networkNumberOfRouterPort)
            break;
        portToAlignWith = (ROUTER_PORT*)portToAlignWith->llist.next;
    }
    if (portToAlignWith == NULL) {
        panic();
        return false;
    }

    // set prior applicationRouterPort = portWithApp;
    associatedRouterPort = portToAlignWith;

    applicationRouterPort->port_support->datalink.bipParams.nwoLocal_addr =
        portToAlignWith->port_support->datalink.bipParams.nwoLocal_addr;
    applicationRouterPort->port_support->datalink.bipParams.nwoBroadcast_addr =
        portToAlignWith->port_support->datalink.bipParams.nwoBroadcast_addr;
    applicationRouterPort->port_support->datalink.bipParams.nwoPort =
        portToAlignWith->port_support->datalink.bipParams.nwoPort;
    applicationRouterPort->route_info.configuredNet =
        portToAlignWith->route_info.configuredNet;

    return true;
}


bool InitRouterportApp(int associatedNetworkNumber) {
    ROUTER_PORT* rp = SysInitRouterport(100, BPT_APP, "App",
        (uint16_t)associatedNetworkNumber, 0);
    if (rp == NULL) {
        dbMessage(DBD_Config, DB_ERROR, "Null return from SysInitRouterPort for application! NN:%d", associatedNetworkNumber);
        return false;
    }
    applicationRouterPort = rp;
    applicationDatalink = rp->port_support;
    applicationDatalink->portType = BPT_APP;
    applicationDatalink->datalinkId = 100;

    // cannot guarantee sequence of port initializations, so only do this once all done AlignApplicationWithPort((uint16_t)associatedNetworkNumber);
    return true;
}


#if ( VIRTUALDEV == 1 )
bool InitRouterportVirtual(const uint8_t portId, uint16_t networkNumber) {
    ROUTER_PORT* rp = SysInitRouterport(portId, BPT_VIRT, "Virtual", networkNumber, 0);
    if (rp == NULL)
        return false;
    virtualDatalink = rp->port_support;
    virtualDatalink->portType = BPT_VIRT;
    virtualDatalink->datalinkId = PORTID_VIRT;

    return true;
}
#endif // #if ( VIRTUALDEV == 1 )


void Virtual_Router_Init(
    uint32_t deviceInstance,
    const char* deviceName,
    const char* deviceDescription)
{
    routerApplicationEntity = Device_Create_Device_Server( applicationDatalink, deviceInstance, deviceName, deviceDescription, "VirtualRouter", NULL);
    if (routerApplicationEntity == NULL) {
        panic();
        exit(-1);
    }

    AlignApplicationWithPort();
}


void SendIAmRouter(void) {
    // we must make sure that the source net is set up so the Send can filter appropriately
    ROUTER_PORT* port = headRouterPort;

    while (port != NULL) {
        switch (port->port_support->portType) {
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
        case BPT_BBMD:
#endif
#if (BACDL_FD == 1)
        case BPT_FD:
#endif
        case BPT_BIP:
#if (BACDL_MSTP == 1)
        case BPT_MSTP:
#endif
            create_i_am_router_network_message(port, NULL);
            break;
        case BPT_VIRT:
        case BPT_APP:
            break;
        default:
            panic();
            break;
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
}

void SendNetworkNumberIs(void) {
    // we must make sure that the source net is set up so the Send can filter appropriately
    ROUTER_PORT* port = headRouterPort;

    while (port != NULL) {
        switch (port->port_support->portType) {
#if (BACDL_BBMD == 1)
        case BPT_BBMD:
#endif
#if (BACDL_FD == 1)
        case BPT_FD:
#endif
        case BPT_BIP:
#if (BACDL_MSTP == 1)
        case BPT_MSTP:
#endif
            create_network_number_is_network_message(port);
            break;
        case BPT_VIRT:
        case BPT_APP:
            break;
        default:
            panic();
            break;
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
}

void SendWhoIsRouter(void) {
    // we must make sure that the source net is set up so the Send can filter appropriately
    ROUTER_PORT* port = headRouterPort;
    while (port != NULL) {
        switch (port->port_support->portType) {
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
        case BPT_BBMD:
#endif
#if (BACDL_FD == 1)
        case BPT_FD:
#endif
        case BPT_BIP:
#if (BACDL_MSTP == 1)
        case BPT_MSTP:
#endif
            create_who_is_router_network_message(port, NULL);
            break;
        case BPT_VIRT:
        case BPT_APP:
            break;
        default:
            panic();
            break;
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
}

uint16_t get_next_free_dnet(void) {

    ROUTER_PORT* port = headRouterPort;
    uint16_t i = 1;
    while (port) {
        if (port->route_info.configuredNet.net == i) {
            port = headRouterPort;
            i++;
            continue;
        }

        port = (ROUTER_PORT*)port->llist.next;
    }
    return i;
}

void dump_router_table(void) {
    ROUTER_PORT* port = headRouterPort;
    DNET* dnet;

    dbMessage(DBD_UI, DB_ALWAYS, "   Here is the Router Table:");

    while (port != NULL) {
        PORT_SUPPORT* pparams = (PORT_SUPPORT*)port->port_support;

        switch (port->port_support->portType) {
        case BPT_BIP:
            dbMessage(DBD_UI, DB_ALWAYS,
                "IP Port=%d\n\r    iface=%s  ID=%d",
                ntohs(pparams->datalink.bipParams.nwoPort), "''", /*port->iface,*/
                port->port_id);
            ipdump("      Local Addr",
                (unsigned char*)& pparams->datalink.bipParams.nwoLocal_addr);
            dbMessage(DBD_UI, DB_ALWAYS, "");
            //            ipdump("      Bcast Addr", (unsigned char *)&pparams->bipParams.nwoBroadcast_addr);
            dbMessage(DBD_UI, DB_ALWAYS, "");
            break;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
        case BPT_BBMD:
#if 0
            log_lev_printf(LLEV::Debug, "%2d BBMD Port=%5d %-4s %s, %s",
                port->port_id,
                ntohs(port->port_support->datalink.bipParams.nwoPort),
                NwoIPAddrToString(temp1, port->port_support->datalink.bipParams.nwoLocal_addr),
                NwoIPAddrToString(temp2, port->port_support->datalink.bipParams.nwoBroadcast_addr)
            );
#endif
            break;
#endif

#if (BACDL_FD == 1)
        case BPT_FD:
            dbMessage(DBD_UI, DB_ALWAYS, "Foreign Device ID=%d", port->port_id);
            {
#ifdef _MSC_VER
                char temp[50];
                dbMessage(DBD_UI, DB_ALWAYS, "      BBMD address is: %s, TTL:%5d, Remaining:%5d\n",
                    IPEP_ToString(temp, &pparams->datalink.bipParams.fd_ipep),
                    pparams->datalink.bipParams.fd_timetolive,
                    pparams->datalink.bipParams.fd_timeRemaining);
#else
                dbMessage(DBD_UI, DB_ALWAYS,
                    "      BBMD address is: %s, TTL:%5d, Remaining:%5d\n",
                    "m0016", 999, 999);
#endif

            }
            break;
#endif

        case BPT_APP:
            dbMessage(DBD_UI, DB_ALWAYS, "App Port       ID=%d\n\r",
                port->port_id);
            break;
#if (BACDL_MSTP == 1)
        case BPT_MSTP:
            dbMessage(DBD_UI, DB_ALWAYS, "MSTP Port      ID=%d\n\r",
                port->port_id);
            break;
#endif
        case BPT_VIRT:
            dbMessage(DBD_UI, DB_ALWAYS, "Virtual Port   ID=%d\n\r",
                port->port_id);
            break;
        default:
            panic();
            break;
        }
        dbMessage(DBD_UI, DB_ALWAYS, "      Net  %d", port->route_info.configuredNet.net);
        dbMessage(DBD_UI, DB_ALWAYS, "  Busy:%d", port->route_info.configuredNet.busy2);
        dbMessage(DBD_UI, DB_ALWAYS, "\n\r");

        dnet = port->route_info.dnets2;
        while (dnet != NULL) {
            dbMessage(DBD_UI, DB_ALWAYS, "         Downstream nets %d", dnet->net);
            dbMessage(DBD_UI, DB_ALWAYS, "  Mac:%02u.%02u ", dnet->phyMac.bytes[2],
                dnet->phyMac.bytes[3]);
            dbMessage(DBD_UI, DB_ALWAYS, "  Busy:%d", dnet->busy2);
            dbMessage(DBD_UI, DB_ALWAYS, "\n\r");
            dnet = dnet->next;
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
    dbMessage(DBD_UI, DB_ALWAYS, "\nThat's all\n");
}

#endif // BITS_ROUTER_LAYER
#endif // BITS_ROUTER_LAYER

