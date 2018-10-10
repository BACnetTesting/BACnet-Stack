/**************************************************************************
*
*  Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   Dec 3, 2011     BITS    Modifications to this file have been made in compliance
*                           to original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*       For more information: info@bac-test.com
*       For access to source code:  info@bac-test.com
*               or      www.github.com/bacnettesting/bacnet-stack
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*
*********************************************************************/

#ifndef BACNETOBJECTSCHEDULE_H_
#define BACNETOBJECTSCHEDULE_H_

#include "bacenum.h"
#include "schedule.h"
#include "bacdef.h"
#include "bacdevobjpropref.h"

#define MX_DAYS				7
// todo ekh - put in error logging 'instrumentation' for when these get exceeded...
#define MX_SPECIAL_EVENTS	4	

class BO_Schedule : public BACnetObjectWithPV
{

public:
	uint8_t obj_prop_ref_cnt;       /* actual number of obj_prop references */
	uint8_t Priority_For_Writing;   /* (1..16) */
	BACNET_DATE Start_Date;
	BACNET_DATE End_Date;
	BACNET_APPLICATION_DATA_VALUE Schedule_Default;
	BACNET_APPLICATION_DATA_VALUE *Present_Value;   /* must be set to a valid value
												     * default is Schedule_Default */
	BACNET_DAILY_SCHEDULE Weekly_Schedule[MX_DAYS];
	BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE Object_Property_References[BACNET_SCHEDULE_OBJ_PROP_REF_SIZE];
	BACNET_SPECIAL_EVENT Exception_Schedule[MX_SPECIAL_EVENTS];
	uint8_t	ux_ExceptionSchedule;


    BO_Schedule( int instance )
    {
        bacnetType = OBJECT_SCHEDULE;
		this->bacnetInstance = instance;

		Start_Date.year = 1900 + 0xFF;
		Start_Date.month = 1;
		Start_Date.day = 1;
		Start_Date.wday = BACNET_WEEKDAY_ANY;
		End_Date.year = 1900 + 0xFF;
		End_Date.month = 12;
		End_Date.day = 31;
		End_Date.wday = BACNET_WEEKDAY_ANY;
		for (int j = 0; j < MX_DAYS; j++) {
			Weekly_Schedule[j].TV_Count = 0;
		}
		Present_Value = &Schedule_Default;

#if 0
		Schedule_Default.context_specific = false;
		Schedule_Default.tag = BACNET_APPLICATION_TAG_REAL;
		Schedule_Default.type.Real = 21.0;			/* 21 C, room temperature */
#else
		Schedule_Default.context_specific = false;
		Schedule_Default.tag = BACNET_APPLICATION_TAG_BOOLEAN;
		Schedule_Default.type.Boolean = false ;	
#endif

		obj_prop_ref_cnt = 0;						/* no references, add as needed */
		Priority_For_Writing = 16;					/* lowest priority */
		ux_ExceptionSchedule = 0;
	}


	int encode_context_calendar_entry(uint8_t * apdu, uint8_t tag, BACnetCalendarEntry *calendarEntry)
	{
		int len = 1;
		apdu[0] = tag;
		len += encode_context_date( &apdu[len], 0, &calendarEntry->date);
		return len;
	}


	int bacapp_encode_special_event(
		uint8_t * apdu,
		BACNET_SPECIAL_EVENT * value)
	{
		// todo 2 - put in bounds checks..
		int len = encode_context_calendar_entry(apdu, 0, &value->calendarEntry);
		if (len < 0) return len;

		len += encode_opening_tag(&apdu[len], 2);
		for (int i = 0; i < 2; i++)
		{
			len +=  bacapp_encode_time_value (&apdu[len], &value->listOfTimeValues[i]);
		}

		len += encode_closing_tag(&apdu[len], 2);

		len += encode_context_unsigned(&apdu[len], 3, value->priority);
		return len;

	}



