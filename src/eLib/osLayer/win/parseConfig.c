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

#include <string.h>

#include "bitsRouter/bitsRouter.h"
#include "bacstr.h"
#include "osLayer.h"
#include "device.h"
#include "bacversion.h"
#include "appApi.h"

ROUTER_PORT *headRouterport = NULL; /* pointer to list of router ports */
extern ROUTER_PORT *applicationRouterPort;

// "root name" for virtual devices, and "base instance" for virtual device instances TBD, needs UI changes first, tracked using
uint32_t deviceInstanceBase = 2700000;

BACNET_CHARACTER_STRING My_Object_Name;
uint32_t Object_Instance_Number;


bool read_config(const char *filepath)
{
    // todo 3 - inplement a config file processor for windows
    (void)filepath;
    return true;
}


void read_config_auxiliary(const char *filepath)
{
    (void)filepath;
}
