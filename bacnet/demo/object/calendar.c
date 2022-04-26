/**************************************************************************

    Copyright (C) 2011 BACnet Interoperability Testing Services, Inc.

    This program is free software : you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.If not, see <http://www.gnu.org/licenses/>.

        For more information : info@bac-test.com
        For access to source code : info@bac-test.com
        or www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

//#include <stdint.h>
#include <memory.h>
#include "calendar.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"
#include "bacdcode.h"
#include "proplist.h"

LLIST_HDR Calendar_Descriptor_List;

int encode_calendar_entry(uint8_t *apdu, BACNET_CALENDAR_ENTRY *ev)
{
    int apdu_len = 0;

    // encode choice of [0] = date, [1] = date-range, or [2] = weeknday
    switch (ev->tag) {
    case CALENDAR_ENTRY_DATE:
        apdu_len += encode_context_date(&apdu[apdu_len], 0, &ev->CalEntryChoice.date);
        break;
    case CALENDAR_ENTRY_RANGE:
        apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
        apdu_len += encode_application_date(&apdu[apdu_len], &ev->CalEntryChoice.range.startdate);
        apdu_len += encode_application_date(&apdu[apdu_len], &ev->CalEntryChoice.range.enddate);
        apdu_len += encode_closing_tag(&apdu[apdu_len], 1);
        break;
    case CALENDAR_ENTRY_WEEKNDAY:
        apdu_len += encode_tag(&apdu[apdu_len], 2, true, 3);
        apdu[apdu_len++] = ev->CalEntryChoice.weekNday.month;
        apdu[apdu_len++] = ev->CalEntryChoice.weekNday.weekofmonth;
        apdu[apdu_len++] = ev->CalEntryChoice.weekNday.dayofweek;
        break;
    default:
        panic();
        break;
    }
    return apdu_len;
}


static bool compare_calendar_entry(BACNET_DATE *d, BACNET_CALENDAR_ENTRY *ev)
{
    bool  active = false;

    switch (ev->tag) {
    case CALENDAR_ENTRY_DATE:
        if (datetime_wildcard_compare_date(d, &ev->CalEntryChoice.date) == 0) {
          //day of week matters
          if (ev->CalEntryChoice.date.wday == BACNET_WEEKDAY_ANY || ev->CalEntryChoice.date.wday == d->wday) active = true;
        }
        break;
    case CALENDAR_ENTRY_RANGE:
        if (datetime_wildcard_compare_date(d, &ev->CalEntryChoice.range.startdate) >= 0 && datetime_wildcard_compare_date(d, &ev->CalEntryChoice.range.enddate) <= 0) {
          active = true;
        }
        break;
    case CALENDAR_ENTRY_WEEKNDAY:
        if (date_compare_weeknday(d, &ev->CalEntryChoice.weekNday)) active = true;
        break;
    }
    return active;
}


/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Calendar_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_DATE_LIST,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Calendar_Properties_Optional[] = {
    PROP_DESCRIPTION,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Calendar_Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


bool Calendar_Create(
    const uint32_t instance,
    const char *name)
{
    CALENDAR_DESCR *currentObject = (CALENDAR_DESCR *)emm_scalloc('a', sizeof(CALENDAR_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&Calendar_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    for (int i = 0; i < MAX_CALENDAR_EVENTS; i++) {
        currentObject->calendar[i].tag = CALENDAR_ENTRY_NONE;
    }

    return true;
}


void Calendar_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    if (pRequired)
        *pRequired = Calendar_Properties_Required;
    if (pOptional)
        *pOptional = Calendar_Properties_Optional;
    if (pProprietary)
        *pProprietary = Calendar_Properties_Proprietary;
}


// Gets called once for each device

void Calendar_Init(
    void)
{
    ll_Init(&Calendar_Descriptor_List, 100 );
}


bool Calendar_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&Calendar_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Calendar_Count(
    void)
{
    return Calendar_Descriptor_List.count ;
}


uint32_t Calendar_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&Calendar_Descriptor_List, index);
}



CALENDAR_DESCR *Calendar_Instance_To_Object(
    uint32_t object_instance)
{
    return (CALENDAR_DESCR *) Generic_Instance_To_Object(&Calendar_Descriptor_List, object_instance);
}


bool Calendar_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name (&Calendar_Descriptor_List, object_instance, object_name );
}


/* return apdu len, or BACNET_STATUS_ERROR on error */
int Calendar_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;
    int index;
    //BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    CALENDAR_DESCR *currentObject;
    uint8_t *apdu ;

    const BACNET_PROPERTY_ID *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;

    currentObject = Calendar_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0], OBJECT_CALENDAR,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
        Calendar_Object_Name(rpdata->object_instance, &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0], OBJECT_CALENDAR);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len = encode_application_boolean(&apdu[0], currentObject->Present_Value );
        break;

    case PROP_DATE_LIST:
        for (index = 0; index < MAX_CALENDAR_EVENTS; index++) {
            apdu_len += encode_calendar_entry(&apdu[apdu_len], &currentObject->calendar[index] );
        }
        break;

    case PROP_PROPERTY_LIST:
        Calendar_Property_Lists(&pRequired, &pOptional, &pProprietary);
        apdu_len = property_list_encode(
            rpdata,
            pRequired,
            pOptional,
            pProprietary);

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) &&
        (rpdata->object_property != PROP_PRIORITY_ARRAY) && 
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}


