/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

    This file contains changes made by BACnet Interoperability Testing
    Services, Inc. These changes are subject to the permissions,
    warranty terms and limitations above.
    For more information: info@bac-test.com
    For access to source code:  info@bac-test.com
            or      www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include "bacdef.h"
//#include "bacaddr.h"
#include "dlmstp.h"
#include "rs485common.h"
#include "crc.h"
#include "npdu.h"
#include "bits.h"
#include "timerCommon.h"
#include "ringbuf.h"
#include "mstpdef.h"
// todo2 #include "automac.h"
#include "emm.h"
#include "datalink.h"
#include "ese.h"
#include "scope.h"

/* Number of MS/TP Packets Rx/Tx */
uint16_t MSTP_Packets = 0;

/* packet queues */
static DLMSTP_PACKET Receive_Packet;
// static HANDLE Receive_Packet_Flag;
/* mechanism to wait for a frame in state machine */
// HANDLE Received_Frame_Flag;
DLMSTP_PACKET Transmit_Packet;
/* local MS/TP port data - shared with RS-485 */
volatile struct mstp_port_struct_t MSTP_Port;
/* buffers needed by mstp port struct */
// static uint8_t TxBuffer[MAX_MPDU];
static uint8_t RxBuffer[MAX_MPDU_MSTP];
/* The minimum time without a DataAvailable or ReceiveError event */
/* that a node must wait for a station to begin replying to a */
/* confirmed request: 255 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 300 milliseconds.) */
uint16_t Treply_timeout = 260;
/* The minimum time without a DataAvailable or ReceiveError event that a */
/* node must wait for a remote node to begin using a token or replying to */
/* a Poll For Master frame: 20 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 100 milliseconds.) */
uint8_t Tusage_timeout = 50;

/* count must be a power of 2 for ringbuf library */
#ifndef MSTP_TRANSMIT_PACKET_COUNT
#define MSTP_TRANSMIT_PACKET_COUNT 1
#endif
static struct mstp_tx_packet Transmit_Buffer[MSTP_TRANSMIT_PACKET_COUNT];
RING_BUFFER Transmit_Byte_Ringbuffer;

/* data structure for MS/TP PDU Queue */
// moved to mstp.h
//struct mstp_pdu_packet {
//    LLIST_LB    ll_lb;          // must be first
//
//    bool data_expecting_reply;
//    uint8_t destination_mac;
//    // uint16_t length;
//    // uint8_t buffer[MAX_MPDU_MSTP];
//    DLCB *dlcb;
//};

extern MOW_TYPE priorMOW;
extern bool scopetrigger;

/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. [BTC] */
// static uint8_t Nmax_info_frames = MSTP_PDU_PACKET_COUNT;


bool dlmstp_init(
    char *ifname)
{
    unsigned long hThread = 0;
    uint32_t arg_value = 0;

    /* initialize packet queue */
    Receive_Packet.ready = false;
    Receive_Packet.pdu_len = 0;
    
    /* initialize hardware */
    timer_init();
    
    Ringbuf_Init(&Transmit_Byte_Ringbuffer,
        (uint8_t *) Transmit_Buffer,
        sizeof(struct mstp_tx_packet),
        MSTP_TRANSMIT_PACKET_COUNT);
    
    //    Ringbuf_Init(&PDU_Queue, (uint8_t *) & PDU_Buffer,
    //        sizeof(struct mstp_pdu_packet), MSTP_PDU_PACKET_COUNT);
        
    RS485_Initialize();

    // todo2    
//    automac_init();

    MSTP_Port.InputBuffer = &RxBuffer[0];
    MSTP_Port.InputBufferSize = sizeof(RxBuffer);
    // now on Queue MSTP_Port->OutputBuffer = &TxBuffer[0];
    // now on Queue MSTP_Port->OutputBufferSize = sizeof(TxBuffer);
    //    MSTP_Port.SilenceTimer = Timer_Silence;
    //    MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
    
    MSTP_Init(&MSTP_Port);
    
#if 0
    uint8_t data;

    /* FIXME: implement your data storage */
    data = 64; /* I2C_Read_Byte(
                   EEPROM_DEVICE_ADDRESS,
                   EEPROM_MSTP_MAC_ADDR); */
    if (data <= 127)
        MSTP_Port.This_Station = data;
    else
        dlmstp_set_my_address(DEFAULT_MAC_ADDRESS);
    /* FIXME: implement your data storage */
    data = 127; /* I2C_Read_Byte(
                   EEPROM_DEVICE_ADDRESS,
                   EEPROM_MSTP_MAX_MASTER_ADDR); */
    if ((data <= 127) && (data >= MSTP_Port.This_Station))
        MSTP_Port.Nmax_master = data;
    else
        dlmstp_set_max_master(DEFAULT_MAX_MASTER);
    /* FIXME: implement your data storage */
    data = 1;
    /* I2C_Read_Byte(
       EEPROM_DEVICE_ADDRESS,
       EEPROM_MSTP_MAX_INFO_FRAMES_ADDR); */
    if (data >= 1)
        MSTP_Port.Nmax_info_frames = data;
    else
        dlmstp_set_max_info_frames(DEFAULT_MAX_INFO_FRAMES);
#endif
    return true;
}


