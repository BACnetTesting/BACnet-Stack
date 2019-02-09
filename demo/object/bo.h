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

#ifndef BO_H
#define BO_H

#include "config.h"

#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "BACnetObjectBinary.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
// Deprecated since Rev 13    #include "get_alarm_sum.h"
#endif


typedef struct binary_output_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    BACNET_BINARY_PV Present_Value;
    BACNET_BINARY_PV shadow_Present_Value;
    BACNET_BINARY_PV Relinquish_Default;
    BACNET_BINARY_PV feedbackValue;

    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability;
    
    BACNET_EVENT_STATE Event_State;

    BACNET_POLARITY polarity;

#if (BACNET_SVC_COV_B == 1)
    BACNET_BINARY_PV Prior_Value;
    bool Changed;
    bool prior_OOS; // todo 0 - check OOS handling with COV
#endif

    uint16_t priorityFlags;
    BACNET_BINARY_PV priorityArray[BACNET_MAX_PRIORITY];    // todo 1 - can pack this...

#if (INTRINSIC_REPORTING_B == 1)
    uint32_t Time_Delay;
    uint32_t Notification_Class;
    unsigned Event_Enable : 3;
    BACNET_NOTIFY_TYPE Notify_Type;
    ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
    BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
    /* time to generate event notification */
    uint32_t Remaining_Time_Delay;
    /* AckNotification informations */
    ACK_NOTIFICATION Ack_notify_data;
#endif

} BINARY_OUTPUT_DESCR;


void Binary_Output_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Binary_Output_Valid_Instance(
    uint32_t object_instance);

unsigned Binary_Output_Count(
    void);

uint32_t Binary_Output_Index_To_Instance(
    unsigned index);

#if 0 // making static
unsigned Binary_Output_Instance_To_Index(
    uint32_t instance);

bool Binary_Output_Object_Instance_Add(
    uint32_t instance);
#endif

bool Binary_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

#if 0 // making static
bool Binary_Output_Name_Set(
    uint32_t object_instance,
    char *new_name);
#endif

BACNET_BINARY_PV Binary_Output_Present_Value_Get(
    BINARY_OUTPUT_DESCR *currentObject);

//bool Binary_Output_Present_Value_Set(
//    uint32_t object_instance,
//    BACNET_BINARY_PV value,
//    unsigned priority);

char *Binary_Output_Description(
    uint32_t instance);

bool Binary_Output_Description_Set(
    uint32_t instance,
    char *new_name);

BACNET_RELIABILITY Binary_Output_Reliability(
    uint32_t object_instance);

//bool Binary_Output_Reliability_Set(
//    uint32_t object_instance,
//    BACNET_RELIABILITY value);

char *Binary_Output_Inactive_Text(
    uint32_t instance);

bool Binary_Output_Inactive_Text_Set(
    uint32_t instance,
    char *new_name);

char *Binary_Output_Active_Text(
    uint32_t instance);

bool Binary_Output_Active_Text_Set(
    uint32_t instance,
    char *new_name);

BACNET_POLARITY Binary_Output_Polarity(
    uint32_t object_instance);

bool Binary_Output_Polarity_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_POLARITY polarity);

#if 0 // making static
bool Binary_Output_Out_Of_Service(
    uint32_t instance);

void Binary_Output_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value);
#endif

bool Binary_Output_Encode_Value_List(
    const uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

bool Binary_Output_Change_Of_Value(
    uint32_t instance);

void Binary_Output_Change_Of_Value_Clear(
    const uint32_t instance);

int Binary_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Binary_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

bool Binary_Output_Create(
    const uint32_t instance,
    const char *name);

#if (INTRINSIC_REPORTING_B == 1)
int Binary_Output_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

int Binary_Output_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code);
#endif

bool Binary_Output_Delete(
    uint32_t object_instance);

void Binary_Output_Cleanup(
    void);

void Binary_Output_Init(
    void);

BACNET_BINARY_PV Binary_Output_Relinquish_Default(
    uint32_t object_instance);

//bool Binary_Output_Relinquish_Default_Set(
//    uint32_t object_instance,
//    BACNET_BINARY_PV value);

//bool Binary_Output_Present_Value_Relinquish(
//    uint32_t instance,
//    unsigned priority);

unsigned Binary_Output_Present_Value_Priority(
    uint32_t object_instance);

#ifdef TEST
#include "ctest.h"
    void testBinaryOutput(
        Test * pTest);
#endif

#endif
