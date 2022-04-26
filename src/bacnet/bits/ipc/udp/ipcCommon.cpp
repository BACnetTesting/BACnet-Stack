/**************************************************************************
*
* Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
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


#include <malloc.h>
#include <stdio.h>
#include <string.h>
// #include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#include <Mstcpip.h>

#include "ipcCommon.h"

#ifdef GATEWAY
#include "logging.h"
#endif

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

SOCKET listenSocket;
bool showIPChandshake;
static uint peerPortLocal ;

namespace ipc_private
{
    //static void(*callbackStatusLocal) (uint device, bool val);
//static void(*callbackAnaUpdateLocal) (uint device, float val);
//static void(*callbackBinUpdateLocal) (uint device, bool val);
//
//bool ipcRestartRequested;

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

    void ipc_encode_uint16(IPC_PKT *pkt, const uint val)
    {
        ipc_encode_uint8(pkt, ((byte *)&val)[0]);
        ipc_encode_uint8(pkt, ((byte *)&val)[1]);
    }

    void ipc_encode_uint32(IPC_PKT *pkt, const uint val)
    {
        ipc_encode_uint8(pkt, ((byte *)&val)[0]);
        ipc_encode_uint8(pkt, ((byte *)&val)[1]);
        ipc_encode_uint8(pkt, ((byte *)&val)[2]);
        ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    }

    void ipc_encode_float(IPC_PKT *pkt, const float val)
    {
        ipc_encode_uint8(pkt, ((byte *)&val)[0]);
        ipc_encode_uint8(pkt, ((byte *)&val)[1]);
        ipc_encode_uint8(pkt, ((byte *)&val)[2]);
        ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    }

    uint ipc_decode_uint16(byte *buffer)
    {
        uint val = 0;
        ((byte *)&val)[0] = buffer[0];
        ((byte *)&val)[1] = buffer[1];
        return val;
    }

    uint ipc_decode_uint32(byte *buffer)
    {
        uint val = 0;
        ((byte *)&val)[0] = buffer[0];
        ((byte *)&val)[1] = buffer[1];
        ((byte *)&val)[2] = buffer[2];
        ((byte *)&val)[3] = buffer[3];
        return val;
    }

    void ipc_encode_int32(IPC_PKT *pkt, const int val)
    {
        ipc_encode_uint8(pkt, ((byte *)&val)[0]);
        ipc_encode_uint8(pkt, ((byte *)&val)[1]);
        ipc_encode_uint8(pkt, ((byte *)&val)[2]);
        ipc_encode_uint8(pkt, ((byte *)&val)[3]);
    }

    int ipc_decode_int32(byte *buffer)
    {
        int val = 0;
        ((byte *)&val)[0] = buffer[0];
        ((byte *)&val)[1] = buffer[1];
        ((byte *)&val)[2] = buffer[2];
        ((byte *)&val)[3] = buffer[3];
        return val;
    }

    float ipc_decode_float(byte *buffer)
    {
        float val = 0.0f;
        ((byte *)&val)[0] = buffer[0];
        ((byte *)&val)[1] = buffer[1];
        ((byte *)&val)[2] = buffer[2];
        ((byte *)&val)[3] = buffer[3];
        return val;
    }

    bool ipc_decode_boolean(byte *buffer)
    {
        return (*buffer > 0 ) ? true : false;
    }

    IPC_PKT *ipc_create_packet(IPC_COMMAND cmd)
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
        // fill in the length 
        uint len = pkt->len;
        pkt->len = 2;
        ipc_encode_uint16(pkt, len);

        struct sockaddr_in dest;

        memset(&dest, 0, sizeof(struct sockaddr_in));
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        dest.sin_port = htons((u_short)peerPortLocal);

        int rc = sendto(listenSocket, (char *)pkt->buffer, len, 0, (struct sockaddr *) &dest, sizeof(struct sockaddr_in));
        if (rc <= 0)
        {
            //        int err = WSAGetLastError();
            panic();
        }

    }

    void ipc_send_command(IPC_COMMAND cmd)
    {
        IPC_PKT *pkt = ipc_create_packet(cmd);
        if (pkt == NULL)
        {
            panic();
            return;
        }
        ipc_send_packet(pkt);
    }




    DWORD WINAPI ThreadMaint(LPVOID lpParam)
    {
        (void)lpParam;
        while (true)
        {
            Sleep(1500);

            if (showIPChandshake) log_printf("Sending ping");
            ipc_send_command(IPC_COMMAND_PING);
        }
        return 0;
    }



    bool ipc_init_common(uint myPort, uint peerPort, LPTHREAD_START_ROUTINE ThreadListenFunc )
    {
        int threadData = 0;
        WSADATA wd;

        //if (callbackAnaUpdate == NULL || callbackBinUpdate == NULL || callbackStatus == NULL)
        //{
        //    panic();
        //    return false;
        //}

        //callbackStatusLocal = callbackStatus;
        //callbackAnaUpdateLocal = callbackAnaUpdate;
        //callbackBinUpdateLocal = callbackBinUpdate;
        peerPortLocal = peerPort;

        int Result = WSAStartup((1 << 8) | 1, &wd);
        if (Result != 0)
        {
            panic();
            return false;
        }

        listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (listenSocket < 0)
        {
            panic();
            return false;
        }

        /* Allow us to use the same socket for sending and receiving */
        /* This makes sure that the src port is correct when sending */
        int value = 1;
        int rv = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&value,
            sizeof(value));
        if (rv < 0) {
            panic();
            closesocket(listenSocket);
            return false;
        }

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(listenSocket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sin.sin_port = htons((u_short)myPort);
        memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));

        rv = bind(listenSocket, (const struct sockaddr *) &sin,
            sizeof(struct sockaddr_in));
        if (rv < 0) {
            panic();
            closesocket(listenSocket);
            return false;
        }

        // set up a listener thread on the given port, and issues callback whenever a (relevant) packet is received..
        HANDLE threadHandle = CreateThread(NULL, 0, ThreadListenFunc, &threadData, 0, NULL);
        if (threadHandle == NULL)
        {
            panic();
            return false;
        }

        threadHandle = CreateThread(NULL, 0, ThreadMaint, &threadData, 0, NULL);
        if (threadHandle == NULL)
        {
            panic();
            return false;
        }

        return true;
    }

}

