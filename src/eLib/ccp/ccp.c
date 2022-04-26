/****************************************************************************************
*
*   Copyright (C) 2020 ConnectEx, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   As a special exception, if other files instantiate templates or
*   use macros or inline functions from this file, or you compile
*   this file and link it with other works to produce a work based
*   on this file, this file does not by itself cause the resulting
*   work to be covered by the GNU General Public License. However
*   the source code for this file must still be made available in
*   accordance with section (3) of the GNU General Public License.
*
*   This exception does not invalidate any other reasons why a work
*   based on this file might be covered by the GNU General Public
*   License.
*
*   For more information: info@connect-ex.com
*
*   For access to source code :
*
*       info@connect-ex.com
*           or
*       github.com/ConnectEx/BACnet-Dev-Kit
*
****************************************************************************************/

#include "ccp.h"
#include "ccp_config_example.h"
#include "string.h"

// ---------------- CONSTANTS --------------------------------
#define CCP_TIMEOUT 1000 //ms

#define CCP_MAX_PAYLOAD 68
#define CCP_PREAMBLE_LEN 2
#define CCP_HEADER_LEN 3
#define CCP_CRC_LEN 2
#define CCP_MAX_PACKET (CCP_PREAMBLE_LEN + CCP_HEADER_LEN + CCP_MAX_PAYLOAD + CCP_CRC_LEN)
#define CCP_OVERHEAD_LEN (CCP_PREAMBLE_LEN + CCP_HEADER_LEN + CCP_CRC_LEN)
uint8_t CCP_PREAMBLE[2] = {'@','@'};

// -------------- CUSTOM TYPES ---------------------------------
typedef enum {IDLE,PREAMBLE,HEADER,DATA,CRC} CCP_States;

typedef struct CCP_Header {
  uint16_t packet_length;   //16 bit
  CCM_QUEUE_ID queue;       //8 bit
} CCP_Header;

typedef struct CCP_Packet {
  CCP_Header header;    // 24 bits
  uint8_t *payload;     // up to CCP_MAX_PAYLOAD
  uint16_t crc;         // 16 bits
} CCP_Packet;

typedef struct CCP_input {
  CCP_States state;
  int16_t timeout;
  uint8_t header_bytes;
  uint8_t data_bytes;
  uint8_t buffer[CCP_MAX_PACKET];
  uint8_t read_buffer[CCP_COMM_READ_BUFFER_LEN];
  CCP_Packet packet;
} CCP_input;

typedef struct CCP_output {
  uint8_t buffer[CCP_MAX_PACKET];
  uint8_t transfering;
} CCP_output;

typedef struct CCP_Comm {
  CCP_Comm_HAL hal;
  CCP_input input;
  CCP_output output;
} CCP_Comm;

typedef struct CCP_receive_callback {
  CCP_receive_cb_t receive;
  uint8_t queue;
  uint8_t data[CCP_MAX_PAYLOAD];
} CCP_receive_callback;

// ------------ PRIVATE FUNCTION PROTOTYPES ---------------------------------
void parse_byte(uint8_t b, int comm_id);
uint16_t CRC16 (const uint8_t *nData, uint16_t length);
uint16_t generate_packet(uint8_t buffer[], uint8_t queue,uint8_t payload[],uint16_t length);
void print_packet(CCP_Packet *packet);


// ------------- LIBRARY GLOBAL VARIABLES ----------------------------

int registered_comms = 0;
CCP_Comm comms[CCP_MAX_COMM];

int registered_callbacks = 0;
CCP_receive_callback callbacks[CCP_MAX_RECEIVE_CALLBACKS];


// ------------ PUBLIC FUNCTIONS -------------------------------------
void CCP_init() {
  ;
}

void CCP_register_callback(uint8_t queue, CCP_receive_cb_t cb) {

  if (registered_callbacks < CCP_MAX_RECEIVE_CALLBACKS) {
    callbacks[registered_callbacks].receive = cb;
    callbacks[registered_callbacks].queue = queue;
    registered_callbacks++;
  }
}

