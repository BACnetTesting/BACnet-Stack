/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
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

#pragma once

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

// declared down below
//void SendBTApanic(const char *message, ... );
//void SendBTAmessage(const char *message);
//void SendBTAstartMessage(const char *message);
//void SendBTApacketRx(int port_id, BACNET_MAC_ADDRESS *srcPhyMac, BACNET_MAC_ADDRESS *destPhyMac, uint8_t *pdu, int len);
//void SendBTApacketTx(int port_id, BACNET_MAC_ADDRESS *srcPhyMac, BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, int len);
//void SendBTApayload(uint8_t *payload, int sendlength) ;
//void SendBTAprintf( const char *fmt, ...);
//void SendBTAvprintf(const char *fmt, va_list ap );

typedef enum {
    BTApt_BACstd = 0x20,
    BTApt_TextMsg,
    BTApt_StartMsg = 0x26,
    BTApt_PanicMsg = 0x27,            // also 39
    BTApt_MSTPcritStat = 40,       // 40
    BTApt_MSTPframe,               // 41
    BTApt_MSTPtransition,
    BTApt_MSTPstateMachines,
    BTApt_MemoryAllocStat,         // 44
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

    BTA_HaveYouSeenApp = 60,        // 60
    BTA_HaveYouSeenNet,
    BTT_HaveYouSeenUnconfirmed,
    BTA_IhaveSeen,
    BTA_IhaveNotSeen,
    BTA_WhoIs,
    BTA_IAm,
    ClientAlive,
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
*/ 

} BTApt;

#if 1 

void SendBTAmessage(const char *message);
void SendBTAstartMessage(const char *message);
void SendBTApanicMessage(const char *message) ;
void SendBTApanicInt(const char *message, const int value );
void SendBTAmstpFrame(const uint8_t *outbuf, const uint16_t datalen ) ;
void SendBTAmstpPayload(const uint8_t *payload, const uint16_t data_len, const uint8_t function, const uint8_t dest, const uint8_t source ) ;
void SendBTApacketRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApacketTx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApayload(uint8_t *payload, const int sendlength);
void SendBTAmemoryStats(void);
void PrepareBTAheader(const BTApt fc, uint8_t *buffer ) ;
bool BTAready(void);

int encodeUInt16(uint8_t *buf, const uint16_t port);
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

