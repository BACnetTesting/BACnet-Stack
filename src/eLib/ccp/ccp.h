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

#ifndef CCP_H
#define CCP_H
#include "stdint.h"

typedef enum {
    CCP_PASSTHROUGH_QUEUE = 0,
    CCP_FTMQ_QUEUE,
    CCP_DEBUG_QUEUE,
    CCP_BACNET_QUEUE,
    CCP_COMMAND_QUEUE,
    CCP_CONSOLE,
} CCM_QUEUE_ID ;

#define CCP_COMMAND_LOOPBACK            0
#define CCP_COMMAND_RESET_SHORTSTACK    2
#define CCP_COMMAND_NEURON_RESET_PIN    3

#define CCP_COMMAND_EN_DEBUG_QUEUE      4
#define CCP_COMMAND_DIS_DEBUG_QUEUE     5

#define CCP_COMMAND_FTCLICK_HW_VER      6
#define CCP_COMMAND_FTCLICK_SW_VER      7

#define CCP_COMMAND_NODEID              8
#define CCP_COMMAND_NODEID_GET          1
#define CCP_COMMAND_BURST               9

// callback function pointers to be registered to specific queues
typedef void (*CCP_receive_cb_t)(uint8_t comm_id, uint8_t *data, int length);
// callbacks to register a comm interface (uart, spi, i2c)
typedef void (*CCP_comm_init_cb_t)();
typedef void (*CCP_comm_start_cb_t)();
typedef void (*CCP_comm_stop_cb_t)();
typedef void (*CCP_comm_poll_cb_t)();
typedef void (*CCP_comm_send_bytes_cb_t)(uint8_t *bytes, uint16_t length);
typedef void (*CCP_comm_read_bytes_cb_t)(uint8_t *bytes, uint16_t length);
typedef int (*CCP_comm_has_bytes_cb_t)();

typedef struct CCP_Comm_HAL {
  CCP_comm_init_cb_t init;
  CCP_comm_start_cb_t start;
  CCP_comm_stop_cb_t stop;
  CCP_comm_poll_cb_t poll;
  CCP_comm_send_bytes_cb_t send_bytes;
  CCP_comm_read_bytes_cb_t read_bytes;
  CCP_comm_has_bytes_cb_t has_bytes;
} CCP_Comm_HAL;

//CCP functions
void CCP_poll_1msec(); // call every msec to refresh internal timeout and to receive packets
int CCP_sendPacket(uint8_t comm_id, uint8_t queue, uint8_t *data, uint16_t length);
void CCP_register_callback(uint8_t queue, CCP_receive_cb_t cb);
int CCP_register_comm(CCP_Comm_HAL *comm); // returns comm id
void CCP_init();

#endif
