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

#include "osLayer.h"
#include "bacenum.h"

typedef enum
{
    OBJ_TYPE_AI,
    OBJ_TYPE_AO,
    OBJ_TYPE_AV,
    OBJ_TYPE_BI,
    OBJ_TYPE_BO,
    OBJ_TYPE_BV,
    OBJ_TYPE_MSI,
    OBJ_TYPE_MSO,
    OBJ_TYPE_MSV,
    OBJ_TYPE_NC,
    OBJ_TYPE_Calendar,
    OBJ_TYPE_Schedule,
    OBJ_TYPE_Error
} IPC_OBJECT_TYPE;

typedef enum
{
    BACAPI_UNITS_DEGREES_CELSIUS,
} BACAPI_UNITS ;


typedef enum
{
    PROP_PV,
    PROP_DESC,
} IPC_PROPERTY_ID;

void Create_Object(const uint devInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description);
void BACapi_Analog_UpdatePV(const uint32_t deviceInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, float pv);
void BACapi_Analog_Update_Units(const uint32_t deviceInstance, const IPC_OBJECT_TYPE objType, const uint objInstance, BACAPI_UNITS units);

IPC_OBJECT_TYPE BACapi_BACnet_to_IPC_Type(BACNET_OBJECT_TYPE bacType);
