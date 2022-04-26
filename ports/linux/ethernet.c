/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

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

#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */

#include "bacport.h"
#include "bacnet/bacdef.h"
#include "bacnet/datalink/ethernet.h"
#include "bacnet/bacint.h"
#include "bacnet/bits/util/multipleDatalink.h"


/** @file linux/ethernet.c  Provides Linux-specific functions for BACnet/Ethernet. */

/* commonly used comparison address for ethernet */
static uint8_t Ethernet_Broadcast[MAX_ETHERNET_MAC_LEN] ={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* commonly used empty address for ethernet quick compare */
// uint8_t Ethernet_Empty_MAC[MAX_ETHERNET_MAC_LEN] = {0, 0, 0, 0, 0, 0};

/* my local device data - MAC address */
// uint8_t Ethernet_MAC_Address[MAX_ETHERNET_MAC_LEN] = {0};

static int eth802_sockfd = -1; /* 802.2 file handle */
static struct sockaddr eth_addr = {0}; /* used for binding 802.2 */

bool ethernet_valid(
    void)
{
    return (eth802_sockfd >= 0);
}


void ethernet_cleanup(
    void)
{
    if (ethernet_valid())
        close(eth802_sockfd);
    eth802_sockfd = -1;
}

#if 0
/*----------------------------------------------------------------------
 Portable function to set a socket into nonblocking mode.
 Calling this on a socket causes all future read() and write() calls on
 that socket to do only as much as they can immediately, and return
 without waiting.
 If no data can be read or written, they return -1 and set errno
 to EAGAIN (or EWOULDBLOCK).
 Thanks to Bjorn Reese for this code.
----------------------------------------------------------------------*/
int setNonblocking(
    int fd)
{
    int flags;

    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
#endif

/* opens an 802.2 socket to receive and send packets */
static int ethernet_bind(struct sockaddr *eth_addr, char *interface_name)
{
    int sock_fd = -1; /* return value */
#if 0
    int sockopt = 0;
#endif
    int uid = 0;

    fprintf(stderr, "ethernet: opening \"%s\"\n", interface_name);
    /* check to see if we are being run as root */
    uid = getuid();
    if (uid != 0) {
        fprintf(stderr,
            "ethernet: Unable to open an 802.2 socket.  "
            "Try running with root priveleges.\n");
        return sock_fd;
    }
    /* note: on some systems you may have to add or enable in */
    /* modules.conf (or in modutils/alias on Debian with update-modules) */
    /* alias net-pf-17 af_packet */
    /* Then follow it by: # modprobe af_packet */
    /* Note: PF_INET/SOCK_PACKET has been replaced with
       PF_PACKET/(SOCK_PACKET, SOCK_DGRAM, SOCK_RAW). */

    /* Attempt to open the socket for 802.2 ethernet frames */
    if ((sock_fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_802_2))) < 0) {
        /* Error occured */
        fprintf(
            stderr, "ethernet: Error opening socket: %s\n", strerror(errno));
        fprintf(stderr,
            "You might need to add the following to modules.conf\n"
            "(or in /etc/modutils/alias on Debian with update-modules):\n"
            "alias net-pf-17 af_packet\n"
            "Also, add af_packet to /etc/modules.\n"
            "Then follow it by:\n"
            "# modprobe af_packet\n");
        exit(-1);
    }
#if 0
    /* It is very advisable to do a IP_HDRINCL call, to make sure
       that the kernel knows the header is included in the data,
       and doesn't insert its own header into the packet before our data */
    if (setsockopt(sock_fd, IPPROTO_IP, IP_HDRINCL, &sockopt,
            sizeof(sockopt)) < 0) {
        printf("Warning: Cannot set HDRINCL!\n");
    }
#endif
    /* Bind the socket to an address */
    eth_addr->sa_family = PF_INET;
    /* Clear the memory before copying */
    memset(eth_addr->sa_data, '\0', sizeof(eth_addr->sa_data));
    /* Strcpy the interface name into the address */
    strncpy(eth_addr->sa_data, interface_name, sizeof(eth_addr->sa_data) - 1);
    fprintf(stderr, "ethernet: binding \"%s\"\n", eth_addr->sa_data);
    /* Attempt to bind the socket to the interface */
    if (bind(sock_fd, eth_addr, sizeof(struct sockaddr)) != 0) {
        /* Bind problem, close socket and return */
        fprintf(stderr, "ethernet: Unable to bind 802.2 socket : %s\n",
            strerror(errno));
        fprintf(stderr,
            "You might need to add the following to modules.conf\n"
            "(or in /etc/modutils/alias on Debian with update-modules):\n"
            "alias net-pf-17 af_packet\n"
            "Also, add af_packet to /etc/modules.\n"
            "Then follow it by:\n"
            "# modprobe af_packet\n");
        /* Close the socket */
        close(sock_fd);
        exit(-1);
    }

    atexit(ethernet_cleanup);

    return sock_fd;
}

