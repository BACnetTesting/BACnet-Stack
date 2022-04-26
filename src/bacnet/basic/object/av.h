/**************************************************************************
 *
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
 * Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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

#ifndef AV_H
#define AV_H

#include "configProj.h"

#if (BACNET_USE_OBJECT_ANALOG_VALUE == 1 )

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacdef.h"
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/bits/util/BACnetObjectAnalog.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
// Deprecated since Rev 13    #include "get_alarm_sum.h"
#endif


typedef struct {
    BACnetAnalogObject analogCommon;
    float Relinquish_Default;
#ifndef BACNET_USE_COMMANDABLE_VALUE_OBJECTS 
    uint16_t priorityFlags;
    float priorityArray[BACNET_MAX_PRIORITY];
#endif
} ANALOG_VALUE_DESCR;


void Analog_Value_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary);


bool Analog_Value_Valid_Instance(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


unsigned Analog_Value_Count(
    DEVICE_OBJECT_DATA* pDev);


uint32_t Analog_Value_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned objectIndex);


// making static
//int Analog_Input_Instance_To_Index(
//    const DEVICE_OBJECT_DATA *pDev,
//    const uint32_t object_instance);


bool Analog_Value_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name);


char* Analog_Value_Description(
    uint32_t instance);


bool Analog_Value_Description_Set(
    uint32_t instance,
    char* new_name);


bool Analog_Value_Units_Set(
    uint32_t instance,
    uint16_t units);


uint16_t Analog_Value_Units(
    uint32_t instance);


int Analog_Value_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata);


bool Analog_Value_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data);


#if ( BACNET_SVC_COV_B == 1 )
bool Analog_Value_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


void Analog_Value_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


bool Analog_Value_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list);


#if 0 // making static
float Analog_Value_COV_Increment(
    uint32_t instance);


void Analog_Value_COV_Increment_Set(
    uint32_t instance,
    float value);
#endif
#endif // BACNET_SVC_COV_B


#if (INTRINSIC_REPORTING_B == 1)
void Analog_Value_Intrinsic_Reporting(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


int Analog_Value_Event_Information(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA* getevent_data);


int Analog_Value_Alarm_Ack(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_ALARM_ACK_DATA* alarmack_data,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code);


#if ( BACNET_PROTOCOL_REVISION < 13 )
int Analog_Value_Alarm_Summary(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA* getalarm_data);
#endif
#endif

BACNET_OBJECT* Analog_Value_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name,
    const BACNET_ENGINEERING_UNITS units);

float Analog_Value_Present_Value_from_Instance(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);

bool Analog_Value_Delete(
    uint32_t object_instance);

void Analog_Value_Cleanup(
    void);

void Analog_Value_Init(
    void);

ANALOG_VALUE_DESCR* Analog_Value_Instance_To_Object(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);
    
#ifdef TEST
#include "ctest.h"
void testAnalogValue(
    Test* pTest);
#endif  // TEST_XXX

#endif  // ( BACNET_USE_OBJECT_XXX == 1 )   -   configProj.h includes this BACnet Object Type
#endif  // XXX_H include guard
