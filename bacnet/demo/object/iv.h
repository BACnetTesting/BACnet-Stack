/**
 * @file
 * @author Steve Karg
 * @date 2014
 * @brief Integer Value objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Integer Value object is an object with a present-value that
 * uses an INTEGER data type.
 *
 * @section LICENSE
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

#ifndef IV_H
#define IV_H

//#include <stdbool.h>
//#include <stdint.h>
//#include "bacdef.h"
//#include "bacerror.h"
//#include "wp.h"
//#include "rp.h"
#include "nc.h"
#include "BACnetObject.h"

typedef struct integer_object
{
    BACNET_OBJECT   common;         // must be first field in structure due to llist

    BACNET_EVENT_STATE Event_State;
    int Present_Value;
    int shadow_Present_Value;
    int Relinquish_Default ;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability;

    bool Out_Of_Service;
    uint8_t Units;

#if (BACNET_SVC_COV_B == 1)
    int Prior_Value;
    unsigned COV_Increment;
    bool Changed;
#endif

    uint16_t priorityFlags;
    float priorityArray[BACNET_MAX_PRIORITY];

#if (INTRINSIC_REPORTING_B == 1)
    uint32_t Time_Delay;
    uint32_t Notification_Class;
    float High_Limit;
    float Low_Limit;
    float Deadband;
    unsigned Limit_Enable : 2;
    unsigned Event_Enable : 3;
    BACNET_NOTIFY_TYPE Notify_Type;
    ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
    BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
    /* time to generate event notification */
    uint32_t Remaining_Time_Delay;
    /* AckNotification informations */
    ACK_NOTIFICATION Ack_notify_data;
#endif

} INTEGER_VALUE_DESCR;


void Integer_Value_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Integer_Value_Valid_Instance(
    uint32_t object_instance);

unsigned Integer_Value_Count(
    void);

uint32_t Integer_Value_Index_To_Instance(
    unsigned index);

unsigned Integer_Value_Instance_To_Index(
    uint32_t object_instance);

bool Integer_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

int Integer_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Integer_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

bool Integer_Value_Present_Value_Set(
    uint32_t object_instance,
    int32_t value,
    uint8_t priority);
int32_t Integer_Value_Present_Value(
    uint32_t object_instance);

bool Integer_Value_Change_Of_Value(
    uint32_t instance);

void Integer_Value_Change_Of_Value_Clear(
    uint32_t instance);

bool Integer_Value_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

float Integer_Value_COV_Increment(
    uint32_t instance);

void Integer_Value_COV_Increment_Set(
    uint32_t instance,
    float value);

char *Integer_Value_Description(
    uint32_t instance);

bool Integer_Value_Description_Set(
    uint32_t instance,
    char *new_name);

uint16_t Integer_Value_Units(
    uint32_t instance);

bool Integer_Value_Units_Set(
    uint32_t instance,
    uint16_t unit);

bool Integer_Value_Out_Of_Service(
    uint32_t instance);

void Integer_Value_Out_Of_Service_Set(
    uint32_t instance,
    bool oos_flag);

void Integer_Value_Init(
    void);

#ifdef TEST
#include "ctest.h"
    void testInteger_Value(
        Test * pTest);
#endif

#endif
