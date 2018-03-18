/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
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
*********************************************************************/

#pragma once

#include <string>

#include "net.h"
#include "debug.h"

typedef struct {
    int bacnetRouterInstance;
    int bacnetSiteBaseInstance;
    int bacnetGroupBaseInstance;
    int bacnetSiteMaxInstance;
    int bacnetGroupMaxInstance;
    char bacnetName[MAX_CHARACTER_STRING_BYTES];
    std::string     configFilename;
    int bacnetAppListenPort;

    int virtualNetworkNumber;

    int localBACnetPort;
    int localNetworkNumber;

    int localBBMDbacnetPort;
    int localBBMDnetworkNumber;

    bool natRouting;

    int globalBACnetPort;
    int globalNetworkNumber;

    struct sockaddr_in globalIPEP;

    const char *ifName ;			// Assume only one (for now, one day multiple NICs, but what about multiple IP addresses per NIC?

    DB_LEVEL debugLevel;

} ConfigType;


void SetConfigDefaults(ConfigType *config);