static BACNET_CALENDAR_ENTRY tempCalendar[MAX_CALENDAR_EVENTS] ;
  
bool WriteDateList(BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    uint8_t*  apdu = wp_data->application_data;
    bool      error = false;
    int       event = 0;
    int       len = 0;

    CALENDAR_DESCR *currentObject = Calendar_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    memset(tempCalendar, 0, sizeof(currentObject->calendar));

    while (!error && apdu < (wp_data->application_data + wp_data->application_data_len)) {
      error = true;
      // decode one of date/date_range/week_and_day
      if (decode_is_context_tag_with_length(apdu, 0, &len) && len == 1 && (*apdu & 0x07) == 4) {
        // CALENDAR_ENTRY_DATE
          tempCalendar[event].CalEntryChoice.date.year = apdu[1] + 1900 ;
          tempCalendar[event].CalEntryChoice.date.month = apdu[2];
          tempCalendar[event].CalEntryChoice.date.day = apdu[3];
          tempCalendar[event].CalEntryChoice.date.wday = (BACNET_WEEKDAY) apdu[4];
          tempCalendar[event].tag = CALENDAR_ENTRY_DATE;
        error = false;
        len += 4;
      } else if (decode_is_opening_tag(apdu) && ((*apdu >> 4) & 0x0f) == 1) {
        // CALENDAR_ENTRY_RANGE
        len = 1;
        len += decode_application_date(&apdu[len], &tempCalendar[event].CalEntryChoice.range.startdate);
        len += decode_application_date(&apdu[len], &tempCalendar[event].CalEntryChoice.range.enddate);

        if (decode_is_closing_tag(&apdu[len]) && daterange_is_valid(&currentObject->calendar[event].CalEntryChoice.range)) {
            currentObject->calendar[event].tag = CALENDAR_ENTRY_RANGE;
          len += 1;
          error = false;
        }
      } else if (decode_is_context_tag_with_length(apdu, 2, &len) && len == 1 && (*apdu & 0x07) == 3) {
        // CALENDAR_ENTRY_WEEKNDAY
          tempCalendar[event].CalEntryChoice.weekNday.month = apdu[1];
          tempCalendar[event].CalEntryChoice.weekNday.weekofmonth = apdu[2];
          tempCalendar[event].CalEntryChoice.weekNday.dayofweek = apdu[3];
          tempCalendar[event].tag = CALENDAR_ENTRY_WEEKNDAY;
        error = false;
        len += 3;
      }
      else {
          panic();
      }
      apdu += len; 
      event++;
    }
    
    if (apdu == (wp_data->application_data + wp_data->application_data_len)) {
      memcpy( currentObject->calendar, tempCalendar, sizeof(BACNET_CALENDAR_ENTRY) * MAX_CALENDAR_EVENTS);
      return true;
    }
    // todo - what error code to return upon failing to decode date list?
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
    return false;
}


/* returns true if successful */
bool Calendar_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode some of the request */
    if (!bacapp_decode_application_data_safe(wp_data->application_data, wp_data->application_data_len, &value)) {
        value.tag = MAX_BACNET_APPLICATION_TAG;
    }

    /* some properties are read-only */
    switch (wp_data->object_property) {
    case PROP_PROPERTY_LIST:
    case PROP_PRESENT_VALUE:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_DESCRIPTION:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    default:
    	break;
    }

    /*  only array properties can have array options */
    if ((wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }


    switch (wp_data->object_property) {
    case PROP_DATE_LIST:
        if (wp_data->priority == 6) {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        }
        else {
            status = true;
        }
        break;

    default:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    return status;
}


static BACNET_CALENDAR_ENTRY *event_date_index(CALENDAR_DESCR *currentObject, unsigned index)
{
//	unsigned n ;
	panic();
//    for (n = 0, found = 0; n < MAX_CALENDAR_EVENTS; n++) {
//        if ( currentObject->calendar[n].tag && ++found == index) return &currentObject->calendar[n] ;
//    }
    return NULL;
}

static int rr_date_list(
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

    CALENDAR_DESCR *currentObject = Calendar_Instance_To_Object(pRequest->object_instance);
    if (currentObject == NULL) {
        pRequest->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    /* Initialise result flags to all false */
    bitstring_init(&pRequest->ResultFlags);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_FIRST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_LAST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_MORE_ITEMS, false);
    /* See how much space we have */

    uiRemaining = (uint32_t)(MAX_APDU - pRequest->Overhead);

    pRequest->ItemCount = 0;              /* Start out with nothing */

    /* count active recipients */

    for (iTemp = 0; iTemp < MAX_CALENDAR_EVENTS; iTemp++) {
        if (currentObject->calendar[iTemp].tag != CALENDAR_ENTRY_NONE) uiTotal++;
    }
    /* Bail out now if nowt */
    if (uiTotal == 0) return (0);

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
        iTemp = encode_calendar_entry(&apdu[iLen], event_date_index( currentObject, uiIndex ));

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


bool CalendarGetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo)
{
    bool status = false;

    // todo2 - review trend log. there is a check for illegal object. should we copy here, or is it handled higher up?

    switch (pRequest->object_property) {

    case PROP_DATE_LIST:
        pInfo->RequestTypes = RR_BY_POSITION;
        pInfo->Handler = rr_date_list;
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

