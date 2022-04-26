never implemented just blind copy
/****************************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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

// in MSVC, winsock2.h must be included first
#if ! defined ( _MSC_VER  ) && ! defined ( __GNUC__ )
#define BTA_LWIP
#endif



#include "lwip/sockets.h"

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
#include "assert.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// #include <winsock2.h>
#include "net.h"
#endif

#include "CEDebug.h"
#include "btaDebug.h"
#include "emm.h"
#include "ese.h"
#include "bacstr.h"
#include "bacdcode.h"
#include "bacaddr.h"

#if 0
#include "bacdef.h"
#include <memory.h>
#endif

#ifdef _MSC_VER
#include "bip.h"
#endif

#if 0
#include "CEDebug.h"
#endif

#if 0
extern char firstEthernet[20] ;
#endif

#if 0
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


// BTA reporting packet frame structure can be found here: https://goo.gl/JixmbT


// Allowing VisualExpert to switch BTA on/off
#ifndef _MSC_VER
extern SYS_NV*            gSYS_nv;
#endif

bool BTAready()
{
#ifndef _MSC_VER
  if ( ! gSYS_ram->EthernetLink )
  {
    return false ;
  }

  if ( ! gSYS_nv->BACnetBTA_Enable )
  {
  return false ;
  }
#endif

  return true ;
}


// this function does nothing but send the payload and frees allocated
// memory - no formatting at all
void SendBTApayload(uint8_t *payload, int nsendlength)
{
  if ( ! BTAready() )
  {
    emm_free ( payload ) ;
    return ;
  }

#if ( BTA_LOGGING == 1 )
    struct sockaddr_in servaddr;
    int sockfd = -1 ;

#ifdef BTA_LWIP
    int timeoutTimeInMilliSeconds = 4000 ;
    struct sockaddr_in self;
#endif

#if defined ( __GNUC__ )
    struct ifreq ifr;
#endif

    int status;

#if defined ( _MSC_VER  )
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
#if defined ( BTA_LWIP )
        sockfd = lwip_socket(PF_INET, SOCK_DGRAM, 0);
#else
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
        if (sockfd < 0) {
            // todo 2 perror("m0964");
          emm_free ( payload ) ;
          return;
        }
    }

#if defined ( __GNUC__todo1 )
    int sockopt;

    memset(&ifr, 0, sizeof(ifr));

    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), firstEthernet);		// todonext - make same as non-lon port for diagnostics

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        printf("Failed to bind to [%s] - did you run sudo?\n", ifr.ifr_name);
        perror ("m0975");
        emm_free ( payload ) ;
        return;
    }

    status = ioctl(sockfd, SIOCGIFBRDADDR, &ifr);
    if (status < 0) {
        printf("Failed to read broadcast address for %s, status=%d\n", ifr.ifr_name, status);
        perror ( "m0091") ;
        emm_free ( payload ) ;
        return;
    }
    memcpy(&bcastAddress, &ifr.ifr_ifru.ifru_broadaddr, sizeof(bcastAddress));

    status = ioctl(sockfd, SIOCGIFADDR, &ifr);
    if (status < 0) {
        printf("Failed to read local address for %s, status=%d\n", ifr.ifr_name, status);
        perror ( "m1052 - btaDebug") ;
        emm_free ( payload ) ;
        return;
    }
    memcpy(&ourAddress, &ifr.ifr_ifru.ifru_addr, sizeof(ourAddress));
    // hexdump("Loc Addr", &ourAddress, sizeof(ourAddress));

    ////hexdump(ifr.ifr_ifrn.ifrn_name, &((struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr)->sin_addr, 6);
    //   //hexdump(ifr.ifr_ifrn.ifrn_name, (struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr, 8);
#endif

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
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
#if defined ( _MSC_VER  )
    status = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char *)&sockopt, sizeof(sockopt));

#elif defined ( BTA_LWIP )
    if(lwip_setsockopt( sockfd,
                    SOL_SOCKET,
                    SO_RCVTIMEO,
                    &timeoutTimeInMilliSeconds,
                    sizeof(int)) == -1)
        {
        // return ;
        }
#if 0
    if(lwip_setsockopt( sockfd,
                    SOL_SOCKET,
                    SO_BROADCAST,
                    &sockopt,
                    sizeof(int)) == -1)
        {
        // return ;
        }
#endif

#else
    status = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof(sockopt));
    if (status < 0) {
        closesocket(sockfd);
        sockfd = -1;
        //printf("m00981: Failed to set broadcast %d\n", status);
        perror ("m0991");
        emm_free ( payload ) ;
        return;
    }

#endif


    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    servaddr.sin_port = htons(41797);

#ifdef BTA_LWIP
  memset(&self, 0, sizeof(self));
  self.sin_family = AF_INET;
  self.sin_port = htons(0);
  self.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to port
  status = bind(sockfd, (struct sockaddr*)&self, sizeof(self)) ;
  if ( status != 0 )
  {
     status = errno ;
     //printf("socket--bind error");
     close(sockfd);
     emm_free ( payload ) ;
     return ;
  }
#endif

#if defined ( __GNUC__todo  )
    // memset(&servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));
    // bind the socket for receiving messages.. todonext..
    //status = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    //if (status < 0)
    //{
    //    printf("m0010 - Failed to bind\n");
    //    close(sockfd);
    //}
#endif

#if defined ( _MSC_VER ) || defined ( BTA_LWIP )
    // to avoid setting sockopt bcast
    servaddr.sin_addr.s_addr = inet_addr("255.255.255.255");
#else
    me mcpy ( &servaddr.sin_addr.s_addr, &bcastAddress.sin_addr.s_addr, sizeof ( servaddr.sin_addr.s_addr ) ) ;
#endif

#if defined ( _MSC_VER )
    int len = sendto(sockfd, (const char *)payload, nsendlength, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
#else
    int len = sendto(sockfd, payload, nsendlength, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
#endif
    
    if ( len <= 0 )
    {
      ese_enqueue_once(ese036_24_BTA_error) ;
    }

    // todonext, when do we get to close this socket under normal conditions....
    closesocket(sockfd);

#endif  // BTA_LOGGING == 1

    emm_free ( payload ) ;
}

void SendBTAmstpFrame( const uint8_t portId, const bool tx, const uint8_t *frame, uint16_t data_len )
{
  // do this check early to avoid unnecessary CPU
  if ( ! BTAready() ) return ;

  if ( data_len > 512 )
  {
    ese_enqueue(ese036_24_BTA_error);
    return ;
  }

  // we are not going to send the 55, ff,
  data_len -= 2 ;

  u8_t *outbuf = (u8_t *)emm_smalloc('q', MX_BTA_BUFFER);
  if ( outbuf == NULL ) return ;

  PrepareBTAheader(BTAmsgType_MSTPframe, outbuf);
  outbuf[16] = tx ;
  outbuf[17] = portId;
  int datalen = MIN( data_len, MX_BTA_BUFFER-BX_DATAOFFSET) ;
  memcpy((uint8_t *)&outbuf[BX_DATAOFFSET], &frame[2], datalen );
  SendBTApayload(  outbuf, datalen+BX_DATAOFFSET );
}





#if ! defined ( _MSC_VER  ) && ! defined ( __GNUC__ )

extern SYS_RAM*    gSYS_ram;

// this is not required for BTA debugging, just a heartbeat app. Can remove as soon as appropriate.


#endif // _MSC_VER

