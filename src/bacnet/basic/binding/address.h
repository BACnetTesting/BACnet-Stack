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

#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>
#include "configProj.h"
#include "bacnet/readrange.h"

typedef struct devObj_s DEVICE_OBJECT_DATA;
typedef struct _PORT_SUPPORT PORT_SUPPORT;

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1)

#include "readrange.h"
#include "multipleDatalink.h"

typedef struct _BACNET_ROUTE BACNET_ROUTE;

typedef struct Address_Cache_Entry
{
    uint8_t         Flags;
    uint32_t        device_id;
    uint16_t        max_apdu;                  // the maximum APDU size accepted by the DESTINATION DEVICE 

    // cr187466584365384
    // 2019.10.30 EKH: Changed route to path because a _router_ based application is not sensitive to what datalink the device ultimately resides on...
    // BACNET_ROUTE    address;
    BACNET_PATH     bacnetPath;

    uint32_t        TimeToLive;
} ADDRESS_CACHE_ENTRY ; //  Address_Cache[MAX_ADDRESS_CACHE];

void address_init(
    PORT_SUPPORT* datalink);

void address_init_partial(
    PORT_SUPPORT* datalink);

void address_add(
    PORT_SUPPORT* datalink,
    uint32_t device_id,
    uint16_t max_apdu,
    BACNET_PATH * src);

void address_remove_device(
    PORT_SUPPORT* datalink,
    uint32_t device_id);

bool address_get_by_device(
    PORT_SUPPORT *datalink,
    uint32_t device_id,
    uint16_t *max_apdu,
    BACNET_PATH * src);

//bool address_get_route_from_global_addr(
//    const BACNET_GLOBAL_ADDRESS *globAdr,
//    BACNET_ROUTE * src);

//bool address_get_route_from_device_id(
//    const uint32_t device_id,
//    BACNET_ROUTE * src);

bool address_bound(
    PORT_SUPPORT* datalink,
    uint32_t device_id);

bool address_get_by_index(
    PORT_SUPPORT* datalink,
    unsigned objectIndex,
    uint32_t * device_id,
    uint16_t *max_apdu,
    BACNET_PATH * src);

bool address_device_get_by_index(
    PORT_SUPPORT* datalink,
    unsigned index,
    uint32_t * device_id,
    uint32_t * device_ttl,
    uint16_t *max_apdu,
    BACNET_PATH * src);

unsigned address_count(
    PORT_SUPPORT* datalink);

bool address_bind_request(
    PORT_SUPPORT* datalink,
    const uint32_t device_id,
    uint16_t *max_apdu,
    BACNET_PATH * src);

bool address_device_bind_request(
    PORT_SUPPORT* datalink,
    const uint32_t device_id,
    uint32_t * device_ttl,
    uint16_t *max_apdu,
    BACNET_PATH * src);

void address_add_binding(
    uint32_t device_id,
    uint16_t max_apdu,
    BACNET_PATH * src);
#endif // client 

int address_list_encode(
    PORT_SUPPORT* datalink,
    uint8_t * apdu,
    uint16_t apdu_len);

#if  (BACNET_SVC_RR_B == 1)

int rr_address_list_encode(
    DEVICE_OBJECT_DATA* pDev,
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest);

#endif

#if ( BACNET_CLIENT == 1 )

void address_set_device_TTL(
    PORT_SUPPORT* datalink,
    uint32_t device_id,
    uint32_t TimeOut,
    bool StaticFlag);

void address_cache_timer(
    PORT_SUPPORT* datalink,
    uint16_t uSeconds);

//void address_mac_init(
//    BACNET_PATH *mac,
//    uint8_t *adr,
//    uint8_t len);

bool address_mac_from_ascii(
    BACNET_MAC_ADDRESS *mac,
    char *arg);

void address_protected_entry_index_set(uint32_t top_protected_entry_index);
void address_own_device_id_set(uint32_t own_id);

#endif

#endif
