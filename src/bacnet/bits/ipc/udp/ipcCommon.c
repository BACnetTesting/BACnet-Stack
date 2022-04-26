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

//#include <malloc.h>
//#include <stdio.h>
//#include <stdbool.h>
//#include <string.h>
//
//#ifdef _MSC_VER
//#include <WinSock2.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "IPHLPAPI.lib")
//
//#include <Mstcpip.h>
//#else
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <pthread.h>
//#endif
//
#include "net.h"
#include "ipcCommon.h"
#include "bitsUtil.h"
#include "logging.h"
#include "bitsDebug.h"
#include "osLayer.h"

// already defined in net.h #define closesocket close

#ifdef _MSC_VER
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
SOCKET ipcListenSocket;
#else
int ipcListenSocket ;
#endif

typedef uint8_t byte ;

bool showIPChandshake;
static uint peerPortLocal ;

void ipc_encode_uint8(IPC_PKT *pkt, const byte val)
{
    if (pkt->len > MX_BUFFER - 2)
    {
        panic();
        return;
    }
    pkt->buffer[pkt->len++] = val;
}

void ipc_encode_boolean(IPC_PKT *pkt, const bool val)
{
    ipc_encode_uint8(pkt, (byte) val );
}

void ipc_encode_string(IPC_PKT *pkt, const char *string)
{
    // todo2 - make this a) safe, b) bacnet compliant
    ipc_encode_uint8(pkt, (uint8_t)(strlen(string)+1));
    for (uint i = 0; i <= strlen(string); i++) {
        ipc_encode_uint8(pkt, string[i]);
    }
}

void ipc_encode_uint16(IPC_PKT *pkt, const uint val)
{
#ifndef _MSC_VER
    // todo2 - endian issues on other platforms?
// todo #error
#endif
    ipc_encode_uint8(pkt, ((byte *)&val)[1]);
    ipc_encode_uint8(pkt, ((byte *)&val)[0]);
}

void ipc_encode_uint32(IPC_PKT *pkt, const uint val)
{
    ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    ipc_encode_uint8(pkt, ((byte *)&val)[2]);
    ipc_encode_uint8(pkt, ((byte *)&val)[1]);
    ipc_encode_uint8(pkt, ((byte *)&val)[0]);
}

void ipc_encode_float(IPC_PKT *pkt, const float val)
{
    ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    ipc_encode_uint8(pkt, ((byte *)&val)[2]);
    ipc_encode_uint8(pkt, ((byte *)&val)[1]);
    ipc_encode_uint8(pkt, ((byte *)&val)[0]);
}

uint ipc_decode_uint16(byte *buffer, uint *iptr )
{
    uint val = 0;
    ((byte *)&val)[1] = buffer[(*iptr)++];
    ((byte *)&val)[0] = buffer[(*iptr)++];
    return val;
}

uint ipc_decode_uint32(byte *buffer, uint *iptr)
{
    uint val = 0;
    ((byte *)&val)[3] = buffer[(*iptr)++];
    ((byte *)&val)[2] = buffer[(*iptr)++];
    ((byte *)&val)[1] = buffer[(*iptr)++];
    ((byte *)&val)[0] = buffer[(*iptr)++];
    return val;
}

void ipc_encode_int32(IPC_PKT *pkt, const int val)
{
    ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    ipc_encode_uint8(pkt, ((byte *)&val)[2]);
    ipc_encode_uint8(pkt, ((byte *)&val)[1]);
    ipc_encode_uint8(pkt, ((byte *)&val)[0]);
}

int ipc_decode_int32(byte *buffer, uint *iptr)
{
    int val = 0;
    ((byte *)&val)[3] = buffer[(*iptr)++];
    ((byte *)&val)[2] = buffer[(*iptr)++];
    ((byte *)&val)[1] = buffer[(*iptr)++];
    ((byte *)&val)[0] = buffer[(*iptr)++];
    return val;
}

float ipc_decode_float(byte *buffer, uint *iptr)
{
    float val = 0.0f;
    ((byte *)&val)[3] = buffer[(*iptr)++];
    ((byte *)&val)[2] = buffer[(*iptr)++];
    ((byte *)&val)[1] = buffer[(*iptr)++];
    ((byte *)&val)[0] = buffer[(*iptr)++];
    return val;
}

