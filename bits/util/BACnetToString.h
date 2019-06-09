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
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#include "bacdef.h"
#include "datalink.h"

const char *BACnetPktToString(uint8_t *pdu);
const char *BACnetMacAddrToString(BACNET_MAC_ADDRESS *addr);
//const char *BPT_ToString(BPT_TYPE pf);
//BPT_TYPE StringTo_PF(const char *name);
bool StringTo_IPEP(struct sockaddr_in *ipep, const char *string);
const char *IPEP_ToString(char *string, const struct sockaddr_in *ipep);
const char *IPAddr_ToString(char *string, const struct in_addr *ipaddr);			// todo - not sure why I cant define IN_ADDR in linux

