/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *   For more information : info@bac-test.com
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#pragma once

#include "osLayer.h"
#include "bacnet/bacenum.h"
#include "BACnetObject.h"
#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
#include "schedule.h"
#endif
#include "bacnet/bactimevalue.h"

typedef struct _virtDevInfo VirtualDeviceInfo;


void BACapi_Delete_Object(BACNET_OBJECT *objToDelet);
BACNET_OBJECT* BACapi_Find_Object(const uint devInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance);
BACNET_OBJECT* BACapi_Create_Object(const uint devInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, const char *name, const char *description );
BACNET_OBJECT* BACapi_Establish_Object(const uint devInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, const char* name, const char* description);
BACNET_OBJECT* BACapi_Create_Object_with_UserData(const uint devInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, const char* name, const char* description, void* userData);
VirtualDeviceInfo* Create_Device_Virtual_VMAC(
    const uint8_t portId,
    const unsigned devInstance,
    const char* devName,
    const char* devDesc,
    const unsigned vendorId,
    const char* vendorName,
    const uint16_t vMAClocal);

#if ( BAC_DEBUG == 1 )
void Test_BACapi(void);
#endif

// The side-affect behaviour of the BACnet driver will be different Between modifying a value from the server-side as opposed the client-side.
// (This is not a Server-side Device vs Client-side Device issue, but what the action of setting the value triggers.

// Client-side stores

bool BACapi_Set_PV_double_From_Client_Side(
    const uint32_t deviceInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance, 
    const float pv);

bool BACapi_Set_PV_unsigned_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const uint pv);

bool BACapi_Set_PV_boolean_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const bool pv);

bool BACapi_Set_PV_adv_From_Client_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const BACNET_APPLICATION_DATA_VALUE *value );

// Server-side stores

bool BACapi_Set_PV_double_From_Server_Side(
    const uint32_t deviceInstance, 
    const BACNET_OBJECT_TYPE objType, 
    const uint objInstance, 
    const double pv);

bool BACapi_Set_PV_unsigned_From_Server_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const uint pv);

bool BACapi_Set_PV_boolean_From_Server_Side(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    const bool pv);

bool BACapi_Get_PV_double(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    double* pv);

bool BACapi_Get_PV_boolean(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    bool* pv);

bool BACapi_Get_PV_uint(
    const uint32_t deviceInstance,
    const BACNET_OBJECT_TYPE objType,
    const uint objInstance,
    unsigned int* pv);

void BACapi_Analog_Update_Units(const uint32_t deviceInstance, const BACNET_OBJECT_TYPE objType, const uint objInstance, BACNET_ENGINEERING_UNITS units);
BACNET_ENGINEERING_UNITS bits_String_ToUnits(const char *unitsName);

void BACnet_Object_Units_Set(
    BACNET_OBJECT* currentObject, 
    BACNET_ENGINEERING_UNITS units);

// Client-side functionality
void BACapi_Set_Read_Polling_Callback(void(*callback)(void));

void BACapi_Create_Analog_COV_Subscription(const uint32_t devInstance, const BACNET_OBJECT_TYPE objType, const uint32_t objInstance, const double covIncrement, void(*callback)(void));
void BACapi_Create_Binary_COV_Subscription(const uint32_t devInstance, const BACNET_OBJECT_TYPE objType, const uint32_t objInstance, void(*callback)(void));
void BACapi_Create_Read_Polling_Record(const uint32_t devInstance, const BACNET_OBJECT_TYPE objType, const uint32_t objInstance, const uint pollInterval_ms);
void BACapi_Create_Write_Handling_Record(const uint32_t devInstance, const BACNET_OBJECT_TYPE objType, const uint32_t objInstance, const uint pollInterval_ms, void(*callback)(void));

void BACapi_Client_Side_Init(void(*ReadPollingCallbackPtr)(void));

BACNET_APPLICATION_DATA_VALUE* BACapi_adv_Real(double rval);

BACNET_TIME* BACapi_adv_Time(
    uint hrs,
    uint mins,
    uint secs);

BACNET_TIME_VALUE* BACapi_adv_TimeValue(
    BACNET_TIME* time,
    BACNET_APPLICATION_DATA_VALUE* adv);

#if ( BACNET_USE_OBJECT_SCHEDULE == 1) 
void BACapi_Schedule_Weekly_Schedule_Add(
    SCHEDULE_DESCR* schedObj,
    BACNET_DAYS_OF_WEEK weekDay,
    BACNET_TIME_VALUE *timeValue);                 // this value must be created and assigned to using one of the BACapi_adv_...() functions, and never reused, freed etc.

void BACapi_Schedule_Weekly_Schedule_Remove(
    SCHEDULE_DESCR* schedObj,
    BACNET_DAYS_OF_WEEK weekDay,
    BACNET_TIME* time );

void BACapi_Schedule_Special_Event_Add2(
    SCHEDULE_DESCR* schedObj,
    BACNET_SPECIAL_EVENT* specialEvent);

void BACapi_Schedule_Special_Event_Add(
    SCHEDULE_DESCR* schedObj,
    BACNET_SPECIAL_EVENT* specialEvent);

void BACapi_Special_Event_TimeValue_Add(
    BACNET_SPECIAL_EVENT* specialEvent,
    BACNET_TIME_VALUE * timeValue );

BACNET_SPECIAL_EVENT_PERIOD* BACapi_adv_Special_Event_Period_Calendar_Reference( 
    uint32_t calendarObjectInstance );

BACNET_SPECIAL_EVENT_PERIOD* BACapi_adv_Special_Event_Period_Calendar_Entry(
    BACNET_CALENDAR_ENTRY *calendarEntry);

BACNET_SPECIAL_EVENT* BACapi_adv_Special_Event(
    BACNET_SPECIAL_EVENT_PERIOD* specialEventPeriod,
    uint priority );

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_WeekNDay(
    uint month,
    uint week,
    uint dayofweek);

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_Date(
    uint year,
    uint month,
    uint day,
    BACNET_WEEKDAY wday);

BACNET_CALENDAR_ENTRY* BACapi_adv_CalendarEntry_DateRange(
    uint year,
    uint month,
    uint day,
    uint year2,
    uint month2,
    uint day2);
#endif

double BACapi_Get_adv_double(const BACNET_APPLICATION_DATA_VALUE* adv);
BACNET_UNSIGNED_INTEGER   BACapi_Get_adv_unsigned(const BACNET_APPLICATION_DATA_VALUE* adv);

bool BACapi_isAnalogObject(const BACNET_OBJECT_TYPE objType);
bool BACapi_isBinaryObject(const BACNET_OBJECT_TYPE objType);
bool BACapi_isMultistateObject(const BACNET_OBJECT_TYPE objType);

#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
SCHEDULE_DESCR* Iterate_Schedule(const uint32_t deviceInstance, const SCHEDULE_DESCR *priorSchedule);
#endif

void BACapi_Server_Side_Write_Output_Init(
bool (*Update_Function_Write_Output) (const BACNET_OBJECT * currentObject, const int priority, const BACNET_APPLICATION_DATA_VALUE* value));

