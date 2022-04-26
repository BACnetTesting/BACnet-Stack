/****************************************************************************************
 *
 * Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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

/* environment variables used for the command line tools */

#include "configProj.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bacnet/bacdef.h"
#include "bacnet/apdu.h"
// #include "bbmd.h"
//#include "datalink.h"
//#include "handlers.h"
#include "bacnet/datalink/dlenv.h"
#include "bacnet/basic/tsm/tsm.h"
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "netport.h"
#endif
#include "eLib/util/eLibDebug.h"
#include "bvlc.h"

/** @file dlenv.c  Initialize the DataLink configuration. */

#if defined(BACDL_BIP)
/* timer used to renew Foreign Device Registration */
static uint16_t BBMD_Timer_Seconds;
/* BBMD variables */
static long bbmd_timetolive_seconds = 300;
static long bbmd_port = 0xBAC0;
static long bbmd_address = 0;
static long bbmd_mask = 0xFFFFFFFF;
static int bbmd_result = 0;
static BBMD_TABLE_ENTRY BBMD_Table_Entry;


/* Simple setters for BBMD registration variables. */

/** Sets the IPv4 address for BBMD registration.
 * If not set here or provided by Environment variables,
 * no BBMD registration will occur.
 * @param address - IPv4 address (long) of BBMD to register with,
 *                       in network byte order.
 */
void dlenv_bbmd_address_set(
    long address)
{
    bbmd_address = address;
}

/** Set the port for BBMD registration.
 * Default if not set is 0xBAC0.
 * @param port - The port number (provided in network byte order).
 */
void dlenv_bbmd_port_set(
    int port)
{
    bbmd_port = port;
}

/** Set the Lease Time (Time-to-Live) for BBMD registration.
 * Default if not set is 60000 (1000 minutes).
 * @param ttl_secs - The Lease Time, in seconds.
 */
void dlenv_bbmd_ttl_set(
    int ttl_secs)
{
    bbmd_timetolive_seconds = ttl_secs;
}

/** Get the result of the last attempt to register with the indicated BBMD.
 * If we sent a foreign registration request, then see if we've received
 * a NAK in our BVLC handler.
 *
 * @return Positive number (of bytes sent) if registration was successful,
 *         0 if no registration request was made, or
 *         -1 if registration attempt failed.
 */
//int dlenv_bbmd_result(
//    void)
//{
//    if ((bbmd_result > 0) &&
//        (bvlc_get_last_result() == BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK))
//        return -1;
//    /* Else, show our send: */
//    return bbmd_result;
//}
#endif


/** Register as a Foreign Device with the designated BBMD.
 * @ingroup DataLink
 * The BBMD's address, port, and lease time must be provided by
 * internal variables or Environment variables.
 * If no address for the BBMD is provided, no BBMD registration will occur.
 *
 * The Environment Variables depend on define of BACDL_BIP:
 *     - BACNET_BBMD_PORT - 0..65534, defaults to 47808
 *     - BACNET_BBMD_TIMETOLIVE - 0..65535 seconds, defaults to 60000
 *     - BACNET_BBMD_ADDRESS - dotted IPv4 address
 * @return Positive number (of bytes sent) on success,
 *         0 if no registration request is sent, or
 *         -1 if registration fails.
 */

