/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#include <stdint.h>

// in MSVC, winsock2.h must be included first
#if defined(_MSC_VER)
#include <winsock2.h>
#endif
#if defined(__GNUC__)

#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>

#include <linux/if_link.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#include <stdbool.h>
#include <stdio.h>
#endif

#include "btaDebug.h"
#include "stdint.h"
#include "bacstr.h"
#include "emm.h"
#include "ese.h"
#include "bacdcode.h"
#include "bacaddr.h"

#include "osLayer.h"
#include "bitsUtil.h"
#include "bacint.h"
#include "net.h"
#include "bitsDebug.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// A BTA needs to be able to find us, but we cannot guarantee that we will be able to bind on any given ephemeral socket,
// so BITS maintains a list of 'tries', in the expectation that at least ONE of the bind attempts will succeed, and that
// will become our BTA port. The BTA client tries this shortlist every 10 seconds.

uint16_t btaPorts[] = {
#ifndef _MSC_VER
    // BTA most likely going to run on this port, if not already.
    41797,
#endif
    60881,       // 0xEDD1
    61077,
    47824,       // 0xBAD0
    33330,       // Added a few more... can easily see 2, 3 apps running on ome machine, plus a few spare for other collisions
    44440,
    55550,
    61111
};

uint16_t MyPortsTraffic;

uint defaultBTAclientAddr = INADDR_BROADCAST;
uint16_t defaultBTAclientPort = 41797;

// #define MyPortsTerminal 48796
// #define MyPortsPanic 502

// #define MX_BTA_BUFFER   1500

#if !defined(_MSC_VER)
static struct sockaddr_in ourAddress;
#endif

#if 0
static struct sockaddr_in bcastAddress;
#endif

#if defined(__GNUC__)
char firstEthernet[20];
#endif

// forwards to shut GCC up
//int encodeIPEP(unsigned char *buf, struct sockaddr_in *addr);
//int encodeAddr(unsigned char *buf, struct in_addr *addr) ;
//int encodeInt16(unsigned char *buf, uint16_t port) ;
//int encodeInt32(unsigned char *buf, uint32_t *val) ;
//void PrepareBTAheader(unsigned char *buffer, int fc) ;
//void SendBTAmessage(char *message);
//void SendBTApacketTxRx(int port_id, BACNET_MAC_ADDRESS *srcPhyMac, BACNET_MAC_ADDRESS *destPhyMac, uint8_t *pdu, int len, int flagtx);


#if defined(__GNUC__)

// establish a workable ethernet adapter (todo2 - make a list and use all working adapters in future)
void EstablishWorkingAdapter(void);
void EstablishWorkingAdapter(void)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */

#if 0
    // todo, re-enable for debugging only
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
           form of the latter for the common families) */

        printf("%-8s %-12s (%2d) %-6s %-12s %-13s\n",
            ifa->ifa_name,
            (family == AF_PACKET) ? "AF_PACKET" :
            (family == AF_INET) ? "AF_INET" :
            (family == AF_INET6) ? "AF_INET6" : "???",
            family,
            (ifa->ifa_flags & IFF_UP ) ? "UP" : "not up",
            (ifa->ifa_flags & IFF_RUNNING ) ? "RUNNING" : "not running",
            (ifa->ifa_flags & IFF_LOOPBACK ) ? "LOOPBACK" : "not loopback"
            );

        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                      sizeof(struct sockaddr_in6),
                host,
                NI_MAXHOST,
                NULL,
                0,
                NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\t\taddress: <%s>\n", host);

        }
        /*
                else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                    struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;

                    printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                           "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
           stats->tx_packets,
           stats->rx_packets,
           stats->tx_bytes,
           stats->rx_bytes);
   }
   */
    }
