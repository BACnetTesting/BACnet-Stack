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

#ifndef SBUF_H
#define SBUF_H

/* Functional Description: Static buffer library for deeply
   embedded system. See the unit tests for usage examples. */

#include <stdint.h>
#include <stdbool.h>
#include "bacnet/bacnet_stack_exports.h"

struct static_buffer_t {
    uint8_t *data;      /* block of memory or array of data */
    unsigned size;      /* actual size, in bytes, of the block of data */
    unsigned count;     /* number of bytes in use */
    unsigned processed; // when decoding incoming buffer for example, bytes already processed. (Allowing us to calculate remaining pdu length for example.
};

typedef struct static_buffer_t STATIC_BUFFER;

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus_disable */

    BACNET_STACK_EXPORT
    void sbuf_init(
        STATIC_BUFFER * b,      /* static buffer structure */
        uint8_t *data,          /* data block */
        unsigned size);         /* actual size, in bytes, of the data block or array of data */

    /* returns true if size==0, false if size > 0 */
    BACNET_STACK_EXPORT
    bool sbuf_empty(
        STATIC_BUFFER const *b);

    /* returns the data block, or NULL if not initialized */
    BACNET_STACK_EXPORT
    uint8_t *sbuf_data(
        STATIC_BUFFER const *b);

    /* returns the max size of the data block */
    BACNET_STACK_EXPORT
    unsigned sbuf_size(
        STATIC_BUFFER * b);
    /* returns the number of bytes used in the data block */
    BACNET_STACK_EXPORT
    unsigned sbuf_count(
        STATIC_BUFFER * b);
    /* returns the number of bytes used in the data block */
    unsigned sbuf_remaining(
        STATIC_BUFFER* b);

    /* returns true if successful, false if not enough room to append data */
    BACNET_STACK_EXPORT
    bool sbuf_put(
        STATIC_BUFFER * b,      /* static buffer structure */
        unsigned offset,        /* where to start */
        uint8_t *data,          /* data to add */
        unsigned data_size);    /* how many to add */

    /* returns true if successful, false if not enough room to append data */
    BACNET_STACK_EXPORT
    bool sbuf_append(
        STATIC_BUFFER * b,      /* static buffer structure */
        uint8_t *data,          /* data to append */
        unsigned data_size);    /* how many to append */
    /* returns true if successful, false if count is bigger than size */
    BACNET_STACK_EXPORT
    bool sbuf_truncate(
        STATIC_BUFFER * b,      /* static buffer structure */
        unsigned count);        /* new number of bytes used in buffer */

#ifdef __cplusplus_disable
}
#endif /* __cplusplus_disable */
#endif
