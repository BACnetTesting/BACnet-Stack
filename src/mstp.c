/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2003-2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307
 USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
*
*****************************************************************************************
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

/** @file mstp.c  BACnet Master-Slave Twisted Pair (MS/TP) functions */

/* This clause describes a Master-Slave/Token-Passing (MS/TP) data link  */
/* protocol, which provides the same services to the network layer as  */
/* ISO 8802-2 Logical Link Control. It uses services provided by the  */
/* EIA-485 physical layer. Relevant clauses of EIA-485 are deemed to be  */
/* included in this standard by reference. The following hardware is assumed: */
/* (a)  A UART (Universal Asynchronous Receiver/Transmitter) capable of */
/*      transmitting and receiving eight data bits with one stop bit  */
/*      and no parity. */
/* (b)  An EIA-485 transceiver whose driver may be disabled.  */
/* (c)  A timer with a resolution of five milliseconds or less */

#include <stddef.h>
#include <stdint.h>
#if PRINT_ENABLED
#include <stdio.h>
#endif
#include "crc.h"
#include "emm.h"
#include "btaDebug.h"
#include "rs485common.h"
#include "mstptext.h"
#include "debug.h"
// #include "usbd_cdc_if.h"
#include "dlmstp.h"

// #include "led.h"

#ifdef ESE_DEBUG
#include "ese.h"
#endif
#include "llist.h"
#include "mstp.h"
// #include "scope.h"
#include "datalink.h"
#include "timerCommon.h"

// extern void prints(const char *outstr);

// BMDA Test parameters
int testSetup;
extern uint8_t targetMAC;

/* MS/TP Frame Format */
/* All frames are of the following format: */
/* */
/* Preamble: two octet preamble: X`55', X`FF' */
/* Frame Type: one octet */
/* Destination Address: one octet address */
/* Source Address: one octet address */
/* Length: two octets, most significant octet first, of the Data field */
/* Header CRC: one octet */
/* Data: (present only if Length is non-zero) */
/* Data CRC: (present only if Length is non-zero) two octets, */
/*           least significant octet first */
/* (pad): (optional) at most one octet of padding: X'FF' */

/* The minimum number of DataAvailable or ReceiveError events that must be */
/* seen by a receiving node in order to declare the line "active": 4. */
#define Nmin_octets 4

/* The minimum time without a DataAvailable or ReceiveError event within */
/* a frame before a receiving node may discard the frame: 60 bit times. */
/* (Implementations may use larger values for this timeout, */
/* not to exceed 100 milliseconds.) */
/* At 9600 baud, 60 bit times would be about 6.25 milliseconds */
/* const uint16_t Tframe_abort = 1 + ((1000 * 60) / 9600); */
#ifndef Tframe_abort
#define Tframe_abort 95
#endif

/* The maximum time a node may wait after reception of a frame that expects */
/* a reply before sending the first octet of a reply or Reply Postponed */
/* frame: 250 milliseconds. */
#define Treply_delay 250

/* Repeater turnoff delay. The duration of a continuous logical one state */
/* at the active input port of an MS/TP repeater after which the repeater */
/* will enter the IDLE state: 29 bit times < Troff < 40 bit times. */
#define Troff 30

/* The minimum time without a DataAvailable or ReceiveError event */
/* that a node must wait for a station to begin replying to a */
/* confirmed request: 255 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 300 milliseconds.) */
#ifndef Treply_timeout
#define Treply_timeout 295
#endif

/* The minimum time without a DataAvailable or ReceiveError event that a */
/* node must wait for a remote node to begin using a token or replying to */
/* a Poll For Master frame: 20 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 100 milliseconds.) */
#ifndef Tusage_timeout
// todo 0 - why does 95 work but 20 not? 
#define Tusage_timeout 20 // 95
// #define Tusage_timeout 95
#endif

/* we need to be able to increment without rolling over */
#define INCREMENT_AND_LIMIT_UINT8(x) {if (x < 0xFF) x++;}

/* data structure for MS/TP PDU Queue */
//struct mstp_pdu_packet
//{
//    LLIST_LB    ll_lb;         // must be first
//
//    bool data_expecting_reply;
//    uint8_t destination_mac;
//    DLCB *dlcb;
//};

#define MSTP_LED_FLASH_DURATION 20

extern int goJitter;

bool MSTP_Line_Active(
    volatile struct mstp_port_struct_t *mstp_port)
{
    return (mstp_port->EventCount > Nmin_octets);
}

void MSTP_Fill_BACnet_Address(
    BACNET_ADDRESS * src,
    uint8_t mstp_address)
{
    int i = 0;

    if (mstp_address == MSTP_BROADCAST_ADDRESS) {
        /* mac_len = 0 if broadcast address */
        src->mac_len = 0;
        src->mac[0] = 0;
    }
    else {
        src->mac_len = 1;
        src->mac[0] = mstp_address;
    }
    /* fill with 0's starting with index 1; index 0 filled above */
    for (i = 1; i < MAX_MAC_LEN; i++) {
        src->mac[i] = 0;
    }
    src->net = 0;
    src->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src->adr[i] = 0;
    }
}

bool scopetrigger = false;

static void PrepareTriggers(void)
{
    testSetup = 0;
    scopetrigger = true;
}

uint16_t MSTP_Create_Frame(
    uint8_t * buffer,           /* where frame is loaded */
    uint16_t buffer_len,        /* amount of space available */
    uint8_t frame_type,         /* type of frame to send - see defines */
    uint8_t destination,        /* destination address */
    uint8_t source,             /* source address */
    uint8_t * data,             /* any data to be sent - may be null */
    uint16_t data_len)          /* number of bytes of data (up to 501) */
{
    uint8_t crc8 = 0xFF;        /* used to calculate the crc value */
    uint16_t crc16 = 0xFFFF;    /* used to calculate the crc value */
    uint16_t index = 0;         /* used to load the data portion of the frame */

    /* not enough to do a header */
    if (buffer_len < 8)
        return 0;

    // our tests
    bool badTokenCRC = false;
    bool shortToken = false;
    int leadingFFs = 0;
    uint8_t badchar = 0xFF;
    bool shortdata = false;

    if (destination == targetMAC && testSetup != 0 ) {
        if (frame_type == FRAME_TYPE_TOKEN) {
            switch (testSetup) {
            case 'D':
                shortdata = true;
                PrepareTriggers();
                break;
            case 'F':
                prints("<M- Sending 7 leading 0xFFs\r\n");
                leadingFFs = 7;
                PrepareTriggers();
                break;
            case '5':
                prints("<M- Sending 7 leading 0x55s\r\n");
                badchar = 0x55;
                leadingFFs = 7;
                PrepareTriggers();
                break;
            case 'N':
                prints("<M- Sending 7 leading 'p's (noise)\r\n");
                badchar = 'p';
                leadingFFs = 7;
                PrepareTriggers();
                break;
            case 'T':
                prints("<M- Sending Token with Bad CRC\r\n");
                badTokenCRC = true;
                PrepareTriggers();
                break;
            case 'S':
                prints("<M- Sending Short Token (missing length and CRC)\r\n");
                shortToken = true;
                PrepareTriggers();
                break;
            }
        }

        if (frame_type == FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY) {
            switch (testSetup) {
            case 'D':
                shortdata = true;
                PrepareTriggers();
                break;
            }
        }
    }

    for (int i = 0; i < leadingFFs; i++) {
        buffer[index++] = badchar ;
    }

    buffer[index++] = 0x55;
    buffer[index++] = 0xFF;
    buffer[index++] = frame_type;
    crc8 = CRC_Calc_Header(frame_type, crc8);
    buffer[index++] = destination;
    crc8 = CRC_Calc_Header(destination, crc8);
    buffer[index++] = source;
    crc8 = CRC_Calc_Header(source, crc8);

    if (!shortToken) {
        buffer[index++] = data_len >> 8; /* MSB first */
        crc8 = CRC_Calc_Header(buffer[index-1], crc8);
        buffer[index++] = data_len & 0xFF;
        crc8 = CRC_Calc_Header(buffer[index-1], crc8);

        if (badTokenCRC) {
            buffer[index++] = crc8;   // bad CRC
        }
        else {
            buffer[index++] = ~crc8;
        }
    }

    uint16_t dataLength = data_len ;

    while (dataLength && data && (index < buffer_len)) {
        if (shortdata && dataLength == 3 ) {
            // drop one byte - third one for grins 
            prints("<M- Sending short data\r\n");
            shortdata = false;
        }
        else {
            buffer[index] = *data;
            crc16 = CRC_Calc_Data(buffer[index], crc16);
        }
        data++;
        index++;
        dataLength--;
    }

    /* append the data CRC if necessary */
    if (data_len) {
        if ((index + 2) <= buffer_len) {
            crc16 = ~crc16;
            buffer[index] = crc16 & 0xFF; /* LSB first */
            index++;
            buffer[index] = crc16 >> 8;
            index++;
        }
        else {
            return 0;
        }
    }

    return index;       /* returns the frame length */
}


