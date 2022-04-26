/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef AO_H
#define AO_H

#include "config.h"

#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "BACnetObjectAnalog.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
// Deprecated since Rev 13    #include "get_alarm_sum.h"
#endif


typedef struct analog_output_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    BACNET_EVENT_STATE Event_State;
    float Present_Value;
    float shadow_Present_Value;
    float Relinquish_Default ;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability;

    bool Out_Of_Service;
    uint8_t Units;

#if (BACNET_SVC_COV_B == 1)
    float Prior_Value;
    float COV_Increment;
    bool Changed;
    bool prior_OOS; // todo 0 - check OOS handling with COV
#endif

    uint16_t priorityFlags;
    float priorityArray[BACNET_MAX_PRIORITY];

#if (INTRINSIC_REPORTING_B== 1)
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

} ANALOG_OUTPUT_DESCR;


void Analog_Output_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Analog_Output_Valid_Instance(
    uint32_t object_instance);

unsigned Analog_Output_Count(
    void);

uint32_t Analog_Output_Index_To_Instance(
    unsigned objectIndex);

//unsigned Analog_Output_Instance_To_Index(
//    uint32_t instance);

bool Analog_Output_Object_Instance_Add(
    uint32_t instance);

//float Analog_Output_Present_Value(
//    uint32_t object_instance);

//unsigned Analog_Output_Present_Value_Priority(
//    uint32_t object_instance);

//bool Analog_Output_Present_Value_Set(
//    uint32_t object_instance,
//    float value,
//    unsigned priority);

//bool Analog_Output_Present_Value_Relinquish(
//    uint32_t object_instance,
//    unsigned priority);

#if 0
float Analog_Output_Relinquish_Default(
    uint32_t object_instance);
#endif

bool Analog_Output_Relinquish_Default_Set(
    ANALOG_OUTPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value);

bool Analog_Output_Encode_Value_List(
    const uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

float Analog_Output_COV_Increment(
    uint32_t instance);

void Analog_Output_COV_Increment_Set(
    uint32_t instance,
    float value);

bool Analog_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

bool Analog_Output_Name_Set(
    uint32_t object_instance,
    char *new_name);

char *Analog_Output_Description(
    uint32_t instance);

bool Analog_Output_Description_Set(
    uint32_t instance,
    char *new_name);

#if 0
BACNET_RELIABILITY Analog_Output_Reliability(
    uint32_t object_instance);

bool Analog_Output_Reliability_Set(
    uint32_t object_instance,
    BACNET_RELIABILITY value);
#endif

bool Analog_Output_Units_Set(
    uint32_t instance,
    uint16_t units);

uint16_t Analog_Output_Units(
    uint32_t instance);

#if (INTRINSIC_REPORTING_B == 1)
void Analog_Output_Intrinsic_Reporting(
    uint32_t object_instance);
#endif

#if 0
bool Analog_Output_Out_Of_Service(
    uint32_t instance);

void Analog_Output_Out_Of_Service_Set(
    uint32_t instance,
    bool oos_flag);
#endif

int Analog_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Analog_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

#if ( BACNET_SVC_COV_B == 1 )
bool Analog_Output_Change_Of_Value(
    const uint32_t instance);

void Analog_Output_Change_Of_Value_Clear(
    const uint32_t instance);

#endif

float Analog_Output_Present_Value_from_Instance(
    const uint32_t instance);

bool Analog_Output_Create(
    uint32_t object_instance,
    const char *objectName );
        
bool Analog_Output_Delete(
    uint32_t object_instance);

void Analog_Output_Cleanup(
    void);

void Analog_Output_Init(
    void);

#if ( INTRINSIC_REPORTING_B == 1 )

int Analog_Output_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

int Analog_Output_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE * error_code);

// Deprecated since Rev 13   
//int Analog_Output_Alarm_Summary(
//    unsigned index,
//    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);
#endif

#ifdef TEST
#include "ctest.h"
void testAnalogOutput(
    Test * pTest);
#endif

#endif
