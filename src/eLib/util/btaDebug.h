/****************************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
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

#include <stdbool.h>

#include "osLayer.h"

#if defined ( OS_LAYER_WIN  )
#include <varargs.h>
#else
// GCC no longer uses varargs
#include <stdarg.h>
#endif

#include "bacnet/bacdef.h"

#define BTA_LOGGING             1       // 1 to enable all BTA, debugging messages
#define BACNET_IP               1

#define MX_BTA_BUFFER   1450     // EK: Reducing to avoid IP fragmentation 
#define BX_DATAOFFSET   22

typedef enum {

    BTApt_BACstd = 0x20,            // Changing to hex for helping when viewing Wireshark captures
    BTApt_TextMsg,
    BTApt_StartMsg = 0x26,
    BTApt_PanicMsg = 0x27,          // 39    0x27
    BTApt_MSTPcritStat = 40,        // 40    0x28
    BTApt_MSTPframe,                // 41
    BTApt_MSTPtransition,
    BTApt_MSTPstateMachines,
    BTApt_MemoryAllocStat,          // 44
    BTApt_Hexdump = 47,
    
    BTA_HaveYouSeenApp = 60,        // 60
    BTA_HaveYouSeenNet,
    BTT_HaveYouSeenUnconfirmed,
    BTA_IhaveSeen,
    BTA_IhaveNotSeen,
    BTA_WhoIs,
    BTA_IAm,
    ClientAlive,                    // 67  
    BTT_ActAsRouterToNetwork,
    BTT_Ack,
    BTT_SendNetworkMessage,         // 70
    BCR_Control,
    BCR_Payload,
    BCR_PingPong,
    TextMsgNotice,
    TextMsgWarning,
    TextMsgInfo,
    SimplestPing = 77,              // 77

} BTApt;


void SendBTAmessage(const char *message);
void SendBTAmessageF1(char *message, int val1 );
void SendBTAhexdump(const char *message, const void *buffer, const uint16_t len );
void SendBTAstartMessage(const char *message);
void SendBTApanicMessage(const char *message) ;
void SendBTApanicInt(const char *message, const int value );
void SendBTAmstpFrame(const uint8_t portId, const bool tx, const uint8_t *outbuf, const uint16_t datalen ) ;
void SendBTAmstpPayload(const uint8_t portId, const bool tx, const uint16_t data_len, const uint8_t function, const uint8_t dest, const uint8_t source, const uint8_t *payload ) ;
void SendBTApacketRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApacketTx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApayload(uint8_t *payload, const int sendlength);
void SendBTAmemoryStats(void);
// not static, cos used by emm.c
void PrepareBTAheader(const BTApt fc, uint8_t *buffer ) ;
bool BTA_Ready(void); // different datalinks may have different statii
void BTA_DatalinkTick(uint usTimeout);
bool BTA_DatalinkInit(void);

int encodeUInt8(uint8_t *buf, const uint8_t val);
int encodeUInt16(uint8_t *buf, const uint16_t val);
int encodeUInt32(uint8_t *buf, const uint32_t val);