#if 0
int dlenv_register_as_foreign_device(
    void)
{
    int retval = 0;

#if defined(BACDL_BIP)
    char *pEnv = NULL;
    unsigned a[4] = {0};
    char bbmd_env[32] = "";
    unsigned entry_number = 0;
    int c;

    pEnv = getenv("BACNET_BBMD_PORT");
    if (pEnv) {
        bbmd_port = strtol(pEnv, NULL, 0);
        if (bbmd_port > 0xFFFF) {
            bbmd_port = 0xBAC0;
        }
    }
    pEnv = getenv("BACNET_BBMD_TIMETOLIVE");
    if (pEnv) {
        bbmd_timetolive_seconds = strtol(pEnv, NULL, 0);
        if (bbmd_timetolive_seconds > 0xFFFF) {
            bbmd_timetolive_seconds = 0xFFFF;
        }
    }
    pEnv = getenv("BACNET_BBMD_ADDRESS");
    if (pEnv) {
        bbmd_address = bip_getaddrbyname(pEnv);
    }
    if (bbmd_address) {
        struct in_addr addr;
        addr.s_addr = bbmd_address;
        dbMessage(DBD_ALL, DB_DEBUG, "Registering with BBMD at %s:%ld for %ld seconds\n",
            inet_ntoa(addr), bbmd_port, bbmd_timetolive_seconds);
        panic();
    //        portParams->datalink.bipParams.fd_timetolive);
    //
    //    retval =
    //        bvlc_register_with_bbmd(portParams, addr.s_addr,
    //            portParams->datalink.bipParams.nwoPort,
    //            portParams->datalink.bipParams.fd_timetolive);
    //
        if (retval < 0)
        dbMessage(DBD_ALL, DB_ERROR, "FAILED to Register with BBMD at %s \n",
                inet_ntoa(addr));
        BBMD_Timer_Seconds = (uint16_t) bbmd_timetolive_seconds;
    } 
    else {
        for (entry_number = 1; entry_number <= MAX_BBMD_ENTRIES; entry_number++) {
            sprintf(bbmd_env,"BACNET_BDT_ADDR_%u", entry_number);
            pEnv = getenv(bbmd_env);
            if (pEnv) {
                bbmd_address = bip_getaddrbyname(pEnv);
            } else if (entry_number == 1) {
                /* BDT 1 is self (note: can be overridden) */
                panic();
                //bbmd_address = bip_get_addr();
            }
            if (bbmd_address) {
                bbmd_port = 0xBAC0;
                sprintf(bbmd_env,"BACNET_BDT_PORT_%u", entry_number);
                pEnv = getenv(bbmd_env);
                if (pEnv) {
                    bbmd_port = strtol(pEnv, NULL, 0);
                    if (bbmd_port > 0xFFFF) {
                        bbmd_port = 0xBAC0;
                    }
                } else if (entry_number == 1) {
                    /* BDT 1 is self (note: can be overridden) */
                    panic();
                    // bbmd_port = bip_get_port();
                }
                bbmd_mask = 0xFFFFFFFF;
                sprintf(bbmd_env,"BACNET_BDT_MASK_%u", entry_number);
                pEnv = getenv(bbmd_env);
                if (pEnv) {
                    c = sscanf(pEnv, "%3u.%3u.%3u.%3u",
                        &a[0],&a[1],&a[2],&a[3]);
                    if (c == 4) {
                        bbmd_mask =
                            ((a[0]&0xFF)<<24)|((a[1]&0xFF)<<16)|
                            ((a[2]&0xFF)<<8)|(a[3]&0xFF);
                    }
                }
                BBMD_Table_Entry.valid = true;
                BBMD_Table_Entry.dest_address.s_addr = bbmd_address;
                BBMD_Table_Entry.dest_port = bbmd_port;
                BBMD_Table_Entry.broadcast_mask.s_addr = bbmd_mask;
                panic();
                // bvlc_add_bdt_entry_local(&BBMD_Table_Entry);
            }
        }
    }
    bbmd_result = retval;
#endif
    return retval;
}
#endif


/** Datalink maintenance timer
 * @ingroup DataLink
 *
 * Call this function to renew our Foreign Device Registration
 * @param elapsed_seconds Number of seconds that have elapsed since last called.
 */

#if 0
#if defined(BBMD_ENABLED) && BBMD_ENABLED
void dlenv_maintenance_timer(
    uint16_t elapsed_seconds)
{
    if (BBMD_Timer_Seconds) {
        if (BBMD_Timer_Seconds <= elapsed_seconds) {
            BBMD_Timer_Seconds = 0;
        } else {
            BBMD_Timer_Seconds -= elapsed_seconds;
        }

        if (BBMD_Timer_Seconds == 0) {
            (void) dlenv_register_as_foreign_device();
            /* If that failed (negative), maybe just a network issue.
             * If nothing happened (0), may be un/misconfigured.
             * Set up to try again later in all cases. */
            BBMD_Timer_Seconds = (uint16_t) bbmd_timetolive_seconds;
        }
    }
}
#endif
#endif