#define BAC_SHOWFRAMETIME
#ifdef BAC_SHOWFRAMETIME
// static uint32_t usecFrameTimer;
static char tbuf[1000];
#endif
uint32_t maxMeasured ;

// uint8_t priorFrameType;
MOW_TYPE priorMOW;
MOW_TYPE onTheWireMOW;
uint8_t lastOwnerOfToken;
extern int keepsendin;

void dumpTimeStamp(void) 
{
    // note, this address is from the _prior_ frame, which is in fact, what we want to measure...
    // 0 - acutherm
    // 3 - cc router
    // 41 - asi router
    
//    if(mstp_port->DestinationAddress != 0 &&
//    mstp_port->DestinationAddress != 1 &&
//    mstp_port->DestinationAddress != 3 &&
//    mstp_port->DestinationAddress != 41 &&
//    mstp_port->DestinationAddress != 79 &&
//    mstp_port->DestinationAddress != 121) return;
    
    if(!onTheWireMOW.valid) return; 
    if (!priorMOW.valid) return ;
    
    if (onTheWireMOW.frameType == FRAME_TYPE_TOKEN) lastOwnerOfToken = onTheWireMOW.dest;
    
    uint32_t deltaTime;
    if (onTheWireMOW.firstByteTimestamp > priorMOW.lastByteTimestamp) {
        deltaTime = onTheWireMOW.firstByteTimestamp - priorMOW.lastByteTimestamp;
    } 
    else {
        deltaTime = onTheWireMOW.firstByteTimestamp + (0xFFFFFFFF - priorMOW.lastByteTimestamp);
        deltaTime += 1;
    }
    deltaTime = deltaTime / (SystemCoreClock / 1000000u);
    
    static uint heartbeat = 1000 ;
    if (!heartbeat) {
        prints("<M- Heartbeat\r\n");
        heartbeat = 1000;
    }
    else {
        heartbeat--;
    }

    // if (mstp_port->DestinationAddress == 0)
    {
        if ( ! goJitter && priorMOW.frameType == FRAME_TYPE_TOKEN && deltaTime > 15000)    // Tusage_delay
            {
                // bits_ScopeTrigger();
                sprintf(tbuf,
                    // "%3u %3u %u %-10u, %3u %3u %u %-10u, %6u\r\n", 
                    "Tusage_delay fail: Prior: %3u %3u %u, This late: %3u %3u %u, Delta: %6u\r\n", 
                    priorMOW.source,
                    priorMOW.dest,
                    priorMOW.frameType,
                    onTheWireMOW.source,
                    onTheWireMOW.dest,
                    onTheWireMOW.frameType,
                    deltaTime);
                CDC_Transmit_FS((uint8_t *) tbuf, strlen(tbuf));
            }
        
        if ( ! goJitter && priorMOW.frameType == FRAME_TYPE_TOKEN && onTheWireMOW.source == 79 )
        {
            // uint32_t delta = mstp_port->deltaTime / (SystemCoreClock / 1000000u);
            if(deltaTime > maxMeasured) 
            {
                maxMeasured = deltaTime;
                
#if 1
                sprintf(tbuf,
                    "%u, %u\r\n", 
                    onTheWireMOW.source,
                    deltaTime);
#else
            
                sprintf(tbuf,
                    // "%3u %3u %u %-10u, %3u %3u %u %-10u, %6u\r\n", 
                    "%3u %3u %u, %3u %3u %u, %6u\r\n", 
                    priorMOW.source,
                    priorMOW.dest,
                    priorMOW.frameType,
                    // priorMOW.lastByteTimestamp,

                    onTheWireMOW.source,
                    onTheWireMOW.dest,
                    onTheWireMOW.frameType,
                    // onTheWireMOW.firstByteTimestamp,

                    // onTheWireMOW.firstByteTimestamp - priorMOW.lastByteTimestamp);
                    deltaTime);
#endif
                CDC_Transmit_FS((uint8_t *) tbuf, strlen(tbuf));
            }
        }
        
        if ( goJitter && priorMOW.frameType == FRAME_TYPE_TEST_RESPONSE)
        {
//            if (keepsendin) {
//                if (deltaTime > maxMeasured) {
//                    maxMeasured = deltaTime;
//                    sprintf(tbuf,
//                        "%u\r\n", 
//                        deltaTime);
//                    CDC_Transmit_FS((uint8_t *) tbuf, strlen(tbuf));
//                }
//            }
//            else {
                sprintf(tbuf,
                    ":%u\r\n", 
                    deltaTime);
                CDC_Transmit_FS((uint8_t *) tbuf, strlen(tbuf));
//            }
        }
    }
}


bool matchMSTP(void *llitem, void *matchdata)
{
    (void)matchdata;      // not used.
    struct mstp_pdu_packet *pkt =  (struct mstp_pdu_packet *) llitem ;
    return pkt->dlcb->isDERresponse ;
    
//     return (((DLCB *)llitem)->isDERresponse);
    // EKH: After reflection, if there is a postponed frame on the queue, and another frame arrives then there will be an out-of-sequence event.
    // However, if ALL DERs are postponed, then there are no issues.
    //    Note: A router (should?) always has to automatically postpone - there is no way of knowing when a DER frame will be responded to from the far side.. 
    // If DERs are NOT postponed AUTOMATICALLY, then there can only be an issue if the first DER frame gets postponed by a timeout, and the next DER frame is then processed.. and we do not do this.
    //
}


