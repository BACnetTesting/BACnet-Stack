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

 *   As a special exception, if other files instantiate templates or
 *   use macros or inline functions from this file, or you compile
 *   this file and link it with other works to produce a work based
 *   on this file, this file does not by itself cause the resulting
 *   work to be covered by the GNU General Public License. However
 *   the source code for this file must still be made available in
 *   accordance with section (3) of the GNU General Public License.
 *
    For more information : info@bac-test.com
        For access to source code : 
                info@bac-test.com
                    or 
                www.github.com/bacnettesting/bacnet-stack

 *********************************************************************/

#include "bacnet/bits/util/BACnetToString.h"
#include "bitsUtil.h"
#include "multipleDatalink.h"
#include "bacnet/whois.h"
#include "bacnet/bactext.h"
#include "eLib/util/eLibUtil.h"

// This has to be a bullet-proof pdu-to-string decoder...
// Decode first, then convert

typedef struct
{
    const int   val;
    const char  *name;
} StringTable ;


// "BACnet Port Type"
StringTable BPT_Typetab[] =
{
    { BPT_APP,      "Appl"} ,
    { BPT_VIRT,     "Virt"} ,
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    { BPT_BBMD,     "BBMD"} ,
#endif
#if (BACDL_FD == 1)
    { BPT_FD,       "FD"} ,
#endif
#if (BACDL_ETHERNET == 1)
    { BPT_Ethernet, "Eth"} ,
#endif
#if (BACDL_MSTP == 1)
    { BPT_MSTP,     "MS/TP"} ,
#endif
    { BPT_BIP,      "IP"}
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


// Converts ASCII IPEP to network order IP Address and Port
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
    struct in_addr  *nwoIPaddr,
    const char *string)
{
    unsigned mac[4];
    int count = 0;

    count = sscanf(string, "%u.%u.%u.%u", &mac[0], &mac[1], &mac[2], &mac[3] );
    if (count != 4) return false;

    // ipaddr is in wire order, so take advantage of the fact we don't have to shuffle bytes, and we can ignore endian dependencies

    ((uint8_t *)nwoIPaddr)[0] = mac[0];
    ((uint8_t *)nwoIPaddr)[1] = mac[1];
    ((uint8_t *)nwoIPaddr)[2] = mac[2];
    ((uint8_t *)nwoIPaddr)[3] = mac[3];

    return true;
}


const char *IPAddr_Port_ToString(char *string, const struct in_addr *nwoIPaddr, const uint16_t nwoPort)
{
    // ipaddr will always be in network order
    sprintf(
        string,
        "%u.%u.%u.%u:%u",
        ((uint8_t *)&nwoIPaddr->s_addr)[0],
        ((uint8_t *)&nwoIPaddr->s_addr)[1],
        ((uint8_t *)&nwoIPaddr->s_addr)[2],
        ((uint8_t *)&nwoIPaddr->s_addr)[3],
        ntohs(nwoPort) );
    return string;
}


const char *IPEP_ToString (
    char *string,
    const struct sockaddr_in *ipep)
{
    return IPAddr_Port_ToString(string, &ipep->sin_addr, ipep->sin_port);
}


