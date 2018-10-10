/**************************************************************************
*
* Copyright (C) 2015 Nikola Jelic <nikola.jelic@euroicc.com>
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

#ifndef SCHEDULE_H
#define SCHEDULE_H

//#include <stdbool.h>
//#include <stdint.h>
//
//#include "bacenum.h"
//#include "bacdef.h"
//#include "bacapp.h"
//#include "datetime.h"
//#include "bacerror.h"
//#include "wp.h"
//#include "rp.h"
//#include "bacdevobjpropref.h"
#include "bactimevalue.h"
#include "readrange.h"
//#include "BACnetObject.h"
#include "calendar.h"

#ifndef BACNET_WEEKLY_SCHEDULE_SIZE
#define BACNET_WEEKLY_SCHEDULE_SIZE 2           /* maximum number of data points for each day [BTC - check what happens when overflowing] */
#endif

#ifndef BACNET_SCHEDULE_OBJ_PROP_REF_SIZE
#define BACNET_SCHEDULE_OBJ_PROP_REF_SIZE 2     /* maximum number of obj prop references [BTC - check what happens when overflowing] */
#endif

#define MX_EXCEPTION_SCHEDULE			2      // user modifiable [BTC - check what happens when overflowing] 
#define MX_SPECIAL_EVENT_TIME_VALUES	2

typedef enum
{
    EXCEPTION_CALENDAR_NONE,
    EXCEPTION_CALENDAR_ENTRY,
    EXCEPTION_CALENDAR_REFERENCE
} Exception_Type ;


typedef struct bacnet_daily_schedule {
    BACNET_TIME_VALUE	Time_Values[BACNET_WEEKLY_SCHEDULE_SIZE];
    uint16_t			ux_TimeValues ;
} BACNET_DAILY_SCHEDULE;


/*
	BACnetSpecialEvent ::= SEQUENCE {
		period CHOICE {
			calendar-entry          [0] BACnetCalendarEntry,
			calendar-reference      [1] BACnetObjectIdentifier
			},
		list-of-time-values     [2] SEQUENCE OF BACnetTimeValue,
		event-priority          [3] Unsigned (1..16)
}
*/

typedef struct  BACnet_Special_Event
{
	Exception_Type      type;
	union {
		BACNET_CALENDAR_ENTRY	calendarEntry;
		uint32_t				calendarReferenceInstance;
	} choice ;
	BACNET_TIME_VALUE	listOfTimeValues[MX_SPECIAL_EVENT_TIME_VALUES];
	uint8_t             ux_TimeValues1;
	uint8_t	            priority;
} BACNET_SPECIAL_EVENT;


typedef struct schedule_descr {

    BACNET_OBJECT   common;         // must be first field in structure due to llist

    BACNET_APPLICATION_DATA_VALUE   Present_Value;   /* must be set to a valid value. Default is Schedule_Default */
    
    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;

#if ( BACNET_SVC_COV_B == 1 )
    bool Changed;
#endif

    /* Effective Period: Start and End Date */
    BACNET_DATE_RANGE   Effective_Period;

    /* Properties concerning Present Value */
    BACNET_DAILY_SCHEDULE Weekly_Schedule[MAX_BACNET_DAYS_OF_WEEK];
    
	BACNET_SPECIAL_EVENT	Exception_Schedule[MX_EXCEPTION_SCHEDULE];
	uint8_t					ux_special_events;

    BACNET_APPLICATION_DATA_VALUE				Schedule_Default;
    
	BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE		Object_Property_References[BACNET_SCHEDULE_OBJ_PROP_REF_SIZE];
    uint8_t ux_ObjectPropertyList;       

    uint8_t Priority_For_Writing;   /* (1..16) */

    BACNET_APPLICATION_TAG ScheduleTag;

    bool Event_State;

} SCHEDULE_DESCR;


void Schedule_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

bool Schedule_Valid_Instance(
    uint32_t object_instance);

unsigned Schedule_Count(
    void);

uint32_t Schedule_Index_To_Instance(
    unsigned index);

// 2018.06.17 obsolete unsigned Schedule_Instance_To_Index(
//    uint32_t instance);
    
// static void Schedule_Out_Of_Service_Set(
// uint32_t object_instance,
//    bool value);
        
bool Schedule_Out_Of_Service(
    uint32_t object_instance);

bool Schedule_Description_Set(
    uint32_t instance,
    char *new_name);


int Schedule_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Schedule_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

float Schedule_Present_Value(
    uint32_t object_instance);

bool Schedule_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

bool Schedule_Name_Set(
    uint32_t object_instance,
    char *new_name);

char *Schedule_Description(
    uint32_t instance);

bool Schedule_Description_Set(
    uint32_t instance,
    char *new_name);


int Schedule_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Schedule_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

float Schedule_Present_Value(
    uint32_t object_instance);

void Schedule_Present_Value_Set(
    uint32_t object_instance,
    float value);

// static void Schedule_Out_Of_Service_Set(
// uint32_t object_instance,
//    bool value);

/* utility functions for calculating current Present Value
 * if Exception Schedule is to be added, these functions must take that into account */
bool Schedule_In_Effective_Period(SCHEDULE_DESCR * desc,
    BACNET_DATE * date);

void Schedule_Recalculate_PV(SCHEDULE_DESCR * desc,
    BACNET_WEEKDAY wday,
    BACNET_TIME * time);

bool Schedule_GetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo);

void Schedule_Init(
    void);

bool Schedule_Create(
    const uint32_t instance,
    const char *name);

#endif
