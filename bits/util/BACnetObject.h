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
// #include "bacApp.h"
#include "wp.h"
#include "rp.h"

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

int encode_application_bitstring3(uint8_t *apdu, const bool one, const bool two, const bool three);

int encode_application_bitstring2(uint8_t *apdu, const bool one, const bool two);

int encode_status_flags(uint8_t *apdu, bool alarm, bool fault, bool oos);

int encode_application_event_time_stamps(uint8_t *apdu, BACNET_READ_PROPERTY_DATA *rpdata, BACNET_DATE_TIME *Event_TimeStamps);

bool WP_ValidateTagType(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    BACNET_APPLICATION_TAG expectedTag);

bool WP_ValidateEnumType(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue);

bool WP_ValidateRangeReal(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    double low,
    double high);

bool WP_ValidateEnumTypeAndRange(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint16_t    maxRange);

bool WP_StorePropertyFloat(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    float *tValue);

bool WP_StorePropertyEnumRanged(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint16_t    maxRange,
    int *tValue);
