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

#include "config.h"

#if (BACNET_USE_OBJECT_ANALOG_VALUE == 1 )

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "alarm_ack.h"
#include "getevent.h"
#include "get_alarm_sum.h"
#endif

#include "BACnetObjectAnalog.h"

typedef struct analog_value_descr {

    BACNET_OBJECT   common;

    BACNET_EVENT_STATE Event_State;
    BACNET_RELIABILITY Reliability;
    
    bool Out_Of_Service;
    uint16_t Units;
    float Present_Value;
    
#if ( BACNET_SVC_COV_B == 1 )
    float Prior_Value;
    float COV_Increment;
    bool Changed;
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

} ANALOG_VALUE_DESCR;


void Analog_Value_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Analog_Value_Valid_Instance(
    uint32_t object_instance);

unsigned Analog_Value_Count(
    void);

uint32_t Analog_Value_Index_To_Instance(
    unsigned index);

// Returns -1 is Instance not found, by design.
unsigned Analog_Value_Instance_To_Index(
    uint32_t object_instance);

bool Analog_Value_Object_Instance_Add(
    uint32_t instance);

bool Analog_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

bool Analog_Value_Name_Set(
    uint32_t object_instance,
    char *new_name);

char *Analog_Value_Description(
    uint32_t instance);

bool Analog_Value_Description_Set(
    uint32_t instance,
    char *new_name);

bool Analog_Value_Units_Set(
    uint32_t instance,
    uint16_t units);

uint16_t Value_Input_Units(
    uint32_t instance);

int Analog_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Analog_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

// static
//bool Analog_Value_Present_Value_Set(
//    ANALOG_VALUE_DESCR *currentObject,
//    float value,
//    uint8_t priority);

// static
//float Analog_Value_Present_Value(
//    uint32_t object_instance);

// static
//void Analog_Value_Out_Of_Service_Set(
//    const uint32_t object_instance,
//    const bool oos_flag);

#if ( BACNET_SVC_COV_B == 1 )
bool Analog_Value_Change_Of_Value(
    uint32_t instance);

void Analog_Value_Change_Of_Value_Clear(
    uint32_t instance);

bool Analog_Value_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

float Analog_Value_COV_Increment(
    uint32_t instance);

void Analog_Value_COV_Increment_Set(
    uint32_t instance,
    float value);
#endif

char *Analog_Value_Description(
    uint32_t instance);
    
bool Analog_Value_Description_Set(
    uint32_t instance,
    char *new_name);

BACNET_RELIABILITY Analog_Value_Reliability(
    uint32_t object_instance);
    
bool Analog_Value_Reliability_Set(
    uint32_t object_instance,
    BACNET_RELIABILITY value);

uint16_t Analog_Value_Units(
    uint32_t instance);

bool Analog_Value_Units_Set(
    uint32_t instance,
    uint16_t unit);

//bool Analog_Value_Out_Of_Service(
//    uint32_t instance);

// duplicate
//void Analog_Value_Out_Of_Service_Set(
//    uint32_t instance,
//    bool oos_flag);

/* note: header of INTRINSIC_REPORTING_Bfunction is required
   even when INTRINSIC_REPORTING_B is not defined */
void Analog_Value_Intrinsic_Reporting(
    uint32_t object_instance);

#if (INTRINSIC_REPORTING_B == 1)
int Analog_Value_Event_Information(
    unsigned objectIndex,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

int Analog_Value_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CODE * error_code);

int Analog_Value_Alarm_Summary(
    unsigned objectIndex,
    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);
#endif

bool Analog_Value_Create(
    const uint32_t instance,
    const char *name,
    const BACNET_ENGINEERING_UNITS units);

void Analog_Value_Update(
	const uint32_t instance,
	const double value );

double Analog_Value_Present_Value_from_Instance ( const uint32_t instance ) ;

bool Analog_Value_Delete(
    uint32_t object_instance);

void Analog_Value_Cleanup(
    void);

void Analog_Value_Init(
    void);

#ifdef TEST
#include "ctest.h"
void testAnalog_Value(
    Test * pTest);
#endif

#endif  // use_av

#endif /* AV_H */