// Timer that indicates line silence
uint32_t SilenceTimer(
    volatile mstp_port_struct *mstp_port) 
{
    return timer_elapsed_time(&mstp_port->silenceTimer);
}


void SilenceTimerReset(
    volatile mstp_port_struct *mstp_port)
{
    timer_elapsed_start(&mstp_port->silenceTimer);
}


void dlmstp_cleanup(
    void)
{
    /* nothing to do for static buffers */
}


/* returns number of bytes sent on success, zero on failure */
int dlmstp_send_pdu(
    BACNET_ADDRESS * dest,          /* destination address */
    BACNET_NPCI_DATA * npci_data,   /* network information */
    uint8_t * pdu,                  /* any data to be sent - may be null */
    unsigned pdu_len)               /* number of bytes of data */
{
    DLCB *dlcb;
    
    if (npci_data->data_expecting_reply )
    {
        dlcb = alloc_dlcb_new_message('a', dest->mac[0] );
    }
    else
    {
        dlcb = alloc_dlcb_response('b', dest->mac[0] );
    }
    if (dlcb == NULL) return 0 ;

    if (dlcb->optr > MAX_MPDU_MSTP) {
        dlcb_free(dlcb);
        panic();
        return 0;
    }

#if ( BAC_DEBUG == 1 )
    dlcb_check(dlcb);
#endif
    
    dlcb->optr = pdu_len;
    memcpy(dlcb->Handler_Transmit_Buffer, pdu, pdu_len);

    // ----

    int bytes_sent = dlcb->optr;

    struct mstp_pdu_packet *pkt = (struct mstp_pdu_packet *) emm_scalloc('t', sizeof(struct mstp_pdu_packet));
    if (pkt == NULL) {
        dlcb_free(dlcb);
        return 0;
    }

    // hmm.. do we care todo1
    pkt->data_expecting_reply = npci_data->data_expecting_reply ;

    pkt->dlcb = dlcb;

    // pkt->length = dlcb-> ;
    if(dest->mac_len)
    {
        pkt->destination_mac = dest->mac[0];
    }
    else
    {
        /* mac_len = 0 is a broadcast address */
        pkt->destination_mac = MSTP_BROADCAST_ADDRESS;
    }

    if (!ll_Enqueue(&MSTP_Port.mstpOutputQueuePtr, pkt)) {
        // SendBTApanicMessage("mxxxx - MSTP output queue full");
        ese_enqueue_once(ese009_09_mstp_output_queue_full);
        dlcb_free(dlcb);
        emm_free(pkt);
        // slight rearrange due to possible race-condition
        return 0;
    }

    return bytes_sent;
}


uint16_t dlmstp_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,              /* PDU data */
    uint16_t max_pdu,           /* amount of space available in the PDU  */
    unsigned timeout)           /* milliseconds to wait for a packet */
{
    
    uint16_t pdu_len = 0;
    //    DWORD wait_status = 0;

        (void) max_pdu;
    /* see if there is a packet available, and a place
       to put the reply (if necessary) and process it */
    //    wait_status = WaitForSingleObject(Receive_Packet_Flag, timeout);
    //    if (wait_status == WAIT_OBJECT_0) {
    if(Receive_Packet.ready) {
        if (Receive_Packet.pdu_len) {
            MSTP_Packets++;
            if (src) {
                memmove(src,
                    &Receive_Packet.address,
                    sizeof(Receive_Packet.address));
            }
            if (pdu) {
                memmove(pdu,
                    &Receive_Packet.pdu,
                    sizeof(Receive_Packet.pdu));
            }
            pdu_len = Receive_Packet.pdu_len;
        }
        Receive_Packet.ready = false;
    }
    //    }

        return pdu_len;
}



