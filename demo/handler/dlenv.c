/**************************************************************************
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
*********************************************************************/

/* environment variables used for the command line tools */
//#include <stddef.h>
#include <stdint.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "config.h"
//#include "bacdef.h"
#include "apdu.h"
#include "datalink.h"
//#include "handlers.h"
//#include "dlenv.h"
#include "bvlc.h"

#if ( BACNET_CLIENT == 1 ) || MAX_TSM_TRANSACTIONS
#include "tsm.h"
#endif

#ifdef _MSC_VER
#include <WinSock2.h>
#endif


extern DLINK_SUPPORT *datalinkSupportHead;

/** @file dlenv.c  Initialize the DataLink configuration. */

#if defined(BACDL_BIP)
/* timer used to renew Foreign Device Registration */
// static uint16_t BBMD_Timer_Seconds;
/* BBMD variables */
// static long bbmd_timetolive_seconds = 60000;
// static long bbmd_port = 0xBAC0;
// static long bbmd_address = 0;
static int bbmd_result = 0;

/* Simple setters for BBMD registration variables. */

/** Sets the IPv4 address for BBMD registration.
 * If not set here or provided by Environment variables,
 * no BBMD registration will occur.
 * @param address - IPv4 address (long) of BBMD to register with,
 *                       in network byte order.
 */
//void dlenv_bbmd_address_set(
//    long address)
//{
//    bbmd_address = address;
//}

/** Set the port for BBMD registration.
 * Default if not set is 0xBAC0.
 * @param port - The port number (provided in network byte order).
 */
//void dlenv_bbmd_port_set(
//    int port)
//{
//    bbmd_port = port;
//}

/** Set the Lease Time (Time-to-Live) for BBMD registration.
 * Default if not set is 60000 (1000 minutes).
 * @param ttl_secs - The Lease Time, in seconds.
 */
//void dlenv_bbmd_ttl_set(
//    int ttl_secs)
//{
//    bbmd_timetolive_seconds = ttl_secs;
//}

/** Get the result of the last attempt to register with the indicated BBMD.
 * If we sent a foreign registration request, then see if we've received
 * a NAK in our BVLC handler.
 *
 * @return Positive number (of bytes sent) if registration was successful,
 *         0 if no registration request was made, or
 *         -1 if registration attempt failed.
 */
int dlenv_bbmd_result(
    void)
{
    if ((bbmd_result > 0) &&
        (bvlc_get_last_result() == BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK))
        return -1;
    /* Else, show our send: */
    return bbmd_result;
}
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

int dlenv_register_as_foreign_device(
    DLINK_SUPPORT  *portParams
    )
{
    int retval;

    struct in_addr addr;
    addr.s_addr = portParams->bipParams.fd_ipep.sin_addr.s_addr ; // bbmd_address;

    dbTraffic(DB_DEBUG, "Registering with BBMD at %s:%ld for %ld seconds\n",
        inet_ntoa(addr), 
        ntohs(portParams->bipParams.nwoPort), 
        portParams->bipParams.fd_timetolive );

    retval =
        bvlc_register_with_bbmd(portParams, addr.s_addr ,  
        portParams->bipParams.nwoPort,
        portParams->bipParams.fd_timetolive );

    if (retval < 0) {
        dbTraffic(DB_ERROR, "FAILED to Register with BBMD at %s \n",
            inet_ntoa(addr));
    }

    portParams->bipParams.fd_timeRemaining = portParams->bipParams.fd_timetolive;

    bbmd_result = retval;

    return retval;
}


/** Datalink maintenance timer
 * @ingroup DataLink
 *
 * Call this function to renew our Foreign Device Registration
 * @param elapsed_seconds Number of seconds that have elapsed since last called.
 */

void dlenv_maintenance_timer(
    DLINK_SUPPORT  *portParams,
    uint16_t elapsed_seconds)
{
    if (portParams->bipParams.fd_timeRemaining ) {
        if (portParams->bipParams.fd_timeRemaining <= elapsed_seconds) {
            portParams->bipParams.fd_timeRemaining = 0;
        } else {
            portParams->bipParams.fd_timeRemaining -= elapsed_seconds;
        }
        if (portParams->bipParams.fd_timeRemaining == 0) {
            (void) dlenv_register_as_foreign_device(portParams);
            /* If that failed (negative), maybe just a network issue.
             * If nothing happened (0), may be un/misconfigured.
             * Set up to try again later in all cases. */
            portParams->bipParams.fd_timeRemaining = portParams->bipParams.fd_timetolive;
        }
    }
}

