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

//#include <stdio.h>
//#include <stdint.h>
#include "bacnet/basic/services.h"
//#include "bacnet/bacint.h"
//#include "bacnet/datalink/bip.h"
#include "bacport.h"
#include "bacnet/datalink/bip.h"
#include "bacnet/datalink/datalink.h"

/* alternate methods of choosing broadcast address */
#ifndef USE_INADDR
#define USE_INADDR 0
#endif
#ifndef USE_CLASSADDR
#define USE_CLASSADDR 0
#endif

bool BIP_Debug = false;

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


#if 0
// not used by MSVC testing so far....
/* To fill a need, we invent the gethostaddr() function. */
static long gethostaddr(
    void)
{
    struct hostent *host_ent;
    char host_name[255];

    if (gethostname(host_name, sizeof(host_name)) != 0)
        return -1;

    if ((host_ent = gethostbyname(host_name)) == NULL)
        return -1;

    if (((uint8_t *)host_ent->h_addr)[0] != 192) {
        // Cisco VPN adapter interfering
        dbMessage(DBD_ALL, DB_ERROR, "NOTE!! Host not local (todo 2 - remove this warning before delivery)");
    }

    dbMessage(DBD_ALL, DB_NOTE, "host: %s at %u.%u.%u.%u", host_name,
               (unsigned) ((uint8_t *) host_ent->h_addr)[0],
               (unsigned) ((uint8_t *) host_ent->h_addr)[1],
               (unsigned) ((uint8_t *) host_ent->h_addr)[2],
               (unsigned) ((uint8_t *) host_ent->h_addr)[3]);

    /* note: network byte order */
    return *(long *) host_ent->h_addr;
}
#endif


#if (!defined(USE_INADDR) || (USE_INADDR == 0)) && \
 (!defined(USE_CLASSADDR) || (USE_CLASSADDR == 0))
/* returns the subnet mask in network byte order */
static uint32_t getIpMaskForIpAddress(
    uint32_t ipAddress)
{
    /* Allocate information for up to 16 NICs */
    IP_ADAPTER_INFO AdapterInfo[16];
    /* Save memory size of buffer */
    DWORD dwBufLen = sizeof(AdapterInfo);
    uint32_t ipMask = INADDR_BROADCAST;
    bool found = false;

    PIP_ADAPTER_INFO pAdapterInfo;

    /* GetAdapterInfo:
       [out] buffer to receive data
       [in] size of receive data buffer */
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo,
                                     &dwBufLen);
    if (dwStatus == ERROR_SUCCESS) {
        /* Verify return value is valid, no buffer overflow
           Contains pointer to current adapter info */
        pAdapterInfo = AdapterInfo;

        do {
            IP_ADDR_STRING *pIpAddressInfo = &pAdapterInfo->IpAddressList;
            do {
                unsigned long adapterAddress =
                    inet_addr(pIpAddressInfo->IpAddress.String);
                unsigned long adapterMask =
                    inet_addr(pIpAddressInfo->IpMask.String);
                if (adapterAddress == ipAddress) {
                    ipMask = adapterMask;
                    found = true;
                }
                pIpAddressInfo = pIpAddressInfo->Next;
            } while (pIpAddressInfo && !found);
            /* Progress through linked list */
            pAdapterInfo = pAdapterInfo->Next;
            /* Terminate on last adapter */
        } while (pAdapterInfo && !found);
    }

    return ipMask;
}
#endif