#endif // ifdef 0 / debugging

    // look for a suitable adapter to use
    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue ;
        if (!(ifa->ifa_flags & IFF_UP)) continue ;
        if (!(ifa->ifa_flags & IFF_RUNNING)) continue ;
        if ((ifa->ifa_flags & IFF_LOOPBACK)) continue ;     
        
        // So, IPv4, up,running and not loopback, use it!
        // printf("tada, our adapter is : %s\n", ifa->ifa_name);
        strncpy(firstEthernet, ifa->ifa_name, sizeof(firstEthernet));
        firstEthernet[sizeof(firstEthernet) - 1] = 0;

        break;
    }

    freeifaddrs(ifaddr);
}
#endif

// encodes ipendpoint into out buffer
int encodeIPEP(unsigned char *buf, struct sockaddr_in *addr)
{
    memcpy(buf, &addr->sin_addr.s_addr, 4);
    memcpy(&buf[4], &addr->sin_port, 2);
    return 6;
}


#if 0
int encodeAddr(unsigned char *buf, struct in_addr *addr)
{
    memcpy(buf, addr, 4);
    return 4;
}
#endif


int encodeUInt16(uint8_t *buf, const uint16_t value)
{
    uint16_t nwoint = htons(value);
    memcpy(buf, &nwoint, 2);
    return 2;
}


int encodeUInt32(unsigned char *buf, const uint32_t val)
{
    // todo1 endian issue here
    // memcpy(buf, &val, 4);
    buf[0] = ((uint8_t *)&val)[3];
    buf[1] = ((uint8_t *)&val)[2];
    buf[2] = ((uint8_t *)&val)[1];
    buf[3] = ((uint8_t *)&val)[0];
    return 4;
}

#if defined(_MSC_VER)

// Clock gettime for win32 from StackOverflow: http://goo.gl/WHR94q

LARGE_INTEGER getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

void clock_gettime(struct timeval *tv)
{
    LARGE_INTEGER           t;
    FILETIME				f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    // static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        //LARGE_INTEGER performanceFrequency;
        //initialized = 1;
        //usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        //if (usePerformanceCounter) {
        //	QueryPerformanceCounter(&offset);
        //	frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        //} else {
        offset = getFILETIMEoffset();
        frequencyToMicroseconds = 10.;
        //}
    }

    //if (usePerformanceCounter) QueryPerformanceCounter(&t);
    //else {
    GetSystemTimeAsFileTime(&f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    //}

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = (LONGLONG) microseconds;

    // t.QuadPart -= 116444736000000000 ;

    tv->tv_sec = (LONG)(t.QuadPart / 1000000);
    tv->tv_usec = t.QuadPart % 1000000;
}
#endif

// BTA reporting packet frame structure can be found here: https://goo.gl/JixmbT

