/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 by Steve Karg

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

/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples.
   This library only uses a byte sized chunk.
   This library is designed for use in Interrupt Service Routines
   and so is declared as "static inline" */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * FIFO data structure
 *
 * @{
 */
typedef struct
{
    uint32_t    timestamp;
    uint8_t     inbyte;
} TS_CHAR ;
    
struct fifo_buffer_t {
    /** first byte of data */
    volatile unsigned head;
    /** last byte of data */
    volatile unsigned tail;
    /** block of memory or array of data */
    volatile TS_CHAR *buffer;
    /** length of the data */
    unsigned buffer_len;
};

typedef struct fifo_buffer_t FIFO_BUFFER;


unsigned FIFO_Count(
    FIFO_BUFFER const *b);

bool FIFO_Full(
    FIFO_BUFFER const *b);

bool FIFO_Available(
    FIFO_BUFFER const *b,
    unsigned count);

bool FIFO_Empty(
    FIFO_BUFFER const *b);

//TS_CHAR *FIFO_Peek(
//    FIFO_BUFFER const *b);

void FIFO_Get(
    FIFO_BUFFER * b,
    TS_CHAR *ch );

unsigned FIFO_Pull(
    FIFO_BUFFER * b,
    TS_CHAR * data_bytes,
    unsigned length);

bool FIFO_Put(
    FIFO_BUFFER *b,
    TS_CHAR *data_byte);

bool FIFO_Add(
    FIFO_BUFFER * b,
    TS_CHAR *data_bytes,
    unsigned count);

void FIFO_Flush(
    FIFO_BUFFER * b);

/* note: buffer_len must be a power of two */
void FIFO_Init(
    FIFO_BUFFER *b,
    volatile TS_CHAR * buffer,
    unsigned buffer_len);

#ifdef TEST
#include "ctest.h"
void testFIFOBuffer(
    Test * pTest);
#endif

#endif
