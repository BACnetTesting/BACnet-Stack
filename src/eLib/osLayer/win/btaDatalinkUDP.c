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
#include <winsock2.h>

#include <stdint.h>
#include "eLib/util/emm.h"
#include "eLib/util/eLibUtil.h"
#include "eLib/util/btaDebug.h"

#include "eLib/util/eLibUtil.h"
#include "eLib/util/eLibDebug.h"
#include "osNet.h"
#include "bacnet/bits/util/bitsIpUtil.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/bacint.h"
#include "eLib/util/llist.h"
#include "eLib/util/btaInternal.h"
#include "eLib/util/eLibUtil.h"

#define BTAtraffic2 41797

extern bool bacnetShuttingDown;

static uint16_t btaPorts[] = {
#ifndef _MSC_VER
        // on a windows machine, leave this port clear so BTA can use it
        BTAtraffic2,
#endif
        60881,       // 0xEDD1
        61077,
        47824,       // 0xBAD0
        33330,       // Added a few more... can easily see 2, 3 apps running on ome machine, plus a few spare for other collisions
        44440,
        55550,
        61111
};

static uint16_t MyPortsTraffic;

// config file may have a say in this... so we can't make static.
// Note: MSVC does not allow us to initialize with htons(), so don't try...
uint32_t nwoBTAclientAddr ;
uint16_t nwoBTAclientPort ;

// static bool btaReady;
static SOCKET sock_fd;
static struct sockaddr_in btaClientIPEP;

struct sockaddr_in btaLocalIPEP;

static LLIST_HDR    outgoingBTAqueue;
static time_t       lastHeard;


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
            printf("   BTA listen port is %d\n", btaPorts[i]);
            MyPortsTraffic = btaPorts[i];
            return true;
        }
    }
    panic();
    return false;
}


static void ProcessBTAclientMessage(uint8_t *buffer,
    int len,
    struct sockaddr_in *sinIPEP,
    int sinlen)
{
    uint16_t functionCode;
    uint8_t rlen = decode_unsigned16(buffer, &functionCode);
    if (rlen == 0) {
        panic();
        return;
    }

    switch (functionCode) {

    case ClientAlive:
        // todo2 - we should check if this is a known client, and if new, add to the list... and remove expired ones...
        if (!isEqualIPEP(sinIPEP, &btaClientIPEP)) {
            // and now we have a port to send to!
            btaClientIPEP = *sinIPEP;

            char tbuf[100];
            // Do a connect operation to categorically establish which of our perhaps multiple adapters (and port) we are chatting on
            socklen_t namelen = sizeof(btaLocalIPEP);
            connect(sock_fd, (struct sockaddr *) sinIPEP, sizeof(struct sockaddr_in));
            int err = getsockname(sock_fd, (struct sockaddr *) &btaLocalIPEP, &namelen);
            if (err) {
                printf("getsockname error\r\n");
            }
            else {
                printf("   local = %s:%d\n\r", IPAddr_ToString(tbuf, &btaLocalIPEP.sin_addr), ntohs(btaLocalIPEP.sin_port));
                // clientIsKnown = true;
            }

            // disconnect, or we will never hear again...
            // Connectionless sockets may dissolve the association by connecting to an address with the sa_family member of sockaddr set to AF_UNSPEC 
            struct sockaddr_in dConnect;
            dConnect.sin_family = AF_UNSPEC;
            connect(sock_fd, (struct sockaddr *) &dConnect, sizeof(struct sockaddr_in));
            
            // and update our default BTA address
            nwoBTAclientAddr = sinIPEP->sin_addr.s_addr;
            nwoBTAclientPort = sinIPEP->sin_port;
        }

        // make a connection to establish local IP address..
        // int tsocket = socket(AF_INET, SOCK_DGRAM, 0);
        // connect(tsocket, (struct sockaddr *) sinIPEP, sizeof(struct sockaddr_in));
        // }
        // SendBTAmessage("got a sinlen");
        time(&lastHeard);
        break;

    case BTApt_BACstd:
        // ignore other whining nodes
        break;

    default:
        // well, there is other traffic out there..
        break;
    }
}


// this function does nothing but send the payload and frees allocated
// memory - no formatting at all
void SendBTApayloadUDPnow(uint8_t *payload, const int sendlength)
{
    if(nwoBTAclientAddr)     {
        // we have a destination, even if it is a broadcast
    
        struct sockaddr_in servaddr;

        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = nwoBTAclientAddr;
        servaddr.sin_port = nwoBTAclientPort;

        sendto(sock_fd,
            (const char *)payload,
            sendlength,
            0,
            (struct sockaddr *)&servaddr,
            sizeof(servaddr));

        // todonext, when do we get to close this socket under normal conditions....
        // close(sockfd);
    }

    emm_free(payload);
}


void SendBTApayloadUDP(BTAQ_CB *btaq_cb)
{
    if (!BTA_Ready()) {
        emm_free(btaq_cb->payload);
        emm_free(btaq_cb);
        return;
    }

    // we _expect_ to overrun the queue if there is no active listener....
    if (!ll_Enqueue(&outgoingBTAqueue, btaq_cb)) {
        emm_free(btaq_cb->payload);
        emm_free(btaq_cb);
    }
}