void dlmstp_fill_bacnet_address(
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

/* for the MS/TP state machine to use for putting received data */
uint16_t MSTP_Put_Receive(
    volatile struct mstp_port_struct_t *mstp_port)
{
    uint16_t pdu_len = 0;
    //    bool rc;

    if(!Receive_Packet.ready) {
        /* bounds check - maybe this should send an abort? */
        pdu_len = mstp_port->DataLength;
        if (pdu_len > sizeof(Receive_Packet.pdu))
        {
            pdu_len = sizeof(Receive_Packet.pdu);
        }
        memmove((void *) &Receive_Packet.pdu[0],
            (void *) &mstp_port->InputBuffer[0],
            pdu_len);
        dlmstp_fill_bacnet_address(&Receive_Packet.address,
            mstp_port->SourceAddress);
        Receive_Packet.pdu_len = mstp_port->DataLength;
        Receive_Packet.ready = true;
        //        rc = ReleaseSemaphore(Receive_Packet_Flag, 1, NULL);
        //        (void) rc;
    }

    return pdu_len;
}

/* for the MS/TP state machine to use for getting data to send */
/* Return: amount of PDU data */
//uint16_t MSTP_Get_Send(
//    volatile struct mstp_port_struct_t * mstp_port,
//    unsigned timeout)
//{
//    /* milliseconds to wait for a packet */
//    uint16_t pdu_len = 0;
//    uint8_t destination = 0; /* destination address */
//
//    (void) timeout;
//    if (!Transmit_Packet.ready) {
//        return 0;
//    }
//    /* load destination MAC address */
//    if (Transmit_Packet.address.mac_len) {
//        destination = Transmit_Packet.address.mac[0];
//    }
//    else {
//        destination = MSTP_BROADCAST_ADDRESS;
//    }
//    
//    if ((MAX_MAC_HEADER_MSTP + Transmit_Packet.pdu_len) > MAX_MPDU_MSTP) {
//        // todo 0 panic 
//        return 0;
//    }
//    
//    /* convert the PDU into the MSTP Frame */
//    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],
//        /* <-- loading this */
//        sizeof( mstp_port->OutputBuffer ),
//        Transmit_Packet.frame_type,
//        destination,
//        mstp_port->This_Station,
//        &Transmit_Packet.pdu[0],
//        Transmit_Packet.pdu_len);
//    Transmit_Packet.ready = false;
//
//    return pdu_len;
//}


static bool dlmstp_compare_data_expecting_reply(
    uint8_t * request_pdu,
    uint16_t request_pdu_len,
    uint8_t src_address,
    uint8_t * reply_pdu,
    uint16_t reply_pdu_len,
    BACNET_ADDRESS * dest_address)
{
    uint16_t offset;
    /* One way to check the message is to compare NPDU
       src, dest, along with the APDU type, invoke id.
       Seems a bit overkill */
    struct DER_compare_t {
        BACNET_NPCI_DATA npci_data;
        BACNET_ADDRESS address;
        uint8_t pdu_type;
        uint8_t invoke_id;
        uint8_t service_choice;
    };
    struct DER_compare_t request;
    struct DER_compare_t reply;

    /* unused parameters */
    request_pdu_len = request_pdu_len;
    reply_pdu_len = reply_pdu_len;
    /* decode the request data */
    request.address.mac[0] = src_address;
    request.address.mac_len = 1;
    offset =
        (uint16_t) npdu_decode(&request_pdu[0],
        NULL,
        &request.address,
        &request.npci_data);
    if (request.npci_data.network_layer_message) {
        return false;
    }
    request.pdu_type = request_pdu[offset] & 0xF0;
    if (request.pdu_type != PDU_TYPE_CONFIRMED_SERVICE_REQUEST) {
        return false;
    }
    request.invoke_id = request_pdu[offset + 2];
    /* segmented message? */
    if (request_pdu[offset] & BIT3)
        request.service_choice = request_pdu[offset + 5];
    else
        request.service_choice = request_pdu[offset + 3];
    /* decode the reply data */
    bacnet_address_copy(&reply.address, dest_address);
    offset =
        (uint16_t) npdu_decode(&reply_pdu[0],
        &reply.address,
        NULL,
        &reply.npci_data);
    if (reply.npci_data.network_layer_message) {
        return false;
    }
    /* reply could be a lot of things:
       confirmed, simple ack, abort, reject, error */
    reply.pdu_type = reply_pdu[offset] & 0xF0;
    switch (reply.pdu_type) {
    case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
        reply.invoke_id = reply_pdu[offset + 2];
        /* segmented message? */
        if (reply_pdu[offset] & BIT3)
            reply.service_choice = reply_pdu[offset + 5];
        else
            reply.service_choice = reply_pdu[offset + 3];
        break;
    case PDU_TYPE_SIMPLE_ACK:
        reply.invoke_id = reply_pdu[offset + 1];
        reply.service_choice = reply_pdu[offset + 2];
        break;
    case PDU_TYPE_COMPLEX_ACK:
        reply.invoke_id = reply_pdu[offset + 1];
        /* segmented message? */
        if (reply_pdu[offset] & BIT3)
            reply.service_choice = reply_pdu[offset + 4];
        else
            reply.service_choice = reply_pdu[offset + 2];
        break;
    case PDU_TYPE_ERROR:
        reply.invoke_id = reply_pdu[offset + 1];
        reply.service_choice = reply_pdu[offset + 2];
        break;
    case PDU_TYPE_REJECT:
    case PDU_TYPE_ABORT:
        reply.invoke_id = reply_pdu[offset + 1];
        break;
    default:
        return false;
    }
    /* these don't have service choice included */
    if ((reply.pdu_type == PDU_TYPE_REJECT) ||
        (reply.pdu_type == PDU_TYPE_ABORT)) {
        if (request.invoke_id != reply.invoke_id) {
            return false;
        }
    }
    else {
        if (request.invoke_id != reply.invoke_id) {
            return false;
        }
        if (request.service_choice != reply.service_choice) {
            return false;
        }
    }
    if (request.npci_data.protocol_version != reply.npci_data.protocol_version) {
        return false;
    }
#if 0
    /* the NDPU priority doesn't get passed through the stack, and
       all outgoing messages have NORMAL priority */
    if (request.npci_data.priority != reply.npci_data.priority) {
        return false;
    }
#endif
    
    if (!bacnet_address_same(&request.address, &reply.address)) {
        return false;
    }

    return true;
}


struct mstp_tx_packet *pkt;
volatile struct  mstp_port_struct_t *tx_mstp_port;
MSTP_TX_STATE tx_mstp_state = MSTP_TX_STATE_IDLE;

static bool MSTP_Transmit_FSM(
    volatile mstp_port_struct *mstp_port )
{

    tx_mstp_port = mstp_port;

MSTP_TX_START:
    switch (tx_mstp_state) {
    case MSTP_TX_STATE_IDLE:
        if (!Ringbuf_Empty(&Transmit_Byte_Ringbuffer)) {
            /* get the packet - but don't remove it from queue */
            pkt = (struct mstp_tx_packet *)
                Ringbuf_Peek(&Transmit_Byte_Ringbuffer);
            tx_mstp_state = MSTP_TX_STATE_SILENCE_WAIT;
        }
        break;

    case MSTP_TX_STATE_SILENCE_WAIT:
        if (rs485_turnaround_elapsed(mstp_port)) {

            if (scopetrigger) {
                bits_ScopeTrigger();
                scopetrigger = false;
            }

            rs485_rts_enable(mstp_port, true);
            pkt->index = 0;
            rs485_byte_send(mstp_port, pkt->buffer[pkt->index]);
            tx_mstp_state = MSTP_TX_STATE_SEND_WAIT;
            /* optimize a little - for slower CPUs */
            goto MSTP_TX_START;
        }
        break;

    case MSTP_TX_STATE_SEND_WAIT:
        break;
        // the rest will be done by the Tx Interrupt handler
        //    if (rs485_byte_sent(mstp_port)) {
    //        pkt->index++;
    //        if (pkt->index < pkt->length) {
    //            /* send next byte */
    //            rs485_byte_send(mstp_port, pkt->buffer[pkt->index]);
    //            /* optimize a little - for slower CPUs */
    //            goto MSTP_TX_START;
    //        }
    //        else {
    //            state = MSTP_TX_STATE_STOP;
    //        }
    //    }
    //    break;
    //case MSTP_TX_STATE_STOP:
    //    if (rs485_byte_sent(mstp_port) && rs485_frame_sent(mstp_port)) {
    //        rs485_rts_enable(mstp_port, false);
    //        /* remove the packet from the queue */
    //        (void) Ringbuf_Pop(&Transmit_Queue, NULL);
    //        state = MSTP_TX_STATE_IDLE;
    //    }
    //    break;
    //default:
    //    state = MSTP_TX_STATE_IDLE;
    //    break;
    }

    return (tx_mstp_state != MSTP_TX_STATE_IDLE);
}

extern bool scopetrigger ;

void dllmstp_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,
    /* port to send from */
    uint8_t *data,                                    
    uint16_t data_len)
{
    struct mstp_tx_packet *pkt;
    pkt = (struct mstp_tx_packet *) Ringbuf_Data_Peek(&Transmit_Byte_Ringbuffer);
    if (pkt) {
        for (int i = 0; i < data_len; i++) {
            pkt->buffer[i] = data[i];
        }
        pkt->length = data_len;
        Ringbuf_Data_Put(&Transmit_Byte_Ringbuffer, (uint8_t *)pkt);
    }
}


