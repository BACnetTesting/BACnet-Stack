/**
* @file
* @author Steve Karg
* @date 2004


Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

July 1, 2017    BITS    Modifications to this file have been made in compliance
to original licensing.

This file contains changes made by BACnet Interoperability Testing
Services, Inc. These changes are subject to the permissions,
warranty terms and limitations above.
For more information: info@bac-test.com
For access to source code:  info@bac-test.com
or      www.github.com/bacnettesting/bacnet-stack

####COPYRIGHTEND####

*/
#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>

/**
* FIFO data structure
*
* @{
*/
struct fifo_buffer_t {
    /** first byte of data */
    volatile unsigned head;
    /** last byte of data */
    volatile unsigned tail;
    /** block of memory or array of data */
    volatile uint8_t *buffer;
    /** length of the data */
    unsigned buffer_len;
};
typedef struct fifo_buffer_t FIFO_BUFFER;
/** @} */

unsigned FIFO_Count(
    FIFO_BUFFER const *b);

bool FIFO_Full(
    FIFO_BUFFER const *b);

    bool FIFO_Available(
        FIFO_BUFFER const *b,
        unsigned count);

    bool FIFO_Empty(
        FIFO_BUFFER const *b);

    uint8_t FIFO_Peek(
        FIFO_BUFFER const *b);

    uint8_t FIFO_Get(
        FIFO_BUFFER * b);

    unsigned FIFO_Pull(
        FIFO_BUFFER * b,
        uint8_t * data_bytes,
        unsigned length);

    bool FIFO_Put(
        FIFO_BUFFER * b,
        uint8_t data_byte);

    bool FIFO_Add(
        FIFO_BUFFER * b,
        uint8_t * data_bytes,
        unsigned count);

    void FIFO_Flush(
        FIFO_BUFFER * b);

/* note: buffer_len must be a power of two */
    void FIFO_Init(
        FIFO_BUFFER * b,
        volatile uint8_t * buffer,
        unsigned buffer_len);

#ifdef TEST
#include "ctest.h"
void testFIFOBuffer(
    Test * pTest);
#endif

#endif