void PrepareBTAheader(const BTApt fc, uint8_t *buffer)
{
    unsigned long tosendlong;

    buffer[0] = 0;
    buffer[1] = fc;

    // uint32_t    ourAddr;

#if _MSC_VER
    struct timeval gettime_now;

    // this is a very high resolution time, but from start of execution, not UTC... todo - combine..
    clock_gettime(&gettime_now);

    // time
    tosendlong = htonl(gettime_now.tv_sec);
    memcpy(&buffer[2], &tosendlong, 4);

    tosendlong = htonl(gettime_now.tv_usec * 1000);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

#ifdef SOMELINUX
    // this is better, returning to prior method for a while until this subject
    // is revisited (so as to highlight that it is incomplete).

    // struct timeval gettime_now ;
    SYSTEMTIME st;
    FILETIME ft;
    ULONGLONG hectomicrosecs;      // 64-bit

    long seconds;
    long milliseconds;

    // this is a very high resolution time, but from start of execution, not UTC... todo 2 - combine..
    //clock_gettime( &gettime_now);

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

    hectomicrosecs = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    // counts of 100ns intervals since Jan 1, 1601, and reduced to seconds,
    // seconds won't fit in 32 bit, so do offset to linux time.. 1970-01-01
    // 00:00:00 +0000 (UTC) is 1970 - 1601 = 369 years = 369 * 31,556,925.9936
    // seconds = 11644505691.6384 seconds

    hectomicrosecs -= 116445056916384 * 1000;

    // time
    // seconds - remember - UTC !
    tosendlong = (unsigned long)(hectomicrosecs / 10000000);
    tosendlong = htonl(tosendlong);
    memcpy(&buffer[2], &tosendlong, 4);

    // time
    // nanosecs
    tosendlong = (long)((hectomicrosecs * 100) % 1000000000);
    tosendlong = htonl(tosendlong);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

#if ! defined ( _MSC_VER  ) 	// some linux
    struct timespec gettime_now;

    clock_gettime(CLOCK_REALTIME, &gettime_now);

    // time
    tosendlong = htonl(gettime_now.tv_sec);
    memcpy(&buffer[2], &tosendlong, 4);
    tosendlong = htonl(gettime_now.tv_nsec);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

    // remote agent IP
#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
//    ourAddr = bip_get_addr();
    // ourAddr = 0;

    // todo 2 - this trick only works if we know the destination of the peer....
    // i.e. we should wait for a ping from BTA before trying to establish local
    // address/interface..
    // struct sockaddr_in sin;
    // TryGetLocalAddress(&sin);
    // ourAddr = sin.sin_addr.s_addr;

    // todo3 - see
    // http://stackoverflow.com/questions/2432493/how-to-detemine-which-network-interface-ip-address-will-be-used-to-send-a-pack
    // encodeUInt32(&buffer[10], ourAddr);
    // don't bother with the port, it is not used by BTA anyway.

    encodeUInt32(&buffer[10], 0x02020202);
    encodeUInt16(&buffer[14], 0x0);

#else
    encodeIPEP(&buffer[10], &ourAddress);
#endif

}

static struct sockaddr_in destIPEP;
static bool clientIsKnown;

#if _MSC_VER
static void ProcessBTAclientMessage(uint8_t *buffer, int len,
                                    struct sockaddr_in *sinIPEP, int sinlen)
#else
static void ProcessBTAclientMessage(uint8_t *buffer, int len,
                                    struct sockaddr_in *sinIPEP,
                                    socklen_t sinlen)
#endif
{
    uint16_t functionCode;
    uint8_t rlen = decode_unsigned16(buffer, &functionCode);
    if (rlen == 0)
    {
        panic();
        return;
    }

    switch (functionCode)
    {

        case ClientAlive:
            // printf("client alive\n");
            // and now we have a port to send to!
            destIPEP = *sinIPEP;
            clientIsKnown = true;
            // SendBTAmessage("got a sinlen");
            break;

        default:
            // well, there is other traffic out there..
            // todo2 - when ports get decoupled, we can start checking for
            // unexpected messages again panic(); no.. there is lots of other
            // traffic out there
            break;
    }
}

static bool btaReady;

static int sock_fd;

static bool FindBTAport(void)
{
    for (int i = 0; i < sizeof(btaPorts) / sizeof(uint16_t); i++) {
        // bind to our port
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(btaPorts[i]);
        memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
        int status = bind(sock_fd, (const struct sockaddr *)&sin, sizeof(struct sockaddr));
        if (status == 0) {
            printf("BTA listen port is %d\n", btaPorts[i]);
            MyPortsTraffic = btaPorts[i];
            return true;
        }
    }
    panic();
    return false;
}


