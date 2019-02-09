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

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "appApi.h"

#ifdef _MSC_VER
typedef void(*ThreadFunction) (void *);
#else
typedef void *(*ThreadFunction)(void *);
#endif


typedef unsigned int uint;


// #ifndef panic
// #define panic() printf("Panic - %s %d\n", __FILE__, __LINE__ )
//#define log_printf printf
// #endif

typedef enum
{
    IPC_COMMAND_PING = 10,
    IPC_COMMAND_PONG,
    IPC_COMMAND_RESTART,
    IPC_COMMAND_CREATE_NETWORK_IP,
    IPC_COMMAND_CREATE_NETWORK_VIRT,
    IPC_COMMAND_CREATE_NETWORK_MSTP,
    IPC_COMMAND_CREATE_DEVICE_ROUTER,
    IPC_COMMAND_CREATE_DEVICE_VIRTUAL,
    IPC_COMMAND_ANA_UPDATE,
    IPC_COMMAND_BIN_UPDATE,
    IPC_COMMAND_UPDATE_DEVICE_STATUS,
    IPC_COMMAND_CREATE_OBJECT,

} IPC_MSG_TYPE ;

//typedef enum
//{
//    OBJ_TYPE_AI,
//    OBJ_TYPE_AO,
//    OBJ_TYPE_BI,
//    OBJ_TYPE_BO,
//    OBJ_TYPE_AV,
//    OBJ_TYPE_BV,
//    OBJ_TYPE_NC,
//    OBJ_TYPE_Calendar,
//    OBJ_TYPE_Schedule,
//} IPC_OBJECT_TYPE ;
//
//typedef enum
//{
//    PROP_PV,
//    PROP_DESC,
//} IPC_PROPERTY_ID ;
//
typedef enum
{
    IPCTT_FLOAT,
    IPCTT_ENUM,
    IPCTT_CHARSTR,
} IPC_TYPE_TAG ;


#define MX_CHARSTRING   100 

typedef struct
{
    IPC_TYPE_TAG    type;
    union
    {
        float       floatVal;
        uint16_t    uint16val;
        int16_t     int16val;
        uint32_t    uint32val;
        int32_t     int32val;
        char        charStringVal[MX_CHARSTRING];
    };
} IPC_DATA ;

typedef struct
{
    IPC_MSG_TYPE    msgType;
    uint32_t        deviceInstance;
    IPC_OBJECT_TYPE objectType;
    uint32_t        objectInstance;
    IPC_PROPERTY_ID propertyId;
    IPC_DATA        data;
} IPC_PAYLOAD ;


#define MX_BUFFER 800

typedef struct
{
    uint    len;
    uint8_t buffer[MX_BUFFER];
} IPC_PKT ;


// todo - use named pipes? UUnix Domain Sockets? 0MQ? pseudo-random ports.... (and supply 'my listen port' in the ping/discover)
#define IPC_BACNET_LISTEN_PORT         46000
#define IPC_APPLICATION_LISTEN_PORT    46001


void ThreadMaint(void *param);

void ipc_encode_boolean(IPC_PKT *pkt, const bool val);
void ipc_encode_uint16(IPC_PKT *pkt, const uint val);
void ipc_encode_uint32(IPC_PKT *pkt, const uint val);
void ipc_encode_float(IPC_PKT *pkt, const float val);
void ipc_encode_int32(IPC_PKT *pkt, const int val);
void ipc_encode_uint8(IPC_PKT *pkt, const uint8_t val);
void ipc_encode_string(IPC_PKT *pkt, const char *string);

uint ipc_decode_uint16(uint8_t *buffer, uint *iptr );
uint ipc_decode_uint32(uint8_t *buffer, uint *iptr);
int ipc_decode_int32(uint8_t *buffer, uint *iptr);
float ipc_decode_float(uint8_t *buffer, uint *iptr);
bool ipc_decode_boolean(uint8_t *buffer, uint *iptr);
void ipc_decode_string(uint8_t *buffer, uint *iptr, char *target, const uint bufSize );

void ipc_send_packet(IPC_PKT *pkt);
void ipc_send_command(IPC_MSG_TYPE cmd);
IPC_PKT *ipc_create_packet(IPC_MSG_TYPE cmd);

bool ipc_init_common(uint myPort, uint peerPort, void (*ThreadListenFunc)(void *));
