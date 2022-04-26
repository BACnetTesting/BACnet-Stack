/**************************************************************************
*
* Copyright (C) 2015 ConnectEx, Inc.
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

#ifndef CALENDAR_H
#define CALENDAR_H

#include "rp.h"
#include "wp.h"
#include "BACnetObject.h"
#include "datetime.h"
#include "readrange.h"

#define MAX_CALENDAR_EVENTS  32

typedef enum
{
    CALENDAR_ENTRY_NONE=0,          // code depends on this being 0
    CALENDAR_ENTRY_DATE,
    CALENDAR_ENTRY_RANGE,
    CALENDAR_ENTRY_WEEKNDAY
} CAL_ENTRY_TAG ;

/*
BACnetCalendarEntry ::= CHOICE {
date        [0] Date,
dateRange   [1] BACnetDateRange,
weekNDay    [2] BACnetWeekNDay
}
*/

typedef struct BACnetCalendarEntry
{
	CAL_ENTRY_TAG			tag;
    union
    {
        BACNET_DATE         date;
        BACNET_DATE_RANGE   range;
        BACNET_WEEKNDAY     weekNday;
    } CalEntryChoice;
} BACNET_CALENDAR_ENTRY;

typedef struct {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    bool Present_Value;
	BACNET_CALENDAR_ENTRY   calendar[MAX_CALENDAR_EVENTS];

#if 0
    bool Out_Of_Service;
    uint8_t Units;
    BACNET_RELIABILITY Reliability;
    
#if ( BACNET_SVC_COV_B == 1 )
    BACNET_EVENT_STATE Event_State;
    BACNET_RELIABILITY Reliability;
    float Prior_Value;
    float COV_Increment;
    bool Changed;
#endif

#if (INTRINSIC_REPORTING == 1)
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

#endif

} CALENDAR_DESCR;


#define BACNET_WEEK_N_DAY_INIT(month, weekOfMonth, dayOfWeek) \
    {(month), (weekOfMonth), (dayOfWeek)}
    
#define BACNET_DATE_VALUE(d) \
    ((d).day + (d).month * 100 + (d).year * 10000)
 
void Calendar_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Calendar_Valid_Instance(
    uint32_t object_instance);

unsigned Calendar_Count(
    void);

uint32_t Calendar_Index_To_Instance(
    unsigned index);

//unsigned Calendar_Instance_To_Index(
//    uint32_t instance);
//
//bool Calendar_Object_Instance_Add(
//    uint32_t instance);

bool Calendar_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

//bool Calendar_Name_Set(
//    uint32_t object_instance,
//    char *new_name);
//
//char *Calendar_Description(
//    uint32_t instance);
//
//bool Calendar_Description_Set(
//    uint32_t instance,
//    char *new_name);


int Calendar_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Calendar_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

//float Calendar_Present_Value(
//    uint32_t object_instance);
//
//void Calendar_Present_Value_Set(
//    uint32_t object_instance,
//    float value);
//
//void Calendar_Out_Of_Service_Set(
//    uint32_t object_instance,
//    bool oos_flag);

//bool Calendar_Create(
//    const uint32_t instance,
//    const char *name );

bool CalendarGetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo);

void Calendar_Init(
    void);

// static
//bool compare_calendar_entry(
//    BACNET_DATE * d,
//    BACNET_CALENDAR_ENTRY * ev);

int encode_calendar_entry(
    uint8_t *apdu,
	BACNET_CALENDAR_ENTRY *ev);

bool Calendar_Create(
    const uint32_t instance,
    const char *name);

#endif
