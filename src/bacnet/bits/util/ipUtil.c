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

#include "bitsIpUtil.h"
#include "eLib/util/eLibDebug.h"
#include "bacnet/basic/sys/sbuf.h"

// copy data, everything in network order...
void bits_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *mac, const uint32_t nwoIpAddr, const uint16_t nwoPort)
{
#if ( BAC_DEBUG == 1 )
    mac->signature = 'M';
#endif

    mac->bytes[0] = ((uint8_t *)&nwoIpAddr)[0];
    mac->bytes[1] = ((uint8_t *)&nwoIpAddr)[1];
    mac->bytes[2] = ((uint8_t *)&nwoIpAddr)[2];
    mac->bytes[3] = ((uint8_t *)&nwoIpAddr)[3];

    mac->bytes[4] = ((uint8_t *)&nwoPort)[0];
    mac->bytes[5] = ((uint8_t *)&nwoPort)[1];

    mac->len = 6;
}


void bits_bacnet_mac_to_ipAddr_port(
    const BACNET_MAC_ADDRESS* mac,
    struct in_addr* ipaddr,
    uint16_t* nwoPort)
{
#if ( BAC_DEBUG == 1 )
    if (mac->signature != 'M') {
        panic();
        return;
    }
#endif

    memcpy(&ipaddr->s_addr, mac->bytes, 4 );
    memcpy(nwoPort, &mac->bytes[4], 2);
}


unsigned bits_sbuf_to_ipAddr_port (
    STATIC_BUFFER *sbuf,
    struct in_addr* address,
    uint16_t* port)
{
    if (sbuf_remaining(sbuf) < 6) {
        panic();
        return 0;
    }

    memcpy(&address->s_addr, &sbuf->data[sbuf->processed], 4);
    memcpy(port, &sbuf->data[sbuf->processed+4], 2);
    sbuf->processed += 6;

    return 6;
}


bool isEqualIPEP(const struct sockaddr_in *ipepA, const struct sockaddr_in *ipepB)
{
    if (ipepA->sin_port != ipepB->sin_port) return false;
    if (ipepA->sin_addr.s_addr != ipepB->sin_addr.s_addr) return false;
    return true;
}


// not only IP, but other datalinks too. Move to bitsDatalink.c
void bacnet_mac_clear(
    BACNET_MAC_ADDRESS *mac)
{
    memset(mac, 0, sizeof(BACNET_MAC_ADDRESS));
#if ( BAC_DEBUG == 1 )
    mac->signature = 'M';
#endif
}


void bacnet_mac_set_uint16(
    BACNET_MAC_ADDRESS *mac,
    const uint16_t mac16)
{
    bacnet_mac_clear(mac);
    mac->bytes[0] = (uint8_t)((mac16 >> 8) & 0xff);
    mac->bytes[1] = (uint8_t)((mac16 >> 0) & 0xff);
    mac->len = 2;
}


#if ( BAC_DEBUG == 1)
bool bacnet_mac_check(
    const BACNET_MAC_ADDRESS * mac)
{
    if (mac->signature != 'M')
    {
        panic();
        return false;
    }

    if (mac->len > 7)
    {
        panic();
        return false;
    }
    return true;
}
#endif
