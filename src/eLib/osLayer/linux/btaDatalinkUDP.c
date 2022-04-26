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

#include <stdint.h>
#include "eLib/util/emm.h"
#include "bacnet/bits/util/bitsUtil.h"
#include "eLib/util/btaDebug.h"

#include "eLib/util/eLibDebug.h"
#include "osNet.h"
// #include "eLib/util/emmIpUtil.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/bacint.h"
#include "eLib/util/llist.h"
#include "eLib/util/btaInternal.h"

#define BTAtraffic2 41797

uint16_t btaPorts[] = {
        BTAtraffic2,
        60881,       // 0xEDD1
        61077,
        47824,       // 0xBAD0
        33330,       // Added a few more... can easily see 2, 3 apps running on ome machine, plus a few spare for other collisions
        44440,
        55550,
        61111
};

static uint16_t MyPortsTraffic;

static uint32_t nwoDefaultBTAclientAddr ;
static uint16_t nwoDefaultBTAclientPort ;

// static bool btaReady;
static int sock_fd;
static struct sockaddr_in btaClientIPEP;

struct sockaddr_in btaLocalIPEP;

static LLIST_HDR    outgoingBTAqueue;
static time_t       lastHeard;

bits_mutex_static(btaMutex);

// static bool BTA_Ready(void);

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
            dbMessage( DBD_Config, DB_UNUSUAL_TRAFFIC, "BTA listen port is %d (socket:%d)\n", btaPorts[i], sock_fd);
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
    socklen_t sinlen)
{
    uint16_t functionCode;
    uint8_t rlen = decode_unsigned16(buffer, &functionCode);
    if (rlen == 0) {
        panic();
        return;
    }

    switch (functionCode) {

    case ClientAlive:
        // we should check if this is a known client, and if new, add to the list... and remove expired ones...
        if (!isEqualIPEP(sinIPEP, &btaClientIPEP)) {
            // and now we have a port to send to!
            btaClientIPEP = *sinIPEP;

            char tbuf[100];
            // printf("received from %s\r\n", IPEP_ToString(tbuf, sinIPEP) );
            // {
            // Do a connect operation to categorically establish which of our perhaps multiple adapters (and port) we are chatting on
            socklen_t namelen = sizeof(btaLocalIPEP);
            connect(sock_fd, (struct sockaddr *) sinIPEP, sizeof(struct sockaddr_in));
            int err = getsockname(sock_fd, (struct sockaddr *) &btaLocalIPEP, &namelen);
            if (err) {
                dbMessage(DBD_UI, DB_ERROR, "getsockname error");
            }
            else {
                dbMessage(DBD_UI, DB_ALWAYS, "BTA connection on %s:%d", IPAddr_ToString(tbuf, &btaLocalIPEP.sin_addr), ntohs(btaLocalIPEP.sin_port));
                // clientIsKnown = true;
            }

            // disconnect, or we will never hear again...
            // Connectionless sockets may dissolve the association by connecting to an address with the sa_family member of sockaddr set to AF_UNSPEC 
            struct sockaddr_in dConnect = { 0 } ;   // initializing, mainly so valgrind does not whine.
            dConnect.sin_family = AF_UNSPEC;
            connect(sock_fd, (struct sockaddr *) &dConnect, sizeof(struct sockaddr_in));
            
            // and update our default BTA address
            nwoDefaultBTAclientAddr = sinIPEP->sin_addr.s_addr;
            nwoDefaultBTAclientPort = sinIPEP->sin_port;
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
        break;
    }
}


// this function does nothing but send the payload and frees allocated
// memory - no formatting at all
static void SendBTApayloadUDPnow(uint8_t *payload, const int sendlength)
{
    if(nwoDefaultBTAclientAddr)
    {
        // we have a destination, even if it is a broadcast
    
        struct sockaddr_in servaddr;

        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = nwoDefaultBTAclientAddr;
        servaddr.sin_port = nwoDefaultBTAclientPort;

        sendto(sock_fd,
            (const char *)payload,
            sendlength,
            0,
            (struct sockaddr *)&servaddr,
            sizeof(servaddr));
    }

    emm_free(payload);
}


static time_t   lastSent;

void BTAflushOutgoing(void)
{
    // BTA_Ready() will give us one chance on startup to send here-i-am message
    if (!BTA_Ready()) return;

    // for UDP BTA, we can _always_ clear the queue before doing anything else.
    // is there an outgoing packet to send?

    // ll_Count() removed to eliminate race condition when multithreading
    BTAQ_CB* qcb = (BTAQ_CB*)ll_Dequeue(&outgoingBTAqueue);
    while ( qcb != NULL ) {
        SendBTApayloadUDPnow(qcb->payload, qcb->sendlength);
        emm_free(qcb);
        time(&lastSent);
        qcb = (BTAQ_CB*)ll_Dequeue(&outgoingBTAqueue);
    }
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

    BTAflushOutgoing();
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
    int datalen = BITS_MIN(strlen(message), MX_BTA_BUFFER - BX_DATAOFFSET - 1);
    strncpy((char *)&outbuf[BX_DATAOFFSET], message, datalen);
    SendBTApayloadUDPnow(outbuf, datalen + BX_DATAOFFSET);
#endif
    
}


static void BTAlistenThread(void *params)
{
    while (true) {

        while (true) {
            
            BTAflushOutgoing();

            if (BTA_Ready()) {
                time_t tt = time(NULL);
                if (time(NULL) - lastSent > 60) {
                    // just say 'Hi'
                    BTAsendHeartbeat();
                    lastSent = time(NULL);
                }
            }

            // is there an incoming packet ?
            fd_set read_fds;
            int max;
            struct timeval select_timeout;

            // make it delay _some_ so we don't hose the cpu
            select_timeout.tv_sec = 0;
            select_timeout.tv_usec = 5000;

            FD_ZERO(&read_fds);
            FD_SET(sock_fd, &read_fds);
            max = sock_fd;

            if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {

                uint8_t pdu[100];
                struct sockaddr_in sin;

                socklen_t sin_len = sizeof(sin);

                int received_bytes = recvfrom(sock_fd,
                    (char *)&pdu[0],
                    sizeof(pdu),
                    0,
                    (struct sockaddr *)&sin,
                    &sin_len);

                // the above blocks, (normally)
                if (received_bytes <= 0) {
                    // something went very wrong....
                }
                else {
                    ProcessBTAclientMessage(pdu, received_bytes, &sin, sin_len);
                }
            }

            // we throttle using the select (500us)
            msSleep(10);
        }
        // restarting.. code something here
        panic();
        msSleep(10000);
    }
}




static bool initAlreadyAttempted;
static bool initFailed;
static bool firstTimeFreePassUsed;
static bool reinitcheck;

// diff between BTAinit and BTA_Ready - we can be initialized, but not heard from BTA client, so we are not 'ready'

static bool BTAinitUDP(void)
{
    if (reinitcheck) {
        panic();
        return false ;
    }
    reinitcheck = true;

    ll_Init(&outgoingBTAqueue, 10);

    nwoDefaultBTAclientAddr = htonl(INADDR_BROADCAST);
    nwoDefaultBTAclientPort = htons(BTAtraffic2);

    //     int sockopt = 1; // yes, we need it to be set

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("m0964 - could not establish BTA socket");
        return false;
    }

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

    int sockopt = 1;

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
    bits_mutex_lock(btaMutex);

    if (!initAlreadyAttempted) {
        initFailed = !BTAinitUDP();
        initAlreadyAttempted = true;
    }
    if (initFailed) {
        bits_mutex_unlock(btaMutex);
        return false;
    }

    // On startup, before BTA has a chance to connect, we allow ONE message to go out/be queued
    if (!firstTimeFreePassUsed) {
        firstTimeFreePassUsed = true;
        bits_mutex_unlock(btaMutex);
        return true;
    }


    // OK, from this point on, we are only 'ready' if we have actually heard from a BTA client in the last 10 seconds

    if (btaClientIPEP.sin_addr.s_addr == 0) {
        bits_mutex_unlock(btaMutex);
        return false;
    }

    time_t timenow;
    time(&timenow);

    if ((timenow - lastHeard) > 60 * 5) {
        bits_mutex_unlock(btaMutex);
        return false;
    }

    bits_mutex_unlock(btaMutex);
    return true;
}

