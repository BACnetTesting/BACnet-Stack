/**************************************************************************
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           to original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*       For more information: info@bac-test.com
*       For access to source code:  info@bac-test.com
*               or      www.github.com/bacnettesting/bacnet-stack
*
*********************************************************************/

#pragma once

#include "stdint.h"

typedef struct _PORT_SUPPORT PORT_SUPPORT;

typedef struct
{
    int                 block_valid;

    PORT_SUPPORT        *portParams;        // original raw packet
    BACNET_MAC_ADDRESS  phySrc;
    uint8_t             *npdu;
    uint16_t            npdu_len;

} RX_DETAILS;


