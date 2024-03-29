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

#ifndef BACREAL_H
#define BACREAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bacnet/bacnet_stack_exports.h"

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus_disable */

    BACNET_STACK_EXPORT
    int decode_real_safe(
        const uint8_t * apdu,
        const uint16_t len_value,
        float *real_value);

    BACNET_STACK_EXPORT
    int decode_real(
        const uint8_t * apdu,
        float *real_value);

    BACNET_STACK_EXPORT
    int encode_bacnet_real(
        float value,
        uint8_t * apdu);
    BACNET_STACK_EXPORT

    int decode_double(
        const uint8_t * apdu,
        double *real_value);

    BACNET_STACK_EXPORT
    int decode_double_safe(
        const uint8_t * apdu,
        const uint16_t len_value,
        double *double_value);

    BACNET_STACK_EXPORT
    int encode_bacnet_double(
        double value,
        uint8_t * apdu);

#ifdef __cplusplus_disable
}
#endif /* __cplusplus_disable */
#endif
