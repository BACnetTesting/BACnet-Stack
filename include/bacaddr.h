/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef BACADDR_H
#define BACADDR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
// #include "datalink.h"

//#if defined ( DEVICE_H )
//#error This file must be included before device.h
//#endif
//#if defined ( DATALINK_H ) 
//#error This file must be included before datalink.h
//#endif

typedef struct _BACNET_ROUTE BACNET_ROUTE;


bool bacnet_mac_same(
    const BACNET_MAC_ADDRESS *mac1,
    const BACNET_MAC_ADDRESS *mac2);

void bacnet_mac_clear(
    BACNET_MAC_ADDRESS *mac);

void bacnet_mac_copy(
    BACNET_MAC_ADDRESS *target,
    const BACNET_MAC_ADDRESS *src);

void bacnet_mac_check(
    const BACNET_MAC_ADDRESS * mac);

void bacnet_address_copy(
    BACNET_GLOBAL_ADDRESS * dest,
    const BACNET_GLOBAL_ADDRESS * src);

void bacnet_address_clear(BACNET_GLOBAL_ADDRESS *adr);

bool bacnet_address_same(
    const BACNET_GLOBAL_ADDRESS *adr,
    const BACNET_GLOBAL_ADDRESS *adr2);

bool bacnet_path_same(
    const BACNET_PATH * dest,
    const BACNET_PATH * src);

void bacnet_path_copy(
    BACNET_PATH * dest,
    const BACNET_PATH * src);

bool bacnet_route_same(
    const BACNET_ROUTE *r1,
    const BACNET_ROUTE *r2);

//BACNET_MAC_ADDRESS *ResolveLocalMac( 
//    BACNET_PATH *targetPath );

void bacnet_path_set_broadcast_global(BACNET_PATH *dest);

void bacnet_path_set_broadcast_local(BACNET_PATH *dest);

#endif