static void BTAlistenThread(void *params)
{
    while (true)
    {
        //sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        //if (sock_fd < 0)
        //{
        //    panic();
        //    return;
        //}

//            /* Allow us to use the same socket for sending and receiving */
//            /* This makes sure that the src port is correct when sending */
//#ifdef _MSC_VER
//        char sockopt = 1;
//#else
//        int sockopt = 1;
//#endif
        //int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt,
        //                        sizeof(sockopt));
        //if (status < 0)
        //{
        //    closesocket(sock_fd);
        //    panic();
        //    return;
        //}

        ///* allow us to send a broadcast */
        //int status = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &sockopt,
        //                    sizeof(sockopt));
        //if (status < 0)
        //{
        //    closesocket(sock_fd);
        //    panic();
        //    return;
        //}

        //if ( ! FindBTAport ( ) )
        //{
        //    closesocket(sock_fd);
        //    return;
        //}

        //btaReady = true;

        while (true)
        {
            uint8_t pdu[10];
            // memset(pdu, 0x88, 100);

            struct sockaddr_in sin;

#ifdef _MSC_VER
            int sin_len = sizeof(sin);
#else
            socklen_t sin_len = sizeof(sin);
#endif

            int received_bytes = recvfrom(sock_fd, (char *)&pdu[0], sizeof(pdu),
                                          0, (struct sockaddr *)&sin, &sin_len);

            // the above blocks, (normally)
            if (received_bytes <= 0)
            {
            // something went very wrong....
#ifdef _MSC_VER
                int Code = WSAGetLastError();
                if (Code != 10040) {
                    printf("BTA stack initialization failed"
                        " error code: %i %s\n",
                        Code, winsock_error_code_text(Code));
                    break;
                }
#else
                panic();
#endif  
            }

            ProcessBTAclientMessage(pdu, received_bytes, &sin, sin_len);
        }

        // restarting
        panic();
        msSleep(10000);
    }
}


static bool initAlreadyAttempted;
bool BTAready(void)
{
    if (btaReady)
        return true;
    if (initAlreadyAttempted)
        return false;
    printf("Enable BTA with call to BTAinit() for future use.\n");
    BTAinit();
    return true;
}

void BTAinit()
{
    if (btaReady)
        return;
    if (initAlreadyAttempted)
        return;
    initAlreadyAttempted = true;

#if defined(_MSC_VER)
    WSADATA wd;
    int Result = WSAStartup((1 << 8) | 1, &wd);
        if (Result != 0)
        {
        // we are going to be compeletely useless, so terminate
        int Code = WSAGetLastError();
        // cant use BTA here, we have not been able to init it!
        printf("WSA library did not initialize\n");
        return ;
        }
#endif

    // static int sockfd = -1;

#ifdef PLATFORM_BB
    struct ifreq ifr;
    int status;
#endif

//     int sockopt = 1; // yes, we need it to be set

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("m0964");
        return;
    }

#ifdef PLATFORM_BB_TODO1

    memset(&ifr, 0, sizeof(ifr));

    // establish a workable ethernet adapter (todo2 - make a list and use all
    // working adapters in future)
    EstablishWorkingAdapter();

    snprintf(
        ifr.ifr_name, sizeof(ifr.ifr_name), "%s",
        firstEthernet); // todonext - make same as non-lon port for diagnostics

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
        sizeof(ifr)) < 0) {
        printf("Failed to bind to [%s] - did you run sudo?\n", ifr.ifr_name);
        perror("m0975");
        emm_free(payload);
        return;
    }

    status = ioctl(sockfd, SIOCGIFBRDADDR, &ifr);
    if (status < 0) {
        printf("Failed to read broadcast address for %s, status=%d\n",
            ifr.ifr_name, status);
        perror("m0091");
        emm_free(payload);
        return;
    }

    uint32_t bcastAddress;
    memcpy(&bcastAddress, &ifr.ifr_ifru.ifru_broadaddr, sizeof(bcastAddress));

    status = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (status < 0) {
        printf("Failed to read local address for %s, status=%d\n", ifr.ifr_name,
            status);
        perror("m1052 - btaDebug");
        emm_free(payload);
        return;
    }

    // uint32_t ourAddress;
    memcpy(&ourAddress, &ifr.ifr_ifru.ifru_addr, sizeof(ourAddress));
    // hexdump("Loc Addr", &ourAddress, sizeof(ourAddress));

    ////hexdump(ifr.ifr_ifrn.ifrn_name, &((struct sockaddr_in
    ///*)&ifr.ifr_ifru.ifru_addr)->sin_addr, 6);
    //   //hexdump(ifr.ifr_ifrn.ifrn_name, (struct sockaddr_in *)
    //   &ifr.ifr_ifru.ifru_addr, 8);
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
    //    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)
    //    &sockopt, sizeof(sockopt));
