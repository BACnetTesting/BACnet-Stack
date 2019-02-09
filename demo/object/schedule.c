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

//#include <stdbool.h>
//#include <stdint.h>
#include <memory.h>

//#include "bacdef.h"
//#include "bacdcode.h"
#include "bacenum.h"
//#include "bactext.h"
//#include "config.h"
//#include "device.h"
#include "handlers.h"
//#include "timestamp.h"
#include "schedule.h"
#include "emm.h"
#include "bitsDebug.h"
#include "calendar.h"
#include "datetime.h"
#include "llist.h"

LLIST_HDR Schedule_Descriptor_List;

static const BACNET_PROPERTY_ID Schedule_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EFFECTIVE_PERIOD,
    PROP_SCHEDULE_DEFAULT,
    PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES,
    PROP_PRIORITY_FOR_WRITING,
    PROP_RELIABILITY,           // this is required for schedule, but not for e.g. AI
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Schedule_Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_WEEKLY_SCHEDULE,
    PROP_EXCEPTION_SCHEDULE,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Schedule_Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


bool Schedule_Create(
    const uint32_t instance,
    const char *name)
{
    SCHEDULE_DESCR *currentObject = (SCHEDULE_DESCR *)emm_scalloc('a', sizeof(SCHEDULE_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&Schedule_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    currentObject->Present_Value.tag = BACNET_APPLICATION_TAG_NULL;
    currentObject->Out_Of_Service = false;
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;

    return true;
}


void Schedule_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    if (pRequired)
        *pRequired = Schedule_Properties_Required;
    if (pOptional)
        *pOptional = Schedule_Properties_Optional;
    if (pProprietary)
        *pProprietary = Schedule_Properties_Proprietary;
}


int encode_context_calendar_entry(uint8_t *apdu, uint8_t tag, BACNET_CALENDAR_ENTRY *calendarEntry)
{
    int len = 1;
    apdu[0] = tag;
    len += encode_context_date(&apdu[len], 0, &calendarEntry->CalEntryChoice.date);
    return len;
}


// encode_daily_schedule?
//int encode_weekly_schedule(uint8_t * apdu, uint32_t day)
//{
//    BACNET_TIME_VALUE time_value;
//    int len = 0;
//
//    time_value.Value.context_specific = true;
//
//    len += encode_opening_tag(&apdu[len], 0);
//    // encode list of time/value pairs
//    for (unsigned n = 0; n < nv->u.DailyEventCount[day]; n++) {
//        if (nv->u.WeeklySchedule[day][n].value.u == NULL_VALUE) {
//            time_value.Value.tag = BACNET_APPLICATION_TAG_NULL;
//        }
//        else {  /* 32-bit copy */
//            time_value.Value.type.Unsigned_Int = nv->u.WeeklySchedule[day][n].value.u;
//            time_value.Value.tag = nv->u.ScheduleTag;
//        }
//        datetime_copy_time(&time_value.Time, &nv->u.WeeklySchedule[day][n].time);
//        len += bacapp_encode_time_value(&apdu[len], &time_value);
//    }
//    len += encode_closing_tag(&apdu[len], 0);
//    return len;
//}


int encode_special_event(uint8_t *apdu, BACNET_SPECIAL_EVENT *exception )
{
    //    BACNET_SPECIAL_EVENT *exception = NULL;
    int len = 0;

    // [0,1] CalendarEntry / reference
    switch (exception->type) {

    case EXCEPTION_CALENDAR_ENTRY:
        len += encode_opening_tag(&apdu[len], 0);
        len += encode_calendar_entry(&apdu[len], &exception->choice.calendarEntry);
        len += encode_closing_tag(&apdu[len], 0);
        break;

    case EXCEPTION_CALENDAR_REFERENCE:
        len += encode_context_object_id(&apdu[len], 1, OBJECT_CALENDAR, exception->choice.calendarReferenceInstance);
        break;
    }

    // [2] listOfTimeValues
    BACNET_TIME_VALUE time_value;
    time_value.Value.context_specific = true;
    len += encode_opening_tag(&apdu[len], 2);
    int i ;
    for (i = 0; i < exception->ux_TimeValues1; i++) {
        datetime_copy_time(&time_value.Time, &exception->listOfTimeValues[i].Time);
        len += bacapp_encode_time_value(&apdu[len], &exception->listOfTimeValues[i]);
    }

    len += encode_closing_tag(&apdu[len], 2);

    // [3] priority
    len += encode_context_unsigned(&apdu[len], 3, exception->priority);

    return len;
}


static int weekly_event_count(BACNET_DAILY_SCHEDULE *weeklySchedule)
{
    int count = 0;
    int i;
    for (i = 0; i < MAX_BACNET_DAYS_OF_WEEK; i++) {
        count += weeklySchedule[i].ux_TimeValues;
    }
    return count;
}


// Gets called once for each device

void Schedule_Init(
    void)
{
    ll_Init(&Schedule_Descriptor_List, 100);
}


bool Schedule_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&Schedule_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Schedule_Count(
    void)
{
    return Schedule_Descriptor_List.count;
}


uint32_t Schedule_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&Schedule_Descriptor_List, index);
}


SCHEDULE_DESCR *Schedule_Instance_To_Object(
    uint32_t object_instance)
{
    return (SCHEDULE_DESCR *)Generic_Instance_To_Object(&Schedule_Descriptor_List, object_instance);
}


bool Schedule_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name(&Schedule_Descriptor_List, object_instance, object_name);
}


static void Schedule_Out_Of_Service_Set(
    SCHEDULE_DESCR *currentObject,
    const bool value)
{
#if ( BACNET_SVC_COV_B == 1 )
    if (currentObject->Out_Of_Service != value) {
        currentObject->Changed = true;
    }
#endif

    currentObject->Out_Of_Service = value;
}


