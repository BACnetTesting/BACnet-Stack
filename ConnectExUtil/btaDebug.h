/**************************************************************************
*
* Copyright (C) 2016 ConnectEx, Inc. <info@connect-ex.com>
*
* Permission is hereby granted, to whom a copy of this software and 
* associated documentation files (the "Software") is provided by ConnectEx, Inc.
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
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

// #include <stdint.h>
#include "bacdef.h"

#if 0

#define MX_BTA_BUFFER   600     // good enough for MSTP
#define BX_DATAOFFSET   22

typedef enum {
    BTAmsgType_Terminal = 7,
    BTAmsgType_BACstd = 32,
    BTAmsgType_TextMsg,
    BTAmsgType_StartMsg = 0x26,
    BTAmsgType_PanicMsg,
    BTAmsgType_MSTPcritStat = 40,       // 40
    BTAmsgType_MSTPframe,               // 41
    BTAmsgType_MSTPtransition,
    BTAmsgType_MSTPstateMachines,
    BTAmsgType_MemoryAllocStat,         // 44

} BTAmsgType;

void SendBTAmessage(char *message);
void SendBTAstartMessage(char *message);
void SendBTApanicMessage(char *message) ;
void SendBTApanicInt(char *message, int value );
void SendBTAmstpFrame( uint8_t *outbuf, uint16_t datalen ) ;
void SendBTAmstpPayload( uint8_t *payload, uint16_t data_len, uint8_t function, uint8_t dest, uint8_t source ) ;
void SendBTApacketRx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApacketTx(const int port_id, const BACNET_MAC_ADDRESS *srcPhyMac, const BACNET_MAC_ADDRESS *destPhyMac, const uint8_t *pdu, const int len);
void SendBTApayload(uint8_t *payload, int nsendlength);
void SendBTAmemoryStats(void);
void PrepareBTAheader(BTAmsgType fc, uint8_t *buffer ) ;

int encodeUInt16(uint8_t *buf, uint16_t port);
int encodeUInt32(unsigned char *buf, uint32_t val);

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


