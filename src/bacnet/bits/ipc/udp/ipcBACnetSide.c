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

#include "config.h"

#if ( BITS_USE_IPC == 1 )

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "ai.h"
#include "ao.h"
#include "av.h"
#include "bi.h"
#include "bo.h"
#include "bv.h"
#include "nc.h"
#include "bacenum.h"

#ifdef _MSC_VER
// #include <Windows.h>

//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib, "IPHLPAPI.lib")
#endif

#ifdef __GNUC__
// #include <socket.h>
#endif

#include "logging.h"
// #include "datalink.h"
#include "ipcBACnetSide.h"
#include "device.h"

typedef unsigned char byte;

#ifdef _MSC_VER
extern SOCKET ipcListenSocket;
#else
extern int ipcListenSocket ;
#endif

// static uint peerPortLocal ;

//static void(*callbackStatusLocal) (uint device, bool val);
//static void(*callbackAnaUpdateLocal) (uint device, float val);
//static void(*callbackBinUpdateLocal) (uint device, bool val);

extern bool showIPChandshake ;
bool ipcRestartRequested;
bool showIPCtraffic;

void Create_Object( const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description)
{
    VirtualDeviceInfo *dev = Find_Virtual_Device(devInstance);
    if (dev == NULL) {
        panicDesc("No device found");
        return;
    }

    BACnetObject *obj = NULL;

    switch (objType) {
    case OBJ_TYPE_AI:
        obj = new AnalogInputObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogInputs.push_back(obj);
        break;
    case OBJ_TYPE_BI:
        obj = new BinaryInputObject(objInstance, name, description);
        dev->pDev->binaryInputs.push_back(obj);
        break;
    case OBJ_TYPE_AO:
        obj = new AnalogOutputObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogOutputs.push_back(obj);
        break;

#if (BACNET_USE_OBJECT_ANALOG_VALUE == 1)
    case OBJ_TYPE_AV:
        obj = new AnalogValueObject(objInstance, UNITS_KILOWATTS, name, description);
        dev->pDev->analogValues.push_back(obj);
        break;
#endif

    case OBJ_TYPE_BO:
        obj = new BinaryOutputObject(objInstance, name, description);
        dev->pDev->binaryOutputs.push_back(obj);
        break;

    case OBJ_TYPE_BV:
        obj = new BinaryValueObject(objInstance, name, description);
        dev->pDev->binaryValues.push_back(obj);
        break;
        
#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
    case OBJ_TYPE_NC:
        obj = new NotificationClass(objInstance, name, description);
        dev->pDev->notificationClasses.push_back(obj);
        break;
#endif

    default:
        panic();
        break;
    }
}


