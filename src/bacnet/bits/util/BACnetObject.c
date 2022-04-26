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
 *   As a special exception, if other files instantiate templates or
 *   use macros or inline functions from this file, or you compile
 *   this file and link it with other works to produce a work based
 *   on this file, this file does not by itself cause the resulting
 *   work to be covered by the GNU General Public License. However
 *   the source code for this file must still be made available in
 *   accordance with section (3) of the GNU General Public License.
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

#include "bacnet/timestamp.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/service/h_wp.h"

int encode_application_bitstring3(uint8_t *apdu, const bool one, const bool two, const bool three)
{
    BACNET_BIT_STRING bit_string;
    bitstring_init(&bit_string);
    bitstring_set_bit(&bit_string, 0, one);
    bitstring_set_bit(&bit_string, 1, two);
    bitstring_set_bit(&bit_string, 2, three);
    return encode_application_bitstring(apdu, &bit_string);
}


int encode_application_bitstring2(uint8_t *apdu, const bool one, const bool two)
{
    BACNET_BIT_STRING bit_string;
    bitstring_init(&bit_string);
    bitstring_set_bit(&bit_string, 0, one);
    bitstring_set_bit(&bit_string, 1, two);
    return encode_application_bitstring(apdu, &bit_string);
}


int encode_application_event_time_stamps(uint8_t *apdu, BACNET_READ_PROPERTY_DATA *rpdata, BACNET_DATE_TIME *Event_Time_Stamps)
{
    int apdu_len;

    if (rpdata->array_index == 0)
        apdu_len =
        encode_application_unsigned(&apdu[0],
            MAX_BACNET_EVENT_TRANSITION);
    /* if no index was specified, then try to encode the entire list */
    /* into one packet. */
    else if (rpdata->array_index == BACNET_ARRAY_ALL) {
        int i, len;
        apdu_len = 0;
        for (i = 0; i < MAX_BACNET_EVENT_TRANSITION; i++) {
            len =
                encode_opening_tag(&apdu[apdu_len],
                    TIME_STAMP_DATETIME);
            len +=
                encode_application_date(&apdu[apdu_len + len],
                    &Event_Time_Stamps[i].date);
            len +=
                encode_application_time(&apdu[apdu_len + len],
                    &Event_Time_Stamps[i].time);
            len +=
                encode_closing_tag(&apdu[apdu_len + len],
                    TIME_STAMP_DATETIME);

            /* add it if we have room */
            if ((apdu_len + len) < MAX_APDU) {
                apdu_len += len;
            }
            else {
                rpdata->error_code =
                    ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                apdu_len = BACNET_STATUS_ABORT;
                break;
            }
        }
    }
    else if (rpdata->array_index <= MAX_BACNET_EVENT_TRANSITION) {
        apdu_len =
            encode_opening_tag(&apdu[0], TIME_STAMP_DATETIME);
        apdu_len +=
            encode_application_date(&apdu[apdu_len],
                &Event_Time_Stamps[rpdata->array_index - 1].date);
        apdu_len +=
            encode_application_time(&apdu[apdu_len],
                &Event_Time_Stamps[rpdata->array_index - 1].time);
        apdu_len +=
            encode_closing_tag(&apdu[apdu_len], TIME_STAMP_DATETIME);
    }
    else {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
        apdu_len = BACNET_STATUS_ERROR;
    }
    return apdu_len;
}


int encode_status_flags(uint8_t *apdu, bool alarm, bool fault, bool oos)
{
    BACNET_BIT_STRING bit_string;
    bitstring_init(&bit_string);
    bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, alarm);
    bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, fault);
    bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, oos);
    return encode_application_bitstring(apdu, &bit_string);
}


