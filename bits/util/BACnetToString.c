/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#include <stdio.h>

#include "bacTarget.h"
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


bool StringTo_IPEP (
    SOCKADDR_IN *ipep,
    const char *string)
{
    unsigned mac[6];
    unsigned port;
    int count = 0;

    count = sscanf(string, "%u.%u.%u.%u:%u", &mac[0], &mac[1], &mac[2],
        &mac[3], &port);

    if (count != 5) return false;

    // ipaddr is in wire order, so take advantage of the fact we don't have to shuffle bytes, and remove MSVC dependencies

    ((uint8_t *)&ipep->sin_addr)[0] = mac[0];
    ((uint8_t *)&ipep->sin_addr)[1] = mac[1];
    ((uint8_t *)&ipep->sin_addr)[2] = mac[2];
    ((uint8_t *)&ipep->sin_addr)[3] = mac[3];

    ipep->sin_port = htons(port);

    return true;
}

const char *IPEP_ToString (
    char *string,
    const SOCKADDR_IN *ipep)
{
    sprintf(string, "%u.%u.%u.%u:%u",
    		((uint8_t *)&ipep->sin_addr)[0],
			((uint8_t *)&ipep->sin_addr)[1],
			((uint8_t *)&ipep->sin_addr)[2],
			((uint8_t *)&ipep->sin_addr)[3],
        ntohs(ipep->sin_port));
    return string;
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


char *BVLCToString(uint8_t *pdu)
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
    int apdu_offset;
//    int apdu_len;
    BACNET_ADDRESS src;
    BACNET_ADDRESS dest;

    int iptr = 0;
    if (pdu[iptr++] != 1) return "Wrong Ver";

    uint8_t npci = pdu[iptr++];
    if (npci != 1)
    {
        return "Does not start with BAC version (1)";
    }

    apdu_offset = npdu_decode(pdu, &dest, &src, &npci_data);

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
            addr->adr[0],
            addr->adr[1],
            addr->adr[2],
            addr->adr[3],
            (addr->adr[4] << 8) | addr->adr[5]);
#else
        sprintf(tbuf, "%03d.%03d.%03d.%03d:%d",
            addr->adr[0],
            addr->adr[1],
            addr->adr[2],
            addr->adr[3],
            (addr->adr[4] << 8) | addr->adr[5]);
#endif
        return tbuf;
    }

    return "m0001 - todo";
}