/* Get the reply to a DATA_EXPECTING_REPLY frame, or nothing */
uint16_t MSTP_Get_Reply(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{
    /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0; /* return value */
    uint8_t destination = 0; /* destination address */
    bool matched = false;

    (void) timeout;
    if (!Transmit_Packet.ready) {
        return 0;
    }
    /* load destination MAC address */
    if (Transmit_Packet.address.mac_len == 1) {
        destination = Transmit_Packet.address.mac[0];
    }
    else {
        return 0;
    }
    if ((MAX_MAC_HEADER_MSTP + Transmit_Packet.pdu_len) > MAX_MPDU_MSTP) {
        return 0;
    }
    /* is this the reply to the DER? */
    matched =
        dlmstp_compare_data_expecting_reply(&mstp_port->InputBuffer[0],
        mstp_port->DataLength,
        mstp_port->SourceAddress,
        &Transmit_Packet.pdu[0],
        Transmit_Packet.pdu_len,
        &Transmit_Packet.address);
    if (!matched)
        return 0;
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],
        /* <-- loading this */
        sizeof ( mstp_port->OutputBuffer),
        Transmit_Packet.frame_type,
        destination,
        mstp_port->This_Station,
        &Transmit_Packet.pdu[0],
        Transmit_Packet.pdu_len);
    Transmit_Packet.ready = false;

    return pdu_len;
}


