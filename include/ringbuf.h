/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stdbool.h>

/**
* ring buffer power of two alignment macro
*
* @{
*/
#ifndef NEXT_POWER_OF_2
#define B2(x)    (   (x) | (   (x) >> 1) )
#define B4(x)    ( B2(x) | ( B2(x) >> 2) )
#define B8(x)    ( B4(x) | ( B4(x) >> 4) )
#define B16(x)   ( B8(x) | ( B8(x) >> 8) )
#define B32(x)   (B16(x) | (B16(x) >>16) )
#define NEXT_POWER_OF_2(x) (B32((x)-1) + 1)
#endif
/** @} */

/**
* ring buffer data structure
*
* @{
*/
struct ring_buffer_t {
    /** block of memory or array of data */
    volatile uint8_t *buffer;
    /** how many bytes for each chunk */
    unsigned element_size;
    /** number of chunks of data */
    unsigned element_count;
    /** where the writes go */
    volatile unsigned head;
    /** where the reads come from */
    volatile unsigned tail;
    /* maximum depth reached */
    volatile unsigned depth;
};
typedef struct ring_buffer_t RING_BUFFER;
/** @} */


    unsigned Ringbuf_Count(RING_BUFFER const *b);
    unsigned Ringbuf_Depth(RING_BUFFER const *b);
    unsigned Ringbuf_Depth_Reset(RING_BUFFER *b);
    unsigned Ringbuf_Size(RING_BUFFER const *b);
    bool Ringbuf_Full(RING_BUFFER const *b);
    bool Ringbuf_Empty(RING_BUFFER const *b);
    /* tail */
    volatile uint8_t *Ringbuf_Peek(RING_BUFFER const *b);
    bool Ringbuf_Pop(RING_BUFFER * b,
        uint8_t * data_element);
    bool Ringbuf_Put_Front(RING_BUFFER * b,
        uint8_t * data_element);
    /* head */
    bool Ringbuf_Put(RING_BUFFER * b,
        uint8_t * data_element);
    /* pair of functions to use head memory directly */
    volatile uint8_t *Ringbuf_Data_Peek(RING_BUFFER * b);
    bool Ringbuf_Data_Put(RING_BUFFER * b, volatile uint8_t *data_element);
    /* Note: element_count must be a power of two */
    bool Ringbuf_Init(RING_BUFFER * b,
        volatile uint8_t * buffer,
        unsigned element_size,
        unsigned element_count);

#ifdef TEST
#include "ctest.h"
void testRingBufPowerOfTwo(Test * pTest);
void testRingBufSizeSmall(Test * pTest);
void testRingBufSizeLarge(Test * pTest);
void testRingBufSizeInvalid(Test * pTest);
#endif

#endif
