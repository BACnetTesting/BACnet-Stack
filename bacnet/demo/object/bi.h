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


typedef struct binary_input_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    BACNET_BINARY_PV Present_Value;
    BACNET_BINARY_PV shadow_Present_Value;

    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability ;
    
    BACNET_EVENT_STATE Event_State;

    BACNET_POLARITY polarity;

#if (BACNET_SVC_COV_B == 1)
    BACNET_BINARY_PV Prior_Value;
    bool Changed;
    bool prior_OOS;
#endif

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
    BACNET_BINARY_PV alarmValue;
#endif

} BINARY_INPUT_DESCR;


void Binary_Input_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Binary_Input_Valid_Instance(
    uint32_t object_instance);

unsigned Binary_Input_Count(
    void);

uint32_t Binary_Input_Index_To_Instance(
    unsigned index);

#if 0 // making static
unsigned Binary_Input_Instance_To_Index(
    uint32_t instance);

bool Binary_Input_Object_Instance_Add(
    uint32_t instance);
#endif

bool Binary_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

#if 0 // making static
bool Binary_Input_Name_Set(
    uint32_t object_instance,
    char *new_name);
#endif

//BACNET_BINARY_PV Binary_Input_Present_Value(
//    uint32_t object_instance);

//bool Binary_Input_Present_Value_Set(
//    uint32_t object_instance,
//    BACNET_BINARY_PV value);

char *Binary_Input_Description(
    uint32_t instance);

bool Binary_Input_Description_Set(
    uint32_t instance,
    char *new_name);

//BACNET_RELIABILITY Binary_Input_Reliability_Get(
//    BINARY_INPUT_DESCR *currentObject);

//bool Binary_Input_Reliability_Set(
//    BINARY_INPUT_DESCR *currentObject,
//    BACNET_RELIABILITY value);

char *Binary_Input_Inactive_Text(
    uint32_t instance);

bool Binary_Input_Inactive_Text_Set(
    uint32_t instance,
    char *new_name);

char *Binary_Input_Active_Text(
    uint32_t instance);

bool Binary_Input_Active_Text_Set(
    uint32_t instance,
    char *new_name);

BACNET_POLARITY Binary_Input_Polarity_Get(
    BINARY_INPUT_DESCR *currentObject);

bool Binary_Input_Polarity_Set(
    BINARY_INPUT_DESCR *currentObject,
    BACNET_POLARITY polarity);

#if 0 // making static
bool Binary_Input_Out_Of_Service(
    uint32_t object_instance);

void Binary_Input_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value);
#endif

#if ( BACNET_SVC_COV_B == 1 )
bool Binary_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

bool Binary_Input_Change_Of_Value(
    uint32_t instance);

void Binary_Input_Change_Of_Value_Clear(
    uint32_t instance);
#endif

int Binary_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Binary_Input_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

bool Binary_Input_Create(
    const uint32_t instance,
    const char *name);

#if (INTRINSIC_REPORTING_B == 1)
int Binary_Input_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

int Binary_Input_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code);
#endif

bool Binary_Input_Delete(
    uint32_t object_instance);

void Binary_Input_Cleanup(
    void);

void Binary_Input_Init(
    void);

#ifdef TEST
#include "ctest.h"
void testBinaryInput(
    Test * pTest);
#endif

#endif
#endif /* BI_H */
