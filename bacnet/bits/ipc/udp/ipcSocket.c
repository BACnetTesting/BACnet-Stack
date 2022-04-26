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


// UDP based IPC layer for controlling BACnet Stack.
// On linux, uses "Unix Domain Sockets", on Windows, just normal Datagram Sockets...

#include <stdio.h>
#include <winsock2.h>

#include "ipcApplication.h"
#include "bitsDebug.h"

#pragma comment(lib, "Ws2_32.lib")

bool timeToTerminate;

static void threadFuncIPCsocket(void *param)
{
    static WSADATA wsaData;
    static uint8_t inBuf[MAX_IPC_PDU];
    static uint8_t outBuf[MAX_IPC_PDU];
    static UINT optr ;
    static SOCKET sock_fd;
    static struct sockaddr_in	ipcSocketIPEP;

    //-----------------------------------------------
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        // dbTraffic(DB_UNEXPECTED_ERROR, "WSAStartup failed with error %d", iResult);
        panic();
        return;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        // dbTraffic(DB_UNEXPECTED_ERROR, "Failed to open socket");
        panic();
        return;
    }

    // bind to our port

    int sin_len = sizeof(ipcSocketIPEP);

    ipcSocketIPEP.sin_family = AF_INET;
    ipcSocketIPEP.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ipcSocketIPEP.sin_port = htons( 55555 );
    memset(&(ipcSocketIPEP.sin_zero), '\0', sizeof(ipcSocketIPEP.sin_zero));
    int rv = bind(sock_fd, (const struct sockaddr *) &ipcSocketIPEP, sizeof(struct sockaddr));
    if (rv < 0) {
        panic();
        closesocket(sock_fd);
        return;
    }


    while (!timeToTerminate) {
        optr = 0;
        uint16_t level;
        int64_t id;
        int32_t sched;
        float fval;

        int received_bytes = recvfrom(sock_fd, (char *)inBuf, MAX_IPC_PDU, 0, (struct sockaddr *) &ipcSocketIPEP, &sin_len);

        // todo 2 - check len (2nd byte)
        if (received_bytes < 0) {
            panic();
        }

        if (received_bytes > 0) {
            printf("msg recd\n");
            // process packet
        }
    }
    closesocket(sock_fd);
}


void ipcSocketInit(void)
{
    int rc = _beginthread(threadFuncIPCsocket, 0, NULL);
    if (rc < 0) {
        panic();
        return ;
    }
}