void MSTP_Create_And_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,      /* port to send from */
    uint8_t frame_type,                                 /* type of frame to send - see defines */
    uint8_t destination,                                /* destination address */
    uint8_t source,                                     /* source address */
    uint8_t * data,                                     /* any data to be sent - may be null */
    uint16_t data_len)                                  /* number of bytes of data (up to 501) */
{
    /* number of bytes of data (up to 501) */
    uint16_t len; /* number of bytes to send */

    len =
        MSTP_Create_Frame((uint8_t *) & mstp_port->OutputBuffer[0],
            sizeof ( mstp_port->OutputBuffer),
            frame_type,
            destination,
            source,
            data,
            data_len);

    priorMOW = onTheWireMOW;
    onTheWireMOW.source = source;
    onTheWireMOW.frameType = frame_type;
    onTheWireMOW.dest = destination;
    onTheWireMOW.firstByteTimestamp = DWT->CYCCNT;               // note - approximate, it does get a bit queued
    onTheWireMOW.valid = true;
    dumpTimeStamp();
    
    SendBTAmstpFrame( 1,            // portID
                     true,          // tx
                     mstp_port->OutputBuffer,
                     len );
    
    dllmstp_Send_Frame(mstp_port, (uint8_t *) &mstp_port->OutputBuffer[0], len);
    /* FIXME: be sure to reset SilenceTimer() after each octet is sent! */
}


void MSTP_Receive_Frame_FSM(
    volatile struct mstp_port_struct_t *mstp_port)
{
    MSTP_RECEIVE_STATE receive_state = mstp_port->receive_state;
    
    //    if (mstp_port->DataAvailable)
    //    {
    //        printf("StateIn:%2d ", mstp_port->receive_state);
    //        printf("%02x ", mstp_port->DataRegister );
    //    }
        
    //    CDC_Transmit_FS(".", 1 );
    
    switch (mstp_port->receive_state) {

        
        /* In the IDLE state, the node waits for the beginning of a frame. */
    case MSTP_RECEIVE_STATE_IDLE:
        /* EatAnError */
        if (mstp_port->ReceiveError == true) {
            prints("Err in Idle\r\n");
            mstp_port->ReceiveError = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
            /* wait for the start of a frame. */
        }
        else if (mstp_port->DataAvailable == true) {
            /* Preamble1 */
            if (mstp_port->DataRegister == 0x55) {
                /* receive the remainder of the frame. */
                mstp_port->receive_state = MSTP_RECEIVE_STATE_PREAMBLE;

                // dumpTimeStamp(mstp_port);
                onTheWireMOW.firstByteTimestamp = mstp_port->timestamp;
            }
            else {
                prints("Not 0x55\r\n");
            }
            mstp_port->DataAvailable = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
        }
        break;
        
        /* In the PREAMBLE state, the node waits for the second octet of the preamble. */
    case MSTP_RECEIVE_STATE_PREAMBLE:
        /* Timeout */
        if (SilenceTimer(mstp_port) > Tframe_abort) {
            /* a correct preamble has not been received */
            /* wait for the start of a frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
        /* Error */
        else if(mstp_port->ReceiveError == true)
        {
            mstp_port->ReceiveError = false;
            SilenceTimerReset(mstp_port);
            led_rx_error(MSTP_LED_FLASH_DURATION*100);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
            /* wait for the start of a frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
        else if (mstp_port->DataAvailable == true) {
            /* Preamble2 */
            if (mstp_port->DataRegister == 0xFF) {
                mstp_port->Index = 0;
                mstp_port->HeaderCRC = 0xFF;
                /* receive the remainder of the frame. */
                mstp_port->receive_state = MSTP_RECEIVE_STATE_HEADER;
            }
            /* ignore RepeatedPreamble1 */
            else if(mstp_port->DataRegister == 0x55) {
                /* wait for the second preamble octet. */
            }
            /* NotPreamble */
            else {
                /* wait for the start of a frame. */
                mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
            }
            mstp_port->DataAvailable = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
        }
        break;
        
        /* In the HEADER state, the node waits for the fixed message header. */
    case MSTP_RECEIVE_STATE_HEADER:
        /* Timeout */
        if (SilenceTimer(mstp_port) > Tframe_abort) {
            /* indicate that an error has occurred during the reception of a frame */
            mstp_port->ReceivedInvalidFrame = true;
            /* wait for the start of a frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
            /* Error */
        else if(mstp_port->ReceiveError == true) {
            mstp_port->ReceiveError = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
            /* indicate that an error has occurred during the reception of a frame */
            mstp_port->ReceivedInvalidFrame = true;
            /* wait for the start of a frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
        else if (mstp_port->DataAvailable == true) {
            /* FrameType */
            if (mstp_port->Index == 0) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
                // priorFrameType = mstp_port->FrameType;
                mstp_port->FrameType = mstp_port->DataRegister;
                mstp_port->Index = 1;
            }
            /* Destination */
            else if (mstp_port->Index == 1) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
                mstp_port->DestinationAddress = mstp_port->DataRegister;
                mstp_port->Index = 2;
            }
            /* Source */
            else if (mstp_port->Index == 2) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
                mstp_port->SourceAddress = mstp_port->DataRegister;
                mstp_port->Index = 3;
            }
            /* Length1 */
            else if (mstp_port->Index == 3) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
                mstp_port->DataLength = mstp_port->DataRegister * 256;
                mstp_port->Index = 4;
            }
            /* Length2 */
            else if (mstp_port->Index == 4) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
                mstp_port->DataLength += mstp_port->DataRegister;
                mstp_port->Index = 5;
            }
            /* HeaderCRC */
            else if (mstp_port->Index == 5) {
                mstp_port->HeaderCRC =
                    CRC_Calc_Header(mstp_port->DataRegister,
                        mstp_port->HeaderCRC);
#ifdef MSTP_ANALYZER
                mstp_port->HeaderCRCActual = mstp_port->DataRegister;
#endif
                /* don't wait for next state - do it here */
                if (mstp_port->HeaderCRC != 0x55) {
                    /* BadCRC */
                    prints("BAD CRC\r\n");
                    led_rx_error(MSTP_LED_FLASH_DURATION*100);
                    /* indicate that an error has occurred during
                       the reception of a frame */
                    mstp_port->ReceivedInvalidFrame = true;
                    /* wait for the start of the next frame. */
                    mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
                }
                else {
                    led_rx_on_interval(MSTP_LED_FLASH_DURATION);
                    if (mstp_port->DataLength == 0) {
                        /* NoData */
                        if ((mstp_port->DestinationAddress == mstp_port->This_Station) ||
                            (mstp_port->DestinationAddress == MSTP_BROADCAST_ADDRESS)) {
                            /* ForUs */
                            /* indicate that a frame with no data has been received */
                            mstp_port->ReceivedValidFrame = true;
                            mstp_port->btaReceivedValidFrame = true;
                        }
                        else {
                            /* NotForUs */
                            mstp_port->ReceivedValidFrameNotForUs = true;
                            mstp_port->btaReceivedValidFrame = true;
                        }
                        
                        // usecFrameTimer = DWT->CYCCNT;
                        priorMOW = onTheWireMOW;
                        onTheWireMOW.dest = mstp_port->DestinationAddress;
                        onTheWireMOW.source = mstp_port->SourceAddress;
                        onTheWireMOW.frameType = mstp_port->FrameType;
                        onTheWireMOW.valid = true;
                        onTheWireMOW.lastByteTimestamp = DWT->CYCCNT;
                        dumpTimeStamp();
                        
                        /* wait for the start of the next frame. */
                        mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
                    }
                    else {
                        led_rx_data_frame(MSTP_LED_FLASH_DURATION);

                        /* receive the data portion of the frame. */
                        if ((mstp_port->DestinationAddress == mstp_port->This_Station) ||
                            (mstp_port->DestinationAddress == MSTP_BROADCAST_ADDRESS)) {
                            if (mstp_port->DataLength <= mstp_port->InputBufferSize) {
                                /* Data */
                                mstp_port->receive_state = MSTP_RECEIVE_STATE_DATA;
                            }
                            else {
                                /* FrameTooLong */
                                // todo 0 - our current skip data still plugs into input buffer, so this check is void... and it should not be!!
                                mstp_port->receive_state = MSTP_RECEIVE_STATE_SKIP_DATA;
                            }
                        }
                        else {
                            /* NotForUs */
                            mstp_port->receive_state = MSTP_RECEIVE_STATE_SKIP_DATA;
                        }
                        mstp_port->Index = 0;
                        mstp_port->DataCRC = 0xFFFF;
                    }
                }
            }
            /* (Steve Karg wrote this) not per MS/TP standard, but it is a case not covered */
            else {
                mstp_port->ReceiveError = false;
                /* indicate that an error has occurred during  */
                /* the reception of a frame */
                mstp_port->ReceivedInvalidFrame = true;
                /* wait for the start of a frame. */
                mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
            }
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
            mstp_port->DataAvailable = false;
        }
        break;

        /* In the DATA state, the node waits for the data portion of a frame. */
    case MSTP_RECEIVE_STATE_DATA:
    case MSTP_RECEIVE_STATE_SKIP_DATA:
        /* Timeout */
        if (SilenceTimer(mstp_port) > Tframe_abort) {
            /* indicate that an error has occurred during the reception of a frame */
            mstp_port->ReceivedInvalidFrame = true;
            /* wait for the start of the next frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
        /* Error */
        else if(mstp_port->ReceiveError == true) {
            mstp_port->ReceiveError = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
            /* indicate that an error has occurred during the reception of a frame */
            mstp_port->ReceivedInvalidFrame = true;
            /* wait for the start of the next frame. */
            mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        }
        else if(mstp_port->DataAvailable == true) {
            if (mstp_port->Index < mstp_port->DataLength) {
                /* DataOctet */
                mstp_port->DataCRC =
                    CRC_Calc_Data(mstp_port->DataRegister,
                        mstp_port->DataCRC);
                if (mstp_port->Index < mstp_port->InputBufferSize) {
                    mstp_port->InputBuffer[mstp_port->Index] =
                        mstp_port->DataRegister;
                }
                mstp_port->Index++;
                mstp_port->receive_state = MSTP_RECEIVE_STATE_DATA;         // todo 0 - this bombs us out of SKIP case. Cant be right
            }
            else if (mstp_port->Index == mstp_port->DataLength) {
                /* CRC1 */
                mstp_port->DataCRC =
                    CRC_Calc_Data(mstp_port->DataRegister,
                        mstp_port->DataCRC);
#ifdef MSTP_ANALYZER
                mstp_port->DataCRCActualMSB = mstp_port->DataRegister;
#endif
                mstp_port->Index++;
                mstp_port->receive_state = MSTP_RECEIVE_STATE_DATA;          // todo 0 - this bombs us out of SKIP case. Cant be right
            }
            else if (mstp_port->Index == (mstp_port->DataLength + 1)) {
                /* CRC2 */
                mstp_port->DataCRC =
                    CRC_Calc_Data(mstp_port->DataRegister,
                        mstp_port->DataCRC);
#ifdef MSTP_ANALYZER
                mstp_port->DataCRCActualLSB = mstp_port->DataRegister;
#endif
                /* STATE DATA CRC - no need for new state */
                /* indicate the complete reception of a valid frame */
                if (mstp_port->DataCRC == 0xF0B8) {
                    if (mstp_port->receive_state == MSTP_RECEIVE_STATE_DATA) {
                        /* ForUs */
                        mstp_port->ReceivedValidFrame = true;
                        mstp_port->btaReceivedValidFrame = true;
                    }
                    else {
                        /* NotForUs */
                        mstp_port->ReceivedValidFrameNotForUs = true;
                        mstp_port->btaReceivedValidFrame = true;
                    }

                    priorMOW = onTheWireMOW;
                    onTheWireMOW.dest = mstp_port->DestinationAddress;
                    onTheWireMOW.source = mstp_port->SourceAddress;
                    onTheWireMOW.frameType = mstp_port->FrameType;
                    onTheWireMOW.valid = true;
                    dumpTimeStamp();
                }
                else {
                    mstp_port->ReceivedInvalidFrame = true;
                    // todo3 - add bta diagnostics for invalid frames, noise etc.
                }
                mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
            }
            else {
                mstp_port->ReceivedInvalidFrame = true;
                mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
            }
            mstp_port->DataAvailable = false;
            SilenceTimerReset(mstp_port);
            INCREMENT_AND_LIMIT_UINT8(mstp_port->EventCount);
        }
        break;

    default:
        /* shouldn't get here - but if we do... */
        mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
        break;
    }
    
    //    printf("StateOut:%2d\r\n ", mstp_port->receive_state);
    
    
        //    static int initial_state;
        //    if (mstp_port->receive_state != initial_state)
        //    {
        //        printf("State %d -> %d\r\n", initial_state, mstp_port->receive_state);
        //        initial_state = mstp_port->receive_state;
        //    }
}

