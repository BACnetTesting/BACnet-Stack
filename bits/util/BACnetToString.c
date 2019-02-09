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
        For access to source code : 
                info@bac-test.com
                    or 
                www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#include <stdio.h>

// #include "bacTarget.h"
#include "config.h"
//#include "bacenum.h"
#include "bacdef.h"
#include "npdu.h"
#include "BACnetToString.h"
#include "datalink.h"
#include "bitsUtil.h"
#include "net.h"

// This has to be a bullet-proof pdu-to-string decoder...
// Decode first, then convert

typedef struct
{
    const int   val;
    const char  *name;
} StringTable ;

StringTable BPT_Typetab[] =
{
    { BPT_BBMD,    "BBMD"} ,
    { BPT_MSTP,    "MSTP"} ,
    { BPT_BIP,     "B/IP"}
};


const BPT_TYPE StringTo_PF(const char *name)
{
    for (uint i = 0; i < sizeof(BPT_Typetab) / sizeof(StringTable); i++)
    {
        if (isMatchCaseInsensitive( name, BPT_Typetab[i].name )) return (const BPT_TYPE) BPT_Typetab[i].val ;
    }
    panic();
    return BPT_BIP ;
}


const char *BPT_ToString(BPT_TYPE pf)
{
    for (uint i = 0; i < sizeof(BPT_Typetab) / sizeof(StringTable); i++)
    {
        if ((int)pf == BPT_Typetab[i].val) return BPT_Typetab[i].name;
    }
    panic();
    return "m0022";
}

bool StringTo_IPEP (
    struct sockaddr_in  *ipep,
    const char *string)
{
    unsigned mac[6];
    unsigned port;
    int count = 0;

    count = sscanf(string, "%u.%u.%u.%u:%u", &mac[0], &mac[1], &mac[2],
        &mac[3], &port);

    if (count != 5) return false;

    // ipaddr is in wire order, so take advantage of the fact we don't have to shuffle bytes, and we can ignore endian dependencies

    ((uint8_t *)&ipep->sin_addr)[0] = mac[0];
    ((uint8_t *)&ipep->sin_addr)[1] = mac[1];
    ((uint8_t *)&ipep->sin_addr)[2] = mac[2];
    ((uint8_t *)&ipep->sin_addr)[3] = mac[3];

    ipep->sin_port = htons(port);

    return true;
}


// returns ipaddr in network order 
bool StringTo_IPaddr (
    struct in_addr  *ipaddr,
    const char *string)
{
    unsigned mac[4];
    int count = 0;

    count = sscanf(string, "%u.%u.%u.%u", &mac[0], &mac[1], &mac[2], &mac[3] );
    if (count != 4) return false;

    // ipaddr is in wire order, so take advantage of the fact we don't have to shuffle bytes, and we can ignore endian dependencies

    ((uint8_t *)&ipaddr)[0] = mac[0];
    ((uint8_t *)&ipaddr)[1] = mac[1];
    ((uint8_t *)&ipaddr)[2] = mac[2];
    ((uint8_t *)&ipaddr)[3] = mac[3];

    return true;
}


const char *IPEP_ToString (
    char *string,
    const struct sockaddr_in *ipep)
{
    sprintf(string, "%u.%u.%u.%u:%u",
    		((uint8_t *)&ipep->sin_addr)[0],
			((uint8_t *)&ipep->sin_addr)[1],
			((uint8_t *)&ipep->sin_addr)[2],
			((uint8_t *)&ipep->sin_addr)[3],
			ntohs(ipep->sin_port));
    return (const char *) string;
}


const char *IPAddr_ToString(
    char *string,
    const struct in_addr *ipaddr )
{
    sprintf(string, "%u.%u.%u.%u",
    		((uint8_t *)&ipaddr->s_addr)[0],
			((uint8_t *)&ipaddr->s_addr)[1],
			((uint8_t *)&ipaddr->s_addr)[2],
			((uint8_t *)&ipaddr->s_addr)[3] ) ;
    return (const char *) string;
}


const char *BVLCToString(uint8_t *pdu)
{
    int iptr = 0;
    if (pdu[iptr++] != 0x81) return "Not BVLC";

    BACNET_BVLC_FUNCTION bvlcf = (BACNET_BVLC_FUNCTION)pdu[iptr++];
        switch (bvlcf)
        {
        case BVLC_REGISTER_FOREIGN_DEVICE:
            return "BVLC: Register Foreign Device";
        case BVLC_ORIGINAL_BROADCAST_NPDU:
            return "BVLC: Original Broadcast";
        case BVLC_ORIGINAL_UNICAST_NPDU:
            return "BVLC: Original Unicast";
        case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
            return "BVLC: Distribute Broadcast";
        case BVLC_FORWARDED_NPDU:
            return "BVLC: Forward";
        default:
            break;
        }
    return "Decode Err";
}


const char *BACnetPktToString(uint8_t *pdu)
{
    BACNET_NPCI_DATA npci_data;
    // int apdu_offset;
//    int apdu_len;
    BACNET_GLOBAL_ADDRESS src;
    BACNET_GLOBAL_ADDRESS dest;

    int iptr = 0;
    if (pdu[iptr++] != 1) return "Wrong Ver";

    uint8_t npci = pdu[iptr++];
    if (npci != 1)
    {
        return "Does not start with BAC version (1)";
    }

    npci_decode(pdu, &dest, &src, &npci_data);

    if (npci_data.network_layer_message) return "Net Msg";

    return "App Msg";
}



const char *BACnetMacAddrToString(BACNET_MAC_ADDRESS *addr)
{
    static char tbuf[30];

    if ( addr->len == 6)
    {
        // ipep format 
#ifdef _MSC_VER
        sprintf_s(tbuf, sizeof(tbuf), "%03d.%03d.%03d.%03d:%d",
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3],
            (addr->bytes[4] << 8) | addr->bytes[5]);
#else
        sprintf(tbuf, "%03d.%03d.%03d.%03d:%d",
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3],
            (addr->bytes[4] << 8) | addr->bytes[5]);
#endif
        return tbuf;
    }

    return "m0026 - todo";
}

