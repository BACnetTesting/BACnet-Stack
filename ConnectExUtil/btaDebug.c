/**************************************************************************
*
* Copyright (C) 2014-2016 ConnectEx, Inc. <info@connect-ex.com>
*
* Permission is hereby granted, to whom a copy of this software and 
* associated documentation files (the "Software") is provided by ConnectEx, Inc.
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
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

#ifdef _MSC_VER
#include "assert.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// #include <winsock2.h>
#include "net.h"
#include "emm.h"
#endif

#include "btaDebug.h"
#if 0
#include "bacdef.h"
#include <memory.h>
#endif

#if 0
#include "bip.h"
#include "CEDebug.h"
#endif

#if 0
extern char firstEthernet[20] ;
#endif

#ifndef _MSC_VER
static struct sockaddr_in ourAddress;
#endif

#if 0
static struct sockaddr_in bcastAddress;
#endif

// forwards to shut GCC up
int encodeIPEP(unsigned char *buf, struct sockaddr_in *addr);

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


int encodeUInt16(uint8_t *buf, uint16_t port)
{
    uint16_t nwoint = htons(port);
    memcpy(buf, &nwoint, 2);
    return 2;
}

int encodeUInt32(unsigned char *buf, uint32_t val)
{
    // memcpy(buf, &val, 4);
  buf[0] = ((uint8_t *)&val)[3] ;
  buf[1] = ((uint8_t *)&val)[2] ;
  buf[2] = ((uint8_t *)&val)[1] ;
  buf[3] = ((uint8_t *)&val)[0] ;
  return 4;
}

#ifdef _MSC_VER

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

    tv->tv_sec = (LONG) ( t.QuadPart / 1000000 ) ;
    tv->tv_usec = t.QuadPart % 1000000;
}
#endif

void PrepareBTAheader( BTAmsgType fc, uint8_t *buffer)
{
    unsigned long tosendlong;

    buffer[0] = 0;
    buffer[1] = fc;

#ifdef _somelinux
    uint32_t    ourAddr;

#if 1
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
    // this is better, returning to prior method for a while until this subject is revisited (so as to highlight that it is incomplete).

    // struct timeval gettime_now ;
    SYSTEMTIME st;
    FILETIME ft;
    ULONGLONG hectomicrosecs;      // 64-bit

    long seconds;
    long milliseconds;

    // this is a very high resolution time, but from start of execution, not UTC... todo - combine..
    //clock_gettime( &gettime_now);

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

    hectomicrosecs = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    // counts of 100ns intervals since Jan 1, 1601, and reduced to seconds, seconds won't fit in 32 bit, so do offset to linux time.. 1970-01-01 00:00:00 +0000 (UTC)
    // is 1970 - 1601 = 369 years = 369 * 31,556,925.9936 seconds = 11644505691.6384 seconds

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

#if 0
    struct timespec gettime_now;

    clock_gettime(CLOCK_REALTIME, &gettime_now);

    // time
    tosendlong = htonl(gettime_now.tv_sec);
    memcpy(&buffer[2], &tosendlong, 4);
    tosendlong = htonl(gettime_now.tv_nsec);
    memcpy(&buffer[6], &tosendlong, 4);
#endif

    // remote agent IP
#ifdef _MSC_VER
//    ourAddr = bip_get_addr();
    ourAddr = 0;
    encodeInt32(&buffer[10], &ourAddr);
    // don't bother with the port, it is not used by BTA anyway.
#else
    encodeIPEP(&buffer[10], &ourAddress);
#endif

}


// this function does nothing but send the payload and frees allocated
// memory - no formatting at all
void SendBTApayload(uint8_t *payload, int nsendlength)
{
    struct sockaddr_in servaddr;
    static int sockfd = -1 ;
    unsigned char outbuf[MX_BTA_BUFFER];
    //    int debugSendlen ;

    // printf("Send1\n");

#ifdef PLATFORM_BB
    struct ifreq ifr;
#endif

    int status;

#ifdef _MSC_VER
    int sockopt;
    static int Result = -1 ;
    int Code;
    WSADATA wd;

    // only do this once
    if (Result == -1) {
        Result = WSAStartup((1 << 8) | 1, &wd);
        /*Result = WSAStartup(MAKEWORD(2,2), &wd); */
        if (Result != 0) {
            Code = WSAGetLastError();
            printf("TCP/IP stack initialization failed\n" " error code: %i %s\n",
                   Code, winsock_error_code_text(Code));
            exit(1);
        }
    }
#endif

    if (sockfd < 0) {
        // only do this once, it is a faily expensive process...
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("m0964");
            free ( payload ) ;
            return;
        }
    }

#ifdef PLATFORM_BB

    memset(&ifr, 0, sizeof(ifr));

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), firstEthernet);		// todonext - make same as non-lon port for diagnostics

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        printf("Failed to bind to [%s] - did you run sudo?\n", ifr.ifr_name);
        perror ("m0975");
            free ( payload ) ;
        return;
    }

    status = ioctl(sockfd, SIOCGIFBRDADDR, &ifr);
    if (status < 0) {
        printf("Failed to read broadcast address for %s, status=%d\n", ifr.ifr_name, status);
        perror ( "m0091") ;
            free ( payload ) ;
        return;
    }
    memcpy(&bcastAddress, &ifr.ifr_ifru.ifru_broadaddr, sizeof(bcastAddress));

    status = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (status < 0) {
        printf("Failed to read local address for %s, status=%d\n", ifr.ifr_name, status);
        perror ( "m1052 - btaDebug") ;
            free ( payload ) ;
        return;
    }
    memcpy(&ourAddress, &ifr.ifr_ifru.ifru_addr, sizeof(ourAddress));
    // hexdump("Loc Addr", &ourAddress, sizeof(ourAddress));

    ////hexdump(ifr.ifr_ifrn.ifrn_name, &((struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr)->sin_addr, 6);
    //   //hexdump(ifr.ifr_ifrn.ifrn_name, (struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr, 8);
