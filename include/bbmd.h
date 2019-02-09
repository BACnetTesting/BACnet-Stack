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

// Just a note. BVLC (and bvlc.h) is required, and perfectly usable, in the absence of a BBMD.
// Henceforth, bvlc.h has been modified and bbmd.h created to separate the requirements.

#pragma once 

#include <stdint.h>
//#include <stdio.h>
#include "stdbool.h"
#include "config.h"

#include "net.h"
//#include <stdlib.h>
//#include <time.h>
//#define WIN32_LEAN_AND_MEAN
//#define STRICT 1
//#include <windows.h>
//#include <stdint.h>
//#include <inaddr.h>

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)

#ifdef DATALINK_H
#error This file (bbmd.h) must be included before datalink.h
#endif

typedef struct _PORT_SUPPORT PORT_SUPPORT;

#ifndef MAX_BBMD_ENTRIES
#define MAX_BBMD_ENTRIES 200
#endif

#ifndef MAX_FD_ENTRIES
#define MAX_FD_ENTRIES 16
#endif

/*Each device that registers as a foreign device shall be placed
in an entry in the BBMD's Foreign Device Table (FDT). Each
entry shall consist of the 6-octet B/IP address of the registrant;
the 2-octet Time-to-Live value supplied at the time of
registration; and a 2-octet value representing the number of
seconds remaining before the BBMD will purge the registrant's FDT
entry if no re-registration occurs. This value will be initialized
to the 2-octet Time-to-Live value supplied at the time of
registration.*/

typedef struct {
    bool valid;
    /* BACnet/IP address */
    struct in_addr dest_address;
    /* BACnet/IP port number - not always 47808=BAC0h */
    uint16_t dest_port;
    /* seconds for valid entry lifetime */
    uint16_t time_to_live;
    /* our counter */
    time_t seconds_remaining;   /* includes 30 second grace period */
} FD_TABLE_ENTRY;

typedef struct {
    /* true if valid entry - false if not */
    bool valid;
    /* BACnet/IP address */
    struct in_addr dest_address;        /* in network format */
    /* BACnet/IP port number - not always 47808=BAC0h */
    uint16_t dest_port;                 /* in network format */
    /* Broadcast Distribution Mask */
    struct in_addr broadcast_mask;      /* in network format */
} BBMD_TABLE_ENTRY;

int bvlc_get_bdt_local(
    const BBMD_TABLE_ENTRY** table);

/* Invalidate all entries in the broadcast distribution table */
void bbmd_clear_bdt_local(
    PORT_SUPPORT *portParams);

void bbmd_clear_fdt_local(
    PORT_SUPPORT *portParams);    // todo3 - I dont think karg had this, add to BTC?

/* Add new entry to broadcast distribution table. Returns true if the new
* entry was added successfully */
bool bvlc_add_bdt_entry_local(
    BBMD_TABLE_ENTRY* entry);

/* Disable NAT handling of BBMD.
*/
void bvlc_disable_nat(void);

void bvlc_maintenance_timer(
    time_t seconds);

#endif
