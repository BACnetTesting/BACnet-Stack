/*
 -------------------------------------------
 Copyright (C) 2004 Steve Karg

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
 
 
     Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

 *
 *   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
 *
 *   July 1, 2017    BITS    Modifications to this file have been made in compliance
 *                           with original licensing.
 *
 *
 ****************************************************************************************/

#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include "mstp.h"

void RS485_Set_Interface(
    char *ifname);

const char *RS485_Interface(
    void);

void RS485_Initialize(
    void);

void RS485_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,  /* port specific data */
    uint8_t * buffer,       /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes);       /* number of bytes of data (up to 501) */

// returns true if we have received a character
bool RS485_Check_UART_Data(
    volatile struct mstp_port_struct_t *mstp_port);

//uint32_t RS485_Get_Port_Baud_Rate(
//    volatile struct mstp_port_struct_t *mstp_port);

//uint32_t RS485_Get_Baud_Rate(
//    void);

//bool RS485_Set_Baud_Rate(
//    uint32_t baud);

bool rs485_byte_sent(
    volatile struct mstp_port_struct_t *mstp_port);
    
void rs485_byte_send(
    volatile struct mstp_port_struct_t *mstp_port,
    uint8_t tx_byte);

bool rs485_frame_sent(
    volatile struct mstp_port_struct_t *mstp_port);

void rs485_rts_enable(
    volatile struct mstp_port_struct_t *mstp_port,
    bool enable);

void RS485_Cleanup(
    void);

bool rs485_turnaround_elapsed(
    volatile struct mstp_port_struct_t *mstp_port);

#endif
