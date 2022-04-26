/**************************************************************************
 *
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef BI_H
#define BI_H

#include "configProj.h"

#if (BACNET_USE_OBJECT_BINARY_INPUT == 1 )

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacdef.h"
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/bits/util/BACnetObjectBinary.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
// Deprecated since Rev 13    #include "get_alarm_sum.h"
#endif


typedef struct {
    BACnetBinaryObject binaryCommon;
} BINARY_INPUT_DESCR;


//    BACNET_NOTIFY_TYPE Notify_Type ;
void Binary_Input_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary);


bool Binary_Input_Valid_Instance(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


unsigned Binary_Input_Count(
    DEVICE_OBJECT_DATA* pDev);


uint32_t Binary_Input_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index);


// making static
//int Analog_Input_Instance_To_Index(
//    const DEVICE_OBJECT_DATA *pDev,
//    const uint32_t object_instance);


bool Binary_Input_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name);


char* Binary_Input_Description(
    uint32_t instance);


bool Binary_Input_Description_Set(
    uint32_t instance,
    char* new_name);


int Binary_Input_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata);


bool Binary_Input_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data);


#if ( BACNET_SVC_COV_B == 1 )
bool Binary_Input_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


void Binary_Input_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


bool Binary_Input_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list);


#if 0 // making static
float Binary_Input_COV_Increment(
    uint32_t instance);


void Binary_Input_COV_Increment_Set(
    uint32_t instance,
    float value);
#endif
#endif // BACNET_SVC_COV_B


#if (INTRINSIC_REPORTING_B == 1)
void Binary_Input_Intrinsic_Reporting(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


int Binary_Input_Event_Information(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA* getevent_data);


int Binary_Input_Alarm_Ack(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_ALARM_ACK_DATA* alarmack_data,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code);


#if ( BACNET_PROTOCOL_REVISION < 13 )
int Binary_Input_Alarm_Summary(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA* getalarm_data);
#endif
#endif

BACNET_OBJECT* Binary_Input_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name);


BACNET_BINARY_PV Binary_Input_Present_Value_from_Instance(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);

bool Binary_Input_Delete(
    uint32_t object_instance);


void Binary_Input_Cleanup(
    void);


void Binary_Input_Init(
    void);


//AnalogInputObject *Analog_Input_Instance_To_Object(
//    DEVICE_OBJECT_DATA *pDev,
//    uint32_t object_instance);


#ifdef TEST
#include "ctest.h"
void testBinaryInput(
    Test* pTest);
#endif

#endif
#endif /* BI_H */
