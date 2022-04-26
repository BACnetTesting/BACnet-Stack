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

#include "configProj.h"
#include "osNet.h"
#include "bacnet/bacdef.h"
#include "bacnet/basic/sys/sbuf.h"

typedef struct _PORT_SUPPORT PORT_SUPPORT;

typedef struct 
{
    struct in_addr local_address;
    struct in_addr broadcast_address;
    struct in_addr netmask;
} IP_ADDR_PARAMS ;


void bacnet_mac_clear(
    BACNET_MAC_ADDRESS *mac);

#if ( BAC_DEBUG == 1)
bool bacnet_mac_check(
    const BACNET_MAC_ADDRESS * mac);
#endif

bool isEqualIPEP(
    const struct sockaddr_in *ipepA,
    const struct sockaddr_in *ipepB);

void bits_ipAddr_port_to_bacnet_mac(
    BACNET_MAC_ADDRESS *mac,
    const uint32_t nwoIpAddr,
    const uint16_t nwoPort);

void bits_bacnet_mac_to_ipAddr_port(
    const BACNET_MAC_ADDRESS* mac,
    struct in_addr* ipaddr, 
    uint16_t *nwoPort);

unsigned bits_sbuf_to_ipAddr_port(
    STATIC_BUFFER* sbuf,
    struct in_addr* address,
    uint16_t* port);

int get_local_address_ioctl(
    const char *ifname,
    struct in_addr *addr,
    int request);

bool bits_get_port_params(
    PORT_SUPPORT *datalink,
    IP_ADDR_PARAMS *params);