#if 0
// TODO LATER   
/* master node FSM states */
typedef enum {
    AUTOMAC_STATE_IDLE = 0,
    AUTOMAC_STATE_PFM = 1,
    AUTOMAC_STATE_TOKEN = 2,
    AUTOMAC_STATE_TESTING = 3,
    AUTOMAC_STATE_CONFIRM = 4
} AUTOMAC_STATE;
/* buffer used to send and validate a response - size is min APDU size */
static uint8_t AutoMAC_Test_Buffer[50];
void dlmstp_automac_hander(
    void)
{
    static AUTOMAC_STATE state = AUTOMAC_STATE_IDLE;
    uint8_t mac = 0;
    uint32_t serial_number = 0;
    uint16_t vendor_id = 0;
    bool take_address = false;
    bool start_over = false;

    switch (state) {
    case AUTOMAC_STATE_IDLE:
        if ((MSTP_Flag.ReceivedValidFrame) ||
            (MSTP_Flag.ReceivedValidFrameNotForUs)) {
            MSTP_Flag.ReceivedValidFrame = false;
            MSTP_Flag.ReceivedValidFrameNotForUs = false;
            /* store stats until we get a MAC */
            automac_emitter_set(SourceAddress);
            switch (FrameType) {
            case FRAME_TYPE_TOKEN:
                automac_token_set(SourceAddress);
                break;
            case FRAME_TYPE_POLL_FOR_MASTER:
                automac_pfm_set(DestinationAddress);
                break;
            default:
                break;
            }
        }
        else if (MSTP_Flag.ReceivedInvalidFrame) {
            MSTP_Flag.ReceivedInvalidFrame = false;
        }
        else if (automac_pfm_cycle_complete()) {
            mac = automac_free_address_random();
            if (automac_free_address_valid(mac)) {
                automac_address_set(mac);
                state = AUTOMAC_STATE_PFM;
            }
            else {
                /* start over again */
                automac_init();
                state = AUTOMAC_STATE_IDLE;
            }
        }
        else if (rs485_silence_elapsed(automac_time_slot())) {
            /*  long silence indicates we are alone or
               with other silent devices */
            SourceAddress = automac_address();
            state = AUTOMAC_STATE_TESTING;
        }
        break;
    case AUTOMAC_STATE_PFM:
        if ((MSTP_Flag.ReceivedValidFrame) ||
            (MSTP_Flag.ReceivedValidFrameNotForUs)) {
            MSTP_Flag.ReceivedValidFrame = false;
            MSTP_Flag.ReceivedValidFrameNotForUs = false;
            /* store stats until we get a MAC */
            switch (FrameType) {
            case FRAME_TYPE_POLL_FOR_MASTER:
                mac = automac_address();
                if (mac == SourceAddress) {
                    /* start over again */
                    automac_init();
                    state = AUTOMAC_STATE_IDLE;
                }
                else if (mac == DestinationAddress) {
                    MSTP_Send_Frame
                        (FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER,
                        SourceAddress,
                        mac,
                        NULL,
                        0);
                    state = AUTOMAC_STATE_TOKEN;
                }
                break;
            case FRAME_TYPE_TEST_REQUEST:
                mac = automac_address();
                if ((mac == DestinationAddress) ||
                    (mac == SourceAddress)) {
                    /* start over again */
                    automac_init();
                    state = AUTOMAC_STATE_IDLE;
                }
                break;
            case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
            case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
            case FRAME_TYPE_TEST_RESPONSE:
            case FRAME_TYPE_TOKEN:
                mac = automac_address();
                if (mac == SourceAddress) {
                    /* start over again */
                    automac_init();
                    state = AUTOMAC_STATE_IDLE;
                }
                break;
            default:
                break;
            }
        }
        else if (MSTP_Flag.ReceivedInvalidFrame) {
            MSTP_Flag.ReceivedInvalidFrame = false;
        }
        else if (rs485_silence_elapsed(automac_time_slot())) {
            /* start over again */
            automac_init();
            state = AUTOMAC_STATE_IDLE;
        }
        break;
    case AUTOMAC_STATE_TOKEN:
        if ((MSTP_Flag.ReceivedValidFrame) ||
            (MSTP_Flag.ReceivedValidFrameNotForUs)) {
            MSTP_Flag.ReceivedValidFrame = false;
            MSTP_Flag.ReceivedValidFrameNotForUs = false;
            switch (FrameType) {
            case FRAME_TYPE_TOKEN:
                mac = automac_address();
                if (mac == SourceAddress) {
                    /* start over again */
                    automac_init();
                    state = AUTOMAC_STATE_IDLE;
                }
                else if (mac == DestinationAddress) {
                    state = AUTOMAC_STATE_TESTING;
                }
                break;
            default:
                /* start over again */
                automac_init();
                state = AUTOMAC_STATE_IDLE;
                break;
            }
        }
        else if (MSTP_Flag.ReceivedInvalidFrame) {
            MSTP_Flag.ReceivedInvalidFrame = false;
        }
        else if (rs485_silence_elapsed(automac_time_slot())) {
            /* start over again */
            automac_init();
            state = AUTOMAC_STATE_IDLE;
        }
        break;
    case AUTOMAC_STATE_TESTING:
        /* I have the token - confirm my MAC with a quick test */
        mac = automac_address();
        vendor_id = Device_Vendor_Identifier();
        encode_unsigned16(&AutoMAC_Test_Buffer[0], vendor_id);
        serial_number = Device_Object_Instance_Number();
        encode_unsigned32(&AutoMAC_Test_Buffer[2], serial_number);
        MSTP_Send_Frame(FRAME_TYPE_TEST_REQUEST,
            SourceAddress,
            mac,
            &AutoMAC_Test_Buffer[0],
            6);
        state = AUTOMAC_STATE_CONFIRM;
        break;
    case AUTOMAC_STATE_CONFIRM:
        /* we may timeout if our chosen MAC is unique */
        if (MSTP_Flag.ReceivedInvalidFrame) {
            MSTP_Flag.ReceivedInvalidFrame = false;
            start_over = true;
        }
        else if ((MSTP_Flag.ReceivedValidFrame) ||
            (MSTP_Flag.ReceivedValidFrameNotForUs)) {
            MSTP_Flag.ReceivedValidFrame = false;
            MSTP_Flag.ReceivedValidFrameNotForUs = false;
            mac = automac_address();
            if ((mac == DestinationAddress) && (DataLength >= 6)) {
                decode_unsigned16(&InputBuffer[0], &vendor_id);
                decode_unsigned32(&InputBuffer[2], &serial_number);
                if ((vendor_id == Device_Vendor_Identifier()) &&
                    (serial_number == Device_Object_Instance_Number())) {
                    take_address = true;
                }
                else {
                    start_over = true;
                }
            }
            else {
                start_over = true;
            }
        }
        else if (rs485_silence_elapsed(300)) {
            /* use maximum possible value for Treply_timeout */
            /* in case validating device doesn't support Test Request */
            /* no response and no collission */
            take_address = true;
        }
        if (take_address) {
            /* take the address */
            This_Station = automac_address();
            DestinationAddress = automac_next_station(This_Station);
            if (DestinationAddress < 128) {
                MSTP_Send_Frame(FRAME_TYPE_TOKEN,
                    DestinationAddress,
                    This_Station,
                    NULL,
                    0);
            }
            state = AUTOMAC_STATE_IDLE;
        }
        else if (start_over) {
            /* start over again */
            automac_init();
            state = AUTOMAC_STATE_IDLE;
        }
        break;
    default:
        break;
    }
}
#endif