int CCP_register_comm(CCP_Comm_HAL *comm) { // returns comm id

  if (registered_comms < CCP_MAX_COMM) {
    comms[registered_comms].hal = *comm;
    // input init
    comms[registered_comms].input.timeout = 0;
    comms[registered_comms].input.state = IDLE;
    // output init
    comms[registered_comms].output.transfering = 0;

    return registered_comms++;
  }
  return -1;
}

void CCP_poll_1msec() {

  for (int i = 0; i < registered_comms; i++) { // poll each registered comm
    comms[i].input.timeout--;
    if (comms[i].input.timeout < 0)
      comms[i].input.timeout = 0;
    if (comms[i].input.timeout == 0) {
      comms[i].input.state = IDLE;
    }
    comms[i].hal.poll();
    int available = comms[i].hal.has_bytes();
    if (available > 0) {
      if (available > CCP_COMM_READ_BUFFER_LEN)
        available = CCP_COMM_READ_BUFFER_LEN;
      comms[i].hal.read_bytes(comms[i].input.read_buffer, available);
      for (int j = 0; j < available; j++)
        parse_byte(comms[i].input.read_buffer[j], i);
      comms[i].input.timeout = CCP_TIMEOUT;
    }
  }
}


//send the packet to serial
int CCP_sendPacket(uint8_t comm_id, uint8_t queue, uint8_t *data, uint16_t length)
{
  if (comms[comm_id].output.transfering == 0) {
    comms[comm_id].output.transfering = 1;
    uint16_t packet_length = generate_packet(comms[comm_id].output.buffer, queue, data, length);
    comms[comm_id].hal.send_bytes(comms[comm_id].output.buffer, packet_length);
    comms[comm_id].output.transfering = 0;
    return 0; // transfer started
  }
  return -1;  // comm busy, can't start a new transfer
}


//---------------- PRIVATE FUNCTIONS ----------------------------------------

void parse_byte(uint8_t b, int comm_id) {

  CCP_input *input = &(comms[comm_id].input);

  switch (input->state){
    case IDLE:
      if (b == CCP_PREAMBLE[0]) {
        // received first byte of CCP preamble
        input->buffer[0] = b;
        input->state = PREAMBLE;
        input->header_bytes = 0;
        input->data_bytes = 0;
      }
      break;

    case PREAMBLE:
      if (b == CCP_PREAMBLE[1]) {
        input->buffer[1] = b;
        input->state = HEADER;
      } else { // bad data, restart state machine
        input->state = IDLE;
      }
      break;

    case HEADER:
      if (input->header_bytes == 0) {
        input->buffer[CCP_PREAMBLE_LEN] = b;
        input->packet.header.packet_length = b;
        input->header_bytes++;
      } else if (input->header_bytes == 1) {
        input->buffer[CCP_PREAMBLE_LEN+1] = b;
        input->packet.header.packet_length |= ((uint16_t)(b)) << 8;
        if (input->packet.header.packet_length < CCP_OVERHEAD_LEN){ //bad packet, too small
            input->state = IDLE;
        } else if (input->packet.header.packet_length > CCP_MAX_PACKET - CCP_PREAMBLE_LEN){ // bad packet, too long
            input->state = IDLE;
        } else {
            input->header_bytes++;
        }
      } else if (input->header_bytes == 2) {
        input->buffer[CCP_PREAMBLE_LEN+2] = b;
        input->packet.header.queue = b;
        input->state = DATA;
      }
      break;

    case DATA:
      if (input->data_bytes < (input->packet.header.packet_length - (CCP_OVERHEAD_LEN)))
        input->buffer[CCP_PREAMBLE_LEN + CCP_HEADER_LEN + input->data_bytes++] = b;
      else if (input->data_bytes == (input->packet.header.packet_length - (CCP_OVERHEAD_LEN))) {
        // first crc byte
        input->buffer[input->packet.header.packet_length - CCP_CRC_LEN] = b;
        input->packet.crc = b;
        input->state = CRC;
        // debug print the received data
        /*for(uint16_t i = 0;i<remaining_data-sizeof(packet.crc);i++ ){
          LOG("Payload[");
          LOG(i);
          LOG("]:");
          packet.payload[i] = receive_byte();
          LOGN((char)packet.payload[i]);
        }*/
      }
      break;

    case CRC:
      input->buffer[input->packet.header.packet_length - CCP_CRC_LEN] |= ((uint16_t)(b)) << 8;;
      input->packet.crc |= ((uint16_t)(b)) << 8;
      // check crc
      uint16_t in_crc = CRC16(input->buffer, input->packet.header.packet_length - CCP_CRC_LEN);
      if (in_crc != input->packet.crc) {
        // bad crc, drop packet
        input->state = IDLE;
        break;
      } else {      // call received callback
        for (int i = 0; i < registered_callbacks; i++) {
          if (callbacks[i].queue == input->packet.header.queue) {
            memcpy(callbacks[i].data, (input->buffer) + CCP_PREAMBLE_LEN + CCP_HEADER_LEN, input->packet.header.packet_length - CCP_OVERHEAD_LEN);
            callbacks[i].receive(comm_id, callbacks[i].data, input->packet.header.packet_length - CCP_OVERHEAD_LEN);
            input->state = IDLE;
            break;
          }
        }
      }
      /*LOG("crc:");
      LOGN(input->packet.crc);

      if (check_crc(packet)){
        LOGN("Packet received correctly");
      } else {
        LOGN("ERROR! WRONG CRC");
      }*/
      break;

    default:
      input->state = IDLE;
      break;
  }
}