/* function to find the local ethernet MAC address */
static int get_local_hwaddr(const char *ifname, unsigned char *mac)
{
    struct ifreq ifr;
    int fd;
    int rv; /* return value - error value from df or ioctl call */

    /* determine the local MAC address */
    strcpy(ifr.ifr_name, ifname);
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0)
        rv = fd;
    else {
        rv = ioctl(fd, SIOCGIFHWADDR, &ifr);
        if (rv >= 0) /* worked okay */
            memcpy(mac, ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);
    }

    return rv;
}


bool ethernet_init(
    PORT_SUPPORT *portParams,
    const char *interface_name)
{
    if (interface_name) {
        get_local_hwaddr(interface_name, portParams->localMAC->bytes );
        eth802_sockfd = ethernet_bind(&eth_addr, interface_name);
    } else {
        get_local_hwaddr("eth0", portParams->localMAC->bytes);
        eth802_sockfd = ethernet_bind(&eth_addr, "eth0");
    }
    portParams->localMAC->len = MAX_ETHERNET_MAC_LEN;

    return ethernet_valid();
}


/* function to send a packet out the 802.2 socket */
/* returns number of bytes sent on success, negative on failure */
void ethernet_send_npdu(
    const PORT_SUPPORT* datalink,
    const DLCB *dlcb)
{
    uint8_t mtu[MAX_LPDU_ETHERNET] ;
    int mtu_len = 0;
    int i ;

    /* don't waste time if the socket is not valid */
    if (eth802_sockfd < 0) {
        fprintf(stderr, "ethernet: 802.2 socket is invalid!\n");
        return -1;
    }

    /* load destination ethernet MAC address */
    if ( dlcb->bacnetPath.localMac.len == MAX_ETHERNET_MAC_LEN) {
        for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++) {
            mtu[mtu_len] = dlcb->bacnetPath.localMac.bytes[i];
            mtu_len++;
        }
    }
    else if (dlcb->bacnetPath.localMac.len == 0) {
        // Assume broadcast
        for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++)
        {
            mtu[mtu_len] = 0xff ;
            mtu_len++;
        }
    }
    else
    {
        panic();
        return;
    }

    /* load source ethernet MAC address */
    if ( datalink->localMAC->len == MAX_ETHERNET_MAC_LEN) {
        for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++) {
            mtu[mtu_len] = datalink->localMAC->bytes[i];
            mtu_len++;
        }
    } else {
        panic();
        return;
    }

    if ((14 + 3 + dlcb->optr ) > MAX_LPDU_ETHERNET) {
        // LogError("ethernet.c: PDU is too big to send!\n");
        panic();
        return;
    }

    /* packet length */
    // ed2txinto - watch out here, i am not sure what exactly is required
    // mtu_len += encode_unsigned16(&mtu[12], 3 /*DSAP,SSAP,LLC */ + dlcb->optr );
    mtu_len += encode_unsigned16(&mtu[mtu_len], 3 /*DSAP,SSAP,LLC */ + dlcb->optr);

    /* Logical PDU portion */
    mtu[mtu_len++] = 0x82; /* DSAP for BACnet */
    mtu[mtu_len++] = 0x82; /* SSAP for BACnet */
    mtu[mtu_len++] = 0x03; /* Control byte in header */
    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->optr);
    mtu_len += dlcb->optr;

    /* Send the packet */
    sendto(eth802_sockfd, &mtu, mtu_len, 0,
        (struct sockaddr *)&eth_addr, sizeof(struct sockaddr));
        

    dlcb_free(dlcb);
}


