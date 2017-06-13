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
 -------------------------------------------
####COPYRIGHTEND####*/
//#include <stddef.h>
//#include <stdint.h>
//#include <stdbool.h>
//// #include "config.h"
#include "bacdef.h"
//#include "bacaddr.h"
#include "datalink.h"
#include "CEDebug.h"

/** @file bacaddr.c  BACnet Address structure utilities */

void bacnet_mac_copy(
    BACNET_MAC_ADDRESS * dest,
    const BACNET_MAC_ADDRESS * src)
{
    memcpy(dest, src, sizeof(BACNET_MAC_ADDRESS));
}

void bacnet_address_clear ( BACNET_GLOBAL_ADDRESS *adr )
{
  memset ( adr, 0, sizeof ( BACNET_GLOBAL_ADDRESS ) ) ;
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
    memcpy(dest, src, sizeof(BACNET_PATH));
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

bool bacnet_mac_same(
    const BACNET_MAC_ADDRESS *mac1, 
    const BACNET_MAC_ADDRESS *mac2)
{
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
//    uint8_t i ;             /* loop counter */
//    uint8_t max_len ;       /* used for dynamic max */

    // todo3 - this is very suspicious, if what we want are the same objects, why bother with the rest
    // so why is this used, and where are the subtle use differences between bacnet_address_same and address_match
    if (dest == src)    /* same ? */
        return true;

    if (dest->net != src->net)
        return false;

    //if (dest->len != src->len)
    return bacnet_mac_same(&dest->mac, &src->mac);
}


bool bacnet_path_same(
    const BACNET_PATH * dest,
    const BACNET_PATH * src)
{
    uint8_t i;             /* loop counter */
    uint8_t max_len;       /* used for dynamic max */

    if (dest == src)    /* same ? */
        return true;

    if (dest->glAdr.net != src->glAdr.net)
        return false;

    //if (dest->len != src->len)
    //    return false;

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

    // todo2 - panic if localmac not the same !! should never occur (unless rediscovering)...

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


#if  0
// never used.
void set_broadcast_addr_global(BACNET_GLOBAL_ADDRESS *dest)
{
    dest->net = 65535;
    dest->mac.len = 0;      // indicates b'cast on remote net
}

void set_broadcast_addr_local(BACNET_GLOBAL_ADDRESS *dest)
{
    dest->net = 0;
    dest->mac.len = 0;      // indicates b'cast on remote net
}
#endif


void bacnet_path_set_broadcast_global(BACNET_PATH *dest)
{
    dest->localMac.len = 0;     // indicates b'cast on local net
    dest->glAdr.net = 0xffff ;
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

