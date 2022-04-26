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

#pragma once

#include "ipc/udp/ipcUDPcommon.h"

bool ipcClient_init_Application(void);
void ipcClient_send_restart(void);

// these are 'helper' functions, they encapsulate some of the detail, allowing reduced parameter set
void ipcClient_update_analog_input_pv( uint deviceInstance, uint aiObjectInstance, float value );
void ipcClient_update_binary_input_pv(uint deviceInstance, uint objectInstance, int value);

// these are the baseline functions for interacting with the BACnet object model.

void ipcClient_create_object(const uint devInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, const char *objName, const char *objDescription);
void ipcClient_create_device_virtual(const uint portId, const uint devInstance, const char *devName, const char *devDescription);
void ipcClient_create_device_router(const uint portId, const uint devInstance, const char *devName, const char *devDescription);                // portId is so user can choose where this device is 'homed'
void ipcClient_create_network_virtual(const uint portId, const uint networkNumber);
void ipcClient_create_network_IP(const uint portId, const uint networkNumber, const char *iface, const uint IPport, const bool bbmd );
void ipcClient_create_network_MSTP(const uint portId, const uint networkNumber, const uint baud, const uint MAC, const uint maxMasters, const uint maxInfoFrames );

void ipcClient_update_object_analog(uint deviceInstance, BACNET_OBJECT_TYPE objType, uint objInstance, BACNET_PROPERTY_ID propertyId, float value);
void ipcClient_update_object_binary(uint deviceInstance, BACNET_OBJECT_TYPE objType, uint objInstance, BACNET_PROPERTY_ID propertyId, int value);
void ipcClient_update_device_status(uint deviceInstance, bool stat);



// adding functions for controlling/interacting with the M4 (via rpMsg)

typedef enum
{
    M4_Modbus, M4_BACnetMSTP   
} M4_mode ;

void ipcClient_set_M4_mode( M4_mode mode );
void ipcClient_set_M4_baud(uint baud);
void ipcClient_set_M4_MAC(uint mac);
void ipcClient_set_M4_max_masters(uint max);
void ipcClient_set_M4_info_frames(uint max);

void ipcClient_send_payload( const uint8_t *payload, const uint len);
void ipcClient_recv_payload(uint8_t *payloadDest, const uint expLen, const uint timeoutMS );       // Hui insisted on expected length (as in he could calculate the expected length. Review this.

