/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include "configProj.h"
#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacnet/bacdcode.h"
#include "bacnet/datalink/bip.h"
#include "osNet.h"
#include "eLib/util/eLibDebug.h"
#include <net/if.h>

// additional attempts to diagnose that struct ifreq definition issue
#include <stddef.h>
#include "bacnet/bacdef.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

//#include "debug.h"
#include "bacnet/bits/util/bitsIpUtil.h"
#include "bacnet/bits/util/multipleDatalink.h"

/** @file linux/bip-init.c  Initializes BACnet/IP interface (Linux). */

bool BIP_Debug = false;

#define h_addr h_addr_list[0] /* for backward compatibility */

/* gets an IP address by name, where name can be a
   string that is an IP address in dotted form, or
   a name that is a domain name
   returns 0 if not found, or
   an IP address in network byte order */
long bip_getaddrbyname(
    const char *host_name)
{
    struct hostent *host_ent;

    if ((host_ent = gethostbyname(host_name)) == NULL) {
        return 0;
        }

    return *(long *) host_ent->h_addr ; // h_addr needs -std=gnu11 to be set, else... h_addr_list[0]; (and many more)
}


static int get_local_ifr_ioctl(
    const char *ifname,
    struct ifreq *ifr,
    int request)
{
    int fd;
    int rv;     /* return value */

    strncpy(ifr->ifr_name, ifname, sizeof(ifr->ifr_name));
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

#if 0
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)ifr, sizeof(struct ifreq)) < 0)
    {
        printf("Failed to bind to [%s] - Did you run SUDO?? \n", ifr->ifr_name);
        close(fd);
        return -1;
    }
#endif

    if (fd < 0) {
        printf("Socket open failed 1\n");
        rv = fd;
    }
    else {
        rv = ioctl(fd, request, ifr);
        close(fd);
    }

    return rv;
}


int get_local_address_ioctl(
    const char *ifname,
    struct in_addr *addr,
    int request)
{
    struct ifreq ifr ; // = { {{0}} };
    struct sockaddr_in *tcpip_address;
    int rv;

    rv = get_local_ifr_ioctl(ifname, &ifr, request);
    if (rv >= 0) {
        tcpip_address = (struct sockaddr_in *)(void *) &ifr.ifr_addr;
        memcpy(addr, &tcpip_address->sin_addr, sizeof(struct in_addr));
    }

    return rv;
}


/** Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        Eg, for Linux, ifname is eth0, ath0, arc0, and others.
 */
int bip_set_interface(
    PORT_SUPPORT *portParams,
    const char *ifname)
{
    //    struct in_addr local_address;
    //    struct in_addr broadcast_address;
    //    struct in_addr netmask;
    IP_ADDR_PARAMS portIpParams = { 0 } ;
    int rv;

    if (!bits_get_port_params(portParams, &portIpParams))
    {
        dbMessage(DBD_ALL, DB_ERROR, "Unable to set interface parameters for %s", ifname);
        // we could test and return, but if the above call fails, we should continue to set our port's parameters to 0
    }

    bits_set_port_params(portParams, &portIpParams);

    return 0;
}


/** Initialize the BACnet/IP services at the given interface.
 * @ingroup DLBIP
 * -# Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 * -# Opens a UDP socket
 * -# Configures the socket for sending and receiving
 * -# Configures the socket so it can send broadcasts
 * -# Binds the socket to the local IP address at the specified port for
 *    BACnet/IP (by default, 0xBAC0 = 47808).
 *
 * @note For Linux, ifname is eth0, ath0, arc0, and others.
 *       For Windows, ifname is the dotted ip address of the interface.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "eth0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(
    PORT_SUPPORT *portParams,
    const char *ifname)
{
    int status = 0; /* return from socket lib calls */
    struct sockaddr_in sin;
    int sockopt;
    int sock_fd;

#ifdef BAC_DEBUG
    if (ifname == NULL) {
        panic();
        // until we handle the "ALL" case (and broadcasts on every interface, we will insist on a ifName!
        // bip_set_interface(portParams, "eth0") ;
        return false ;
    }
#endif

    /* attempt to set establish interface parameters (ipaddr, bcast addr, netmask). Don't sweat if not found... perhaps
     * DCHP adapter, and cable unplugged... We will try again later (after e.g. we receive a packet)
     * See: cr10941947194794 */
    bip_set_interface(portParams, ifname);

    /* assumes that the driver has already been initialized */
    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    portParams->datalink.bipParams.socket = sock_fd ;
    if (sock_fd < 0) {
        fprintf(stderr, "bip: failed to allocate a socket.\n");
        return false;
    }

    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    sockopt = 1;
    status =
        setsockopt(sock_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &sockopt,
        sizeof(sockopt));
    if (status < 0) {
        close(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }

    /* allow us to send a broadcast */
    status =
        setsockopt(sock_fd,
        SOL_SOCKET,
        SO_BROADCAST,
        &sockopt,
        sizeof(sockopt));
    if (status < 0) {
        close(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }


    // this man felt my pain trying to bind to a specific address
    // https://stackoverflow.com/questions/25070649/linux-bind-udp-listening-socket-to-specific-interface-or-find-out-the-interfac

#if 0
    // for Multiport routing, we may need to be sure to bind to a specific atapter, which needs to operate with root privileges
    // Another problem: If we bind to IP address, we don't receive broadcasts, so that is a no-go...

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", portParams->ifName );

    dbMessage(DBD_ALL, DB_NOTE, "Explicit binding to Adapter [%s]", ifr.ifr_name);
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
    {
        dbMessage(DBD_ALL, DB_ERROR, "failed to set socket option BINDTODEVICE [%s]", ifr.ifr_name);
        return false ;
    }
#endif

    /* bind the socket to the local port number and IP address */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = portParams->datalink.bipParams.nwoPort;
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
    status =
        bind(sock_fd, (const struct sockaddr *) &sin, sizeof(struct sockaddr_in));
    if (status < 0) {
        close(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }

    return true;
}


/** Cleanup and close out the BACnet/IP services by closing the socket.  */
void bip_cleanup(
    PORT_SUPPORT *portParams)
{
}

/** Get the netmask of the BACnet/IP's interface via an ioctl() call.
 * @param netmask [out] The netmask, in host order.
 * @return 0 on success, else the error from the ioctl() call.
 */
int bip_get_local_netmask(
    struct in_addr *netmask)
{
    int rv;
    char *ifname = getenv("BACNET_IFACE");      /* will probably be null */
    if (ifname == NULL)
        ifname = (char *) "eth0";
    rv = get_local_address_ioctl(ifname, netmask, SIOCGIFNETMASK);
    return rv;
}
