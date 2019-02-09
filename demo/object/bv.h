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

#ifndef BV_H
#define BV_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "bacdef.h"
#include "rp.h"
#include "wp.h"

#include "BACnetObjectBinary.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#endif
typedef struct binary_value_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_RELIABILITY shadowReliability;
    BACNET_BINARY_PV   Present_Value;
    BACNET_BINARY_PV   shadow_Present_Value;

    BACNET_EVENT_STATE Event_State;

#if ( BACNET_SVC_COV_B == 1 )
    BACNET_BINARY_PV Prior_Value;
    bool Changed;
    bool prior_OOS;
#endif

    BACNET_BINARY_PV priorityArray[BACNET_MAX_PRIORITY];
    uint16_t priorityFlags;
    BACNET_BINARY_PV   Relinquish_Default;

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
    ACK_NOTIFICATION    Ack_notify_data;
    BACNET_BINARY_PV    alarmValue;
#endif

} BINARY_VALUE_DESCR;

void Binary_Value_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Binary_Value_Valid_Instance(
    uint32_t object_instance);

unsigned Binary_Value_Count(
    void);

uint32_t Binary_Value_Index_To_Instance(
    unsigned objectIndex);

// Returns -1 is Instance not found, by design.
int Binary_Value_Instance_To_Index(
    uint32_t object_instance);

bool Binary_Value_Object_Instance_Add(
    uint32_t instance);

bool Binary_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

bool Binary_Value_Name_Set(
    uint32_t object_instance,
    char *new_name);

char *Binary_Value_Description(
    uint32_t instance);

bool Binary_Value_Description_Set(
    uint32_t instance,
    char *new_name);

// static now 2018.07.04 
//BACNET_RELIABILITY Binary_Value_Reliability(
//    uint32_t object_instance);
//    
//bool Binary_Value_Reliability_Set(
//    uint32_t object_instance,
//    BACNET_RELIABILITY value);

char *Binary_Value_Inactive_Text(
    uint32_t instance);
    
bool Binary_Value_Inactive_Text_Set(
    uint32_t instance,
    char *new_name);

char *Binary_Value_Active_Text(
    uint32_t instance);
    
bool Binary_Value_Active_Text_Set(
    uint32_t instance,
    char *new_name);

int Binary_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Binary_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

void Binary_Value_Change_Of_Value_Clear(
    uint32_t instance);

bool Binary_Value_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list);

bool Binary_Value_Change_Of_Value(
    uint32_t instance);

//BACNET_BINARY_PV Binary_Value_Present_Value(
//    uint32_t instance);

// static
//bool Binary_Value_Present_Value_Set(
//    uint32_t instance,
//    BACNET_BINARY_PV value);
//
//bool Binary_Value_Out_Of_Service(
//    uint32_t instance);

// static
//void Binary_Value_Out_Of_Service_Set(
//    uint32_t instance,
//    bool value);

// duplicate
//char *Binary_Value_Inactive_Text(
//    uint32_t instance);
//
//bool Binary_Value_Inactive_Text_Set(
//    uint32_t instance,
//
//    char *new_name);
//char *Binary_Value_Active_Text(
//    uint32_t instance);
//
//bool Binary_Value_Active_Text_Set(
//    uint32_t instance,
//    char *new_name);

// not a valid property
//BACNET_POLARITY Binary_Value_Polarity(
//    uint32_t instance);
//
//bool Binary_Value_Polarity_Set(
//    uint32_t object_instance,
//    BACNET_POLARITY polarity);

// Deprecated since Rev 13   
//int Analog_Input_Alarm_Summary(
//    unsigned index,
//    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);

bool Binary_Value_Create(
    const uint32_t instance,
    const char *name );

void Binary_Value_Update(
	const uint32_t instance,
	const bool value );

//bool Binary_Value_Delete(
//    uint32_t object_instance);
//
//void Binary_Value_Cleanup(
//    void);

void Binary_Value_Init(
    void);

#ifdef TEST
#include "ctest.h"
void testBinary_Value(
    Test * pTest);
#endif

#endif /* BV_H */
