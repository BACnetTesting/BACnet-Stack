/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 *
 *****************************************************************************************
 *
 *   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
 *
 *   July 1, 2017    BITS    Modifications to this file have been made in compliance
 *                           with original licensing.
 *
 *   This file contains changes made by BACnet Interoperability Testing
 *   Services, Inc. These changes are subject to the permissions,
 *   warranty terms and limitations above.
 *   For more information: info@bac-test.com
 *   For access to source code:  info@bac-test.com
 *          or      www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/
 
#ifdef _MSC_VER
#include <memory.h>
#else
// for IAR compiler - memcpy..
#include <string.h>
#endif

#include "bacnet/bacdef.h"
#include "bacnet/bacaddr.h"
#include "eLib/util/eLibDebug.h"
#include "bacnet/bits/util/multipleDatalink.h"

/** @file bacaddr.c  BACnet Address structure utilities */

void bacnet_mac_copy(
    BACNET_MAC_ADDRESS * dest,
    const BACNET_MAC_ADDRESS * src)
{
#if ( BAC_DEBUG == 1 )
    if ( src->signature != 'M' )
    {
      panic();
      return ;
    }
    if (src->len != 1 && src->len != 0 && src->len != 6 && src->len != 2 ) 
    {
      panic();
      return ;
    }
#endif
    memcpy(dest, src, sizeof(BACNET_MAC_ADDRESS));
}




void bacnet_address_clear ( BACNET_GLOBAL_ADDRESS *adr )
{
  adr->net = 0 ;
  bacnet_mac_clear ( &adr->mac ) ;
}

void bacnet_address_copy(
    BACNET_GLOBAL_ADDRESS * dest,
    const BACNET_GLOBAL_ADDRESS * src)
{
    memcpy(dest, src, sizeof(BACNET_GLOBAL_ADDRESS));
}


void bacnet_path_copy(
    BACNET_PATH * dest,
    const BACNET_PATH * src)
{
#if ( BAC_DEBUG == 1 )
    bacnet_mac_check ( &src->localMac ) ;
#endif
    memcpy(dest, src, sizeof(BACNET_PATH));
}


void bacnet_path_clear(
    BACNET_PATH * dest)
{
	bacnet_mac_clear( &dest->localMac );
	bacnet_address_clear( &dest->glAdr );
}


void bacnet_route_copy(
    BACNET_ROUTE *rdest,
    const BACNET_ROUTE *rsrc)
{
    memcpy(rdest, rsrc, sizeof(BACNET_ROUTE));
}


void bacnet_route_clear(BACNET_ROUTE *rdest )
{
    memset(rdest, 0, sizeof(BACNET_ROUTE));
}


bool bacnet_route_same(const BACNET_ROUTE *r1, const BACNET_ROUTE *r2)
{
    // todo 3 there is an opportunity for duplicate node detection here - take it !
    if (r1->portParams != r2->portParams) return false;
    return (bacnet_path_same(&r1->bacnetPath, &r2->bacnetPath));
}


bool bacnet_mac_same(
    const BACNET_MAC_ADDRESS *mac1,
    const BACNET_MAC_ADDRESS *mac2)
{
  if ( mac1->len != mac2->len ) return false ;
    int max_len = mac1->len;
    if (max_len > MAX_MAC_LEN) {
        max_len = MAX_MAC_LEN;
        panic();
        return false;
    }

    for (int i = 0; i < max_len; i++) {
        if (mac1->bytes[i] != mac2->bytes[i])
            return false;
    }
    return true;
}


bool bacnet_address_same(
    const BACNET_GLOBAL_ADDRESS * dest,
    const BACNET_GLOBAL_ADDRESS * src)
{
    // todo 3 - this is very suspicious, if what we want are the same objects, why bother with the rest
    // so why is this used, and where are the subtle use differences between bacnet_address_same and address_match
    if (dest == src)    /* same ? */
        return true;

    if (dest->net != src->net)
        return false;

    return bacnet_mac_same(&dest->mac, &src->mac);
}

void bacnet_address_clr(BACNET_GLOBAL_ADDRESS *adr)
{
    memset(adr, 0, sizeof(BACNET_GLOBAL_ADDRESS));
}