#else
    status =
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));
    if (status < 0) {
        close(sockfd);
        printf("Failed to set reuse %d\n", status);
        perror("m0029");
        return;
    }
#endif

#ifdef _MSC_VER
    char sockopt = 1;
#else
    int sockopt = 1;
#endif

    /* allow us to send a broadcast */
#if defined(_MSC_VER) || defined(__GNUC__)
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&sockopt,
        sizeof(sockopt));
#elif defined(BTA_LWIP)
    if (lwip_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
        &timeoutTimeInMilliSeconds, sizeof(int)) == -1) {
        // return ;
    }
#else
    sockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof(sockopt));
    if (status < 0) {
        closesocket(sockfd);
        sockfd = -1;
        printf("m00981: Failed to set broadcast %d\n", status);
        perror("m0991");
        emm_free(payload);
        return;
    }

#endif

    if (!FindBTAport()) {
        closesocket(sock_fd);
        return;
    }

    btaReady = true;

    bitsCreateThread(BTAlistenThread, NULL);
    initAlreadyAttempted = true;
}

// this function does nothing but send the payload and frees allocated
// memory - no formatting at all
void SendBTApayload(uint8_t *payload, const int sendlength)
{
    if (!BTAready())
    {
        emm_free(payload);
        return;
    }

    struct sockaddr_in servaddr;

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl( defaultBTAclientAddr );
    servaddr.sin_port = htons( defaultBTAclientPort );

#ifdef BTA_LWIP
    memset(&self, 0, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(0);
    self.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to port
    status = bind(sockfd, (struct sockaddr *)&self, sizeof(self));
    if (status != 0)
    {
        status = errno;
        // printf("socket--bind error");
        close(sockfd);
        emm_free(payload);
        return;
    }
#else
    // memset(&servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));
    // bind the socket for receiving messages.. todonext..
    // status = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(struct
    // sockaddr));  if (status < 0)
    //{
    //    printf("m0010 - Failed to bind\n");
    //    close(sockfd);
    //}
#endif

#if defined(_MSC_VER) || defined(BTA_LWIP)
    servaddr.sin_addr.s_addr = inet_addr("255.255.255.255");
#else
    // memcpy ( &servaddr.sin_addr.s_addr, &bcastAddress.sin_addr.s_addr, sizeof
    // ( servaddr.sin_addr.s_addr ) ) ;
    if (clientIsKnown)
    {
        servaddr = destIPEP;
    }
    else
    {
        servaddr.sin_addr.s_addr = inet_addr("192.168.1.255");
    }
#endif

    // hexdump("Dest Addr5", &servaddr.sin_addr.s_addr,
    // sizeof(servaddr.sin_addr.s_addr));

#if defined(_MSC_VER) || defined(__GNUC__)
    sendto(sock_fd, (const char *)payload, sendlength, 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr));
#else
    sendto(sockfd, payload, sendlength, 0, (struct sockaddr *)&servaddr,
           sizeof(servaddr));
#endif

    // todonext, when do we get to close this socket under normal conditions....
    // close(sockfd);

    emm_free(payload);
}


