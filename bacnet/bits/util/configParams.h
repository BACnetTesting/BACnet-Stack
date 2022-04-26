/****************************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

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

