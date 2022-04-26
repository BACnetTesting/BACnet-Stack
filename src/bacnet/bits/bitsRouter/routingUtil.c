/****************************************************************************************
 *
 * Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
 *
 *       <info@bac-test.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <mem.h>
#include "assert.h"
#include "eLib/util/eLibDebug.h"
#include "bitsRouter.h"
#include "bacnet/bacaddr.h"
// #include "osLayer.h"
#include "bacnet/bits/util/BACnetObject.h"
#include "bacnet/basic/object/device.h"
#include "osLayer.h"

extern ROUTER_PORT *headRouterPort ;

static bits_mutex_define(mutexDnet);

void RoutingUtilInit(void)
{
    bits_mutex_init(mutexDnet);
}


DNET* find_dnet(uint16_t net)
{
    ROUTER_PORT *port = headRouterPort;
    DNET *dnet;

    if (net == BACNET_BROADCAST_NETWORK || net == 0)
    {
        // null also indicates 'all' ports
        panicDesc("m0017 - why are we looking for an illegal/bcast net?");
        return NULL;
    }

    while (port != NULL)
    {
        if (port->route_info.configuredNet.net == net) return &port->route_info.configuredNet;

        dnet = port->route_info.dnets2;
        while (dnet != NULL)
        {
            if (dnet->net == net) return dnet;
            dnet = dnet->next;
        }
        port = (ROUTER_PORT *)port->llist.next;
    }

    return NULL;
}


// todo 3 - there is a duplication of effort here, investigate cr38572034572057
ROUTER_PORT* find_routerport_from_net(
    const uint16_t net,
    BACNET_MAC_ADDRESS *addr)
{
    // note - addr can be null if we are just looking for the port, for example, during
    // a who-is-router-to-network.
  
    ROUTER_PORT *port = headRouterPort;
    DNET *dnet;
  
    /* for broadcast messages no search is needed */
    if (net == BACNET_BROADCAST_NETWORK || net == 0)
    {
        // null also indicates 'all' ports
        panic();
        return NULL;
    }
  
    while (port != NULL)
    {

        /* check if DNET is directly connected to the router */
        if (net == port->route_info.configuredNet.net)
        {
            // todo 3 - this can block us from responding to a who-is-network message if the router is busy. Think about this
            if(IsDNETbusy(&port->route_info.configuredNet)) return NULL;
            // if it is, then the mac address does not exist in the cache,
            // but will be determined later
            return port;
        }
        /* else search router ports DNET list */
        else if(port->route_info.dnets2)
        {
            dnet = port->route_info.dnets2;
            while (dnet != NULL)
            {
                if (net == dnet->net)
                {
                    // todo 3 - this can block us from responding to a who-is-network message if the router is busy. Think about this
                    if(IsDNETbusy(dnet)) return NULL;
                    if (addr)
                    {
                        bacnet_mac_copy(addr, &dnet->phyMac);
                    }
                    return port;
                }
                dnet = dnet->next;
            }
        }
        port = (ROUTER_PORT *)port->llist.next;
    }
    return NULL;
}


ROUTER_PORT* find_routerport_from_portID(
    const uint16_t portID)
{
    ROUTER_PORT* port = headRouterPort;
    while (port != NULL)
    {
        /* check if DNET is directly connected to the router */
        if (portID == port->port_id)
        {
            return port;
        }
        port = (ROUTER_PORT*)port->llist.next;
    }
    panic();
    return NULL;
}


void InitDnet(DNET *dnet, uint16_t networkNumber)
{
    dnet->net = networkNumber;
    dnet->busy2 = false;
    dnet->next = NULL;
}


void add_dnet(
    RT_ENTRY *route_info,
    uint16_t networkNumber,
    BACNET_MAC_ADDRESS *macAddr)
{

    DNET *dnet = route_info->dnets2;
    DNET *tmp = NULL;

    if (dnet == NULL)
    {
        route_info->dnets2 = (DNET *)malloc(sizeof(DNET));
        if (route_info->dnets2 == NULL)
        {
            return;
        }
        //route_info->dnets->phyMac.len = addr->len;
        //memmove(route_info->dnets->phyMac.adr, addr->adr, MAX_MAC_LEN);
        bacnet_mac_copy(&route_info->dnets2->phyMac, macAddr);

        InitDnet(route_info->dnets2, networkNumber);
        //route_info->dnets->net = net;
        //route_info->dnets->busy2 = false;
        //route_info->dnets->next = NULL;
    }
    else
    {

        while (dnet != NULL)
        {
            if (dnet->net == networkNumber)       /* make sure NETs are not repeated, this will happen all the time due to repeated I-Am-Router etc.*/
            {
                return;
            }
            tmp = dnet;
            dnet = dnet->next;
        }

        dnet = (DNET *)malloc(sizeof(DNET));
        if (dnet == NULL)
        {
            panicDesc("m0019");
            return;
        }

        bacnet_mac_copy(&dnet->phyMac, macAddr);

        InitDnet(dnet, networkNumber);

        if (tmp != NULL) {
            tmp->next = dnet;
        }
    }
}


void remove_dnet(
    RT_ENTRY *route_info,
    uint16_t net)
{
    bits_mutex_lock(mutexDnet);

    DNET *dnet = route_info->dnets2;
    DNET *prev;

    if (dnet == NULL)
    {
        bits_mutex_unlock(mutexDnet);
        return;
    }

    // check the first one

    if(dnet->net == net)
    {
        route_info->dnets2 = dnet->next;
        free(dnet);
        bits_mutex_unlock(mutexDnet);
        return;
    }

    // step to the second one
    prev = dnet;
    dnet = dnet->next;

    while (dnet != NULL)
    {
        if (dnet->net == net)
        {
            prev->next = dnet->next;
            free(dnet);
            bits_mutex_unlock(mutexDnet);
            return;
        }
        prev = dnet;
        dnet = dnet->next;
    }

    bits_mutex_unlock(mutexDnet);
}

void cleanup_dnets(
    DNET *dnets)
{

    DNET *dnet = dnets;
    while (dnet != NULL)
    {
        dnet = dnet->next;
        free(dnets);
        dnets = dnet;
    }
}


#endif // #if ( BITS_ROUTER_LAYER == 1)
