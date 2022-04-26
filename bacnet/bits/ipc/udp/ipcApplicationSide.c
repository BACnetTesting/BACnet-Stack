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
//#include <Windows.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "IPHLPAPI.lib")
//#endif
//
//#ifdef __GNUC__
//#include <sys/socket.h>
//#endif
//
#include "bitsDebug.h"
#include "net.h"
#include "logging.h"
#include "ipcApplicationSide.h"

#ifdef _MSC_VER
extern SOCKET ipcListenSocket;
#else
extern int ipcListenSocket;
#endif

typedef unsigned char byte;

extern bool showIPChandshake;

static void ThreadListenFuncApplicationSide(void *params)
{
	(void) params ;
    while (true)
    {
        struct sockaddr from;
        socklen_t       fromlen = sizeof ( struct sockaddr ) ;
        memset(&from, 0, sizeof(struct sockaddr));
        byte buffer[MX_BUFFER];
        int len = recvfrom(ipcListenSocket, (char *) buffer, MX_BUFFER, 0, &from, &fromlen );
        if (len <= 0)
        {
//            int err = WSAGetLastError();
            continue;
        }


        uint devInstance;
        uint objInstance;
        IPC_PROPERTY_ID propertyID;
        IPC_OBJECT_TYPE objType;
        float value;
        bool bvalue;
        uint iptr = 0;

		IPC_MSG_TYPE function = (IPC_MSG_TYPE)ipc_decode_uint16(buffer, &iptr);
		switch (function)
		{
		case IPC_COMMAND_PING:
			// simply send a pong
			if (showIPChandshake)log_printf("Ping received, sending pong");
			ipc_send_command(IPC_COMMAND_PONG);
			break;

		case IPC_COMMAND_PONG:
			// todo - reset a timer...
			if (showIPChandshake) log_printf("pong received");
			break;

        default:
            panic();
        }
    }
}

void ipcClient_update_object_analog(uint deviceInstance, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propertyId, float value)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_ANA_UPDATE);
    if (pkt == NULL)
    {
        panic();
        return;
    }
    ipc_encode_uint32(pkt, (uint)deviceInstance);
    ipc_encode_uint16(pkt, (uint)objType);
    ipc_encode_uint32(pkt, objInstance);
    ipc_encode_uint16(pkt, (uint)propertyId);
    ipc_encode_float(pkt, value );
    ipc_send_packet(pkt);
}

void ipcClient_update_object_binary(uint deviceInstance, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propertyId, int value)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_BIN_UPDATE);
    if (pkt == NULL)
    {
        panic();
        return;
    }
    ipc_encode_uint32(pkt, (uint)deviceInstance);
    ipc_encode_uint16(pkt, (uint)objType);
    ipc_encode_uint32(pkt, objInstance);
    ipc_encode_uint16(pkt, (uint)propertyId);
    ipc_encode_int32(pkt, value);
    ipc_send_packet(pkt);
}


void ipcClient_update_device_status(uint deviceInstance, bool stat)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_UPDATE_DEVICE_STATUS);
    if (pkt == NULL)
    {
        panic();
        return;
    }
    ipc_encode_uint32(pkt, (uint)deviceInstance);
    ipc_encode_boolean(pkt, stat);
    ipc_send_packet(pkt);
}



void ipcClient_send_restart(void)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_RESTART);
    if (pkt == NULL)
    {
        panic();
        return;
    }
    ipc_send_packet(pkt);
}


// Device Instance, Object Instance, New value
void ipcClient_update_analog_input_pv(uint deviceInstance, uint objectInstance, float value)
{
    ipcClient_update_object_analog(deviceInstance, OBJ_TYPE_AI, objectInstance, PROP_PV, value);
}


void ipcClient_update_binary_input_pv(uint deviceInstance, uint objectInstance, int value)
{
	ipcClient_update_object_binary(deviceInstance, OBJ_TYPE_BI, objectInstance, PROP_PV, value);
}


bool ipcClient_init_Application(void)
{
	return ipc_init_common( IPC_APPLICATION_LISTEN_PORT, IPC_BACNET_LISTEN_PORT, ThreadListenFuncApplicationSide);
}


void ipcClient_create_object( const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *objName, const char *objDescription)
{ 
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_CREATE_OBJECT);
    if (pkt == NULL) {
        panic();
        return;
    }

    ipc_encode_uint32(pkt, devInstance);
    ipc_encode_uint16(pkt, objType);
    ipc_encode_uint32(pkt, objInstance);
    ipc_encode_string(pkt, objName);
    ipc_encode_string(pkt, objDescription);

    ipc_send_packet(pkt);
}


void ipcClient_create_device_virtual(const uint portId, const uint devInstance, const char *devName, const char *devDescription)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_CREATE_DEVICE_VIRTUAL);
    if (pkt == NULL) {
        panic();
        return;
    }

    ipc_encode_uint16(pkt, portId);
    ipc_encode_uint32(pkt, devInstance);
    ipc_encode_string(pkt, devName);
    ipc_encode_string(pkt, devDescription);

    ipc_send_packet(pkt);
}


void ipcClient_create_device_router(const uint portId, const uint devInstance, const char *devName, const char *devDescription)
{
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_CREATE_DEVICE_ROUTER);
    if (pkt == NULL) {
        panic();
        return;
    }

    ipc_encode_uint16(pkt, portId);
    ipc_encode_uint32(pkt, devInstance);
    ipc_encode_string(pkt, devName);
    ipc_encode_string(pkt, devDescription);

    ipc_send_packet(pkt);
}


void ipcClient_create_network_virtual(const uint portId, const uint networkNumber)
{ 
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_CREATE_NETWORK_VIRT);
    if (pkt == NULL) {
        panic();
        return;
    }
    ipc_encode_uint16(pkt, portId);
    ipc_encode_uint16(pkt, networkNumber);

    ipc_send_packet(pkt);
}


void ipcClient_create_network_IP(const uint portId, const uint networkNumber, const char *iface, const uint IPport, const bool bbmd)
{ 
    IPC_PKT *pkt = ipc_create_packet(IPC_COMMAND_CREATE_NETWORK_IP);
    if (pkt == NULL) {
        panic();
        return;
    }
    ipc_encode_uint16(pkt, portId);
    ipc_encode_uint16(pkt, networkNumber);
    ipc_encode_uint16(pkt, IPport);
    ipc_encode_boolean(pkt, bbmd);
    ipc_encode_string(pkt, iface);

    ipc_send_packet(pkt);
}


void ipcClient_create_network_MSTP(const uint portId, const uint networkNumber, const uint baud, const uint MAC, const uint maxMasters, const uint maxInfoFrames)
{ }