#endif

#ifdef _MSC_VER
    sockopt = 1;

    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &sockopt, sizeof(sockopt));
    if (status < 0) {
        closesocket(sockfd);
        // printf("Failed to set reuse %d\n", status);
        // perror ( "m1056") ;
        emm_free ( payload ) ;
        return;
    }
#endif

    /* allow us to send a broadcast */
#ifdef _MSC_VER
    status = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char *)&sockopt, sizeof(sockopt));
#else
    status = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof(sockopt));
    if (status < 0) {
        closesocket(sockfd);
        sockfd = -1;
        printf("m00981: Failed to set broadcast %d\n", status);
        perror ("m0991");
        free ( payload ) ;
        return;
    }
    
#endif


    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    servaddr.sin_port = htons(MyPortsTraffic);

    // memset(&servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));
    // bind the socket for receiving messages.. todonext..
    //status = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    //if (status < 0)
    //{
    //    printf("m0010 - Failed to bind\n");
    //    close(sockfd);
    //}

    
//    PrepareBTAheader(outbuf, fc);

//    memcpy(&outbuf[16], payload, nsendlength);

#if defined ( _MSC_VER ) || defined ( BTA_LWIP )
    // to avoid setting sockopt bcast
    servaddr.sin_addr.s_addr = inet_addr("255.255.255.255");
#else
    me mcpy ( &servaddr.sin_addr.s_addr, &bcastAddress.sin_addr.s_addr, sizeof ( servaddr.sin_addr.s_addr ) ) ;
#endif

    // hexdump("Dest Addr5", &servaddr.sin_addr.s_addr, sizeof(servaddr.sin_addr.s_addr));

#if defined ( _MSC_VER ) 
// lwip_sendto(int s, const void *data, size_t size, int flags,
//       const struct sockaddr *to, socklen_t tolen)
    sendto(sockfd, (const char *)payload, nsendlength, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
#else
    sendto(sockfd, payload, nsendlength, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
#endif

    // todonext, when do we get to close this socket under normal conditions....
    closesocket(sockfd);
    
#endif  // DELIVERY_CANDIDATE == 0
  

    // todo1 free ( payload ) ;
}

static void SendBTAtext ( BTAmsgType pt, char *message)
{
    uint8_t *outbuf = (uint8_t *)emm_dmalloc('b', MX_BTA_BUFFER);
    if ( outbuf == NULL ) return ;
    memset(outbuf, 0, MX_BTA_BUFFER);
    PrepareBTAheader(pt, outbuf);
    int datalen = MIN(strlen(message), MX_BTA_BUFFER-BX_DATAOFFSET-1) ;
    strncpy((char *)&outbuf[BX_DATAOFFSET], message, datalen );
    SendBTApayload( outbuf, datalen+BX_DATAOFFSET );
}

void SendBTAmessage(char *message)
{
  SendBTAtext( BTAmsgType_TextMsg, message );
}

void SendBTAstartMessage(char *message)
{
  SendBTAtext( BTAmsgType_StartMsg, message );
}

void SendBTApanicMessage(char *message)
{
  SendBTAtext( BTAmsgType_PanicMsg, message );
}

#define MX_TMESSAGE     100
void SendBTApanicInt(char *message, int value )
{
  char tmessage[MX_TMESSAGE] ;
  int len = strlen ( message ) ;
  if ( len > MX_TMESSAGE - 10 ) return ;
  strncpy ( tmessage, message, MX_TMESSAGE ) ;
  tmessage[len] = hex2char(value>>4);
  tmessage[len+1] = hex2char(value%4);
  tmessage[len+2] = 0;
  SendBTApanicMessage( tmessage );
}



static void SendBTApacketTxRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len, const int flagtx)
{
//     unsigned char outbuf[1500];
    int i ;

    u8_t *outbuf = (u8_t *)emm_dmalloc('b', MX_BTA_BUFFER);
    if (outbuf == NULL) return;

#ifdef MEM_OVERFLOW_CHECK
    assert(srcPhyMac->len <= sizeof(BACNET_MAC_ADDRESS));
    assert(destPhyMac->len <= sizeof(BACNET_MAC_ADDRESS));
#endif

    PrepareBTAheader(BTAmsgType_BACstd, outbuf );
    
    int optr = 16 ;
    // prep the payload

    // flags... Rx
    outbuf[optr++] = (flagtx) ? 1 : 0;

    // process ID

    optr += encodeUInt16(&outbuf[optr], (uint16_t) port_id );

    // source
    outbuf[optr++] = srcPhyMac->len;
    for (i = 0; i < srcPhyMac->len; i++) {
        outbuf[optr++] = srcPhyMac->bytes[i];
    }

    // dest
    outbuf[optr++] = destPhyMac->len;
    for ( i = 0; i < destPhyMac->len; i++) {
        outbuf[optr++] = destPhyMac->bytes[i];
    }

    // now the packet itself
    memcpy(&outbuf[optr], pdu, len);
    optr += len;

    SendBTApayload(outbuf, optr);
}


void SendBTApacketTx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len)
{
    SendBTApacketTxRx( port_id, srcPhyMac, destPhyMac, pdu,  len, 1);
}

void SendBTApacketRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const  uint8_t *pdu, const int len)
{
    SendBTApacketTxRx( port_id, srcPhyMac, destPhyMac, pdu, len, 0);
}