// call this as often as possible, 1 to 5ms _at least_, per the spec.
void dlmstp_tick(
    volatile struct mstp_port_struct_t *mstp_port)
{
    bool transmitting = false;
    bool run_master = false;

#if 0
    if (This_Station == 255) {
        automac_enabled_set(true);
    }
#endif
    if (mstp_port->receive_state == MSTP_RECEIVE_STATE_IDLE) {
        transmitting = MSTP_Transmit_FSM(mstp_port);
    }

    if (transmitting == false) {

        if (mstp_port->ReceivedValidFrame == false &&
            mstp_port->ReceivedValidFrameNotForUs == false &&
            mstp_port->ReceivedInvalidFrame == false) {
            /* only do receive state machine while we don't have a frame */
            // and then do it at least once (for the timers)
            // and keep repeating if there are more characters to process,
            // unless of course, we end up receiving a whole frame..
            RS485_Check_UART_Data(mstp_port);    
            do
            {
                MSTP_Receive_Frame_FSM(mstp_port);
            } while (
                mstp_port->ReceivedValidFrame == false &&
                mstp_port->ReceivedValidFrameNotForUs == false &&
                mstp_port->ReceivedInvalidFrame == false &&
                    RS485_Check_UART_Data(mstp_port))
        ;
        }
    }
    
#if 0    
    // todo1 temp
    mstp_port->ReceivedInvalidFrame = false;
    mstp_port->ReceivedValidFrame = false;
    mstp_port->ReceivedValidFrameNotForUs = false;
    return;
#endif  

    if (mstp_port->btaReceivedValidFrame) {
        mstp_port->btaReceivedValidFrame = false;
        // done its job - 48us HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
        if (mstp_port->DataLength > 0) {
        }
        if (mstp_port->DataLength > 0) {
            mstp_port->DataLength = mstp_port->DataLength;
        }
        SendBTAmstpPayload(
                           1,
                           false, // rx 
                           mstp_port->DataLength, mstp_port->FrameType, mstp_port->DestinationAddress,
                           mstp_port->SourceAddress, mstp_port->InputBuffer );
        // done its job - 48us HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    }

    
    /* only do master state machine while rx is idle */
    if ((mstp_port->receive_state == MSTP_RECEIVE_STATE_IDLE) && (transmitting == false)) {
        //        if ((mstp_port->This_Station != 255) && (mstp_port->ReceivedValidFrameNotForUs)) {
        //            mstp_port->ReceivedValidFrameNotForUs = false;
        //            
        //            if ((mstp_port->SourceAddress == mstp_port->This_Station)) {
        //                 && automac_enabled()) {
        //                /* duplicate MAC on the wire */
        //                automac_init();
        //                mstp_port->This_Station = 255;
        //            }
        //        }
        //        else {

        // have to de-optimize the compiler here...
        bool flag = true;
        while( flag ) {
            flag = MSTP_Master_Node_FSM(mstp_port);
            /* do nothing while some states fast transition */
        }
        //        }
    }
#if 0     
    if (This_Station != 255) {
        /* if there is a packet that needs processed, do it now. */
        if (MSTP_Flag.ReceivePacketPending) {
            MSTP_Flag.ReceivePacketPending = false;
            pdu_len = DataLength;
            src->mac_len = 1;
            src->mac[0] = SourceAddress;
            /* data is already in the pdu pointer */
        }
    }
#endif        // todo1 temp
    mstp_port->ReceivedInvalidFrame = false;
    mstp_port->ReceivedValidFrame = false;
    mstp_port->ReceivedValidFrameNotForUs = false;
    
    //    CDC_Transmit_FS("-", 1);
}


void dlmstp_set_mac_address(
    uint8_t mac_address)
{
    /* Master Nodes can only have address 0-127 */
    if (mac_address <= 127) {
        MSTP_Port.This_Station = mac_address;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           mac_address,
           EEPROM_MSTP_MAC_ADDR); */
        if (mac_address > MSTP_Port.Nmax_master)
            dlmstp_set_max_master(mac_address);
    }

}