void SendBTAhexdump(const char *message, const void *buffer, const uint16_t len)
{
    if (!BTAready()) return ;
    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return ;
    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(BTApt_Hexdump, outbuf);

    BACNET_CHARACTER_STRING tstring;
    characterstring_init_ansi(&tstring, message);
    uint16_t apdu_len = encode_application_character_string(&outbuf[20], &tstring);

    encodeUInt16(&outbuf[16], apdu_len);          // space allocated for message

    // if there is not enough space, trim the len
    uint16_t newlen = MIN(len, MX_BTA_BUFFER - 20 - apdu_len);
    encodeUInt16(&outbuf[18], newlen);

    memcpy(&outbuf[20 + apdu_len], buffer, newlen);
    SendBTApayload(outbuf, 20 + apdu_len + newlen);
}


static void SendBTAtext(const BTApt pt, const char *message)
{
    // do this check early to avoid unnecessary CPU
    if(!BTAready())
    {
        ese_enqueue_once(ese028_1C_Attempt_to_send_BTA_before_IP_ready);
        return ;
    }

    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return ;
    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(pt, outbuf);
    int datalen = MIN(strlen(message), MX_BTA_BUFFER - BX_DATAOFFSET - 1);
    strncpy((char *)&outbuf[BX_DATAOFFSET], message, datalen);
    SendBTApayload(outbuf, datalen + BX_DATAOFFSET);
}

void SendBTAmessage(const char *message)
{
    SendBTAtext(BTApt_TextMsg, message);
}

void SendBTAstartMessage(const char *message)
{
    SendBTAtext(BTApt_StartMsg, message);
}

void SendBTApanicMessage(const char *message)
{
    SendBTAtext(BTApt_PanicMsg, message);
}

#define MX_TMESSAGE     100

static void EncodeInt(char *tmessage, int *optr, int value)
{
    tmessage[(*optr)++] = ((value / 1000) % 10) + '0';
    tmessage[(*optr)++] = ((value / 100) % 10) + '0';
    tmessage[(*optr)++] = ((value / 10) % 10) + '0';
    tmessage[(*optr)++] = ((value / 1) % 10) + '0';
    tmessage[(*optr)++] =  0;
}

void SendBTApanicInt(const char *message, const int value)
{
    if (!BTAready()) return ;

    char tmessage[MX_TMESSAGE];
    int len = strlen(message);
    if (len > MX_TMESSAGE - 10) return ;
    strncpy(tmessage, message, MX_TMESSAGE);
    tmessage[len++] =  ' ';
    tmessage[len++] =  ' ';
    EncodeInt(tmessage, &len, value);
    SendBTApanicMessage(tmessage);
}

void SendBTAmessageF1(char *message, int value)
{
    char tmessage[MX_TMESSAGE];
    if (!BTAready()) return ;
    int len = strlen(message);
    if (len > MX_TMESSAGE - 10) return ;
    strncpy(tmessage, message, MX_TMESSAGE);
    tmessage[len++] =  ' ';
    tmessage[len++] =  ' ';
    EncodeInt(tmessage, &len, value);
    tmessage[len++] =  0;
    SendBTAmessage(tmessage);
}

void SendBTAmstpFrame(const uint8_t *frame, const uint16_t data_len)
{
    // do this check early to avoid unnecessary CPU
    if(!BTAready()) return;

    if (data_len > 512 || data_len < 2) {
        // a bit recursive? panic();
        return;
    }

    // we are not going to send the 55, ff,

    // unsigned char outbuf[MX_BTA_BUFFER];
    uint8_t *outbuf = (uint8_t *)emm_smalloc('c', MX_BTA_BUFFER);
    if (outbuf == NULL) return ;

    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(BTApt_MSTPframe, outbuf);

    // we are not going to send the 55, ff, so subtract 2 from len
    int datalen = MIN(data_len - 2, MX_BTA_BUFFER - BX_DATAOFFSET);

    memcpy((uint8_t *)&outbuf[BX_DATAOFFSET], &frame[2], datalen);
    SendBTApayload(outbuf, datalen + BX_DATAOFFSET);
}

