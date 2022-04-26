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
 *  2018.07.04 EKH Diffed in hints from Binary Value for future reference
 *
 ****************************************************************************************/

#ifndef MSO_H
#define MSO_H
#include "configProj.h"

#if (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1 )

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacdef.h"
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/bits/util/BACnetObjectMultistate.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
// Deprecated since Rev 13    #include "get_alarm_sum.h"
#endif

/* how many states? 1 to 254 states - 0 is not allowed. */
// 2019.08.13 moving to configProj.h
// #ifndef MULTISTATE_NUMBER_OF_STATES
// #define MULTISTATE_NUMBER_OF_STATES (5)
// #endif


typedef struct {
    BACnetMultistateObject multistateCommon;
    BACNET_EVENT_STATE Event_State;                 // Event_State is a required property
    uint32_t Relinquish_Default;
    uint16_t priorityFlags;
    uint16_t priorityArray[BACNET_MAX_PRIORITY];
} MULTISTATE_OUTPUT_DESCR;


void Multistate_Output_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary);


bool Multistate_Output_Valid_Instance(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


unsigned Multistate_Output_Count(
    DEVICE_OBJECT_DATA* pDev);


uint32_t Multistate_Output_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index);


//unsigned Multistate_Output_Instance_To_Index(
//    const DEVICE_OBJECT_DATA *pDev,
//    const uint32_t object_instance);


bool Multistate_Output_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name);


char* Multistate_Output_Description(
    uint32_t instance);


bool Multistate_Output_Description_Set(
    uint32_t instance,
    char* new_name);


int Multistate_Output_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata);


bool Multistate_Output_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data);


bool Multistate_Output_Object_Instance_Add(
    uint32_t instance);


bool Multistate_Output_Name_Set(
    uint32_t object_instance,
    char* new_name);


uint32_t Multistate_Output_Present_Value(
    uint32_t object_instance);


#if ( BACNET_SVC_COV_B == 1 )
bool Multistate_Output_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


void Multistate_Output_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


bool Multistate_Output_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list);


// uint32_t Multistate_Output_Present_Value(
//    uint32_t instance);


#if 0 // making static
bool Multistate_Output_Present_Value_Set(
    uint32_t instance,
    unsigned value,
    unsigned priority);


bool Multistate_Output_Present_Value_Relinquish(
    uint32_t instance,
    unsigned priority);


bool Multistate_Output_Out_Of_Service(
    uint32_t object_instance);


//void Multistate_Input_Out_Of_Service_Set(
//    uint32_t object_instance,
//    bool value);

#endif
#endif


bool MultistateOutputObject_Description(
    uint32_t instance,
    char* text_string);

BACNET_RELIABILITY Multistate_Output_Reliability(
    uint32_t instance);


bool Multistate_Output_Reliability_Set(
    uint32_t object_instance,
    BACNET_RELIABILITY value);


bool Multistate_Output_State_Text_Set(
    uint32_t instance,
    uint32_t state_index,
    char* new_name);


bool Multistate_Output_Max_States_Set(
    uint32_t instance,
    uint32_t max_states_requested);


//char *Multistate_Input_State_Text(
//    uint32_t object_instance,
//    uint32_t state_index);


#if (INTRINSIC_REPORTING_B == 1)
void Multistate_Output_Intrinsic_Reporting(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance);


int Multistate_Output_Event_Information(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA* getevent_data);


int Multistate_Output_Alarm_Ack(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_ALARM_ACK_DATA* alarmack_data,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code);


#if ( BACNET_PROTOCOL_REVISION < 13 )
int Multistate_Output_Alarm_Summary(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA* getalarm_data);
#endif
#endif

BACNET_OBJECT* Multistate_Output_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name);


double Multistate_Output_Present_Value_from_Instance(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance);


bool Multistate_Output_Delete(
    uint32_t object_instance);


void Multistate_Output_Cleanup(
    void);


void Multistate_Output_Init(
    void);


#ifdef TEST
#include "ctest.h"
void testMultistateOutput(
    Test* pTest);
#endif

#endif  // ( BACNET_USE_OBJECT_XXX == 1 )   -   configProj.h includes this BACnet Object Type
#endif  // XXX_H include guard
