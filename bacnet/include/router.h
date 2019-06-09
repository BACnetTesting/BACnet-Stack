/**************************************************************************
*
*  Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*       For more information: info@bac-test.com
*       For access to source code:  info@bac-test.com
*               or      www.github.com/bacnettesting/bacnet-stack
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
*
*********************************************************************/

#pragma once

#include "msgqueue.h"
#include "datalink.h"

bool InitRouterport(BPT_TYPE type,  uint16_t networkNumber, uint16_t nwoPort ) ;
bool InitRouterportWithNAT(BPT_TYPE type, uint16_t networkNumber, uint16_t nwoPort, SOCKADDR_IN *globalIPEP );
bool InitRouterportForeignDevice(uint16_t networkNumber, SOCKADDR_IN *fdIPEP, uint16_t ttl );

bool AlignApplicationWithPort(uint16_t networkNumberOfRouterPort);