    RP_RETURN Read_Property(BACNET_READ_PROPERTY_DATA * rpdata, int * const apdu_len)
    {
		int i;
        const int *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;

        // todonext4 - I need to get on top of this cascading fiasco
        if ((rpdata == NULL) || (rpdata->application_data == NULL) || (rpdata->application_data_len == 0))
        {
            *apdu_len = 0; // really. 0. todonext2 - why 0 Steve? why not error code??
            return RP_FAILED;
        }

		rpdata->error_class = ERROR_CLASS_OBJECT;
		rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;

		RP_RETURN rc = BACnetObjectWithPV::Read_Property(rpdata, apdu_len);
		if (rc != RP_SEARCH_MORE)
		{
			return rc;
		}

        uint8_t *apdu = rpdata->application_data;

        switch (rpdata->object_property)
        {
        case PROP_PRESENT_VALUE:
        case PROP_SCHEDULE_DEFAULT:
            // printf("todo 1 - supply present value\n");
            *apdu_len = encode_application_real(&apdu[0], 0.0);
            return RP_FOUND_OK;

        case PROP_EFFECTIVE_PERIOD:
            *apdu_len = encode_application_date(&apdu[0], &Start_Date);
            *apdu_len += encode_application_date(&apdu[*apdu_len], &End_Date);
            return RP_FOUND_OK;

        case PROP_PRIORITY_FOR_WRITING:
            *apdu_len = encode_application_unsigned(&apdu[0], Priority_For_Writing);
            return RP_FOUND_OK;

		case PROP_WEEKLY_SCHEDULE:
			if (rpdata->array_index == 0)       /* count, always 7 */
				*apdu_len = encode_application_unsigned(&apdu[0], 7);
			else if (rpdata->array_index == BACNET_ARRAY_ALL) { /* full array */
				int day;
				for (day = 0; day < MX_DAYS; day++) {
					*apdu_len += encode_opening_tag(&apdu[*apdu_len], 0);
					for (i = 0; i < Weekly_Schedule[day].TV_Count;
					i++) {
						*apdu_len +=
							bacapp_encode_time_value(&apdu[*apdu_len],
								&Weekly_Schedule[day].Time_Values[i]);
					}
					*apdu_len += encode_closing_tag(&apdu[*apdu_len], 0);
				}
			}
			else if (rpdata->array_index <= MX_DAYS) {      /* some array element */
				int day = rpdata->array_index - 1;
				*apdu_len += encode_opening_tag(&apdu[*apdu_len], 0);
				for (i = 0; i < Weekly_Schedule[day].TV_Count; i++) {
					*apdu_len +=
						bacapp_encode_time_value(&apdu[*apdu_len],
							&Weekly_Schedule[day].Time_Values[i]);
				}
				*apdu_len += encode_closing_tag(&apdu[*apdu_len], 0);
			}
			else {    /* out of bounds */
				rpdata->error_class = ERROR_CLASS_PROPERTY;
				rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
				*apdu_len = BACNET_STATUS_ERROR;
			}
			return RP_FOUND_OK;

        case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
			for (i = 0; i < obj_prop_ref_cnt; i++) {
				*apdu_len +=
					bacapp_encode_device_obj_property_ref(&apdu[*apdu_len],
						&Object_Property_References[i]);
			}
            return RP_FOUND_OK;

			// todo 2
		//case PROP_EXCEPTION_SCHEDULE:
		//	for (int i = 0; i < ux_ExceptionSchedule; i++)
		//	{
		//		*apdu_len += bacapp_encode_special_event(&apdu[*apdu_len], &Exception_Schedule[i]);
		//	}
		//	return RP_FOUND_OK;

        //case PROP_PROPERTY_LIST:
        //    Schedule_Property_Lists(&pRequired, &pOptional, &pProprietary);
        //    *apdu_len = property_list_encode(rpdata, pRequired, pOptional, pProprietary);
        //    return RP_FOUND_OK;

		default:
			return BACnetObjectWithPV::Read_Property(rpdata, apdu_len);
        }
    }

