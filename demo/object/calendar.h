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
*********************************************************************/

#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "bactimevalue.h"
#include "datetime.h"
#include "schedule.h"

#define MAX_CALENDAR_EVENTS  32

enum {
    CALENDAR_ENTRY_NONE,
    CALENDAR_ENTRY_DATE,
    CALENDAR_ENTRY_RANGE,
    CALENDAR_ENTRY_WEEK
};

#define BACNET_WEEK_N_DAY_INIT(month, weekOfMonth, dayOfWeek) \
    {(month), (weekOfMonth), (dayOfWeek)}

#define BACNET_DATE_VALUE(d) \
    ((d).day + (d).month * 100 + (d).year * 10000)

/*
BACnetCalendarEntry ::= CHOICE {
	date [0] Date,
	dateRange [1] BACnetDateRange,
	weekNDay [2] BACnetWeekNDay
	}
*/
typedef struct BACnet_CalendarEvent {
    uint8_t                 type;       // 0 = date, 1 = date range, 2 week-n-day
    BACNET_CALENDAR_ENTRY   entry;
} BACNET_CALENDAR_EVENT;


void Calendar_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary);

bool Calendar_Valid_Instance(
    uint32_t object_instance);
unsigned Calendar_Count(
    void);
uint32_t Calendar_Index_To_Instance(
    unsigned index);
unsigned Calendar_Instance_To_Index(
    uint32_t instance);
bool Calendar_Object_Instance_Add(
    uint32_t instance);

bool Calendar_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);
bool Calendar_Name_Set(
    uint32_t object_instance,
    char *new_name);

char *Calendar_Description(
    uint32_t instance);
bool Calendar_Description_Set(
    uint32_t instance,
    char *new_name);


int Calendar_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);
bool Calendar_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

float Calendar_Present_Value(
    uint32_t object_instance);
void Calendar_Present_Value_Set(
    uint32_t object_instance,
    float value);

void Calendar_Out_Of_Service_Set(
    uint32_t object_instance,
    bool oos_flag);

void Calendar_Init(
    void);

#endif