void ThreadListenFuncBACnetSide(void *lpParam)
{
	if ( ipcListenSocket < 0 ) {
		panic();
		return ;
	}

	(void) lpParam ;
    while (true) {
        struct sockaddr from;
        socklen_t fromlen = sizeof ( struct sockaddr ) ;
        memset(&from, 0, sizeof(struct sockaddr));
        byte buffer[MX_BUFFER];
        int len = recvfrom(ipcListenSocket, (char *) buffer, MX_BUFFER, 0, &from, &fromlen );
        if (len <= 0)
        {
            panic();
            // int err = WSAGetLastError();
            continue;
        }

    	uint devInstance;
        uint objInstance;
        uint16_t portId ;
        uint16_t networkNumber ;
        uint16_t IPport ;
        bool bbmd ;
        IPC_PROPERTY_ID propertyID;
        IPC_OBJECT_TYPE objType;
        float value;
        bool bvalue;
        uint iptr = 0;
        bool success;
        char name[50];
        char description[100];

        IPC_MSG_TYPE function = (IPC_MSG_TYPE)ipc_decode_uint16(buffer, &iptr);
        uint mlen = ipc_decode_uint16(buffer, &iptr);
        switch ( function )
        { 
        case IPC_COMMAND_PING:
            // simply send a pong
            if (showIPChandshake)log_printf("\nPing received, sending pong");
            ipc_send_command(IPC_COMMAND_PONG);
            break;

        case IPC_COMMAND_PONG:
            // todo - reset a timer...
            if (showIPChandshake) log_printf("\nPong received");
            break;

        case IPC_COMMAND_RESTART:
            ipcRestartRequested = true;
            break;

        case IPC_COMMAND_ANA_UPDATE:
        case IPC_COMMAND_BIN_UPDATE:
            devInstance = ipc_decode_uint32(buffer, &iptr);
            objType = (IPC_OBJECT_TYPE) ipc_decode_uint16(buffer, &iptr);
            objInstance = ipc_decode_uint32(buffer, &iptr);
            propertyID = (IPC_PROPERTY_ID)ipc_decode_uint16(buffer, &iptr);
            switch (function)
            {
            case IPC_COMMAND_ANA_UPDATE:
                value = ipc_decode_float(buffer, &iptr);
                ipc_callBackAnaUpdate(devInstance, objType, objInstance, propertyID, value);
                break;
            case IPC_COMMAND_BIN_UPDATE:
                bvalue = ( ipc_decode_int32(buffer, &iptr) > 0 ) ? true : false ;
                ipc_callBackBinUpdate(devInstance, objType, objInstance, propertyID, bvalue);
                break;
            }
            break;

        case IPC_COMMAND_UPDATE_DEVICE_STATUS:
            devInstance = ipc_decode_uint32(buffer, &iptr );
            bvalue = ipc_decode_boolean( buffer, &iptr ) ;
            ipc_callBackDeviceUpdate(devInstance, bvalue);
            break;

        case IPC_COMMAND_CREATE_NETWORK_VIRT:
            portId = ipc_decode_uint16(buffer, &iptr );
            networkNumber = ipc_decode_uint16(buffer, &iptr);
            success = InitRouterportVirtual(portId, networkNumber );
            break;

        case IPC_COMMAND_CREATE_NETWORK_IP:
            portId = ipc_decode_uint16(buffer, &iptr);
            networkNumber = ipc_decode_uint16(buffer, &iptr);
            IPport = ipc_decode_uint16(buffer, &iptr);
            bbmd = ipc_decode_boolean(buffer, &iptr);
            char iface[50];
            ipc_decode_string(buffer, &iptr, iface, sizeof(iface));
            if (bbmd) {
                InitRouterport(portId, BPT_BBMD, iface, networkNumber, IPport);
            }
            else {
                InitRouterport(portId, BPT_BIP, iface, networkNumber, IPport);
            }
            break;

        case IPC_COMMAND_CREATE_DEVICE_VIRTUAL:
            portId = ipc_decode_uint16(buffer, &iptr);
            devInstance = ipc_decode_uint32(buffer, &iptr);
            ipc_decode_string(buffer, &iptr, name, sizeof(name));
            ipc_decode_string(buffer, &iptr, description, sizeof(description));
            Create_Device_Virtual(portId, devInstance, name, description, 343, "BACnet Interoperability Testing Services, Inc");
            break;

        case IPC_COMMAND_CREATE_DEVICE_ROUTER:
            portId = ipc_decode_uint16(buffer, &iptr);
            devInstance = ipc_decode_uint32(buffer, &iptr);
            ipc_decode_string(buffer, &iptr, name, sizeof(name));
            ipc_decode_string(buffer, &iptr, description, sizeof(description));
            Create_Device_Router(portId, devInstance, name, description, 343, "BACnet Interoperability Testing Services, Inc");
            break;

        case IPC_COMMAND_CREATE_OBJECT:
            devInstance = ipc_decode_uint32(buffer, &iptr);
            objType = (IPC_OBJECT_TYPE) ipc_decode_uint16(buffer, &iptr);
            objInstance = ipc_decode_uint32(buffer, &iptr);
            ipc_decode_string(buffer, &iptr, name, sizeof(name));
            ipc_decode_string(buffer, &iptr, description, sizeof(description));
            Create_Object(devInstance, objType, objInstance, name, description);
            break;

        default:
            panic();
        }
    }
}


void ipc_update_object_analog(uint deviceInstance, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propertyId, float value)
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

void ipc_update_object_binary(uint deviceInstance, IPC_OBJECT_TYPE objType, uint objInstance, IPC_PROPERTY_ID propertyId, int value)
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


void ipc_send_restart(void)
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
void ipc_update_analog_input_pv(uint deviceInstance, uint objectInstance, float value)
{
    ipc_update_object_analog(deviceInstance, OBJ_TYPE_AI, objectInstance, PROP_PV, value);
}



bool ipc_init_BACnet(void)
{
    return ipc_init_common(IPC_BACNET_LISTEN_PORT, IPC_APPLICATION_LISTEN_PORT, ThreadListenFuncBACnetSide);
}


#if 0
bool ipc_init(uint myPort, uint peerPort)
    //void(*callbackStatus) (uint device, bool val),
    //void(*callbackAnaUpdate) (uint device, float val),
    //void(*callbackBinUpdate) (uint device, bool val)
    //)
{
    int threadData = 0 ;
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

    ipcListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

    struct sockaddr_in sin ;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons((u_short)myPort);
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));

    rv = bind(ipcListenSocket, (const struct sockaddr *) &sin,
        sizeof(struct sockaddr_in));
    if (rv < 0) {
        panic();
        closesocket(ipcListenSocket);
        return false;
    }

    // set up a listener thread on the given port, and issues callback whenever a (relevant) packet is received..
    HANDLE threadHandle = CreateThread(NULL, 0, ThreadListenFunc, &threadData, 0, NULL);
    if (threadHandle == NULL)
    {
        panic();
        return false ;
    }

    threadHandle = CreateThread(NULL, 0, ThreadMaint, &threadData, 0, NULL);
    if (threadHandle == NULL)
    {
        panic();
        return false;
    }

    return true;
}
#endif

#endif // BITS_USE_IPC