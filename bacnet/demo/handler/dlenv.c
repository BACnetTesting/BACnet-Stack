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
#include <stdlib.h>
#include "apdu.h"
#include "bbmd.h"
#include "dlenv.h"
#include "configParams.h"
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "netport.h"
#endif
#include "bitsDebug.h"

/** @file dlenv.c  Initialize the DataLink configuration. */

#if defined(BACDL_BIP)
/* timer used to renew Foreign Device Registration */
// static uint16_t BBMD_Timer_Seconds;
/* BBMD variables */
// static long bbmd_timetolive_seconds = 60000;
// static long bbmd_port = 0xBAC0;
// static long bbmd_address = 0;
static int bbmd_result = 0;

// extern ConfigType config;

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

#if 0
int dlenv_register_as_foreign_device(
    PORT_SUPPORT  *portParams
)
{
    int retval;

    struct in_addr addr;
    addr.s_addr = portParams->datalink.bipParams.fd_ipep.sin_addr.s_addr; // bbmd_address;

        fprintf(stderr, "Registering with BBMD at %s:%ld for %ld seconds\n",
        inet_ntoa(addr),
        ntohs(portParams->datalink.bipParams.nwoPort),
        portParams->datalink.bipParams.fd_timetolive);

    retval =
        bvlc_register_with_bbmd(portParams, addr.s_addr,
            portParams->datalink.bipParams.nwoPort,
            portParams->datalink.bipParams.fd_timetolive);

    if (retval < 0) {
            fprintf(stderr, "FAILED to Register with BBMD at %s \n",
            inet_ntoa(addr));
            } else if (entry_number == 1) {
                /* BDT 1 is self (note: can be overridden) */
                bbmd_address = bip_get_addr();
    }

    portParams->datalink.bipParams.fd_timeRemaining = portParams->datalink.bipParams.fd_timetolive;

                } else if (entry_number == 1) {
                    /* BDT 1 is self (note: can be overridden) */
                    bbmd_port = bip_get_port();
    bbmd_result = retval;

    return retval;
}
#endif


#if (BACNET_PROTOCOL_REVISION >= 17)
#if defined(BACDL_BIP)
/**
 * Datalink network port object settings
 */
static void dlenv_network_port_init(void)
{
    uint32_t instance = 0;
    uint32_t address = 0;
    uint32_t broadcast = 0;
    uint32_t test_broadcast = 0;
    uint32_t mask = 0;
    uint16_t port = 0;
    uint8_t mac[4+2] = {0};
    uint8_t prefix = 0;

    instance = Network_Port_Index_To_Instance(0);
    NETWORK_PORT_DESCR *currentObject = Network_Port_Instance_To_Object(instance);
    if (currentObject == NULL) {
        panic();
        return;
    }

    Network_Port_Name_Set(currentObject, "BACnet/IP Port");
    Network_Port_Type_Set(currentObject, PORT_TYPE_BIP);
    panic();
    // port = bip_get_port();
    Network_Port_BIP_Port_Set(currentObject, port);
    panic();
    // address = bip_get_addr();
    memcpy(&mac[0], &address, 4);
    memcpy(&mac[4], &port, 2);
    Network_Port_MAC_Address_Set(currentObject, &mac[0], 6);
    panic();
    //  broadcast = bip_get_broadcast_addr();
    for (prefix = 0; prefix < 32; prefix++) {
        mask = htonl((0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF);
        test_broadcast = (address & mask) | (~mask);
        if (test_broadcast == broadcast) {
            break;
        }
    }
    Network_Port_IP_Subnet_Prefix_Set(currentObject, prefix);
}
#elif defined(BACDL_MSTP)
/**
 * Datalink network port object settings
 */
static void dlenv_network_port_init(void)
{
    uint32_t instance = 0;
    uint8_t mac[1] = {0};

    instance = Network_Port_Index_To_Instance(0);
    Network_Port_Name_Set(instance, "MS/TP Port");
    Network_Port_MSTP_Max_Master_Set(instance, dlmstp_max_master());
    Network_Port_MSTP_Max_Info_Frames_Set(instance, dlmstp_max_info_frames());
    Network_Port_Link_Speed_Set(instance, dlmstp_baud_rate());
    mac[0] = dlmstp_mac_address();
    Network_Port_MAC_Address_Set(instance, &mac[0], 1);
}
#elif defined(BACDL_BIP6)
/**
 * Datalink network port object settings
 */
static void dlenv_network_port_init(void)
{
    uint32_t instance = 0;
    const char *bip_port_name = ;

    instance = Network_Port_Index_To_Instance(0);
    Network_Port_Name_Set(instance, "BACnet/IPv6 Port");

}
#endif
#else
/**
 * Datalink network port object settings
 */
static void dlenv_network_port_init(void)
{
    /* do nothing */
}
#endif

/** Datalink maintenance timer
 * @ingroup DataLink
 *
 * Call this function to renew our Foreign Device Registration
 * @param elapsed_seconds Number of seconds that have elapsed since last called.
 */

void dlenv_maintenance_timer(
    PORT_SUPPORT  *portParams,
    uint16_t elapsed_seconds)
{
#if 0
    if (portParams->datalink.bipParams.fd_timeRemaining) {
        if (portParams->datalink.bipParams.fd_timeRemaining <= elapsed_seconds) {
            portParams->datalink.bipParams.fd_timeRemaining = 0;
        }
        else {
            portParams->datalink.bipParams.fd_timeRemaining -= elapsed_seconds;
        }

        if (portParams->datalink.bipParams.fd_timeRemaining == 0) {
            (void)dlenv_register_as_foreign_device(portParams);
            /* If that failed (negative), maybe just a network issue.
             * If nothing happened (0), may be un/misconfigured.
             * Set up to try again later in all cases. */
            portParams->datalink.bipParams.fd_timeRemaining = portParams->datalink.bipParams.fd_timetolive;
    
        }
    }
    printf("BBMD Enabled\n");
    dlenv_network_port_init();
#endif 
}