/** Initialize the DataLink configuration from Environment variables,
 * or else to defaults.
 * @ingroup DataLink
 * The items configured depend on which BACDL_ the code is built for,
 * eg, BACDL_BIP.
 *
 * For most items, checks first for an environment variable, and, if
 * found, uses that to set the item's value.  Otherwise, will set
 * to a default value.
 *
 * The Environment Variables, by BACDL_ type, are:
 * - BACDL_ALL: (the general-purpose solution)
 *   - BACNET_DATALINK to set which BACDL_ type we are using.
 * - (Any):
 *   - BACNET_APDU_TIMEOUT - set this value in milliseconds to change
 *     the APDU timeout.  APDU Timeout is how much time a client
 *     waits for a response from a BACnet device.
 *   - BACNET_APDU_RETRIES - indicate the maximum number of times that
 *     an APDU shall be retransmitted.
 *   - BACNET_IFACE - set this value to dotted IP address (Windows) of
 *     the interface (see ipconfig command on Windows) for which you
 *     want to bind.  On Linux, set this to the /dev interface
 *     (i.e. eth0, arc0).  Default is eth0 on Linux, and the default
 *     interface on Windows.  Hence, if there is only a single network
 *     interface on Windows, the applications will choose it, and this
 *     setting will not be needed.
 * - BACDL_BIP: (BACnet/IP)
 *   - BACNET_IP_PORT - UDP/IP port number (0..65534) used for BACnet/IP
 *     communications.  Default is 47808 (0xBAC0).
 *   - BACNET_BBMD_PORT - UDP/IP port number (0..65534) used for Foreign
 *       Device Registration.  Defaults to 47808 (0xBAC0).
 *   - BACNET_BBMD_TIMETOLIVE - number of seconds used in Foreign Device
 *       Registration (0..65535). Defaults to 60000 seconds.
 *   - BACNET_BBMD_ADDRESS - dotted IPv4 address of the BBMD or Foreign
 *       Device Registrar.
 * - BACDL_MSTP: (BACnet MS/TP)
 *   - BACNET_MAX_INFO_FRAMES
 *   - BACNET_MAX_MASTER
 *   - BACNET_MSTP_BAUD
 *   - BACNET_MSTP_MAC
 * - BACDL_BIP6: (BACnet/IPv6)
 *   - BACNET_BIP6_PORT - UDP/IP port number (0..65534) used for BACnet/IPv6
 *     communications.  Default is 47808 (0xBAC0).
 *   - BACNET_BIP6_BROADCAST - FF05::BAC0 or FF02::BAC0 or ...
 */
void dlenv_init(
    )
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
#elif defined(BACDL_MSTP)
    pEnv = getenv("BACNET_MAX_INFO_FRAMES");
    if (pEnv) {
        dlmstp_set_max_info_frames(&placeholderPort, strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_max_info_frames(1);
    }
    pEnv = getenv("BACNET_MAX_MASTER");
    if (pEnv) {
        dlmstp_set_max_master(&placeholderPort, strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_max_master(127);
    }
    pEnv = getenv("BACNET_MSTP_BAUD");
    if (pEnv) {
        dlmstp_set_baud_rate(&placeholderPort, strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_baud_rate(38400);
    }
    pEnv = getenv("BACNET_MSTP_MAC");
    if (pEnv) {
        dlmstp_set_mac_address(&placeholderPort, strtol(pEnv, NULL, 0));
    } else {
        dlmstp_set_mac_address(&placeholderPort, 127);
    }
#endif
    pEnv = getenv("BACNET_APDU_TIMEOUT");
    if (pEnv) {
        apdu_timeout_set((uint16_t) strtol(pEnv, NULL, 0));
        fprintf(stderr, "BACNET_APDU_TIMEOUT=%s\r\n", pEnv);
    } else {
#if defined(BACDL_MSTP)
        apdu_timeout_set(60000);
#endif
    }
    pEnv = getenv("BACNET_APDU_RETRIES");
    if (pEnv) {
        apdu_retries_set((uint8_t) strtol(pEnv, NULL, 0));
    }

    /* === Initialize the Datalinks Here === */
    //for (PORT_SUPPORT *ps = headPortSupport; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next)
    //{
    //    if (!datalink_init(&placeholderPort, getenv("BACNET_IFACE"))) {
    //        exit(1);
    //    }
    //}

#if (MAX_TSM_TRANSACTIONS)
    pEnv = getenv("BACNET_INVOKE_ID");
    if (pEnv) {
        tsm_invokeID_set((uint8_t) strtol(pEnv, NULL, 0));
    }
#endif

//    dlenv_register_as_foreign_device();
}

