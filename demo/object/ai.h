/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
*  2018.07.04 EKH Diffed in hints from Binary Value for future reference
*
****************************************************************************************/

#ifndef AI_H
#define AI_H

#include "config.h"

#if (BACNET_USE_OBJECT_ANALOG_INPUT == 1 )

#include <stdbool.h>
#include <stdint.h>
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


typedef struct analog_input_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    float Present_Value;
    float shadow_Present_Value;
    BACNET_ENGINEERING_UNITS Units;

    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability;
    BACNET_EVENT_STATE Event_State;                 // Event_State is a required property
    
#if (BACNET_SVC_COV_B == 1)
    float Prior_Value;
    float COV_Increment;
    bool Changed;
    bool prior_OOS;
#endif

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

} ANALOG_INPUT_DESCR;


void Analog_Input_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Analog_Input_Valid_Instance(
    uint32_t object_instance);

unsigned Analog_Input_Count(
    void);

uint32_t Analog_Input_Index_To_Instance(
    unsigned index);

// making static
//int Analog_Input_Instance_To_Index(
//    const DEVICE_OBJECT_DATA *pDev,
//    const uint32_t object_instance);

bool Analog_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

//bool Analog_Input_Name_Set(
//    DEVICE_OBJECT_DATA *pDev,
//    uint32_t object_instance,
//    char *new_name);

char *Analog_Input_Description(
    uint32_t instance);

bool Analog_Input_Description_Set(
    uint32_t instance,
    char *new_name);

bool Analog_Input_Units_Set(
    uint32_t instance,
    uint16_t units);

uint16_t Analog_Input_Units(
    uint32_t instance);

int Analog_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Analog_Input_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

#if 0
float Analog_Input_Present_Value(
    ANALOG_INPUT_DESCR *currentObject);

void Analog_Input_Present_Value_Set(
    ANALOG_INPUT_DESCR *currentObject,
    float value);

// EKH: 2016.08.07 Obsoleted
//bool Analog_Input_Out_Of_Service(
//    uint32_t object_instance);

//void Analog_Input_Out_Of_Service_Set(
//    DEVICE_OBJECT_DATA *pDev,
//    const uint32_t object_instance,
//    const bool oos_flag);
#endif

#if ( BACNET_SVC_COV_B == 1 )
bool Analog_Input_Change_Of_Value(
    uint32_t instance);

void Analog_Input_Change_Of_Value_Clear(
    uint32_t instance);

bool Analog_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

#if 0
float Analog_Input_COV_Increment(
    uint32_t instance);

void Analog_Input_COV_Increment_Set(
    uint32_t instance,
    float value);
#endif
#endif

void Analog_Input_Intrinsic_Reporting(
    uint32_t object_instance);

#if (INTRINSIC_REPORTING_B == 1)
int Analog_Input_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

int Analog_Input_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE *error_code);

// Deprecated since Rev 13   
//int Analog_Input_Alarm_Summary(
//    unsigned index,
//    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);
#endif

bool Analog_Input_Create(
    const uint32_t instance,
    const char *name);

void Analog_Input_Update(
	const uint32_t instance,
	const double value );

double Analog_Input_Present_Value_from_Instance ( 
    const uint32_t instance ) ;

bool Analog_Input_Delete(
    uint32_t object_instance);

void Analog_Input_Cleanup(
    void);

void Analog_Input_Init(
    void);

//AnalogInputObject *Analog_Input_Instance_To_Object(
//    DEVICE_OBJECT_DATA *pDev,
//    uint32_t object_instance);

#ifdef TEST
#include "ctest.h"
void testAnalogInput(
    Test * pTest);
#endif

#endif // ifdef object_analog
#endif /* AI_H */
