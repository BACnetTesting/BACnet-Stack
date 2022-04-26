/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

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

#include "configProj.h"

#pragma once
#include <stdbool.h>

typedef struct {

    void (*MenuFunc) (int key);
    void (*NextItem) (void);
    void (*EnterItem) (void);
    void (*ExitMenu) (void);
    void (*HelpScreen) (void);
    bool (*HandleKey) (int key);

    int keyOnEnter;
} ACTIVE_MENU ;


#if (BACNET_CLIENT == 1)
void dm_ClientDevice_Setup( int keyOnEnter );
void dm_PollRecord_Setup(int keyOnEnter);
#endif

void dm_ServerDevice_Setup(int keyOnEnter);
void dm_Datalink_Setup(int keyOnEnter);

#if (BITS_ROUTER_LAYER == 1)
void dm_RouterPort_Setup(int keyOnEnter);
void ShowRouterPorts(void);
#endif