uint8_t dlmstp_mac_address(
    void)
{
    return MSTP_Port.This_Station;
}

/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. */
void dlmstp_set_max_info_frames(
    uint8_t max_info_frames)
{
    if (max_info_frames >= 1) {
        MSTP_Port.Nmax_info_frames = max_info_frames;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           (uint8_t)max_info_frames,
           EEPROM_MSTP_MAX_INFO_FRAMES_ADDR); */
    }

}

uint8_t dlmstp_max_info_frames(
    void)
{
    return MSTP_Port.Nmax_info_frames;
}

/* This parameter represents the value of the Max_Master property of the */
/* node's Device object. The value of Max_Master specifies the highest */
/* allowable address for master nodes. The value of Max_Master shall be */
/* less than or equal to 127. If Max_Master is not writable in a node, */
/* its value shall be 127. */
void dlmstp_set_max_master(
    uint8_t max_master)
{
    if (max_master <= 127) {
        if (MSTP_Port.This_Station <= max_master) {
            MSTP_Port.Nmax_master = max_master;
            /* FIXME: implement your data storage */
            /* I2C_Write_Byte(
               EEPROM_DEVICE_ADDRESS,
               max_master,
               EEPROM_MSTP_MAX_MASTER_ADDR); */
        }
    }

}

uint8_t dlmstp_max_master(
    void)
{
    return MSTP_Port.Nmax_master;
}

/* RS485 Baud Rate 9600, 19200, 38400, 57600, 115200 */
void dlmstp_set_baud_rate(
    uint32_t baud)
{
    MSTP_Port.Baud_Rate = baud;
//    RS485_Set_Baud_Rate(baud);
}

//uint32_t dlmstp_baud_rate(
//    void)
//{
//    return RS485_Get_Baud_Rate();
//}

void dlmstp_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0; /* counter */

    my_address->mac_len = 1;
    my_address->mac[0] = MSTP_Port.This_Station;
    my_address->net = 0; /* local only, no routing */
    my_address->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        my_address->adr[i] = 0;
    }

}

void dlmstp_get_broadcast_address(
    BACNET_ADDRESS * dest)  /* destination address */
{
    dest->mac_len = 1;
    dest->mac[0] = MSTP_BROADCAST_ADDRESS;
    dest->net = BACNET_BROADCAST_NETWORK;
    dest->len = 0; /* always zero when DNET is broadcast */
    for (int i = 0; i < MAX_MAC_LEN; i++) {
        dest->adr[i] = 0;
    }
}