//returns the crc16 value(MODBUS) using tables
uint16_t CRC16 (const uint8_t *nData, uint16_t length){

  static const uint16_t crcTable[] = {
  0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
  0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
  0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
  0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
  0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
  0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
  0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
  0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
  0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
  0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
  0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
  0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
  0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
  0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
  0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
  0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
  0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
  0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
  0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
  0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
  0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
  0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
  0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
  0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
  0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
  0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
  0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
  0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
  0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
  0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
  0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
  0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

  uint8_t nTemp;
  uint16_t crc = 0xFFFF;

   while (length--)
   {
      nTemp = *nData++ ^ crc;
      crc >>= 8;
      crc ^= crcTable[nTemp];
   }
   return crc;
}


//Allocate and populate a new packet
uint16_t generate_packet(uint8_t *buffer, uint8_t queue,uint8_t payload[],uint16_t length){
  uint8_t *buff_ptr = buffer;
  // preamble
  memcpy(buff_ptr, CCP_PREAMBLE, CCP_PREAMBLE_LEN);
  buff_ptr += CCP_PREAMBLE_LEN;
  // header
  uint16_t packet_length = CCP_PREAMBLE_LEN + CCP_HEADER_LEN + length + CCP_CRC_LEN;
  buff_ptr[0] = (uint8_t) (packet_length & 0x00ff);
  buff_ptr[1] = (uint8_t) ((packet_length & 0xff00) >> 8);
  buff_ptr[2] = queue;
  // data
  buff_ptr += CCP_HEADER_LEN;
  memcpy(buff_ptr, payload, length);
  // crc
  buff_ptr += length;
  uint16_t crc = CRC16(buffer, packet_length - CCP_CRC_LEN);
  buff_ptr[0] = (uint8_t)(crc & 0x00ff);
  buff_ptr[1] = (uint8_t)((crc & 0xff00) >> 8);

  return packet_length;
}

/*
//print the packet content
void print_packet(CCP_Packet *packet){
  LOG("sizeof(Header):");
  LOGN(sizeof(Header), DEC);

  //LOGN("Printing Packet:");
  //LOG("lenght:");
  //LOGN(packet->header.packet_length);
  //LOG("queue:");
  //LOGN(packet->header.queue);
  uint16_t payload_size = packet->header.packet_length-sizeof(Header)-sizeof(packet->crc);
  for (uint8_t i = 0;i< payload_size;i++){
    //LOG("payload[");
    //LOG(i);
    //LOG("]:");
    //LOGN(packet->payload[i]);
  }
  LOG("crc:");
  LOGN(packet->crc, HEX);
}*/