void ipc_decode_string(uint8_t *buffer, uint *iptr, char *target, const uint bufSize)
{
    uint len = buffer[(*iptr)++];
    if (len > bufSize) {
        panic();
        target[0] = '\0';
        return;
    }
    memcpy(target, &buffer[*iptr], len);
    *iptr += len ;
}

bool ipc_decode_boolean(byte *buffer, uint *iptr)
{
    return (buffer[(*iptr)++] > 0) ? true : false;
}


IPC_PKT *ipc_create_packet(IPC_MSG_TYPE cmd)
{
    IPC_PKT *pkt = (IPC_PKT *)malloc(sizeof(IPC_PKT));
    if (pkt == NULL)
    {
        panic();
        return NULL;
    }
    memset(pkt, 0, sizeof(IPC_PKT));

    ipc_encode_uint16(pkt, (uint)cmd);
    pkt->len += 2;  // make space for length
    return pkt;
}

void ipc_send_packet(IPC_PKT *pkt)
{
	if ( ipcListenSocket < 0 )
	{
		panic();
		return;
	}

    // fill in the length 
    uint len = pkt->len;
    pkt->len = 2;
    ipc_encode_uint16(pkt, len);

    struct sockaddr_in dest;

    memset(&dest, 0, sizeof(struct sockaddr_in));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dest.sin_port = htons((u_short)peerPortLocal);

    int rc = sendto(ipcListenSocket, (char *)pkt->buffer, len, 0, (struct sockaddr *) &dest, sizeof(struct sockaddr_in));
    if (rc <= 0)
    {
        //        int err = WSAGetLastError();
        panic();
    }

}

void ipc_send_command(IPC_MSG_TYPE cmd)
{
    IPC_PKT *pkt = ipc_create_packet(cmd);
    if (pkt == NULL)
    {
        panic();
        return;
    }
    ipc_send_packet(pkt);
}


void ThreadMaint(void *lpParam)
{
    (void)lpParam;
    while (true)
    {
        msSleep(1500);

        if (showIPChandshake) log_printf("Sending ping");
        ipc_send_command(IPC_COMMAND_PING);
    }
}


bool ipc_init_common(uint myPort, uint peerPort, void (*ThreadListenFunc)(void *))
{
    peerPortLocal = peerPort;

#ifdef _MSC_VER
    int threadData = 0;
    WSADATA wd;

    int Result = WSAStartup((1 << 8) | 1, &wd);
    if (Result != 0)
    {
        panic();
        return false;
    }
#endif

    ipcListenSocket = socket(AF_INET,  SOCK_DGRAM, IPPROTO_UDP);
    if (ipcListenSocket < 0)
    {
        panic();
        return false;
    }

    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    int value = 1;
    int rv = setsockopt(ipcListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&value,
        sizeof(value));
    if (rv < 0) {
        panic();
        closesocket(ipcListenSocket);
        return false;
    }

#ifdef _MSC_VER
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(ipcListenSocket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
#endif

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_port = htons((u_short)myPort);
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));

    rv = bind(ipcListenSocket, (const struct sockaddr *) &sin,
        sizeof(struct sockaddr_in));
    if (rv < 0) {
        panic();
        closesocket(ipcListenSocket);
        return false;
    }

#ifdef _MSC_VER
    //// set up a listener thread on the given port, and issues callback whenever a (relevant) packet is received..
    bitsCreateThread(ThreadListenFunc, NULL);
    //HANDLE threadHandle = CreateThread(NULL, 0, ThreadListenFunc, &threadData, 0, NULL);
    //if (threadHandle == NULL)
    //{
    //    panic();
    //    return false;
    //}

    bitsCreateThread(ThreadMaint, NULL);
    //threadHandle = CreateThread(NULL, 0, ThreadMaint, &threadData, 0, NULL);
    //if (threadHandle == NULL)
    //{
    //    panic();
    //    return false;
    //}
#else
	int rcode;
	pthread_t threadvar;
	rcode = pthread_create(&threadvar, NULL,  (void *(*)(void *)) ThreadListenFunc, NULL );
	if (rcode != 0) {
		log_printf("Failed to create thread");
	}
	// so we don't have to wait for the thread to complete before exiting main()
	pthread_detach(threadvar);

	rcode = pthread_create(&threadvar, NULL,  (void *(*)(void *)) ThreadMaint, NULL );
	if (rcode != 0) {
		log_printf("Failed to create thread");
	}
	// so we don't have to wait for the thread to complete before exiting main()
	pthread_detach(threadvar);

#endif


    return true;
}