	virtual RP_RETURN Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data, BACNET_APPLICATION_DATA_VALUE *value, bool * const boolReturn)
	{
		int len ;
		int ilen = 0 ;

   		switch ((int)wp_data->object_property)
		{
		case PROP_EFFECTIVE_PERIOD:
			/* decode first element of the request */
			len = bacapp_decode_application_data(wp_data->application_data, wp_data->application_data_len, value);
			if (len < 0)
			{
				*boolReturn = false;
				return RP_FAILED;
			}

			*boolReturn = WPValidateArgType(value, BACNET_APPLICATION_TAG_DATE, &wp_data->error_class, &wp_data->error_code);
			if (!*boolReturn) return RP_FAILED;
			Start_Date = value->type.Date;

			len = bacapp_decode_application_data(&wp_data->application_data[5], wp_data->application_data_len, value);
			if (len < 0)
			{
				*boolReturn = false;
				return RP_FAILED;
			}

			*boolReturn = WPValidateArgType(value, BACNET_APPLICATION_TAG_DATE, &wp_data->error_class, &wp_data->error_code);
			if (!*boolReturn) return RP_FAILED;

			End_Date = value->type.Date;
			*boolReturn = true;
			return RP_FOUND_OK;

		case PROP_WEEKLY_SCHEDULE:
			// Here we expect 7 possible daily schedules
			for (int day = 0; day < 7; day++)
			{
				Weekly_Schedule[day].TV_Count = 0;

				if ( ! IS_OPENING_TAG(wp_data->application_data[ilen++]))
				{
					// todo 3, should send abort?
					*boolReturn = false;
					return RP_FAILED;
				}
				while ( !IS_CLOSING_TAG(wp_data->application_data[ilen]))
				{
					// we can extract a list of timevalue pairs
					len = bacapp_decode_application_data( &wp_data->application_data[ilen], wp_data->application_data_len, value);
					if (len < 0)
					{
						*boolReturn = false;
						return RP_FAILED;
					}
					ilen += len;

					*boolReturn = WPValidateArgType(value, BACNET_APPLICATION_TAG_TIME, &wp_data->error_class, &wp_data->error_code);
					if (!*boolReturn) return RP_FAILED;

					Weekly_Schedule[day].Time_Values[Weekly_Schedule[day].TV_Count].Time = value->type.Time;

					len = bacapp_decode_application_data( &wp_data->application_data[ilen], wp_data->application_data_len, value);
					if (len < 0)
					{
						*boolReturn = false;
						return RP_FAILED;
					}
					ilen += len;

					// value could be any of a number of primitive types, dont check (for now) todo 4
					Weekly_Schedule[day].Time_Values[Weekly_Schedule[day].TV_Count++].Value = *value;
				}
				// assert closing constructed tag for the daily schedule
				if ( ! IS_CLOSING_TAG(wp_data->application_data[ilen++]))
				{
					// todo 3, should send abort?
					*boolReturn = false;
					return RP_FAILED;
				}
			}
			*boolReturn = true;
			return RP_FOUND_OK;

		case PROP_EXCEPTION_SCHEDULE:
			*boolReturn = true;
			return RP_FOUND_OK;

		case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
			obj_prop_ref_cnt = 0;
			while (ilen < wp_data->application_data_len)
			{
				BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE opValue;
				len = bacapp_decode_device_obj_property_ref(&wp_data->application_data[ilen], &opValue);
				if (len < 0)
				{
					*boolReturn = false;
					return RP_FAILED;
				}
				ilen += len;

				if (obj_prop_ref_cnt < BACNET_SCHEDULE_OBJ_PROP_REF_SIZE)
				{
					Object_Property_References[obj_prop_ref_cnt++] = opValue;
				}
				// todo BTC - test optional index case
			}

			*boolReturn = true;
			// wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
			return RP_FOUND_OK;

		default:
			return BACnetObjectWithPV::Write_Property(wp_data, value, boolReturn);
		}
	}

	bool Schedule_In_Effective_Period( BACNET_DATE * date)
	{
		if (datetime_wildcard_compare_date(&Start_Date, date) <= 0 &&
			datetime_wildcard_compare_date(&End_Date, date) >= 0)
		{
			return true;
		}
		return false;
	}


	void Schedule_Recalculate_PV(
		BACNET_WEEKDAY wday,
		BACNET_TIME * time)
	{
		int i;
		Present_Value = NULL;

		/* for future development, here should be the loop for Exception Schedule */

		for (i = 0;
		i < Weekly_Schedule[wday - 1].TV_Count &&
			Present_Value == NULL; i++) {
			int diff = datetime_wildcard_compare_time(time,
				&Weekly_Schedule[wday - 1].Time_Values[i].Time);
			if (diff >= 0 &&
				Weekly_Schedule[wday - 1].Time_Values[i].Value.tag !=
				BACNET_APPLICATION_TAG_NULL) {
				Present_Value =
					&Weekly_Schedule[wday - 1].Time_Values[i].Value;
			}
		}

		if (Present_Value == NULL)
		{
			Present_Value = &Schedule_Default;
		}


		// todo 1 - and transfer this value (or null) to the list of object references (if they are of the right type that is...)
	}

};


#endif