int Schedule_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    SCHEDULE_DESCR *currentObject;
    int i;

    const BACNET_PROPERTY_ID *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;

    currentObject = Schedule_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0], OBJECT_SCHEDULE,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
        Schedule_Object_Name(rpdata->object_instance, &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0], OBJECT_SCHEDULE);
        break;

    case PROP_PRESENT_VALUE:

        // todo2 - extract common? encoding method
        // make evaluation should be event driven?

        switch (currentObject->Present_Value.tag) {
        case BACNET_APPLICATION_TAG_NULL:
            apdu_len = encode_application_null(&apdu[0]);
            break;
        case BACNET_APPLICATION_TAG_REAL:
            apdu_len = encode_application_real(&apdu[0], currentObject->Present_Value.type.Real);
            break;
        case BACNET_APPLICATION_TAG_BOOLEAN:
            apdu_len = encode_application_boolean(&apdu[0], currentObject->Present_Value.type.Boolean);
            break;
        case BACNET_APPLICATION_TAG_ENUMERATED:
            apdu_len = encode_application_enumerated(&apdu[0], currentObject->Present_Value.type.Enumerated);
            break;
        case BACNET_APPLICATION_TAG_SIGNED_INT:
            apdu_len = encode_application_signed(&apdu[0], currentObject->Present_Value.type.Signed_Int);
            break;
        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            apdu_len = encode_application_unsigned(&apdu[0], currentObject->Present_Value.type.Unsigned_Int);
            break;
        default:
            apdu_len = encode_application_unsigned(&apdu[0], currentObject->Present_Value.type.Unsigned_Int);
            // and for good measure
            panic();
            break;
        }
        break;

    case PROP_STATUS_FLAGS:
        bitstring_init(&bit_string);
#if (INTRINSIC_REPORTING_B == 1)
        bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM,
            currentObject->Event_State ? true : false);
#else
        bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
#endif
        bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
            currentObject->Out_Of_Service);

        apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
        break;

    case PROP_RELIABILITY:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                RELIABILITY_NO_FAULT_DETECTED);
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len =
            encode_application_boolean(&apdu[0],
                currentObject->Out_Of_Service);
        break;

    case PROP_SCHEDULE_DEFAULT:
        switch (currentObject->Schedule_Default.tag) {
        case BACNET_APPLICATION_TAG_NULL:
            apdu_len = encode_application_null(&apdu[0]);
            break;
        case BACNET_APPLICATION_TAG_REAL:
            apdu_len = encode_application_real(&apdu[0], currentObject->Schedule_Default.type.Real);
            break;

        case BACNET_APPLICATION_TAG_BOOLEAN:
            apdu_len = encode_application_boolean(&apdu[0], currentObject->Schedule_Default.type.Boolean);
            break;

        case BACNET_APPLICATION_TAG_ENUMERATED:
            apdu_len = encode_application_enumerated(&apdu[0], currentObject->Schedule_Default.type.Enumerated);
            break;

        case BACNET_APPLICATION_TAG_SIGNED_INT:
            apdu_len = encode_application_signed(&apdu[0], currentObject->Schedule_Default.type.Signed_Int);
            break;

        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            apdu_len = encode_application_unsigned(&apdu[0], currentObject->Schedule_Default.type.Unsigned_Int);
            break;
        default:
            apdu_len = encode_application_unsigned(&apdu[0], currentObject->Schedule_Default.type.Unsigned_Int);
            panic();
            break;
        }
        break;


    case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
        for (i = 0; i < currentObject->ux_ObjectPropertyList; i++) {
            apdu_len +=
                bacapp_encode_device_obj_property_ref(&apdu[apdu_len],
                    &currentObject->Object_Property_References[i]);
        }
        break;

    case PROP_EFFECTIVE_PERIOD:
        /*  BACnet Testing Observed Incident oi00110
            Effective Period of Schedule object not correctly formatted
            Revealed by BACnet Test Client v1.8.16 ( www.bac-test.com/bacnet-test-client-download )
                BITS: BIT00031
            Any discussions can be directed to edward@bac-test.com
            Please feel free to remove this comment when my changes accepted after suitable time for
            review by all interested parties. Say 6 months -> September 2016 */
        apdu_len = encode_application_date(&apdu[0], &currentObject->Effective_Period.startdate);
        apdu_len +=
            encode_application_date(&apdu[apdu_len], &currentObject->Effective_Period.enddate);
        break;


    case PROP_PRIORITY_FOR_WRITING:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                currentObject->Priority_For_Writing);
        break;


    case PROP_WEEKLY_SCHEDULE:
        if (rpdata->array_index == 0)       /* count, always 7 */
            apdu_len = encode_application_unsigned(&apdu[0], 7);
        else if (rpdata->array_index == BACNET_ARRAY_ALL) { /* full array */
            int day;
            for (day = 0; day < 7; day++) {
                apdu_len += encode_opening_tag(&apdu[apdu_len], 0);
                for (i = 0; i < currentObject->Weekly_Schedule[day].ux_TimeValues;
                    i++) {
                    apdu_len +=
                        bacapp_encode_time_value(&apdu[apdu_len],
                            &currentObject->Weekly_Schedule[day].Time_Values[i]);
                }
                apdu_len += encode_closing_tag(&apdu[apdu_len], 0);
            }
        }
        else if (rpdata->array_index <= 7) {      /* some array element */
            int day = rpdata->array_index - 1;
            apdu_len += encode_opening_tag(&apdu[apdu_len], 0);
            for (i = 0; i < currentObject->Weekly_Schedule[day].ux_TimeValues; i++) {
                apdu_len +=
                    bacapp_encode_time_value(&apdu[apdu_len],
                        &currentObject->Weekly_Schedule[day].Time_Values[i]);
            }
            apdu_len += encode_closing_tag(&apdu[apdu_len], 0);
        }
        else {    /* out of bounds */
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
            apdu_len = BACNET_STATUS_ERROR;
        }
        break;


    case PROP_EXCEPTION_SCHEDULE:
        if (rpdata->array_index == 0) {
            apdu_len = encode_application_unsigned(&apdu[0], currentObject->ux_special_events);
        }
        else if (rpdata->array_index > currentObject->ux_special_events && rpdata->array_index != BACNET_ARRAY_ALL) {
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
            apdu_len = BACNET_STATUS_ERROR;
        }
        else {
            for (i = 0; i < currentObject->ux_special_events; i++) {
                if (rpdata->array_index != BACNET_ARRAY_ALL && rpdata->array_index != (i + 1)) continue;
                apdu_len += encode_special_event(&apdu[apdu_len], &currentObject->Exception_Schedule[i] );
            }
        }
        break;

    case PROP_PROPERTY_LIST:
        Schedule_Property_Lists(&pRequired, &pOptional, &pProprietary);
        apdu_len = property_list_encode(
            rpdata,
            pRequired,
            pOptional,
            pProprietary);
        break;

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) &&
        (rpdata->object_property != PROP_PROPERTY_LIST) &&
        (rpdata->object_property != PROP_WEEKLY_SCHEDULE) &&
        (rpdata->object_property != PROP_EXCEPTION_SCHEDULE) &&