void dlenv_init(
    void)
{
    char *pEnv = NULL;

#if defined(BACDL_ALL)
    pEnv = getenv("BACNET_DATALINK");
    if (pEnv) {
        datalink_set(pEnv);
    } else {
        datalink_set(NULL);
    }
#endif
#if defined(BACDL_BIP6)
    BACNET_IP6_ADDRESS addr;
    pEnv = getenv("BACNET_BIP6_BROADCAST");
    if (pEnv) {
        bvlc6_address_set(&addr,
                (uint16_t) strtol(pEnv, NULL, 0), 0, 0, 0, 0, 0, 0,
                BIP6_MULTICAST_GROUP_ID);
        bip6_set_broadcast_addr(&addr);
    } else {
        bvlc6_address_set(&addr,
                BIP6_MULTICAST_SITE_LOCAL, 0, 0, 0, 0, 0, 0,
                BIP6_MULTICAST_GROUP_ID);
        bip6_set_broadcast_addr(&addr);
    }
    pEnv = getenv("BACNET_BIP6_PORT");
    if (pEnv) {
        bip6_set_port((uint16_t) strtol(pEnv, NULL, 0));
    } else {
        bip6_set_port(0xBAC0);
    }
#endif
#if defined(BACDL_BIP)
#if defined(BIP_DEBUG)
    BIP_Debug = true;
#endif
    //pEnv = getenv("BACNET_IP_PORT");
    //if (pEnv) {
    //    bip_set_port(htons((uint16_t) strtol(pEnv, NULL, 0)));
    //} else {
    //    /* BIP_Port is statically initialized to 0xBAC0,
    //     * so if it is different, then it was programmatically altered,
    //     * and we shouldn't just stomp on it here.
    //     * Unless it is set below 1024, since:
    //     * "The range for well-known ports managed by the IANA is 0-1023."
    //     */
    //    if (ntohs(bip_get_port()) < 1024)
    //        bip_set_port(htons(0xBAC0));
    //}
    //
    //pEnv = getenv("BACNET_IP_NAT_ADDR");
    //if (pEnv) {
    //    struct in_addr nat_addr;
    //    nat_addr.s_addr = bip_getaddrbyname(pEnv);
    //    if (nat_addr.s_addr) {
    //        bvlc_set_global_address_for_nat(&nat_addr);
    //    }
    //}

    //printf("BACnet Port = %d\n", ntohs(bip_get_port()));

#if defined(BBMD_ENABLED) && BBMD_ENABLED
    printf("BBMD is enabled\n");
#else
    printf("BBMD *NOT* Enabled\n");
#endif

#elif defined(BACDL_MSTP)
    pEnv = getenv("BACNET_MAX_INFO_FRAMES");
    if (pEnv) {
        dlmstp_set_max_info_frames(strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_max_info_frames(1);
    }
    pEnv = getenv("BACNET_MAX_MASTER");
    if (pEnv) {
        dlmstp_set_max_master(strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_max_master(127);
    }
    pEnv = getenv("BACNET_MSTP_BAUD");
    if (pEnv) {
        dlmstp_set_baud_rate(strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_baud_rate(38400);
    }
    pEnv = getenv("BACNET_MSTP_MAC");
    if (pEnv) {
        dlmstp_set_mac_address(strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_mac_address(127);
    }
#endif

    pEnv = getenv("BACNET_APDU_TIMEOUT");
    if (pEnv) {
        apdu_timeout_set((uint16_t) strtol(pEnv, NULL, 0));
    } else {
#if defined(BACDL_MSTP)
        apdu_timeout_set(60000);
#endif
    }

    pEnv = getenv("BACNET_APDU_RETRIES");
    if (pEnv) {
        apdu_retries_set((uint8_t) strtol(pEnv, NULL, 0));
    }

    ///* === Initialize the Datalink Here === */
    //if (!datalink_init(getenv("BACNET_IFACE"))) {
    //    exit(1);
    //}

//#if (MAX_TSM_TRANSACTIONS)
//    pEnv = getenv("BACNET_INVOKE_ID");
//    if (pEnv) {
//        tsm_invokeID_set((uint8_t) strtol(pEnv, NULL, 0));
//    }
//#endif

    // dlenv_network_port_init();
    // dlenv_register_as_foreign_device();
}

