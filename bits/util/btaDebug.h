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

#if defined ( _MSC_VER  )
#include <varargs.h>
#else
// GCC no longer uses varargs
#include <stdarg.h>
#endif

// #include <stdint.h>
#include "bacdef.h"

#define BTA_LOGGING             1       // 1 to enable all BTA, debugging messages
#define BACNET_IP               1

#define MX_BTA_BUFFER   1200     // good enough for MSTP
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
    
    /*
    BACstd = 0x20,                  // Changing to hex for helping when viewing Wireshark captures
    TextMsg,
    PTP_Delay_Req,
    PTP_Delay_Resp,
    PTP_Follow_Up,
    PTP_Sync,                       // Also used to announce IPEP of Distributed Network Scanner Port
    TextMsgStartup = 0x26,          // also 38
    TextMsgPanic = 0x27,            // also 39

    MSTPCritStat = 0x28,            // 40
    MSTPframe,                      // 41
    MSTPtransition,                 // 42
    MSTPstateMachine,               // 43
    MemoryAllocStat,                // 44
    */

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

#if 1 

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
void PrepareBTAheader(const BTApt fc, uint8_t *buffer ) ;
bool BTA_Ready(void);
void BTA_DatalinkIdle(void);
bool BTA_DatalinkInit(void);

int encodeUInt8(uint8_t *buf, const uint8_t val);
int encodeUInt16(uint8_t *buf, const uint16_t val);
int encodeUInt32(uint8_t *buf, const uint32_t val);

#else

#define SendBTAmessage(message)
#define SendBTAstartMessage(message);
#define SendBTApanicMessage(message);
#define SendBTApanicInt(message, value);
#define SendBTAmstpFrame(outbuf, datalen);
#define SendBTAmstpPayload(payload, data_len, function, dest, source);
#define SendBTApacketRx(port_id,  srcPhyMac,  destPhyMac,  pdu,  len);
#define SendBTApacketTx( port_id,  srcPhyMac,   destPhyMac,  pdu,  len);
#define SendBTApayload( payload,  nsendlength);
#define SendBTAmemoryStats(void);
#define PrepareBTAheader( fc,  buffer);

#define encodeUInt16( buf,  port);
#define encodeUInt32( buf,  val);

#endif