const char *IPAddr_ToString(
    char *string,
    const struct in_addr *ipaddr )
{
    // ipaddr will always be in network order
    sprintf(
        string,
        "%u.%u.%u.%u",
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


static void append(char *tbuf, uint16_t tbuflen, uint16_t *cursor, const char *newstring)
{
    uint16_t len = (uint16_t)strlen(newstring);
    if ((*cursor + len) >= (tbuflen - 1)) {
        panic();
        return ;
    }
    bits_strlcpy( &tbuf[*cursor], newstring, tbuflen - *cursor - len);
    *cursor = *cursor + len;
}


char *bits_itoa(int value, char* str, int radix) {
    static char dig[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
    if (radix == 10 && value < 0) {
        value = -value;
        neg = 1;
    }
    v = value;
    do {
        str[n++] = dig[v % radix];
        v /= radix;
    } while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    for (p = str, q = p + (n - 1); p < q; ++p, --q)
        c = *p, *p = *q, *q = c;
    return str;
}


const char *BACnetGlobalAddressToString(char *tbuf, const uint tbuflen, uint16_t *cursor, const BACNET_GLOBAL_ADDRESS *adr )
{
    int len = snprintf(&tbuf[*cursor], tbuflen - *cursor, "%s/%u", BACnetMacAddrToString( &adr->mac ), adr->net);
    *cursor = *cursor + len;
    return tbuf;
}


const char *BACnetNPDUtoString( char *tbuf, uint tbuflen,  const uint8_t *pdu, const uint16_t pdulen )
{
    BACNET_NPCI_DATA npci_data;
    BACNET_GLOBAL_ADDRESS src;
    BACNET_GLOBAL_ADDRESS dest;

    int iptr = 0;
    uint16_t cursor = 0;
    if (pdu[iptr++] != 1) {
        append (tbuf, tbuflen, &cursor, "Does not start with BAC version (1)" );
        return tbuf;
    }

    int npcilen = npci_decode(pdu, &dest, &src, &npci_data);

    // append(tbuf, tbuflen, &cursor, "  : ");
    BACnetGlobalAddressToString(tbuf, tbuflen, &cursor, &src);
    append(tbuf, tbuflen, &cursor, "  to  ");
    BACnetGlobalAddressToString(tbuf, tbuflen, &cursor, &dest);
    append(tbuf, tbuflen, &cursor, "  :  ");

    if (npci_data.network_layer_message) {
        append(tbuf, tbuflen, &cursor, "Network Message");
        return tbuf;
    }

    // look for a Who-Is
    int32_t low, high;

    switch ((BACNET_PDU_TYPE)pdu[npcilen]) {
    case PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST:
        switch ((BACNET_UNCONFIRMED_SERVICE)pdu[npcilen+1]) {
        case SERVICE_UNCONFIRMED_WHO_IS:

            whois_decode_service_request(&pdu[npcilen + 2], (uint16_t)( pdulen - npcilen - 2) , &low, &high);

            append(tbuf, tbuflen, &cursor, "Who-Is");
            if (low >= 0) {
                char ttbuf[20];
                append(tbuf, tbuflen, &cursor, ", ");
                append(tbuf, tbuflen, &cursor, bits_itoa(low,ttbuf,10));
                append(tbuf, tbuflen, &cursor, " - ");
                append(tbuf, tbuflen, &cursor, bits_itoa(high, ttbuf, 10));
            }

            break;

        case SERVICE_UNCONFIRMED_I_AM:
            append(tbuf, tbuflen, &cursor, "I-Am");
            break;

        default :
            append(tbuf, tbuflen, &cursor, "mxxxx - Application Message");
            break;
        }
        break;

    default:
        append(tbuf, tbuflen, &cursor, "mxxxx - Application Message");
        break;
    }

    return tbuf;

}


const char *BACnetMacAddrToString(const BACNET_MAC_ADDRESS *addr)
{
    static char tbuf[30];

    switch (addr->macType)
    {
    case MAC_TYPE_BIP :
#if ( BACNET_STACK_BIG_ENDIAN == 1 )
        sprintf_s(tbuf, sizeof(tbuf), "%u.%u.%u.%u:%d",
            addr->bytes[5],
            addr->bytes[4],
            addr->bytes[3],
            addr->bytes[2],
            (addr->bytes[1] << 8) | addr->bytes[0]);
#else
        sprintf(tbuf, "%u.%u.%u.%u:%d",
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3],
            (addr->bytes[4] << 8) | addr->bytes[5]);
#endif
        return tbuf;

#if ( BACDL_ETHERNET == 1 )
    case MAC_TYPE_ETHERNET:
        sprintf(tbuf, "%02x-%02x-%02x-%02x-%02x-%02x",
            addr->bytes[0],
            addr->bytes[1],
            addr->bytes[2],
            addr->bytes[3],
            addr->bytes[4],
            addr->bytes[5]);
        return tbuf;
#endif
    }


    switch ( addr->len )
    {
    case 1:
        sprintf(tbuf, "%u", addr->bytes[0]);
        return tbuf;
	    
    case 2:
	    sprintf(tbuf, "%u:%u", 
		    addr->bytes[0],
		    addr->bytes[1] );
		return tbuf ;

    case 0:
        return ("B'Cast");
    }

    return "m0026 - todo";
}


const char *BACnet_ObjectID_ToString(const BACNET_OBJECT_TYPE objectType, const uint32_t objectInstance )
{
    // todo 0 - static review
    static char tbuf[20];
    sprintf(tbuf, "%s:%05u", bactext_object_type_acronym_by_enum(objectType), objectInstance);
    return tbuf;
}