BACNET_OBJECT *Generic_Instance_To_Object(
    const LLIST_HDR *objectHdr,
    const uint32_t objectInstance)
{
    BACNET_OBJECT *bacnetObject = (BACNET_OBJECT *) objectHdr->first;
    while (bacnetObject != NULL) {
        if (bacnetObject->objectInstance == objectInstance) return bacnetObject;
        bacnetObject = (BACNET_OBJECT *)bacnetObject->llist.next;
    }
    // don't complain if not found, this function is used for testing existence before creating new
    return bacnetObject;
}


BACNET_CHARACTER_STRING *BACnetObject_Description_Get(BACNET_OBJECT *bacnetCommon)
{
    return &bacnetCommon->description;
}


bool Generic_Instance_To_Object_Name(
    LLIST_HDR *objectHdr,
    uint32_t objectInstance,
    BACNET_CHARACTER_STRING *object_name)
{
    BACNET_OBJECT *bacnetObject = Generic_Instance_To_Object(objectHdr, objectInstance);
    if (bacnetObject == NULL) return false;
    
    return characterstring_copy(object_name, &bacnetObject->objectName);
}

static void DummyUpdate_real(float *value)
{
    // dummy update is a no-op
}

static void DummyUpdate_Reliability(BACNET_RELIABILITY* reliability)
{
    // dummy update is a no-op
}

static bool DummySetFloat(void* ptrToBACnetObject, const float value)
{

    panicDesc("You need to set up a genuine setter before accessing SetPresentValue_real()");
    return false;
}


static uint DummyGet_unsigned(const void* ptrToBACnetObject)
{

    panicDesc("You need to set up a genuine getter before accessing GetPresentValue_unsigned()");
    return 999 ;
}


static bool DummySet_unsigned(void* ptrToBACnetObject, const uint value)
{

    panicDesc("You need to set up a genuine setter before accessing SetPresentValue_unsigned()");
    return false;
}


static bool DummySetAdv(void* ptrToBACnetObject, const BACNET_APPLICATION_DATA_VALUE * value)
{

    panicDesc("You need to set up a genuine setter before accessing SetPresentValue_adv()");
    return false;
}


static bool DummyGetBoolean(const void* ptrToBACnetObject)
{

    panicDesc("You need to set up a genuine getter before accessing GetPresetnValue_binary()");
    return false ;
}


static bool DummySetBoolean(void* ptrToBACnetObject, const bool value)
{

    panicDesc("You need to set up a genuine setter before accessing SetPresetnValue_binray()");
    return false;
}


void Generic_Object_Init(
	BACNET_OBJECT				*currentObject,
	const	BACNET_OBJECT_TYPE	objectType,
    const   uint32_t			objectInstance,
    const   char				*objectName)
{
	currentObject->objectType = objectType;
	currentObject->objectInstance = objectInstance;
    characterstring_init_ansi(&currentObject->objectName, objectName);
    characterstring_init_ansi(&currentObject->description, "-x-");

	currentObject->reliability = RELIABILITY_NO_FAULT_DETECTED;
	currentObject->Out_Of_Service = false;

    currentObject->InternalUpdate_real = DummyUpdate_real;                 // used for AI PV, etc.
    currentObject->InternalUpdate_Reliability = DummyUpdate_Reliability;

    currentObject->SetPresentValue_adv = DummySetAdv;
    currentObject->SetPresentValue_real = DummySetFloat;
    currentObject->GetPresentValue_unsigned = DummyGet_unsigned;
    currentObject->SetPresentValue_unsigned = DummySet_unsigned;
    currentObject->GetPresentValue_boolean = DummyGetBoolean;
    currentObject->SetPresentValue_boolean = DummySetBoolean;

	currentObject->Event_State = EVENT_STATE_NORMAL;		// event state is not dependent on Intrinsic Service being selected or not

#if (INTRINSIC_REPORTING_B == 1)
	/* notification class not connected */
	currentObject->eventCommon.Notification_Class = BACNET_MAX_INSTANCE;

	currentObject->eventCommon.Notify_Type = NOTIFY_ALARM;
	currentObject->eventCommon.Ack_notify_data.EventState = EVENT_STATE_NORMAL;

	/* initialize Event time stamps using wildcards
	   and set Acked_transitions */
	for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
		datetime_wildcard_set(&currentObject->eventCommon.Event_Time_Stamps[j]);
		currentObject->eventCommon.Acked_Transitions[j].bIsAcked = true;
	}
