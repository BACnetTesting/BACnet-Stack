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
#include "bitsDebug.h"

// copy data, everything in network order...
void bip_ipAddr_port_to_bacnet_mac(BACNET_MAC_ADDRESS *addr, const uint32_t nwoIpAddr, const uint16_t nwoPort)
{
#if ( BAC_DEBUG == 1 )
    addr->signature = 'M';
#endif

    addr->adr[0] = ((uint8_t *)&nwoIpAddr)[0];
    addr->adr[1] = ((uint8_t *)&nwoIpAddr)[1];
    addr->adr[2] = ((uint8_t *)&nwoIpAddr)[2];
    addr->adr[3] = ((uint8_t *)&nwoIpAddr)[3];

    addr->adr[4] = ((uint8_t *)&nwoPort)[0];
    addr->adr[5] = ((uint8_t *)&nwoPort)[1];

    addr->len = 6;
}


bool isEqualIPEP(const struct sockaddr_in *ipepA, const struct sockaddr_in *ipepB)
{
    if (ipepA->sin_port != ipepB->sin_port) return false;
    if (ipepA->sin_addr.s_addr != ipepB->sin_addr.s_addr) return false;
    return true;
}

void bacnet_mac_clear(
    BACNET_MAC_ADDRESS *mac)
{
    memset(mac, 0, sizeof(BACNET_MAC_ADDRESS));
#if ( BAC_DEBUG == 1 )
    mac->signature = 'M';
#endif
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