/* receives an 802.2 framed packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t ethernet_receive(
    PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *mac,
    uint8_t *npdu,
    uint16_t max_pdu
)
//uint16_t ethernet_receive(
//    BACNET_GLOBAL_ADDRESS *src, /* source address */
//    uint8_t *pdu, /* PDU data */
//    uint16_t max_pdu, /* amount of space available in the PDU  */
//    unsigned timeout /* number of milliseconds to wait for a packet. we ommit it
//                        due to winpcap API. */
//)
{
    int received_bytes;
    uint8_t buf[MAX_LPDU_ETHERNET] = { 0 }; /* data */
    uint16_t pdu_len = 0; /* return value */
    fd_set read_fds;
    int max;
    struct timeval select_timeout;

    /* Make sure the socket is open */
    if (eth802_sockfd <= 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    //if (timeout >= 1000) {
    //    select_timeout.tv_sec = timeout / 1000;
    //    select_timeout.tv_usec =
    //        1000 * ((int)timeout - select_timeout.tv_sec * 1000);
    //} else {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 1000 ;
    //}
    FD_ZERO(&read_fds);
    FD_SET(eth802_sockfd, &read_fds);
    max = eth802_sockfd;

    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0)
        received_bytes = read(eth802_sockfd, &buf[0], sizeof(buf));
    else
        return 0;

    /* See if there is a problem */
    if (received_bytes < 0) {
        /* EAGAIN Non-blocking I/O has been selected  */
        /* using O_NONBLOCK and no data */
        /* was immediately available for reading. */
        if (errno != EAGAIN)
            fprintf(stderr, "ethernet: Read error in receiving packet: %s\n",
                strerror(errno));
        return 0;
    }

    if (received_bytes == 0)
        return 0;

    /* the signature of an 802.2 BACnet packet */
    if ((buf[14] != 0x82) && (buf[15] != 0x82)) {
        /*fprintf(stderr,"ethernet: Non-BACnet packet\n"); */
        return 0;
    }
    /* copy the source address */
    mac->len = 6;
    memmove(mac->bytes, &buf[6], 6);

    /* check destination address for when */
    /* the Ethernet card is in promiscious mode */
    if ((memcmp(&buf[0], portParams->localMAC->bytes, MAX_ETHERNET_MAC_LEN) != 0) &&
        (memcmp(&buf[0], Ethernet_Broadcast, MAX_ETHERNET_MAC_LEN) != 0)) {
        /*fprintf(stderr, "ethernet: This packet isn't for us\n"); */
        return 0;
    }

    (void)decode_unsigned16(&buf[12], &pdu_len);
    pdu_len -= 3 /* DSAP, SSAP, LLC Control */;
    /* copy the buffer into the PDU */
    if (pdu_len < max_pdu)
        memmove(&npdu[0], &buf[17], pdu_len);
    /* ignore packets that are too large */
    else
        pdu_len = 0;

    return pdu_len;
}


//void ethernet_set_my_address(
//    BACNET_GLOBAL_ADDRESS *my_address)
//{
//    int i ;
//
//    for (i = 0; i < 6; i++) {
//        Ethernet_MAC_Address[i] = my_address->mac.bytes[i];
//    }
//
//    return;
//}


void ethernet_get_MAC_address(
    const PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS * my_address)
{
    int i ;

    my_address->len = MAX_ETHERNET_MAC_LEN;
    my_address->macType = MAC_TYPE_ETHERNET ;
    for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++) {
        my_address->bytes[i] = portParams->localMAC->bytes[i] ;
    }
}


//void ethernet_get_broadcast_address(
//    BACNET_PATH *dest)        /* destination address */
//{ 
//    int i;
//
//    dest->localMac.macType = MAC_TYPE_ETHERNET;
//    for (i = 0; i < 6; i++) {
//        dest->localMac.bytes[i] = Ethernet_Broadcast[i];
//    }
//    dest->localMac.len = 6;
//    dest->glAdr.net = BACNET_BROADCAST_NETWORK;
//    dest->glAdr.mac.len = 0; /* denotes broadcast address  */
//    for (i = 0; i < MAX_MAC_LEN; i++) {
//        dest->glAdr.mac.bytes[i] = 0;
//    }
//}


#if 0
void ethernet_debug_address(
    const char *info, 
    BACNET_GLOBAL_ADDRESS *dest)
{
    int i = 0; /* counter */

    if (info)
        fprintf(stderr, "%s", info);
    if (dest) {
        fprintf(stderr, "Address:\n");
        fprintf(stderr, "  MAC Length=%d\n", dest->mac_len);
        fprintf(stderr, "  MAC Address=");
        for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++) {
            fprintf(stderr, "%02X ", (unsigned) dest->mac[i]);
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "  Net=%hu\n", dest->net);
        fprintf(stderr, "  Len=%d\n", dest->len);
        fprintf(stderr, "  Adr=");
        for (i = 0; i < MAX_ETHERNET_MAC_LEN; i++) {
            fprintf(stderr, "%02X ", (unsigned) dest->adr[i]);
        }
        fprintf(stderr, "\n");
    }
}
#endif