#endif

}


uint32_t Generic_Index_To_Instance(
    LLIST_HDR *objectHdr,
    uint32_t objectIndex )
{
    unsigned count = 0;
    BACNET_OBJECT *bacnetObject = (BACNET_OBJECT *)objectHdr->first;
    while (bacnetObject != NULL) {
        if (count == objectIndex) return bacnetObject->objectInstance;
        count++;
        bacnetObject = (BACNET_OBJECT *)bacnetObject->llist.next;
    }
    panic();
    return count;
}


BACNET_OBJECT *Generic_Index_To_Object(
    LLIST_HDR *objectHdr,
    const uint32_t objectIndex)
{
    return ((BACNET_OBJECT *)ll_GetPtr(objectHdr, objectIndex));
}


bool WP_ValidateTagType(
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    BACNET_APPLICATION_TAG expectedTag
    )
{
    if (pValue->tag != expectedTag ) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        return false;
    }
    return true;
}


bool WP_ValidateEnumType(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue )
{
    if (pValue->tag != BACNET_APPLICATION_TAG_ENUMERATED) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        return false;
    }
    return true;
}


bool WP_ValidateRangeReal(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    double low, 
    double high )
{
    if (pValue->type.Real < low || pValue->type.Real > high) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    return true;
}

bool WP_ValidateRangeUnsignedInt(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint32_t low,
    uint32_t high)
{
    if (pValue->type.Unsigned_Int < low || pValue->type.Unsigned_Int > high) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    return true;
}

bool WP_ValidateEnumTypeAndRange(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint16_t    maxRange)
{
    if (!WP_ValidateEnumType(wp_data, pValue)) return false;

    if (pValue->type.Enumerated > maxRange) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_REJECT_PARAMETER_OUT_OF_RANGE;
        return false;
    }
    return true;
}


bool WP_StorePropertyEnumRanged(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint16_t    maxRange,
    int *tvalue)
{
    if (pValue->tag != BACNET_APPLICATION_TAG_ENUMERATED) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        return false;
    }

    if (pValue->type.Enumerated > maxRange) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_REJECT_PARAMETER_OUT_OF_RANGE;
        return false;
    }
    *tvalue = pValue->type.Enumerated;
    return true;
}


bool WP_StorePropertyFloat(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE *pValue,
    float *dest )
{
    if (pValue->tag != BACNET_APPLICATION_TAG_REAL) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        return false;
    }
    *dest = pValue->type.Real;
    return true;
}


bool Common_Reliability_Set(
	BACNET_OBJECT *currentObject,
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* value)
{
	if (!WP_ValidateEnumTypeAndRange(wp_data, value, RELIABILITY_PROPRIETARY_MAX)) return false;

	bool status = WPValidateArgType(
		value, 
		BACNET_APPLICATION_TAG_ENUMERATED,
		&wp_data->error_class, 
		&wp_data->error_code);

	if (!status) return status;

	if (currentObject->Out_Of_Service) {
		currentObject->shadowReliability = (BACNET_RELIABILITY)value->type.Enumerated;
		return true;
	}

	wp_data->error_class = ERROR_CLASS_PROPERTY;
	wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
	return false;
}


BACNET_RELIABILITY Common_Reliability_Get(
    BACNET_OBJECT* currentObject)
{
    if (currentObject->Out_Of_Service) {
        return currentObject->shadowReliability;
    }

    currentObject->InternalUpdate_Reliability(&currentObject->reliability);

    return currentObject->reliability;
}