bool bacnet_path_same(
    const BACNET_PATH * dest,
    const BACNET_PATH * src)
{
    uint8_t i;             /* loop counter */
    uint8_t max_len;       /* used for dynamic max */

    // todo 3 - this is very suspicious, if what we want are the same objects, why bother with the rest
    // so why is this used, and where are the subtle use differences between bacnet_address_same and address_match
    if (dest == src)    /* same ? */
        return true;

    if (dest->glAdr.net != src->glAdr.net)
        return false;

    max_len = dest->glAdr.mac.len;
    if (max_len > MAX_MAC_LEN) {
        max_len = MAX_MAC_LEN;
        panic();
        return false;
    }

    for (i = 0; i < max_len; i++) {
        if (dest->glAdr.mac.bytes[i] != src->glAdr.mac.bytes[i])
            return false;
    }

    if (dest->glAdr.net == 0) {
        if (dest->localMac.len != src->localMac.len)
            return false;
        max_len = dest->localMac.len;
        if (max_len > MAX_MAC_LEN)
        {
            panic();
            return false;
        }
        for (i = 0; i < max_len; i++) {
            if (dest->localMac.bytes[i] != src->localMac.bytes[i])
                return false;
        }
    }
    return true;
}


void bacnet_path_set_broadcast_global(BACNET_PATH *dest)
{
#if ( BAC_DEBUG == 1 )
    bacnet_path_clear(dest);
#endif

    dest->localMac.len = 0;     // indicates b'cast on local net
    dest->glAdr.net = BACNET_BROADCAST_NETWORK ;
    dest->glAdr.mac.len = 0;      // indicates b'cast on remote net
}


void bacnet_path_set_broadcast_remote(BACNET_PATH *dest, uint16_t remoteNet )
{
    dest->localMac.len = 0;     // indicates b'cast on local net
    dest->glAdr.net = remoteNet ;
    dest->glAdr.mac.len = 0;      // indicates b'cast on remote net
}


void bacnet_path_set_broadcast_local(BACNET_PATH *dest)
{
    memset( dest, 0, sizeof (BACNET_PATH)) ;
    //dest->localMac.len = 0;        // indicates b'cast on local net
    //dest->glAdr.net = 0;
    //dest->glAdr.mac.len = 0;      // indicates b'cast on remote net
}

void set_local_broadcast_mac(BACNET_MAC_ADDRESS *mac)
{
    mac->len = 0;
}


void copy_bacnet_mac(BACNET_MAC_ADDRESS *target, BACNET_MAC_ADDRESS *src)
{
#if ( DB_DEVELOPMENT == DB_TRUE )
    if (src->len > MAX_MAC_LEN)
    {
        panicDesc("Bad MAC Address");
    }
#endif
    memcpy(target, src, sizeof(BACNET_MAC_ADDRESS));
}


//BACNET_MAC_ADDRESS *ResolveLocalMac(
//    BACNET_PATH *destPath)
//{
//    bacnet_mac
//        if ((destPath->glAdr.net == BACNET_BROADCAST_NETWORK) || ( destPath->localMac.len == 0)) {
//            /* broadcast */
//            // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
//            // // port = BIP_Port;
//            // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
//            targetMac = NULL;   // broadcast
//        }
//        else if ((destPath->glAdr.net > 0) && (destPath->localMac.len == 0)) {
//            /* network specific (remote) broadcast */
//            if (destPath->localMac.len == 6) {
//                // bip_decode_bip_address(dest, &address, &port);
//                targetMac = &destPath->localMac;
//            }
//            else {
//                // address.s_addr = portParams->bipParams.broadcast_addr; // BIP_Broadcast_Address.s_addr;
//                // // port = BIP_Port;
//                targetMac = NULL;   // broadcast
//            }
//            // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
//        }
//        else if (destPath->localMac.len == 6) {
//            // bip_decode_bip_address(dest, &address, &port);
//            // mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
//            targetMac = &destPath->localMac ;
//        }
//        else {
//            panicDesc("m0020: Cannot resolve local MAC from Path for IP");
//            return NULL ;
//        }
//        return targetMac;
//}