// and if we don't have the header information
void SendBTAmstpPayload(const uint8_t *payload, const uint16_t data_len,
                        const uint8_t function, const uint8_t dest,
                        const uint8_t source)
{
    if (!BTAready()) return;

    uint8_t *outbuf = (uint8_t *)emm_smalloc('p', MX_BTA_BUFFER);
    if (outbuf == NULL) return ;

    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(BTApt_MSTPframe, outbuf);
    int datalen = MIN(data_len, MX_BTA_BUFFER - BX_DATAOFFSET - 6);

    // this sends a whole frame
    outbuf[BX_DATAOFFSET + 0] = function;
    outbuf[BX_DATAOFFSET + 1] = dest;
    outbuf[BX_DATAOFFSET + 2] = source;
    outbuf[BX_DATAOFFSET + 3] = (uint8_t)(data_len >> 8);
    outbuf[BX_DATAOFFSET + 4] = (uint8_t)(data_len & 0xff);
    outbuf[BX_DATAOFFSET + 5] = 0;    // header checksum
    memcpy(&outbuf[BX_DATAOFFSET + 6], payload, datalen);

    SendBTApayload(outbuf, datalen + BX_DATAOFFSET + 6);
}

static void SendBTApacketTxRx(const int port_id,
                              const BACNET_MAC_ADDRESS *srcPhyMac,
                              const BACNET_MAC_ADDRESS *destPhyMac,
                              const uint8_t *pdu, const int len,
                              const int flagtx)
{
    //     unsigned char outbuf[1500];
    int i;
    BACNET_MAC_ADDRESS localSrcPhyMac, localDestPhyMac;

    uint8_t *outbuf = (uint8_t *)emm_smalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return;

    bacnet_mac_copy(&localSrcPhyMac, srcPhyMac);
    bacnet_mac_copy(&localDestPhyMac, destPhyMac);

#ifdef DEBUG
    if (!bacnet_mac_check(&localSrcPhyMac)) {
        panic();
        bacnet_mac_clear(&localSrcPhyMac);
    }
    if (!bacnet_mac_check(&localDestPhyMac)) {
        panic();
        bacnet_mac_clear(&localDestPhyMac);
    }
#endif

    PrepareBTAheader(BTApt_BACstd, outbuf);

    int optr = 16;
    // prep the payload

    // flags... Rx
    outbuf[optr++] = (flagtx) ? 1 : 0;

    // process ID

    optr += encodeUInt16(&outbuf[optr], (uint16_t) port_id);

    // source
    outbuf[optr++] = localSrcPhyMac.len;
    for (i = 0; i < localSrcPhyMac.len; i++) {
        outbuf[optr++] = localSrcPhyMac.adr[i];
    }

    // dest
    outbuf[optr++] = localDestPhyMac.len;
    for (i = 0; i < localDestPhyMac.len; i++) {
        outbuf[optr++] = localDestPhyMac.adr[i];
    }

    // now the packet itself
    memcpy(&outbuf[optr], pdu, len);
    optr += len;

    SendBTApayload(outbuf, optr);
}


void SendBTApacketTx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac,
                     const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu,
                     const int len)
{
    SendBTApacketTxRx(port_id, srcPhyMac, destPhyMac, pdu, len, 1);
}

void SendBTApacketRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac,
                     const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu,
                     const int len)
{
    SendBTApacketTxRx(port_id, srcPhyMac, destPhyMac, pdu, len, 0);
}

#if 0
void SendBTAprintf(const char *fmt, ...)
{
    char tformat[1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(tformat, fmt, ap);
    SendBTAmessage(tformat);
    va_end(ap);
}

void SendBTApanic(const char *fmt, ...)
{
    char tformat[1000];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(tformat, fmt, ap);
    SendBTAmessage(tformat);
    va_end(ap);
}

void SendBTAvprintf(const char *fmt, va_list ap)
{
    char tformat[1000];
    vsprintf(tformat, fmt, ap);
    SendBTAmessage(tformat);
}

#endif // _MSC_VER