#if (INTRINSIC_REPORTING_B == 1)
        (rpdata->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}


/* returns true if successful */
bool Schedule_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    /* Ed->Steve, I know that initializing stack values used to be 'safer', but warnings in latest compilers indicate when
        uninitialized values are being used, and I think that the warnings are more useful to reveal bad code flow than the
        "safety: of pre-intializing variables. Please give this some thought let me know if you agree we should start to
        remove initializations */
    int i = 0;
    int len;
    int apdu_len = 0;
    bool status = false;        /* return value */
//    bool boolReturn;
    BACNET_DATE_RANGE daterange;
    BACNET_APPLICATION_DATA_VALUE value;
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE properties[BACNET_SCHEDULE_OBJ_PROP_REF_SIZE];
    BACNET_SPECIAL_EVENT ExceptionSchedule[MX_EXCEPTION_SCHEDULE];
    BACNET_DAILY_SCHEDULE Weekly_Schedule[7];
    uint32_t local_ux_ExceptionSchedule;
    uint32_t ScheduleTag;
    SCHEDULE_DESCR *currentObject;

    /* decode some of the request */
    if (!bacapp_decode_application_data_safe(wp_data->application_data, wp_data->application_data_len, &value)) {
        value.tag = MAX_BACNET_APPLICATION_TAG;
    }

    /* some properties are read-only */
    switch (wp_data->object_property) {
    case PROP_PROPERTY_LIST:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_DESCRIPTION:
    case PROP_RELIABILITY:
        // BTC todo - reliability should be writeable if OOS
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    }

    /*  only array properties can have array options */
    if ((wp_data->array_index != BACNET_ARRAY_ALL) &&
#if (INTRINSIC_REPORTING_B == 1)
        (wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
        (wp_data->object_property != PROP_EXCEPTION_SCHEDULE) &&
        (wp_data->object_property != PROP_WEEKLY_SCHEDULE) &&
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;

    currentObject = Schedule_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Schedule_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_EFFECTIVE_PERIOD:
        /* decode first element of the request */
        len = bacapp_decode_application_data(wp_data->application_data, wp_data->application_data_len, &value);
        if (len > 0 && WPValidateArgType(&value, BACNET_APPLICATION_TAG_DATE, &wp_data->error_class, &wp_data->error_code)) {
            daterange.startdate = value.type.Date;
            len = bacapp_decode_application_data(&wp_data->application_data[len], wp_data->application_data_len, &value);
            if (len > 0 && WPValidateArgType(&value, BACNET_APPLICATION_TAG_DATE, &wp_data->error_class, &wp_data->error_code)) {
                daterange.enddate = value.type.Date;
                if (daterange_is_valid(&daterange)) {
                    memcpy(&currentObject->Effective_Period, &daterange, sizeof(daterange));
                    status = true;
                }
            }
        }
        // todo - correct BTL error code?
        break;

    case PROP_WEEKLY_SCHEDULE:
        ScheduleTag = currentObject->ScheduleTag;
        for (i = 0; i < MAX_BACNET_DAYS_OF_WEEK; i++) {
            if (wp_data->array_index != BACNET_ARRAY_ALL && wp_data->array_index != (i + 1)) continue;

            Weekly_Schedule[i].ux_TimeValues = 0;

            // todo 3 - if no opening tag, should it send abort?
            if (!IS_OPENING_TAG(wp_data->application_data[apdu_len++])) {
                // todo 7 - check error code in BTL conformance docs
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                return false;
            }
            while (!IS_CLOSING_TAG(wp_data->application_data[apdu_len]) && apdu_len < wp_data->application_data_len) {
                // todo - check error code in BTL conformance docs
                if (Weekly_Schedule[i].ux_TimeValues >= BACNET_WEEKLY_SCHEDULE_SIZE) {
                    panic(); // there is a code smell here, how on earth did this happen?
                    return false;
                }
                // we can extract a list of timevalue pairs
                len = bacapp_decode_application_data(&wp_data->application_data[apdu_len], wp_data->application_data_len, &value);
                if (len > 0 && WPValidateArgType(&value, BACNET_APPLICATION_TAG_TIME, &wp_data->error_class, &wp_data->error_code)) {
                    // reject duplicate time values
                    int j ;
                    for (j = 0; j < Weekly_Schedule[i].ux_TimeValues; j++) {
                        if (datetime_compare_time(&Weekly_Schedule[i].Time_Values[j].Time, &value.type.Time) == 0) {
                            wp_data->error_code = ERROR_CODE_DUPLICATE_NAME; // ERROR_CODE_DUPLICATE_ENTRY; // todo1 - re-evaluate this
                            return false;
                        }
                    }
                    if (!datetime_time_is_valid(&value.type.Time)) {
                        return false;
                    }
                    Weekly_Schedule[i].Time_Values[Weekly_Schedule[i].ux_TimeValues].Time = value.type.Time;
                    apdu_len += len;
                    // decode application value
                    len = bacapp_decode_application_data(&wp_data->application_data[apdu_len], wp_data->application_data_len, &value);
                    if (len > 0) {
                        switch (value.tag) {
                        case BACNET_APPLICATION_TAG_NULL:
                            Weekly_Schedule[i].Time_Values[Weekly_Schedule[i].ux_TimeValues].Value.tag = BACNET_APPLICATION_TAG_NULL;
                            break;
                        case BACNET_APPLICATION_TAG_REAL:
                        case BACNET_APPLICATION_TAG_BOOLEAN:
                        case BACNET_APPLICATION_TAG_ENUMERATED:
                        case BACNET_APPLICATION_TAG_SIGNED_INT:
                        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                            // update the tag if not already set
                            if (ScheduleTag == BACNET_APPLICATION_TAG_NULL) ScheduleTag = value.tag;
                            if (ScheduleTag == value.tag) {
                                Weekly_Schedule[i].Time_Values[Weekly_Schedule[i].ux_TimeValues].Value.type.Unsigned_Int = value.type.Unsigned_Int;
                                break;
                            }
                            break;  // todo1 - get this back to Karg?
                        default:
                            // todo 5 - check error code in BTL conformance docs
                            wp_data->error_code = ERROR_CODE_DATATYPE_NOT_SUPPORTED;
                            return false;
                        }
                        apdu_len += len;
                    }
                }
                else {
                    // todo 6 - check error code in BTL conformance docs
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }
                Weekly_Schedule[i].ux_TimeValues++;
            }
            // todo 7 - if no closing tag, should it send abort?
            // assert closing constructed tag for the daily schedule
            if (!IS_CLOSING_TAG(wp_data->application_data[apdu_len++]) && apdu_len == wp_data->application_data_len) {
                // todo 8 - check error code in BTL conformance docs
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                return false;
            }
        }
        if (ScheduleTag != currentObject->ScheduleTag) {
            if (currentObject->ScheduleTag != BACNET_APPLICATION_TAG_NULL) {
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                return false;
            }
            currentObject->ScheduleTag = (BACNET_APPLICATION_TAG)ScheduleTag;
        }
        // in daily schedule now memcpy(&currentObject->DailyEventCount, &DailyEventCount, sizeof(DailyEventCount));
        memcpy(&currentObject->Weekly_Schedule, &Weekly_Schedule, sizeof(Weekly_Schedule));
        status = true;
        break;

    case PROP_EXCEPTION_SCHEDULE:
        if (wp_data->array_index == 0) {
            // writing to array index 0 changes size of array
            len = bacapp_decode_application_data(&wp_data->application_data[apdu_len], wp_data->application_data_len - apdu_len, &value);
            if (len > 0) {
                if (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT || value.type.Unsigned_Int > MX_EXCEPTION_SCHEDULE) {
                    // todo 9 - check error code in BTL conformance docs
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    return false;
                }
                apdu_len += len;
                // sum total count of special and daily events
                i = currentObject->ux_special_events + weekly_event_count(currentObject->Weekly_Schedule);
                // no events but ScheduleTag is non-null?
                if (i == 0 && currentObject->ScheduleTag) {
                    currentObject->ScheduleTag = BACNET_APPLICATION_TAG_NULL;
                }
                // zero/clear unused exception schedules in array...
                for (i = value.type.Unsigned_Int; i < MX_EXCEPTION_SCHEDULE; i++) {
                    memset(&currentObject->Exception_Schedule[i], 0, sizeof(BACNET_SPECIAL_EVENT));
                }
                currentObject->ux_special_events = value.type.Unsigned_Int;
            }
        }
        else if (wp_data->application_data_len == 0) {
            // writing an empty array [btc todo]
            memset(currentObject->Exception_Schedule, 0, sizeof(currentObject->Exception_Schedule));
            currentObject->ux_special_events = 0;
            return true;
        }
        else {
            // allow unspecified index or range 1 ... MAX_SPECIAL_EVENTS
            i = (wp_data->array_index == BACNET_ARRAY_ALL ? currentObject->ux_special_events : wp_data->array_index - 1);
            // invalid array index?
            if (i >= MX_EXCEPTION_SCHEDULE) return false;
            local_ux_ExceptionSchedule = currentObject->ux_special_events;
            memcpy(ExceptionSchedule, &currentObject->Exception_Schedule, sizeof(ExceptionSchedule));
            // ...and insert/append one or more array elements
            uint8_t *apdu ;
            for (apdu = wp_data->application_data; i < MX_EXCEPTION_SCHEDULE, !IS_CLOSING_TAG(wp_data->application_data[apdu_len]), apdu_len < wp_data->application_data_len; i++) {
                if (wp_data->array_index != BACNET_ARRAY_ALL && wp_data->array_index != (i + 1)) continue;
                // expecting context specific tag...
                if (IS_EXTENDED_TAG_NUMBER(*apdu) || !IS_CONTEXT_SPECIFIC(*apdu)) {
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                    return false;
                }
                // clear/reset exception schedule
                memset(&ExceptionSchedule[i], 0, sizeof(BACNET_SPECIAL_EVENT));

                // [0]calendarEntry
                if (decode_is_opening_tag_number(&apdu[apdu_len + 0], 0) && IS_CONTEXT_SPECIFIC(apdu[apdu_len+1])) {
                    apdu_len++;
                    // [0]calendarEntry
                    ExceptionSchedule[i].type = EXCEPTION_CALENDAR_ENTRY;

                    switch ((CAL_ENTRY_TAG)((apdu[apdu_len++] >> 4) & 0x0f)) {
                    case CALENDAR_ENTRY_DATE:
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.date.year = apdu[apdu_len++] + BACNET_EPOCH_YEAR;
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.date.month = apdu[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.date.day = apdu[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.date.wday = (BACNET_WEEKDAY)apdu[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.tag = CALENDAR_ENTRY_DATE;
                        break;

                    case CALENDAR_ENTRY_RANGE:
                        if (!IS_OPENING_TAG(apdu[apdu_len - 1])) {   // opening tag [1E]
                            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                            return false;
                        }
                        apdu_len += decode_application_date(&apdu[apdu_len], &ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.range.startdate);
                        apdu_len += decode_application_date(&apdu[apdu_len], &ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.range.enddate);
                        if (!daterange_is_valid(&ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.range)) {
                            return false;
                        }
                        // expect closing tag [1F]
                        if (decode_is_closing_tag_number(&apdu[apdu_len], 1)) {
                            ExceptionSchedule[i].choice.calendarEntry.tag = CALENDAR_ENTRY_RANGE;
                            apdu_len++;
                        }
                        break;

                    case CALENDAR_ENTRY_WEEKNDAY:
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.weekNday.month = wp_data->application_data[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.weekNday.weekofmonth = wp_data->application_data[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.CalEntryChoice.weekNday.dayofweek = wp_data->application_data[apdu_len++];
                        ExceptionSchedule[i].choice.calendarEntry.tag = CALENDAR_ENTRY_WEEKNDAY;
                        break;

                    default:
                        panic();
                        break;
                    }
                    if (decode_is_closing_tag_number(&apdu[apdu_len], 0)) apdu_len++;
                }
                else if (decode_is_opening_tag_number(&apdu[apdu_len], 1)) {
                    BACNET_OBJECT_TYPE object;
                    uint32_t instance;
                    // point to calendar
                    len = decode_context_object_id(&wp_data->application_data[apdu_len], 1, &object, &instance);
                    if (len > 0 && object == OBJECT_CALENDAR && Calendar_Valid_Instance(instance)) {
                        // calendar ref
                        ExceptionSchedule[i].type = EXCEPTION_CALENDAR_REFERENCE;
                        ExceptionSchedule[i].choice.calendarReferenceInstance = instance;
                        apdu_len += len;
                    }
                }

                // [2]listOfTimeValues // todo - do we want to double check the tag value? todo 3
                if (IS_OPENING_TAG(wp_data->application_data[apdu_len++])) {
                    // extract list of time/value pairs
                    while (!IS_CLOSING_TAG(wp_data->application_data[apdu_len]) && apdu_len < wp_data->application_data_len) {
                        // room for another time/value pair?
                        if (ExceptionSchedule[i].ux_TimeValues1 >= MX_SPECIAL_EVENT_TIME_VALUES) {
                            // todo - check error code in BTL conformance docs
                            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                            return false;
                        }
                        len = bacapp_decode_application_data(&wp_data->application_data[apdu_len], wp_data->application_data_len, &value);
                        if (len > 0 && WPValidateArgType(&value, BACNET_APPLICATION_TAG_TIME, &wp_data->error_class, &wp_data->error_code)) {
                            int j ;
                            for (j = 0; j < ExceptionSchedule[i].ux_TimeValues1; j++) {
                                if (datetime_compare_time(&ExceptionSchedule[i].listOfTimeValues[j].Time, &value.type.Time) == 0) {
                                    wp_data->error_code = ERROR_CODE_DUPLICATE_NAME; // todo1 ERROR_CODE_DUPLICATE_ENTRY;
                                    return false;
                                }
                            }
                            if (!datetime_time_is_valid(&value.type.Time)) {
                                //wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                                return false;
                            }
                            ExceptionSchedule[i].listOfTimeValues[ExceptionSchedule[i].ux_TimeValues1].Time = value.type.Time;
                            apdu_len += len;
                            // decode application value
                            len = bacapp_decode_application_data(&wp_data->application_data[apdu_len], wp_data->application_data_len, &value);
                            if (len > 0) {
                                switch (value.tag) {
                                case BACNET_APPLICATION_TAG_NULL:
                                    ExceptionSchedule[i].listOfTimeValues[ExceptionSchedule[i].ux_TimeValues1].Value.tag = BACNET_APPLICATION_TAG_NULL;
                                    break;
                                case BACNET_APPLICATION_TAG_REAL:
                                case BACNET_APPLICATION_TAG_BOOLEAN:
                                case BACNET_APPLICATION_TAG_ENUMERATED:
                                case BACNET_APPLICATION_TAG_SIGNED_INT:
                                case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                                    // still accepting any type?
                                    if (currentObject->ScheduleTag == BACNET_APPLICATION_TAG_NULL) {
                                        currentObject->ScheduleTag = value.tag;
                                    }
                                    if (value.tag == currentObject->ScheduleTag) {
                                        ExceptionSchedule[i].listOfTimeValues[ExceptionSchedule[i].ux_TimeValues1].Value = value;
                                        break;
                                    }
                                default:
                                    // todo 5 - check error code in BTL conformance docs
                                    wp_data->error_code = ERROR_CODE_DATATYPE_NOT_SUPPORTED;
                                    return false;
                                }
                                apdu_len += len;
                            }
                        }
                        else {
                            // todo 6 - check error code in BTL conformance docs
                            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                            return false;
                        }
                        ExceptionSchedule[i].ux_TimeValues1++;
                    }
                    // todo 7 - if no closing tag, should it send abort?
                    // assert closing constructed tag for the daily schedule
                    if (!IS_CLOSING_TAG(wp_data->application_data[apdu_len++])) {
                        // error code?
                        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                        return false;
                    }
                    uint32_t  priority = BACNET_NO_PRIORITY;

                    // [3] eventPriority
                    len = decode_context_unsigned(&wp_data->application_data[apdu_len], 3, &priority);
                    if (len == BACNET_STATUS_ERROR) {
                        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE ;
                        return false;
                    }
                    // validate priority range 1..16
                    if (priority > BACNET_MAX_PRIORITY) {
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                        return false;
                    }
                    else if (priority == BACNET_NO_PRIORITY) {
                        ExceptionSchedule[i].priority = BACNET_MAX_PRIORITY;
                    }
                    else {
                        ExceptionSchedule[i].priority = priority;
                    }
                    apdu_len += len;
                    // "auto-insert" this index and adjust array size
                    if (local_ux_ExceptionSchedule <= i) local_ux_ExceptionSchedule = (i + 1);
                }
            }

            if (local_ux_ExceptionSchedule) {
                memcpy(currentObject->Exception_Schedule, &ExceptionSchedule, local_ux_ExceptionSchedule * sizeof(BACNET_SPECIAL_EVENT));
                currentObject->ux_special_events = (uint8_t) local_ux_ExceptionSchedule;
                status = true;
            }
        }
        break;

    case PROP_PRIORITY_FOR_WRITING:
        status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT, &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Unsigned_Int < 1 || value.type.Unsigned_Int > 16) {
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
            }
            currentObject->Priority_For_Writing = value.type.Unsigned_Int;
        }
        break;

    case PROP_SCHEDULE_DEFAULT:

        // time to test this code
        switch (value.tag) {
        case BACNET_APPLICATION_TAG_NULL:
        case BACNET_APPLICATION_TAG_REAL:
        case BACNET_APPLICATION_TAG_BOOLEAN:
        case BACNET_APPLICATION_TAG_ENUMERATED:
        case BACNET_APPLICATION_TAG_SIGNED_INT:
        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            currentObject->Schedule_Default = value;
            status = true;
            break;

        default:
            // todo EKH - i think this is correct error code after reading email dated March 27 from BTL Manager (Duffy)
            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            return false;
        }

        // replaced with above ?
        //switch (value.tag) {
        //case BACNET_APPLICATION_TAG_NULL:
        //    break;
        //case BACNET_APPLICATION_TAG_REAL:
        //    currentObject->Schedule_Default.type.Real = value.type.Real;
        //    break;
        //case BACNET_APPLICATION_TAG_BOOLEAN:
        //    // todo1 - check paul's code here
        //    currentObject->Schedule_Default.type.Boolean = value.type.Boolean;
        //    break;
        //case BACNET_APPLICATION_TAG_ENUMERATED:
        //    // todo2 and here
        //    currentObject->Schedule_Default.type.Enumerated = value.type.Enumerated;
        //    break;
        //case BACNET_APPLICATION_TAG_SIGNED_INT:
        //    currentObject->Schedule_Default.type.Signed_Int = value.type.Signed_Int;
        //    break;
        // case BACNET_APPLICATION_TAG_UNSIGNED_INT:
        //    currentObject->Schedule_Default.type.Unsigned_Int = value.type.Unsigned_Int;
        //    break;
        //default:
        //    // todo EKH - i think this is correct error code after reading email dated March 27 from BTL Manager (Duffy)
        //    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        //    return false;
        //}
        //currentObject->Schedule_Default.tag = value.tag;
        break;

    case PROP_PRESENT_VALUE:
        // only allow writes if out-of-service is true (and the types match?) todo2 - test this last theory
        if (currentObject->Out_Of_Service) {
            if (value.tag == currentObject->ScheduleTag) {
                switch (value.tag) {
                case BACNET_APPLICATION_TAG_NULL:
                case BACNET_APPLICATION_TAG_REAL:
                case BACNET_APPLICATION_TAG_BOOLEAN:
                case BACNET_APPLICATION_TAG_ENUMERATED:
                case BACNET_APPLICATION_TAG_SIGNED_INT:
                case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                    currentObject->Present_Value = value;
                    status = true;
                    break;
                default:
                    // todo EKH - i think this is correct error code after reading email dated March 27 from BTL Manager (Duffy)
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    return false;
                }
            }
            else {
                // not sure how we ever get here..
                panic();
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                return false;
            }
        }
        else {
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            return false;
        }
        break;

    case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
        // zero out temporary list of object property references
        memset(properties, 0, sizeof(BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE) * BACNET_SCHEDULE_OBJ_PROP_REF_SIZE);
        // decode list of object property references into temporary list
        while (apdu_len < wp_data->application_data_len) {
            if (i >= BACNET_SCHEDULE_OBJ_PROP_REF_SIZE) {
                //wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
            }
            len = bacapp_decode_device_obj_property_ref(&wp_data->application_data[apdu_len], &properties[i++]);
            if (len < 0) {
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                return false;
            }
            apdu_len += len;
        }
        // save list of object property references
        currentObject->Exception_Schedule->ux_TimeValues1 = i;
        memcpy(currentObject->Object_Property_References, properties, sizeof(properties));
        status = true;    // else we will report an error...
        break;

    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:
    case PROP_RELIABILITY:
#if (INTRINSIC_REPORTING_B == 1)
    case PROP_ACKED_TRANSITIONS:
    case PROP_EVENT_TIME_STAMPS:
#if ( BACNET_PROTOCOL_REVISION >= 14 )
    case PROP_EVENT_DETECTION_ENABLE:
#endif
#endif
    case PROP_PROPERTY_LIST:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        break;

    default:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    return status;
}


bool Schedule_In_Effective_Period(
    SCHEDULE_DESCR *currentObject,
    BACNET_DATE * date)
{
    // Period ::= Choice of {BACnetCalendarEntry | CalendarReference}
    /*
    BACnetCalendarEntry ::= CHOICE {
        date            [0] Date,
        date-range      [1] BACnetDateRange,
        weekNDay        [2] BACnetWeekNDay
        }
    */

    // todo2 - _currently_ we are have only implemented CalendarEntry, daterange....

    if (datetime_wildcard_compare_date(&currentObject->Effective_Period.startdate, date) <= 0 &&
        datetime_wildcard_compare_date(&currentObject->Effective_Period.enddate, date) >= 0) {
        return true ;
    }

    return false ;
}


void Schedule_Recalculate_PV(
    SCHEDULE_DESCR *currentObject,
    BACNET_WEEKDAY wday,
    BACNET_TIME *time)
{
    BACNET_DATE        date;
//  int i;
    // desc->Present_Value.tag = BACNET_APPLICATION_TAG_NULL ;

    /* Exception Schedule - execute if period is active*/

    if (Schedule_In_Effective_Period(currentObject, &date)) {
        BACNET_SPECIAL_EVENT *exceptionSchedule = currentObject->Exception_Schedule;
        //BACNET_EXCEPTION_SCHEDULE*  eol = nv->u.ExceptionSchedule + nv->u.SpecialEventCount;

        //// (1) exception-schedule
        //for (unsigned priority = 0xFF; ex < eol; ex++) {
        //  // search for highest priority active exception
        //  if (ex->type != EXCEPTION_CALENDAR_NONE && ex->priority < priority) {
        //      if (ex->type == EXCEPTION_CALENDAR_ENTRY) {
        //          // evaluate the calendar entry
        //          active = BCD_compare_calendar_event(&now.date, &ex->event);
        //      }
        //      else {
        //          // look at present-value of calendar instance
        //          active = ex->calendar < ALO_InstanceCount(OBJ_BCD) && *(u16_t*)ALO_GetAddress(OBJ_BCD, ex->calendar, 0);
        //      }

        //      if (active) {
        //          BACNET_TIME t = { 0 };

        //          // find most recent transition in exception schedule...
        //          for (int i = 0; i < ex->count; i++) {
        //              if (datetime_wildcard_compare_time(&now.time, &ex->list[i].time) >= 0 && datetime_wildcard_compare_time(&t, &ex->list[i].time) <= 0) {
        //                  if (ex->list[i].value.u != NULL_VALUE) {
        //                      assigned = i + 1;
        //                  }
        //                  else {
        //                      assigned = 0;
        //                  }
        //                  datetime_copy_time(&t, &ex->list[i].time);
        //              }
        //          }

        //          if (assigned) {
        //              ram->PresentValue.u = ex->list[assigned - 1].value.u;
        //              ram->PresentTag = nv->u.ScheduleTag;
        //              priority = ex->priority;
        //          }
        //      }
        //  }

    }


    // original
    //for (i = 0;
    //    i < desc->Weekly_Schedule[wday - 1].TV_Count &&
    //    desc->Present_Value == NULL; i++) {
    //    int diff = datetime_wildcard_compare_time(time,
    //        &desc->Weekly_Schedule[wday - 1].Time_Values[i].Time);
    //    if (diff >= 0 &&
    //        desc->Weekly_Schedule[wday - 1].Time_Values[i].Value.tag !=
    //        BACNET_APPLICATION_TAG_NULL) {
    //        desc->Present_Value =
    //            &desc->Weekly_Schedule[wday - 1].Time_Values[i].Value;
    //    }
    //}

    //if (desc->Present_Value == NULL)
    //    desc->Present_Value = &desc->Schedule_Default;

#if 0
    BTL approved
        // (2) weekly-schedule
        if (!assigned) {
            for (int i = 0; i < nv->u.DailyEventCount[gCLK_ram->DayOfWeek - 1]; i++) {
                TIME_VALUE* tv = nv->u.WeeklySchedule[gCLK_ram->DayOfWeek - 1] + i;
                BACNET_TIME t = { 0 };

                if (datetime_wildcard_compare_time(&now.time, &tv->time) >= 0 && datetime_wildcard_compare_time(&t, &tv->time) <= 0) {
                    if (tv->value.u != NULL_VALUE) {
                        assigned = i + 1;
                    }
                    else {
                        assigned = 0;
                    }
                    datetime_copy_time(&t, &tv->time);
                }
            }

            if (assigned) {
                ram->PresentValue.u = nv->u.WeeklySchedule[gCLK_ram->DayOfWeek - 1][assigned - 1].value.u;
                ram->PresentTag = nv->u.ScheduleTag;
            }
        }
    // (3) schedule-default
    if (!assigned) {
        ram->PresentValue = nv->u.DefaultValue;
        ram->PresentTag = nv->u.DefaultTag;
    }
}
 else {
     // (4) not in effective-period
     ram->PresentTag = BACNET_APPLICATION_TAG_NULL;
     ram->PresentValue.u = NULL_VALUE;
 }

 // trigger WriteProperty and Notifications as needed
 if (prev.u != ram->PresentValue.u) {
     // reset the "synchronize" flags for the list-of-object-property-references
     ram->WritePendingMask = (1 << nv->u.ObjectPropertyCount) - 1;
     ram->WriteTimer = 0;
 }
 else if (ram->WriteTimer) {
     ram->WriteTimer--;
 }

 if (ram->WritePendingMask && !ram->WriteTimer) {
     // synchronize device object property references right now...
     for (int i = 0; i < nv->u.ObjectPropertyCount; i++) {
         // write to this device-property-reference?
         if ((ram->WritePendingMask >> i) & 1) {
             BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE*  ref = nv->u.ObjectProperty + i;

             switch (app_data.tag = ram->PresentTag) {
             case BACNET_APPLICATION_TAG_NULL:
                 break;
             case BACNET_APPLICATION_TAG_REAL:
                 app_data.type.Real = ram->PresentValue.f;
                 break;
             case BACNET_APPLICATION_TAG_BOOLEAN:
                 app_data.type.Boolean = ram->PresentValue.u;
                 break;
             case BACNET_APPLICATION_TAG_ENUMERATED:
                 app_data.type.Enumerated = ram->PresentValue.u;
                 break;
             case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                 app_data.type.Unsigned_Int = ram->PresentValue.u;
                 break;
             case BACNET_APPLICATION_TAG_SIGNED_INT:
                 app_data.type.Signed_Int = ram->PresentValue.i;
                 break;
             }
             // encode the application-data-value as write-property-data
             wp_data_static.application_data_len = bacapp_encode_data(wp_data_static.application_data, &app_data);
             wp_data_static.object_instance = ref->objectIdentifier.instance;
             wp_data_static.object_type = ref->objectIdentifier.type;
             wp_data_static.object_property = ref->propertyIdentifier;
             wp_data_static.priority = nv->u.PriorityForWriting;
             wp_data_static.array_index = BACNET_ARRAY_ALL;
             // send write-property
             if (BSC_WriteProperty(ref)) {
                 // clear write pending bit for this object-property-reference
                 ram->WritePendingMask &= ~(1 << i);
             }
             else {
                 // failed to send write-property (setting retry timer)
                 ram->WriteTimer = 15;
            }
        }
    }
#endif /* (INTRINSIC_REPORTING_B == 1) */
}


int encode_daily_schedule(
    uint8_t * apdu,
    BACNET_DAILY_SCHEDULE *dailySched)
{
    int len = encode_opening_tag(&apdu[0], 0);
    int n ;
    for ( n = 0; n < dailySched->ux_TimeValues; n++) {
        len += bacapp_encode_time_value(&apdu[len], &dailySched->Time_Values[n]);
    }

    len += encode_closing_tag(&apdu[len], 0);
    return len;
}


static int rr_schedule(
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest
)
{
    int iLen = 0;
    int32_t iTemp;
    uint32_t uiTotal = 0;   /* Number of active recipients in the cache */
    uint32_t uiIndex = 0;   /* Current entry number */
    uint32_t uiFirst;       /* Entry number we started encoding from */
    uint32_t uiLast = 0;    /* Entry number we finished encoding on */
    uint32_t uiTarget;      /* Last entry we are required to encode */
    uint32_t uiRemaining;   /* Amount of unused space in packet */

    SCHEDULE_DESCR *currentObject = Schedule_Instance_To_Object(pRequest->object_instance);
    if (currentObject == NULL) {
        pRequest->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    /* Initialise result flags to all false */
    bitstring_init(&pRequest->ResultFlags);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_FIRST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_LAST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_MORE_ITEMS, false);

    /* Total number of list elements */
    switch (pRequest->object_property) {
    case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
        uiTotal = currentObject->ux_ObjectPropertyList;
        break;
    case PROP_EXCEPTION_SCHEDULE:
        uiTotal = currentObject->ux_special_events;
        break;
    case PROP_WEEKLY_SCHEDULE:
        uiTotal = MAX_BACNET_DAYS_OF_WEEK;
        break;
    default:
        panic();
        break;
    }

    /* Bounds check array index */
    if (pRequest->array_index == 0 || (pRequest->array_index > uiTotal && pRequest->array_index != BACNET_ARRAY_ALL)) {
        pRequest->error_class = ERROR_CLASS_PROPERTY;
        pRequest->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
        return 0;
    }
    /* Abort if list is empty */
    if (uiTotal == 0) return 0;
    /* See how much space we have */

    uiRemaining = (uint32_t)(MAX_APDU - pRequest->Overhead);

    pRequest->ItemCount = 0;              /* Start out with nothing */

    if (pRequest->RequestType == RR_READ_ALL) {
        /*
        * Read all the array or as much as will fit in the buffer by selecting
        * a range that covers the whole list and falling through to the next
        * section of code
        */
        pRequest->Count = uiTotal;      /* Full list */
        pRequest->Range.RefIndex = 1;   /* Starting at the beginning */
    }

    if (pRequest->Count < 0) {  /* negative count means work from index backwards */
        /*
        * Convert from end index/negative count to
        * start index/positive count and then process as
        * normal. This assumes that the order to return items
        * is always first to last, if this is not true we will
        * have to handle this differently.
        *
        * Note: We need to be careful about how we convert these
        * values due to the mix of signed and unsigned types - don't
        * try to optimise the code unless you understand all the
        * implications of the data type conversions!
        */

        iTemp = pRequest->Range.RefIndex;       /* pull out and convert to signed */
        iTemp += pRequest->Count + 1;   /* Adjust backwards, remember count is -ve */
        if (iTemp < 1) {        /* if count is too much, return from 1 to start index */
            pRequest->Count = pRequest->Range.RefIndex;
            pRequest->Range.RefIndex = 1;
        }
        else {        /* Otherwise adjust the start index and make count +ve */
            pRequest->Range.RefIndex = iTemp;
            pRequest->Count = -pRequest->Count;
        }
    }
    /* From here on in we only have a starting point and a positive count */

    uiTarget = pRequest->Range.RefIndex + pRequest->Count - 1;  /* Index of last required entry */
    if (uiTarget > uiTotal) uiTarget = uiTotal;                 /* Capped at end of list if necessary */
    uiFirst = uiIndex = pRequest->Range.RefIndex;               /* Record where we started from */
    if (pRequest->Range.RefIndex > uiTotal) return (0);         /* Past the end of list? */

    /* Seek to start position */
    while (uiIndex <= uiTarget) {
        switch (pRequest->object_property) {
        case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
            iTemp = bacapp_encode_device_obj_property_ref(&apdu[iLen], &currentObject->Object_Property_References[uiIndex - 1]);
            break;
        case PROP_EXCEPTION_SCHEDULE:
            iTemp = encode_special_event(&apdu[iLen], &currentObject->Exception_Schedule[uiIndex - 1]);
            break;
        case PROP_WEEKLY_SCHEDULE:
            iTemp = encode_daily_schedule(&apdu[iLen], &currentObject->Weekly_Schedule[uiIndex - 1]);
            break;
        default:
            panic();
            break;
        }

        // out of space or invalid list entry?
        if (iTemp == 0) break;

        uiRemaining -= iTemp;   /* Reduce the remaining space */
        iLen += iTemp;          /* and increase the length consumed */
        uiLast = uiIndex;       /* Record the last entry encoded */
        pRequest->ItemCount++;  /* increment the response count */
        uiIndex++;              /* and get ready for next one */
    }

    /* Set result flags as needed */
    if (pRequest->ItemCount) {
        if (uiIndex < uiTarget)
            bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_MORE_ITEMS, true);
        if (uiFirst == 1)
            bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_FIRST_ITEM, true);
        if (uiLast == uiTotal)
            bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_LAST_ITEM, true);
    }

    return (iLen);
}


bool Schedule_GetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo)
{
    bool status = false;

    // todo2 - review trend log. there is a check for illegal object. should we copy here, or is it handled higher up?

    switch (pRequest->object_property) {

    case PROP_WEEKLY_SCHEDULE:
    case PROP_EXCEPTION_SCHEDULE:
    case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
        pInfo->RequestTypes = RR_BY_POSITION;
        pInfo->Handler = rr_schedule;
        status = true;
        break;

    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_PRESENT_VALUE:
    case PROP_EFFECTIVE_PERIOD:
    case PROP_SCHEDULE_DEFAULT:
    case PROP_PRIORITY_FOR_WRITING:
    case PROP_STATUS_FLAGS:
    case PROP_RELIABILITY:
    case PROP_OUT_OF_SERVICE:
    case PROP_DESCRIPTION:
        pRequest->error_class = ERROR_CLASS_SERVICES;
        pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
        break;

    default:
        pRequest->error_class = ERROR_CLASS_PROPERTY;
        pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    return status;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"


void testSchedule(Test * pTest)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;

    Schedule_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_SCHEDULE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Schedule_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

}


#ifdef TEST_SCHEDULE

int main(void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Schedule", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testSchedule);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_SCHEDULE */
#endif /* TEST */