static char pbuf[1000];

/* returns true if we need to transition immediately */
bool MSTP_Master_Node_FSM(
    volatile struct mstp_port_struct_t * mstp_port)
{
    unsigned length = 0;
    uint8_t next_poll_station ;
    uint8_t next_this_station ;
    uint8_t next_next_station ;
    uint16_t my_timeout = 10, ns_timeout = 0, mm_timeout = 0;
    /* transition immediately to the next state */
    bool transition_now = false;
    MSTP_MASTER_STATE master_state = mstp_port->master_state;

    /* packet from the PDU Queue */
    struct mstp_pdu_packet *pkt;

    // CDC_Transmit_FS("M", 1);
    // printf("D = %d\r\n", mstp_port->DestinationAddress );

    static uint holdTheBus ;
    static uint holdTheBusTestRequest;
    static ETIMER timer;
    if (testSetup != 0) {
        switch (testSetup) {
        case 'H':
            if (mstp_port->master_state == MSTP_MASTER_STATE_USE_TOKEN ) {
                holdTheBus = 500;
                prints("<M- Holding the bus using single byte\r\n");
                PrepareTriggers();
                dllmstp_Send_Frame(mstp_port, (uint8_t *) "UUUU", 4);   // Nmin_octets
                timer_elapsed_start(&timer);
                return false;
                }
            break;
        case 'T':
            if (mstp_port->master_state == MSTP_MASTER_STATE_USE_TOKEN) {
                holdTheBusTestRequest = 1000;
                prints("<M- Holding the bus using Test_Request\r\n");
                PrepareTriggers();
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_TEST_REQUEST,
                    200,                                // a station we know that won't respond
                    mstp_port->This_Station,
                    NULL,
                    0);
                timer_elapsed_start(&timer);
                return false;
            }
            break;
        }
    }

    if (holdTheBus) {
        // Jam the MS/TP bus by sending a byte every 400 ms until done
        // if (SilenceTimer(mstp_port) > 400) {
        if ( timer_elapsed_time( &timer ) > 100 ) {
            prints("<M- Repeating\r\n");
            timer_elapsed_start(&timer);
            dllmstp_Send_Frame(mstp_port, (uint8_t *) "UUUU", 4);   // Nmin_octets
            holdTheBus--;
        }
        return false ;
    }
    if (holdTheBusTestRequest) {
        // Jam the MS/TP bus by sending a byte every 400 ms until done
        // if (SilenceTimer(mstp_port) > 400) {
        if (timer_elapsed_time(&timer) > 100) {
            prints("<M- Repeating\r\n");
            timer_elapsed_start(&timer);
            MSTP_Create_And_Send_Frame(
                mstp_port,
                FRAME_TYPE_TEST_REQUEST,
                200,                                // a station we know that won't respond
                mstp_port->This_Station,
                NULL,
                0);
            holdTheBusTestRequest--;
        }
        return false;
    }


    /* some calculations that several states need */
    next_poll_station =
        (mstp_port->Poll_Station + 1) % (mstp_port->Nmax_master + 1);
    next_this_station =
        (mstp_port->This_Station + 1) % (mstp_port->Nmax_master + 1);
    next_next_station =
        (mstp_port->Next_Station + 1) % (mstp_port->Nmax_master + 1);
    
    switch (mstp_port->master_state) {
    case MSTP_MASTER_STATE_INITIALIZE:
        CDC_Transmit_FS("I", 1);
        /* DoneInitializing */
        /* indicate that the next station is unknown */
        mstp_port->Next_Station = mstp_port->This_Station;
        mstp_port->Poll_Station = mstp_port->This_Station;
        /* cause a Poll For Master to be sent when this node first */
        /* receives the token */
        mstp_port->TokenCount = Npoll;
        mstp_port->SoleMaster = false;
        mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
        transition_now = true;
        break;
    case MSTP_MASTER_STATE_IDLE:
        /* In the IDLE state, the node waits for a frame. */
        if (mstp_port->ReceivedInvalidFrame == true) {
            /* ReceivedInvalidFrame */
            /* invalid frame was received */
            /* wait for the next frame - remain in IDLE */
            mstp_port->ReceivedInvalidFrame = false;
        }
        else if (mstp_port->ReceivedValidFrame == true) {
            if ((mstp_port->DestinationAddress == mstp_port->This_Station)
                || (mstp_port->DestinationAddress == MSTP_BROADCAST_ADDRESS)) {
                // CDC_Transmit_FS("M", 1);
                // printf("for me\r\n");
                /* destined for me! */
                switch (mstp_port->FrameType) {
                case FRAME_TYPE_TOKEN:
                    /* ReceivedToken */
                    /* tokens can't be broadcast */
                    if (mstp_port->DestinationAddress == MSTP_BROADCAST_ADDRESS) {
                        break;
                    }
                    mstp_port->ReceivedValidFrame = false;
                    mstp_port->FrameCount = 0;
                    mstp_port->SoleMaster = false;
                    mstp_port->master_state = MSTP_MASTER_STATE_USE_TOKEN;
                    transition_now = true;
                    break;

                case FRAME_TYPE_POLL_FOR_MASTER:
                    /* ReceivedPFM */
                    MSTP_Create_And_Send_Frame(
                        mstp_port,
                        FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER,
                        mstp_port->SourceAddress,
                        mstp_port->This_Station,
                        NULL,
                        0);
                    break;

                case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                    /* indicate successful reception to the higher layers */
                    (void) MSTP_Put_Receive(mstp_port);
                    break;

                case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                    /*mstp_port->ReplyPostponedTimer = 0; */
                    /* indicate successful reception to the higher layers  */
                    (void) MSTP_Put_Receive(mstp_port);
                    /* broadcast DER just remains IDLE */
                    if (mstp_port->DestinationAddress != MSTP_BROADCAST_ADDRESS ) {
                        mstp_port->master_state = MSTP_MASTER_STATE_ANSWER_DATA_REQUEST;
                    }
                    break;

                case FRAME_TYPE_TEST_REQUEST:
                    MSTP_Create_And_Send_Frame(
                        mstp_port,
                        FRAME_TYPE_TEST_RESPONSE,
                        mstp_port->SourceAddress,
                        mstp_port->This_Station,
                        mstp_port->InputBuffer,
                        mstp_port->DataLength);
                    break;

                case FRAME_TYPE_TEST_RESPONSE:
                default:
                    break;
                }
            }
            /* For DATA_EXPECTING_REPLY, we will keep the Rx Frame for
               reference, and the flag will be cleared in the next state */
            if (mstp_port->master_state != MSTP_MASTER_STATE_ANSWER_DATA_REQUEST) {
                mstp_port->ReceivedValidFrame = false;
            }
        }
        else if (SilenceTimer(mstp_port) >= Tno_token) {
            /* LostToken */
            /* assume that the token has been lost */
            mstp_port->EventCount = 0;      /* Addendum 135-2004d-8 */
            mstp_port->master_state = MSTP_MASTER_STATE_NO_TOKEN;
            /* set the receive frame flags to false in case we received
               some bytes and had a timeout for some reason */
            mstp_port->ReceivedInvalidFrame = false;
            mstp_port->ReceivedValidFrame = false;
            transition_now = true;
        }
        break;

    case MSTP_MASTER_STATE_USE_TOKEN:
        /* In the USE_TOKEN state, the node is allowed to send one or  */
        /* more data frames. These may be BACnet Data frames or */
        /* proprietary frames. */
        /* Note: FIXME: We could wait for up to Tusage_delay */
        if (!ll_GetCount( &mstp_port->mstpOutputQueuePtr )) {
            /* NothingToSend */
            mstp_port->FrameCount = mstp_port->Nmax_info_frames;
            mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
            transition_now = true;
        }
        else {
            uint8_t frame_type;
            pkt = (struct mstp_pdu_packet *) ll_Dequeue( &mstp_port->mstpOutputQueuePtr);

            if (pkt->specialFunction == FRAME_TYPE_TEST_REQUEST) {
                frame_type = FRAME_TYPE_TEST_REQUEST;
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    frame_type,
                    pkt->destination_mac,
                    mstp_port->This_Station,
                    NULL,
                    0);
            }
            else {
                if (pkt->data_expecting_reply) {
                    frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
                }
                else {
                    frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
                }

                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    frame_type,
                    pkt->destination_mac,
                    mstp_port->This_Station,
                    pkt->dlcb->Handler_Transmit_Buffer,
                    pkt->dlcb->optr);

                dlcb_free(pkt->dlcb);
            }

            emm_free(pkt);
            mstp_port->FrameCount++;
            switch (frame_type) {
            case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                if (pkt->destination_mac == MSTP_BROADCAST_ADDRESS) {
                    /* SendNoWait */
                    mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                }
                else {
                    /* SendAndWait */
                    mstp_port->master_state = MSTP_MASTER_STATE_WAIT_FOR_REPLY;
                }
                break;
            case FRAME_TYPE_TEST_REQUEST:
                /* SendAndWait */
                mstp_port->master_state = MSTP_MASTER_STATE_WAIT_FOR_REPLY;
                break;
            case FRAME_TYPE_TEST_RESPONSE:
            case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
            default:
                /* SendNoWait */
                mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                break;
            }
        }
        break;

    case MSTP_MASTER_STATE_WAIT_FOR_REPLY:
        /* In the WAIT_FOR_REPLY state, the node waits for  */
        /* a reply from another node. */
        if (SilenceTimer(mstp_port) >= Treply_timeout) {
            /* ReplyTimeout */
            /* assume that the request has failed */
            mstp_port->FrameCount = mstp_port->Nmax_info_frames;
            mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
            /* Any retry of the data frame shall await the next entry */
            /* to the USE_TOKEN state. (Because of the length of the timeout,  */
            /* this transition will cause the token to be passed regardless */
            /* of the initial value of FrameCount.) */
            CDC_Transmit_FS("T", 1);
            transition_now = true;
        }
        else {
            if (mstp_port->ReceivedInvalidFrame == true) {
                /* InvalidFrame */
                /* error in frame reception */
                mstp_port->ReceivedInvalidFrame = false;
                mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                CDC_Transmit_FS("I", 1);
                transition_now = true;
            }
            else if (mstp_port->ReceivedValidFrame == true) {
                if (mstp_port->DestinationAddress == mstp_port->This_Station) {
                    /* What did we receive? */
                    switch (mstp_port->FrameType) {
                    case FRAME_TYPE_REPLY_POSTPONED:
                        /* ReceivedReplyPostponed */
                        CDC_Transmit_FS("P", 1);
                        mstp_port->master_state =
                            MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                        break;
                    case FRAME_TYPE_TEST_RESPONSE:
                        mstp_port->master_state =
                            MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                        break;
                    case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                        /* ReceivedReply */
                        /* or a proprietary type that indicates a reply */
                        /* indicate successful reception to the higher layers */
                        CDC_Transmit_FS("N", 1);
                                    (void) MSTP_Put_Receive(mstp_port);
                        mstp_port->master_state = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                        break;
                    default:
                        CDC_Transmit_FS("O", 1);
                        /* if proprietary frame was expected, you might
                           need to transition to DONE WITH TOKEN */
                        mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                        break;
                    }
                }
                else {
                    CDC_Transmit_FS("U", 1);
                    /* ReceivedUnexpectedFrame */
                    /* an unexpected frame was received */
                    /* This may indicate the presence of multiple tokens */
                    /* or a device that didn't see activity after passing */
                    /* a token (how lame!). Todo 1*/
                    /* Synchronize with the network. */
                    /* This action drops the token. */
                    mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                }
                mstp_port->ReceivedValidFrame = false;
                transition_now = true;
            }
        }
        break;

    case MSTP_MASTER_STATE_DONE_WITH_TOKEN:
        // CDC_Transmit_FS("C", 1);
        /* The DONE_WITH_TOKEN state either sends another data frame,  */
        /* passes the token, or initiates a Poll For Master cycle. */
        /* SendAnotherFrame */
        if (mstp_port->FrameCount < mstp_port->Nmax_info_frames) {
            /* then this node may send another information frame  */
            /* before passing the token.  */
            mstp_port->master_state = MSTP_MASTER_STATE_USE_TOKEN;
            transition_now = true;
        }
        else if ((mstp_port->SoleMaster == false) &&
            (mstp_port->Next_Station == mstp_port->This_Station)) {
            /* NextStationUnknown - added in Addendum 135-2008v-1 */
            /*  then the next station to which the token
               should be sent is unknown - so PollForMaster */
            mstp_port->Poll_Station = next_this_station;
                
            // this if for our logging so we calculate delta time based on correct station
            // mstp_port->DestinationAddress = mstp_port->Poll_Station;
            // priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                
            MSTP_Create_And_Send_Frame(
                mstp_port,
                FRAME_TYPE_POLL_FOR_MASTER,
                mstp_port->Poll_Station,
                mstp_port->This_Station,
                NULL,
                0);
            mstp_port->RetryCount = 0;
            mstp_port->master_state = MSTP_MASTER_STATE_POLL_FOR_MASTER;
        }
        else if (mstp_port->TokenCount < (Npoll - 1)) {
            /* Npoll changed in Errata SSPC-135-2004 */
            if ((mstp_port->SoleMaster == true) &&
                (mstp_port->Next_Station != next_this_station)) {
                /* SoleMaster */
                /* there are no other known master nodes to */
                /* which the token may be sent (true master-slave operation).  */
                mstp_port->FrameCount = 0;
                mstp_port->TokenCount++;
                mstp_port->master_state = MSTP_MASTER_STATE_USE_TOKEN;
                transition_now = true;
            }
            else {
                /* SendToken */
                /* Npoll changed in Errata SSPC-135-2004 */
                /* The comparison of NS and TS+1 eliminates the Poll For Master  */
                /* if there are no addresses between TS and NS, since there is no  */
                /* address at which a new master node may be found in that case. */
                mstp_port->TokenCount++;
                
                // this if for our logging so we calculate delta time based on correct station
//                mstp_port->DestinationAddress = mstp_port->Next_Station;
//                priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                
                /* transmit a Token frame to NS */
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_TOKEN,
                    mstp_port->Next_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                mstp_port->RetryCount = 0;
                mstp_port->EventCount = 0;
                mstp_port->master_state = MSTP_MASTER_STATE_PASS_TOKEN;
            }
        }
        else if (next_poll_station == mstp_port->Next_Station) {
            if (mstp_port->SoleMaster == true) {
                /* SoleMasterRestartMaintenancePFM */
                mstp_port->Poll_Station = next_next_station;
                
                // this if for our logging so we calculate delta time based on correct station
//                mstp_port->DestinationAddress = mstp_port->Poll_Station;
//                priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_POLL_FOR_MASTER,
                    mstp_port->Poll_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                /* no known successor node */
                mstp_port->Next_Station = mstp_port->This_Station;
                mstp_port->RetryCount = 0;
                /* changed in Errata SSPC-135-2004 */
                mstp_port->TokenCount = 1;
                /* mstp_port->EventCount = 0; removed in Addendum 135-2004d-8 */
                /* find a new successor to TS */
                mstp_port->master_state = MSTP_MASTER_STATE_POLL_FOR_MASTER;
            }
            else {
                /* ResetMaintenancePFM */
                mstp_port->Poll_Station = mstp_port->This_Station;
                
                // this if for our logging so we calculate delta time based on correct station
//                mstp_port->DestinationAddress = mstp_port->Poll_Station;
//                priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                
                /* transmit a Token frame to NS */
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_TOKEN,
                    mstp_port->Next_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                mstp_port->RetryCount = 0;
                /* changed in Errata SSPC-135-2004 */
                mstp_port->TokenCount = 1;
                mstp_port->EventCount = 0;
                mstp_port->master_state = MSTP_MASTER_STATE_PASS_TOKEN;
            }
        }
        else {
            /* SendMaintenancePFM */
            mstp_port->Poll_Station = next_poll_station;
            
            // this if for our logging so we calculate delta time based on correct station
//            mstp_port->DestinationAddress = mstp_port->Poll_Station;
//            priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                
            MSTP_Create_And_Send_Frame(
                mstp_port,
                FRAME_TYPE_POLL_FOR_MASTER,
                mstp_port->Poll_Station,
                mstp_port->This_Station,
                NULL,
                0);
            mstp_port->RetryCount = 0;
            mstp_port->master_state = MSTP_MASTER_STATE_POLL_FOR_MASTER;
        }
        break;

    case MSTP_MASTER_STATE_PASS_TOKEN:
        // CDC_Transmit_FS("P", 1);
        /* The PASS_TOKEN state listens for a successor to begin using */
        /* the token that this node has just attempted to pass. */
        if(SilenceTimer(mstp_port) <= Tusage_timeout) {
            if (mstp_port->EventCount >= Nmin_octets) {
                /* SawTokenUser */
                /* Assume that a frame has been sent by the new token user.  */
                /* Enter the IDLE state to process the frame. */
                mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                transition_now = true;
            }
        }
        else {
            if (mstp_port->RetryCount < Nretry_token) {

                bits_ScopeTrigger();
                sprintf(pbuf, "<M- Resending Token to %d\r\n", mstp_port->Next_Station);
                prints(pbuf);

                /* RetrySendToken */
                mstp_port->RetryCount++;
                /* Transmit a Token frame to NS */
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_TOKEN,
                    mstp_port->Next_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                mstp_port->EventCount = 0;
                /* re-enter the current state to listen for NS  */
                /* to begin using the token. */
            }
            else {
                /* FindNewSuccessor */
                /* Assume that NS has failed.  */
                /* note: if NS=TS-1, this node could send PFM to self! */
                mstp_port->Poll_Station = next_next_station;
                /* Transmit a Poll For Master frame to PS. */
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_POLL_FOR_MASTER,
                    mstp_port->Poll_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                /* no known successor node */
                mstp_port->Next_Station = mstp_port->This_Station;
                mstp_port->RetryCount = 0;
                mstp_port->TokenCount = 0;
                /* mstp_port->EventCount = 0; removed in Addendum 135-2004d-8 */
                /* find a new successor to TS */
                mstp_port->master_state = MSTP_MASTER_STATE_POLL_FOR_MASTER;
            }
        }
        break;

    case MSTP_MASTER_STATE_NO_TOKEN:
        sprintf(pbuf, "Token Lost by %d\r\n", lastOwnerOfToken);
        CDC_Transmit_FS( pbuf, strlen(pbuf) );
        /* The NO_TOKEN state is entered if mstp_port->SilenceTimer() becomes greater  */
        /* than Tno_token, indicating that there has been no network activity */
        /* for that period of time. The timeout is continued to determine  */
        /* whether or not this node may create a token. */
        my_timeout = Tno_token + (Tslot * mstp_port->This_Station);
        if (SilenceTimer(mstp_port) < my_timeout) {
            if (mstp_port->EventCount > Nmin_octets) {
                /* SawFrame */
                /* Some other node exists at a lower address.  */
                /* Enter the IDLE state to receive and process the incoming frame. */
                mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                transition_now = true;
            }
        }
        else {
            ns_timeout = Tno_token + (Tslot * (mstp_port->This_Station + 1));
            
            // mm_timeout in case we overshoot our Tslot due to back clock resolution, etc. Hard to test....
            // Karg's approach is OK, but our max_master may be less than someone else's max_master, so don't 
            // use that, else we risk a collission.  BMDA candidate. todo 1
            // mm_timeout = Tno_token + (Tslot * (mstp_port->Nmax_master + 1));
            mm_timeout = Tno_token + (Tslot * 127 );
            if ((SilenceTimer(mstp_port) < ns_timeout) ||
                (SilenceTimer(mstp_port) > mm_timeout)) {
                /* GenerateToken */
                /* Assume that this node is the lowest numerical address  */
                /* on the network and is empowered to create a token.  */
                mstp_port->Poll_Station = next_this_station;
                /* Transmit a Poll For Master frame to PS. */
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_POLL_FOR_MASTER,
                    mstp_port->Poll_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                /* indicate that the next station is unknown */
                mstp_port->Next_Station = mstp_port->This_Station;
                mstp_port->RetryCount = 0;
                mstp_port->TokenCount = 0;
                /* mstp_port->EventCount = 0;
                   removed Addendum 135-2004d-8 */
                /* enter the POLL_FOR_MASTER state
                   to find a new successor to TS. */
                mstp_port->master_state = MSTP_MASTER_STATE_POLL_FOR_MASTER;
            }
            else {
                /* We missed our time slot!
                   We should never get here unless
                   OS timer resolution is poor or we were busy */
                if (mstp_port->EventCount > Nmin_octets) {
                    /* SawFrame */
                    /* Some other node exists at a lower address.  */
                    /* Enter the IDLE state to receive and
                       process the incoming frame. */
                    mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                    transition_now = true;
                }
            }
        }
        break;

    case MSTP_MASTER_STATE_POLL_FOR_MASTER:
        // CDC_Transmit_FS("?", 1);
        /* In the POLL_FOR_MASTER state, the node listens for a reply to */
        /* a previously sent Poll For Master frame in order to find  */
        /* a successor node. */
        if (mstp_port->ReceivedValidFrame == true) {
            if ((mstp_port->DestinationAddress == mstp_port->This_Station)
                && (mstp_port->FrameType == FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER)) {
                /* ReceivedReplyToPFM */
                mstp_port->SoleMaster = false;
                mstp_port->Next_Station = mstp_port->SourceAddress;
                mstp_port->EventCount = 0;
                        
                //                        mstp_port->DestinationAddress = mstp_port->Next_Station;
                //                        priorFrameType = FRAME_TYPE_POLL_FOR_MASTER;
                        
                /* Transmit a Token frame to NS */
                // CDC_Transmit_FS("R", 1);
                MSTP_Create_And_Send_Frame(
                    mstp_port,
                    FRAME_TYPE_TOKEN,
                    mstp_port->Next_Station,
                    mstp_port->This_Station,
                    NULL,
                    0);
                mstp_port->Poll_Station = mstp_port->This_Station;
                mstp_port->TokenCount = 0;
                mstp_port->RetryCount = 0;
                mstp_port->master_state = MSTP_MASTER_STATE_PASS_TOKEN;
            }
            else {
                /* ReceivedUnexpectedFrame */
                /* An unexpected frame was received.  */
                /* This may indicate the presence of multiple tokens. */
                /* enter the IDLE state to synchronize with the network.  */
                /* This action drops the token. */
                CDC_Transmit_FS("X", 1);
                mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
                transition_now = true;
            }
            mstp_port->ReceivedValidFrame = false;
        }
        else if ((SilenceTimer(mstp_port) > Tusage_timeout) ||
            (mstp_port->ReceivedInvalidFrame == true)) {
            if (mstp_port->SoleMaster == true) {
                /* SoleMaster */
                /* There was no valid reply to the periodic poll  */
                /* by the sole known master for other masters. */
                mstp_port->FrameCount = 0;
                /* mstp_port->TokenCount++; removed in 2004 */
                mstp_port->master_state = MSTP_MASTER_STATE_USE_TOKEN;
                transition_now = true;
            }
            else {
                if (mstp_port->Next_Station != mstp_port->This_Station) {
                    /* DoneWithPFM */
                    /* There was no valid reply to the maintenance  */
                    /* poll for a master at address PS.  */
                    
                    //                    mstp_port->DestinationAddress = mstp_port->Next_Station;
                    //                    priorFrameType = FRAME_TYPE_TOKEN;
                    
                    mstp_port->EventCount = 0;
                    /* transmit a Token frame to NS */
                    MSTP_Create_And_Send_Frame(
                        mstp_port,
                        FRAME_TYPE_TOKEN,
                        mstp_port->Next_Station,
                        mstp_port->This_Station,
                        NULL,
                        0);
                    mstp_port->RetryCount = 0;
                    mstp_port->master_state = MSTP_MASTER_STATE_PASS_TOKEN;
                }
                else {
                    if (next_poll_station != mstp_port->This_Station) {
                        /* SendNextPFM */
                        mstp_port->Poll_Station = next_poll_station;
                        /* Transmit a Poll For Master frame to PS. */
                        MSTP_Create_And_Send_Frame(
                            mstp_port,
                            FRAME_TYPE_POLL_FOR_MASTER,
                            mstp_port->Poll_Station,
                            mstp_port->This_Station,
                            NULL,
                            0);
                        mstp_port->RetryCount = 0;
                        /* Re-enter the current state. */
                    }
                    else {
                        /* DeclareSoleMaster */
                        /* to indicate that this station is the only master */
                        mstp_port->SoleMaster = true;
                        mstp_port->FrameCount = 0;
                        mstp_port->master_state = MSTP_MASTER_STATE_USE_TOKEN;
                        transition_now = true;
                    }
                }
            }
            mstp_port->ReceivedInvalidFrame = false;
        }
        break;

    case MSTP_MASTER_STATE_ANSWER_DATA_REQUEST:
        /* The ANSWER_DATA_REQUEST state is entered when a  */
        /* BACnet Data Expecting Reply, a Test_Request, or  */
        /* a proprietary frame that expects a reply is received. */
        if (ll_GetCount( &mstp_port->mstpOutputQueuePtr)) {
            // if ( OS_Q_GetPtrCond( &PDU_Queue, (void **) &pkt )) {
            pkt = (struct mstp_pdu_packet *) ll_Pluck( &mstp_port->mstpOutputQueuePtr, NULL, matchMSTP);
            //                matched =
            //                    dlmstp_compare_data_expecting_reply(&InputBuffer[0],
            //                    DataLength, SourceAddress, pkt->dlcb->Handler_Transmit_Buffer, pkt->length,
            //                    pkt->destination_mac);
                            // whether this turns out to be a match or not, now that we have been flagged by the queue, need to free later.
                            // needToFree = true ;
        }
        else {
            pkt = NULL;
        }

        if (pkt != NULL) {
            /* Reply */
            /* If a reply is available from the higher layers  */
            /* within Treply_delay after the reception of the  */
            /* final octet of the requesting frame  */
            /* (the mechanism used to determine this is a local matter), */
            /* then call MSTP_Create_And_Send_Frame to transmit the reply frame  */
            /* and enter the IDLE state to wait for the next frame. */
            uint8_t frame_type;
            if (pkt->data_expecting_reply) {
                frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
            }
            else {
                frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
            }
                
            MSTP_Create_And_Send_Frame(
                mstp_port,
                frame_type, 
                pkt->destination_mac, 
                mstp_port->This_Station,
                pkt->dlcb->Handler_Transmit_Buffer, 
                pkt->dlcb->optr);
                    
            mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
            /* clear our flag we were holding for comparison */
            mstp_port->ReceivedValidFrame = false;
        }
        else if (SilenceTimer(mstp_port) > Treply_delay) {
            /* DeferredReply */
            /* If no reply will be available from the higher layers */
            /* within Treply_delay after the reception of the */
            /* final octet of the requesting frame (the mechanism */
            /* used to determine this is a local matter), */
            /* then an immediate reply is not possible. */
            /* Any reply shall wait until this node receives the token. */
            /* Call MSTP_Create_And_Send_Frame to transmit a Reply Postponed frame, */
            /* and enter the IDLE state. */
            MSTP_Create_And_Send_Frame(
                mstp_port,
                FRAME_TYPE_REPLY_POSTPONED,
                mstp_port->SourceAddress,
                mstp_port->This_Station,
                NULL,
                0);
                    
            mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
            /* clear our flag we were holding for comparison */
            mstp_port->ReceivedValidFrame = false;
        }

        if (pkt != NULL) {
            /* clear the queue */
            // OS_Q_Purge(&PDU_Queue);
            // and memory
            dlcb_free(pkt->dlcb);
            emm_free(pkt);
        }
        break;

    default:
        CDC_Transmit_FS("x", 1);
        mstp_port->master_state = MSTP_MASTER_STATE_IDLE;
        break;
    }

    return transition_now;
}

