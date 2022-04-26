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

#ifndef BVLC_H
#define BVLC_H

#include <time.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacenum.h"
#include "osNet.h"

#ifdef DATALINK_H
#error This file must be included before (or rather, by) datalink.h. Remove the include from the C module.
#endif

typedef struct _PORT_SUPPORT PORT_SUPPORT;
typedef struct _BACNET_ROUTE BACNET_ROUTE;
typedef struct _DLCB         DLCB;

#if defined(BBMD_ENABLED) && BBMD_ENABLED
void bvlc_maintenance_timer( 
    PORT_SUPPORT *portParams,
    time_t seconds);
#else
#define bvlc_maintenance_timer(x)
#endif

/* todo 4 
    B/IP devices **shall** be capable of **resolving Internet host names to IP addresses via the use of the Domain Name Service** (see RFC
    1123) in the case where a DNS resolver is available.
*/

void bvlc_cleanup(
    void
    );

uint16_t bbmd_receive(
    PORT_SUPPORT * datalink,    // fills in the portParams
    BACNET_MAC_ADDRESS *rxMac,
    uint8_t * npdu,             /* returns the NPDU */
    uint16_t max_npdu);         /* amount of space available in the NPDU  */

void bbmd_send_npdu (
    PORT_SUPPORT* datalink,
    const DLCB *dlcb);

void fd_send_npdu(
    const PORT_SUPPORT* datalink,
    const DLCB *dlcb );

// made static
//int bvlc_send_mpdu(
//    const PORT_SUPPORT *portParams,
//    const struct sockaddr_in *ipSockAddr,
//    const uint8_t * mtu,
//    const uint16_t mtu_len);

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
int bvlc_encode_write_bdt_init(
    uint8_t * pdu,
    unsigned entries);

int bvlc_encode_read_fdt(
    uint8_t * pdu);

int bvlc_encode_delete_fdt_entry(
    uint8_t * pdu,
    uint32_t address,       /* in network byte order */
    uint16_t port); /* in network byte order */

int bvlc_encode_original_unicast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length);

int bvlc_encode_original_broadcast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length);
#endif

int bvlc_encode_read_bdt(
    uint8_t * pdu);

int bvlc_bbmd_read_bdt(
    BACNET_ROUTE *rxDetails,
    uint32_t bbmd_address,
    uint16_t bbmd_port);

/* registers with a bbmd as a foreign device */
int bvlc_register_with_bbmd(
    PORT_SUPPORT *portParams,
    //uint32_t bbmd_address,  /* in network byte order */
    //uint16_t bbmd_port,     /* in network byte order */
    uint16_t time_to_live_seconds);

/* Note any BVLC_RESULT code, or NAK the BVLL message in the unsupported cases. */
int bvlc_for_non_bbmd2(
    const PORT_SUPPORT *portParams, 
    struct sockaddr_in *sout,
    uint8_t * npdu,
    uint16_t received_bytes);

/* Returns the last BVLL Result we received, either as the result of a BBMD
 * request we sent, or (if not a BBMD or Client), from trying to register
 * as a foreign device. */
BACNET_BVLC_RESULT bvlc_get_last_result(
    void);

/* Returns the current BVLL Function Code we are processing.
 * We have to store this higher layer code for when the lower layers
 * need to know what it is, especially to differentiate between
 * BVLC_ORIGINAL_UNICAST_NPDU and BVLC_ORIGINAL_BROADCAST_NPDU.  */
BACNET_BVLC_FUNCTION bvlc_get_function_code(
    void);


/* Local interface to manage BBMD.
 * The interface user needs to handle mutual exclusion if needed i.e.
 * BACnet packet is not being handled when the BBMD table is modified.
 */


    /* Backup broadcast distribution table to a file.
     * Filename is the BBMD_BACKUP_FILE constant
     */
     
#ifdef BDT_BACKUP
    void bvlc_bdt_backup_local(
        void);

    /* Restore broadcast distribution from a file.
     * Filename is the BBMD_BACKUP_FILE constant
     */
    void bvlc_bdt_restore_local(
        void);
#endif

/* NAT handling
 * If the communication between BBMDs goes through a NAT enabled internet
 * router, special considerations are needed as stated in Annex J.7.8.
 *
 * In short, the local IP address of the BBMD is different than the global
 * address which is visible to the other BBMDs or foreign devices. This is
 * why the source address in forwarded messages needs to be changed to the
 * global IP address.
 *
 * For other considerations/limitations see Annex J.7.8.
 */

/* Set global IP address of a NAT enabled router which is used in forwarded
 * messages. Enables NAT handling.
 */
void bvlc_set_global_address_for_nat(const struct sockaddr_in* addr);

/* Disable NAT handling of BBMD.
 */
// void bvlc_disable_nat(void);

#endif /* */
