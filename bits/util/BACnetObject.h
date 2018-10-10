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

#pragma once

#include "llist.h"
#include "bacstr.h"

typedef struct
{
    LLIST_LB                llist;                     // must be first

    uint32_t                objectInstance;
    BACNET_CHARACTER_STRING objectName;

} BACNET_OBJECT ;


BACNET_OBJECT *Generic_Instance_To_Object(
    LLIST_HDR *objectHdr,
    uint32_t object_instance);

bool Generic_Instance_To_Object_Name(
    LLIST_HDR *objectHdr,
    uint32_t objectInstance,
    BACNET_CHARACTER_STRING *object_name);

void Generic_Object_Init(
    BACNET_OBJECT       *bacnetObject,
    const   uint32_t    objectInstance,
    const   char        *objectName);

uint32_t Generic_Index_To_Instance(
    LLIST_HDR *objectHdr,
    uint32_t objectIndex);

BACNET_OBJECT  *Generic_Index_To_Object(
    LLIST_HDR *objectHdr,
    uint32_t objectIndex);

BACNET_OBJECT *BACnet_Find_Object(const uint32_t deviceInstance, const char *objectType, const uint32_t objectInstance);