//static bool btaReady;
//
//static bool FindBTAport(void)
//{
//    for (int i = 0; i < sizeof(btaPorts) / sizeof(uint16_t); i++) {
//        // bind to our port
//        struct sockaddr_in sin;
//        sin.sin_family = AF_INET;
//        sin.sin_addr.s_addr = htonl(INADDR_ANY);
//        sin.sin_port = htons(btaPorts[i]);
//        memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
//        int status = bind(sock_fd, (const struct sockaddr *)&sin, sizeof(struct sockaddr));
//        if (status == 0) {
//            printf("BTA listen port is %d\n", btaPorts[i]);
//            MyPortsTraffic = btaPorts[i];
//            return true;
//        }
//    }
//    panic();
//    return false;
//}

static void BTAsendHeartbeat(void)
{
    uint8_t *outbuf = (uint8_t *)emm_scalloc('b', BX_DATAOFFSET + 50 /*see message below*/);
    if (outbuf == NULL) return;

    PrepareBTAheader(SimplestPing, outbuf);

    // should we load a payload (or not)
#if 0
    SendBTApayloadUDPnow(outbuf, BX_DATAOFFSET);
#else
    const char *message = "Easy to see in wireshark heartbeat message";
    int datalen = BITS_MIN(bits_strlen(message), MX_BTA_BUFFER - BX_DATAOFFSET - 1);
    strncpy((char *)&outbuf[BX_DATAOFFSET], message, datalen);
    SendBTApayloadUDPnow(outbuf, datalen + BX_DATAOFFSET);
#endif
    
}


// Ticks the datalink, needs to be very aware that multiple threads may be calling on it...
// ( Main application thread for small devices, dedicated BTA thread for linux / win...

void BTA_DatalinkTick( uint usTimeout )
{
    static time_t   lastSent;

    // is there an outgoing packet to send?
    if (ll_GetCount(&outgoingBTAqueue)) {
        BTAQ_CB *qcb = (BTAQ_CB *)ll_Dequeue(&outgoingBTAqueue);
        SendBTApayloadUDPnow(qcb->payload, qcb->sendlength);
        emm_free(qcb);

        time(&lastSent);
    }
    else {
        if (BTA_Ready()) {
            time_t tt = time(NULL);
            if (time(NULL) - lastSent > 60) {
                // just say 'Hi'
                BTAsendHeartbeat();
                lastSent = time(NULL);
            }
        }
    }

    // is there an incoming packet ?
    fd_set read_fds;
    int max;
    struct timeval select_timeout;

    // make it delay _some_ so we don't hose the cpu
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = usTimeout ;

    FD_ZERO(&read_fds);
    FD_SET(sock_fd, &read_fds);
    max = (int) sock_fd;

    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {

        uint8_t pdu[100];
        struct sockaddr_in sin;

        int sin_len = sizeof(sin);

        int received_bytes = recvfrom(sock_fd,
            (char *)&pdu[0],
            sizeof(pdu),
            0,
            (struct sockaddr *)&sin,
            &sin_len);

        // the above blocks, (normally)
        if (received_bytes <= 0) {
            // something went very wrong....
            int Code = WSAGetLastError();
            if (Code != 10040) {
                printf("BTA receive error code: %i %s\n",
                    Code,
                    winsock_error_code_text(Code));
                return ;
            }
        }
        else {
            ProcessBTAclientMessage(pdu, received_bytes, &sin, sin_len);
        }
    }

    // we throttle using the select (500us)
}
// restarting.. code something here


static void BTAlistenThread(void *params)
{
    while ( !bacnetShuttingDown ) {
        while (! bacnetShuttingDown) {
            BTA_DatalinkTick(1000);
        }
            // we throttle using the select (500us)
    }
    // restarting.. code something here
    if (bacnetShuttingDown) return;
    panicDesc("xxx");
    msSleep(10000);
}


static bool initAlreadyAttempted;
static bool initFailed;
static bool firstTimeFreePassUsed;

// diff between BTAinit and BTA_Ready - we can be initialized, but not heard from BTA client, so we are not 'ready'

static bool BTAinitUDP(void)
{
    WSADATA wd;
    int Result = WSAStartup((1 << 8) | 1, &wd);
    if (Result != 0) {
        // we are going to be compeletely useless, so terminate
        int Code = WSAGetLastError();
        // cant use BTA here, we have not been able to init it!
        printf("WSA library did not initialize\n");
        return false;
    }

    ll_Init(&outgoingBTAqueue, 100 );

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("m0964");
        return false;
    }

    char sockopt = 1;

    /* allow us to send a broadcast */
    setsockopt(sock_fd,
        SOL_SOCKET,
        SO_BROADCAST,
        (const char *)&sockopt,
        sizeof(sockopt));

    if (!FindBTAport()) {
        closesocket(sock_fd);
        return false;
    }

    bitsCreateThread(BTAlistenThread, NULL);
    initAlreadyAttempted = true;

    return true;
}


// i.e. are we initilized, and have we heard from BTA client (recently)
bool BTA_Ready(void)
{
    if (!initAlreadyAttempted) {
        initFailed = !BTAinitUDP();
        initAlreadyAttempted = true;
    }

    if (initFailed) {
        return false;
    }

    if (!firstTimeFreePassUsed) {
        firstTimeFreePassUsed = true;
        return true;
    }


    // OK, from this point on, we are only 'ready' if we have actually heard from a BTA client in the last 10 seconds

    if(btaClientIPEP.sin_addr.s_addr == 0) {
        return false;
        }

    time_t timenow;
    time(&timenow);

    static bool onlineOffline ;
    if ((timenow - lastHeard) > 60 * 5)
    	{
    	if ( onlineOffline )
    	{
    		printf("   BTA going offline\n");
    	}
        onlineOffline = false ;
    	return false;
    	}

    onlineOffline = true ;
    return true;
}