/* note: This_Station assumed to be set with the MAC address */
/* note: Nmax_info_frames assumed to be set (default=1) */
/* note: Nmax_master assumed to be set (default=127) */
/* note: InputBuffer and InputBufferSize assumed to be set */
/* note: OutputBuffer and OutputBufferSize assumed to be set */
/* note: SilenceTimer and SilenceTimerReset assumed to be set */
void MSTP_Init(
    volatile struct mstp_port_struct_t *mstp_port)
{
#if 0
    /* FIXME: you must point these buffers to actual byte buckets
       in the dlmstp function before calling this init. */
    mstp_port->InputBuffer = &InputBuffer[0];
    mstp_port->InputBufferSize = sizeof(InputBuffer);
    mstp_port->OutputBuffer = &OutputBuffer[0];
    mstp_port->OutputBufferSize = sizeof(OutputBuffer);
#endif
    /* FIXME: these are adjustable, so you must set these in dlmstp */
    mstp_port->Nmax_info_frames = DEFAULT_MAX_INFO_FRAMES;
    mstp_port->Nmax_master = DEFAULT_MAX_MASTER;         // todo
    /* FIXME: point to functions */
    mstp_port->receive_state = MSTP_RECEIVE_STATE_IDLE;
    mstp_port->master_state = MSTP_MASTER_STATE_INITIALIZE;
    mstp_port->ReceiveError = false;
    mstp_port->DataAvailable = false;
    mstp_port->DataRegister = 0;
    mstp_port->DataCRC = 0;
    mstp_port->DataLength = 0;
    mstp_port->DestinationAddress = 0;
    mstp_port->EventCount = 0;
    mstp_port->FrameType = FRAME_TYPE_TOKEN;
    mstp_port->FrameCount = 0;
    mstp_port->HeaderCRC = 0;
    mstp_port->Index = 0;
    mstp_port->This_Station = 51 ;                                  // todo - get from flash
    mstp_port->Next_Station = mstp_port->This_Station;
    mstp_port->Poll_Station = mstp_port->This_Station;
    mstp_port->ReceivedInvalidFrame = false;
    mstp_port->ReceivedValidFrame = false;
    mstp_port->ReceivedValidFrameNotForUs = false;
    mstp_port->RetryCount = 0;
    mstp_port->SoleMaster = false;
    mstp_port->SourceAddress = 0;
    mstp_port->TokenCount = 0;
    
    SilenceTimerReset(mstp_port);

    ll_Init(&mstp_port->mstpOutputQueuePtr, 100);
    
    // enable M4 cycle counter for timing
    DWT->CTRL |= 1;

}