void set_broadcast_address(
    PORT_SUPPORT *portParams,
    uint32_t net_address)
{
#if defined(USE_INADDR) && USE_INADDR
    /*   Note: sometimes INADDR_BROADCAST does not let me get
       any unicast messages.  Not sure why... */
    (void) net_address ;
    bip_set_broadcast_addr(portParams, INADDR_BROADCAST);
#elif defined(USE_CLASSADDR) && USE_CLASSADDR
    long broadcast_address = 0;

    if (IN_CLASSA(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSA_HOST) | IN_CLASSA_HOST;
    else if (IN_CLASSB(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSB_HOST) | IN_CLASSB_HOST;
    else if (IN_CLASSC(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSC_HOST) | IN_CLASSC_HOST;
    else if (IN_CLASSD(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSD_HOST) | IN_CLASSD_HOST;
    else
        broadcast_address = INADDR_BROADCAST;
    bip_set_broadcast_addr(htonl(broadcast_address));
#else
    /* these are network byte order variables */
    uint32_t broadcast_address = 0;
    uint32_t net_mask = 0;

    net_mask = getIpMaskForIpAddress(net_address);
    if (BIP_Debug) {
        struct in_addr address;
        address.s_addr = net_mask;
        printf("IP Mask: %s\n", inet_ntoa(address));
    }
    broadcast_address = (net_address & net_mask) | (~net_mask);
    bip_set_broadcast_addr(portParams, broadcast_address);
#endif
}


/* on Windows, ifname is the dotted ip address of the interface */
//void bip_set_interface(
//    PORT_SUPPORT *portParams,
//    char *ifname)
//{
//    /* setup local address */
//    if (portParams->bipParams.nwoLocal_addr  == 0) {
//        portParams->bipParams.nwoLocal_addr = inet_addr(ifname) ;
//    }
//
//    /* setup local broadcast address */
//    if (portParams->bipParams.nwoBroadcast_addr == 0) {
//        set_broadcast_address( portParams, portParams->bipParams.nwoLocal_addr);
//    }
//}

const char *winsock_error_code_text(
    const int code)
{
    switch (code) {
    case WSAEACCES:
        return "Permission denied.";
    case WSAEINTR:
        return "Interrupted system call.";
    case WSAEBADF:
        return "Bad file number.";
    case WSAEFAULT:
        return "Bad address.";
    case WSAEINVAL:
        return "Invalid argument.";
    case WSAEMFILE:
        return "Too many open files.";
    case WSAEWOULDBLOCK:
        return "Operation would block.";
    case WSAEINPROGRESS:
        return "Operation now in progress. "
               "This error is returned if any Windows Sockets API "
               "function is called while a blocking function "
               "is in progress.";
    case WSAENOTSOCK:
        return "Socket operation on nonsocket.";
    case WSAEDESTADDRREQ:
        return "Destination address required.";
    case WSAEMSGSIZE:
        return "Message too long.";
    case WSAEPROTOTYPE:
        return "Protocol wrong type for socket.";
    case WSAENOPROTOOPT:
        return "Protocol not available.";
    case WSAEPROTONOSUPPORT:
        return "Protocol not supported.";
    case WSAESOCKTNOSUPPORT:
        return "Socket type not supported.";
    case WSAEOPNOTSUPP:
        return "Operation not supported on socket.";
    case WSAEPFNOSUPPORT:
        return "Protocol family not supported.";
    case WSAEAFNOSUPPORT:
        return "Address family not supported by protocol family.";
    case WSAEADDRINUSE:
        return "Address already in use.";
    case WSAEADDRNOTAVAIL:
        return "Cannot assign requested address.";
    case WSAENETDOWN:
        return "Network is down. "
               "This error may be reported at any time "
               "if the Windows Sockets implementation "
               "detects an underlying failure.";
    case WSAENETUNREACH:
        return "Network is unreachable.";
    case WSAENETRESET:
        return "Network dropped connection on reset.";
    case WSAECONNABORTED:
        return "Software caused connection abort.";
    case WSAECONNRESET:
        return "Connection reset by peer.";
    case WSAENOBUFS:
        return "No buffer space available.";
    case WSAEISCONN:
        return "Socket is already connected.";
    case WSAENOTCONN:
        return "Socket is not connected.";
    case WSAESHUTDOWN:
        return "Cannot send after socket shutdown.";
    case WSAETOOMANYREFS:
        return "Too many references: cannot splice.";
    case WSAETIMEDOUT:
        return "Connection timed out.";
    case WSAECONNREFUSED:
        return "Connection refused.";
    case WSAELOOP:
        return "Too many levels of symbolic links.";
    case WSAENAMETOOLONG:
        return "File name too long.";
    case WSAEHOSTDOWN:
        return "Host is down.";
    case WSAEHOSTUNREACH:
        return "No route to host.";
    case WSASYSNOTREADY:
        return "Returned by WSAStartup(), "
               "indicating that the network subsystem is unusable.";
    case WSAVERNOTSUPPORTED:
        return "Returned by WSAStartup(), "
               "indicating that the Windows Sockets DLL cannot support "
               "this application.";
    case WSANOTINITIALISED:
        return "Winsock not initialized. "
               "This message is returned by any function "
               "except WSAStartup(), "
               "indicating that a successful WSAStartup() has not yet "
               "been performed.";
    case WSAEDISCON:
        return "Disconnect.";
    case WSAHOST_NOT_FOUND:
        return "Host not found. " "This message indicates that the key "
               "(name, address, and so on) was not found.";
    case WSATRY_AGAIN:
        return "Nonauthoritative host not found. "
               "This error may suggest that the name service itself "
               "is not functioning.";
    case WSANO_RECOVERY:
        return "Nonrecoverable error. "
               "This error may suggest that the name service itself "
               "is not functioning.";
    case WSANO_DATA:
        return "Valid name, no data record of requested type. "
               "This error indicates that the key "
               "(name, address, and so on) was not found.";
    default:
        return "unknown";
    }
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
    int rv = 0; /* return from socket lib calls */
    struct sockaddr_in sin;
    int value = 1;
    SOCKET sock_fd = -1;
    int Result;
    int Code;
    WSADATA wd;
    // struct in_addr address;
    struct in_addr broadcast_address;

    Result = WSAStartup((1 << 8) | 1, &wd);
    /*Result = WSAStartup(MAKEWORD(2,2), &wd); */
    if (Result != 0) {
        Code = WSAGetLastError();
        dbMessage(DBD_ALL, DB_ERROR, "TCP/IP stack initialization failed\n" " error code: %i %s\n",
               Code, winsock_error_code_text(Code));
        exit(1);
    }
    // datalink cleanup registered in main.c atexit(bip_cleanup);

    /* has address been set? */
    portParams->datalink.bipParams.nwoBroadcast_addr = INADDR_BROADCAST ;

    if (BIP_Debug) {
        broadcast_address.s_addr = portParams->datalink.bipParams.nwoBroadcast_addr ;
        dbMessage(DBD_ALL, DB_ERROR, "IP Broadcast Address: %s\n",
                inet_ntoa(broadcast_address));
        dbMessage(DBD_ALL, DB_ERROR, "UDP Port: 0x%04X [%hu]\n", ntohs( portParams->datalink.bipParams.nwoPort ),
            ntohs(portParams->datalink.bipParams.nwoPort));
    }

    /* assumes that the driver has already been initialized */
    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    portParams->datalink.bipParams.socket = sock_fd ;
    if (sock_fd < 0) {
        fprintf(stderr, "bip: failed to allocate a socket.\n");
        return false;
    }

    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value,
                    sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set REUSEADDR socket option.\n");
        closesocket(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }
    /* Enables transmission and receipt of broadcast messages on the socket. */
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *) &value,
                    sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set BROADCAST socket option.\n");
        closesocket(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }
#if 0
    /* probably only for Apple... */
    /* rebind a port that is already in use.
       Note: all users of the port must specify this flag */
    // See this: http://stackoverflow.com/questions/14388706/socket-options-so-reuseaddr-and-so-reuseport-how-do-they-differ-do-they-mean-t/14388707#14388707
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, (char *) &value,
                    sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set REUSEPORT socket option.\n");
        closesocket(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }
#endif

    /* bind the socket to the local port number and IP address */
    sin.sin_family = AF_INET;
#if defined(USE_INADDR) && USE_INADDR
    /* by setting sin.sin_addr.s_addr to INADDR_ANY,
       I am telling the IP stack to automatically fill
       in the IP address of the machine the process
       is running on.

       Some server computers have multiple IP addresses.
       A socket bound to one of these will not accept
       connections to another address. Frequently you prefer
       to allow any one of the computer's IP addresses
       to be used for connections.  Use INADDR_ANY (0L) to
       allow clients to connect using any one of the host's
       IP addresses. */
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    /* or we could use the specific adapter address
       note: already in network byte order */
    sin.sin_addr.s_addr = portParams->datalink.bipParams.nwoLocal_addr;
#endif
    sin.sin_port = bip_get_local_port( portParams);
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
    rv = bind(sock_fd, (const struct sockaddr *) &sin,
              sizeof(struct sockaddr));
    if (rv < 0) {
        dbMessage(DBD_ALL, DB_ERROR, "bip: failed to bind to %s port %hu\n",
                inet_ntoa(sin.sin_addr), ntohs( portParams->datalink.bipParams.nwoPort) );
        closesocket(sock_fd);
        portParams->datalink.bipParams.socket = -1;
        return false;
    }

    return true;
}


/** Cleanup and close out the BACnet/IP services by closing the socket.  */
void bip_cleanup(
    PORT_SUPPORT *portParams)
{
    if ( portParams->datalink.bipParams.socket >= 0 ) {
        closesocket(portParams->datalink.bipParams.socket);
    }
    portParams->datalink.bipParams.socket = -1;
    WSACleanup();
}

// todo2: why does CheckLocalAddressKnown() leave socket bound?
// need to make this single-datalink for feature creep
void CheckLocalAddressKnown(PORT_SUPPORT *portParams, struct sockaddr_in *sin)
{
#if todo2
    if (portParams->datalink.bipParams.nwoLocal_addr == 0)
    {
        struct sockaddr_in ourAddr;
        int ourLen = sizeof(ourAddr);
        int err;

#if 0
        // peek at us
        err = getsockname(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            return;
        }

        // simply connect to the startion sending to us. The OS will bind our socket to that interface.
        // If we send to another IP address on the same subnet later, we will be OK
        err = connect(portParams->datalink.bipParams.socket, (struct sockaddr *)sin, sizeof(struct sockaddr));
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            return;
        }

        // peek at us
        err = getsockname(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            return;
        }

        // and unconnect, ignoring possible errors. See Stevens, pg 254
        sin->sin_family = AF_UNSPEC;
        connect(portParams->datalink.bipParams.socket, (struct sockaddr *)sin, sizeof(struct sockaddr));

        portParams->datalink.bipParams.nwoLocal_addr = ourAddr.sin_addr.s_addr;
        set_broadcast_address(portParams, portParams->datalink.bipParams.nwoLocal_addr);
#endif

#if defined ( _MSC_VER )
        // todo1 - I need to worry a bit about this...

        // not known, do a quick connect, check, disconnect
        SOCKET newsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (newsock < 0)
        {
            panic();
            return;
        }

        // quick connect to them
        err = connect(newsock, (struct sockaddr *)sin, sizeof(struct sockaddr));
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            closesocket(newsock);
            return;
        }

        // peek at us
        err = getsockname(newsock, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            closesocket(newsock);
            return;
        }

        closesocket(newsock);
        portParams->datalink.bipParams.nwoLocal_addr = ourAddr.sin_addr.s_addr;

        set_broadcast_address(portParams, portParams->datalink.bipParams.nwoLocal_addr);

        // AND... since we are doing this mainly because we are on a multi-interfaced machine, we need to "lock down" our IP endpoint so we
        // are forever deaf to traffic on the other adapters, else we will end up representing ourselves as multiple devices in some situations.
        // e.g. someone does a 255.255.255.255 broadcast and gets responses from our one node on different adapters... which would be a bad thing, right?
        closesocket(portParams->datalink.bipParams.socket);

        // reopen socket, this time binding to the IP address for the interface
        portParams->datalink.bipParams.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (portParams->datalink.bipParams.socket < 0)
        {
            fprintf(stderr, "bip: failed to allocate a socket.\n");
            return;
        }
        /* Allow us to use the same socket for sending and receiving */
        /* This makes sure that the src port is correct when sending */
        int value = 1;
        err = setsockopt(portParams->datalink.bipParams.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&value,
            sizeof(value));
        if (err < 0)
        {
            fprintf(stderr, "bip: failed to set REUSEADDR socket option.\n");
            closesocket(portParams->datalink.bipParams.socket);
            portParams->datalink.bipParams.socket = -1;
            return;
        }
        /* Enables transmission and receipt of broadcast messages on the socket. */
        err = setsockopt(portParams->datalink.bipParams.socket, SOL_SOCKET, SO_BROADCAST, (char *)&value,
            sizeof(value));
        if (err < 0)
        {
            fprintf(stderr, "bip: failed to set BROADCAST socket option.\n");
            closesocket(portParams->datalink.bipParams.socket);
            portParams->datalink.bipParams.socket = -1;
            return;
        }

        ourAddr.sin_port = portParams->datalink.bipParams.nwoPort;

        err = bind(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, sizeof(struct sockaddr));
        if (err)
        {
            int err2 = WSAGetLastError();
            const char *errmsg = winsock_error_code_text(err2);
            printf("Err = %s\n", errmsg);
            panic();
            closesocket(newsock);
            return;
        }
#endif

    }
#endif
}


void bits_set_port_params(
    PORT_SUPPORT *portParams,
    IP_ADDR_PARAMS *portIpParams)
{
    bip_set_addr(portParams, portIpParams->local_address.s_addr);
    bip_set_netmask_addr(portParams, portIpParams->netmask.s_addr);
    bip_set_broadcast_addr(portParams, portIpParams->broadcast_address.s_addr);
    bip_set_subnet_addr(portParams, portIpParams->local_address.s_addr & portIpParams->netmask.s_addr);
}


bool bits_get_port_params(
    PORT_SUPPORT *datalink,
    IP_ADDR_PARAMS *params)
{
    // for now, we just fake this in windows. We don't yet have an application that needs this 
    params->broadcast_address.S_un.S_addr = datalink->datalink.bipParams.nwoBroadcast_addr;
    params->local_address.S_un.S_addr = datalink->datalink.bipParams.nwoLocal_addr;
    params->netmask.S_un.S_addr = datalink->datalink.bipParams.nwoNetmask_addr;
    return true;
}



