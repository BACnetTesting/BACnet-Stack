/**************************************************************************
 *
 * Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 *  2018.07.04 EKH Diffed in hints from Binary Value for future reference
 *
 ****************************************************************************************/

#include "configProj.h"     /* the project/application specific settings */

#if (BACNET_USE_OBJECT_MULTISTATE_INPUT == 1)

#include "bacnet/datalink/bip.h"
// #include "handlers.h"
#include "ms-input.h"
// #include "bitsDebug.h"
#include "eLib/util/llist.h"
#include "eLib/util/emm.h"
#include "device.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/basic/service/h_wp.h"
#include "bacnet/basic/object/ms-input.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "eventCommon.h"
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_STATUS_FLAGS,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_EVENT_STATE,
    PROP_NUMBER_OF_STATES,
    MAX_BACNET_PROPERTY_ID
};


static const BACNET_PROPERTY_ID Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_RELIABILITY,

#if (INTRINSIC_REPORTING_B == 1)
    PROP_TIME_DELAY,
    PROP_NOTIFICATION_CLASS,
    PROP_EVENT_ENABLE,
    PROP_ACKED_TRANSITIONS,
    PROP_NOTIFY_TYPE,
    PROP_EVENT_TIME_STAMPS,

#if (BACNET_PROTOCOL_REVISION >= 13)
    PROP_EVENT_DETECTION_ENABLE,
#endif

#endif // INTRINSIC_REPORTING_B

    PROP_STATE_TEXT,
    MAX_BACNET_PROPERTY_ID
};


static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Multistate_Input_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// Gets called once for each device
// This only gets called once on startup, and has to initialize for ALL virtual devices.
void Multistate_Input_Init(
    void)
{

#if (INTRINSIC_REPORTING_B == 1)
    /* Set handler for GetEventInformation function */
    handler_get_event_information_set(
        OBJECT_MULTISTATE_INPUT,
        Multistate_Input_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
    handler_alarm_ack_set(
        OBJECT_MULTISTATE_INPUT,
        Multistate_Input_Alarm_Ack);

#if ( BACNET_PROTOCOL_REVISION < 13 )
    /* Set handler for GetAlarmSummary Service */
    // Deprecated since Rev 13
    /* Set handler for GetAlarmSummary Service */
    handler_get_alarm_summary_set(
        OBJECT_MULTISTATE_INPUT,
        Multistate_Input_Alarm_Summary);
#endif // ( BACNET_PROTOCOL_REVISION < 13 )

#endif
}


BACNET_OBJECT* Multistate_Input_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name)
{

    if (Multistate_Input_Valid_Instance(pDev, instance))
    {
        dbMessage(DBD_Config, DB_ERROR, "Attempting to create a duplicate Multistate Input MSI:%05u", instance);
        return NULL;
    }

    BACNET_CHARACTER_STRING tname;
    characterstring_init_ansi(&tname, name);
    BACNET_OBJECT_TYPE foundType;
    uint32_t foundInstance;
    if (Device_Valid_Object_Name(pDev, &tname, &foundType, &foundInstance))
    {
        dbMessage(DBD_Config, DB_ERROR, "Attempting to create a duplicate name of Multistate Input MSI:%s", name);
        return NULL;
    }

    MULTISTATE_INPUT_DESCR* currentObject = (MULTISTATE_INPUT_DESCR*)emm_scalloc('a', sizeof(MULTISTATE_INPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return NULL;
    }

    if (!ll_Enqueue(&pDev->MSI_Descriptor_List, currentObject)) {
        panic();
        return NULL;
    }

    BACnetMultistateObject_Init(&currentObject->multistateCommon, OBJECT_MULTISTATE_INPUT, instance, name);

    return (BACNET_OBJECT*)currentObject;
}


bool Multistate_Input_Valid_Instance(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&pDev->MSI_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Multistate_Input_Count(
    DEVICE_OBJECT_DATA* pDev)
{
    return pDev->MSI_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Multistate_Input_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index)
{
    // todo 4 - can/do we catch out-of-bounds?
    return Generic_Index_To_Instance(&pDev->MSI_Descriptor_List, index);
}


static inline bool isOutOfService(
    MULTISTATE_INPUT_DESCR* currentObject)
{
    return currentObject->multistateCommon.common.Out_Of_Service;
}


static inline bool isInAlarm(
    MULTISTATE_INPUT_DESCR* currentObject)
{
    return currentObject->multistateCommon.common.Event_State != EVENT_STATE_NORMAL;
}


static BACNET_RELIABILITY Multistate_Input_Reliability_Get(
    MULTISTATE_INPUT_DESCR* currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->multistateCommon.common.shadowReliability;
    }

    return Common_Reliability_Get(&currentObject->multistateCommon.common);
}


static bool isInFault(
    MULTISTATE_INPUT_DESCR* currentObject)
{
    return (Multistate_Input_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


MULTISTATE_INPUT_DESCR* Multistate_Input_Instance_To_Object(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    return (MULTISTATE_INPUT_DESCR*)Generic_Instance_To_Object(&pDev->MSI_Descriptor_List, object_instance);
}


static uint16_t Multistate_Input_Present_Value_Get(
    MULTISTATE_INPUT_DESCR* currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->multistateCommon.shadow_Present_Value;
    }
    return currentObject->multistateCommon.Present_Value;
}


#if ( BACNET_SVC_COV_B == 1 )
static void Multistate_Input_COV_Detect_PV_Change(
    MULTISTATE_INPUT_DESCR* currentObject,
    uint16_t newValue)
{
    if (newValue != currentObject->multistateCommon.Prior_Value) {
        dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, "COV: Detected change [%s] [%d] -> [%d]",
            BACnet_ObjectID_ToString(currentObject->multistateCommon.common.objectType, currentObject->multistateCommon.common.objectInstance),
            currentObject->multistateCommon.Prior_Value,
            newValue
        );

        currentObject->multistateCommon.Changed = true;
        currentObject->multistateCommon.Prior_Value = newValue;
    }
}
#endif


static bool Multistate_Input_Present_Value_Set(
	MULTISTATE_INPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA* wp_data,
    BACNET_APPLICATION_DATA_VALUE* value)
{
    bool status = WPValidateArgType(
        value,
		BACNET_APPLICATION_TAG_UNSIGNED_INT,
        &wp_data->error_class, &wp_data->error_code);
    if (!status) return false;

	if (!currentObject->multistateCommon.common.Out_Of_Service) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    }

#if ( BACNET_SVC_COV_B == 1 )
    Multistate_Input_COV_Detect_PV_Change(currentObject, (uint16_t)value->type.Unsigned_Int);
#endif

    if (currentObject->multistateCommon.common.Out_Of_Service) {
        currentObject->multistateCommon.shadow_Present_Value = (uint16_t)value->type.Unsigned_Int;
    }
    else {
        currentObject->multistateCommon.Present_Value = (uint16_t)value->type.Unsigned_Int;
    }
    return true;
}


static uint32_t Multistate_Input_Max_States(
    uint32_t instance)
{
    return MULTISTATE_NUMBER_OF_STATES;
}


// Signature used by device mux
/* note: the object name must be unique within this device */
bool Multistate_Input_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name)
{
    return Generic_Instance_To_Object_Name(
        &pDev->MSI_Descriptor_List,
        object_instance,
        object_name);
}


// todo 3 - resolve this
#if 0
/* note: the object name must be unique within this device */
bool Multistate_Input_Name_Set(
    uint32_t object_instance,
    char* new_name)
{
    unsigned index = 0; /* offset from instance lookup */
    size_t i = 0;       /* loop counter */
    bool status = false;        /* return value */

    index = Multistate_Input_Instance_To_Index(object_instance);
    if (index < MAX_MULTISTATE_INPUTS) {
        status = true;
        /* FIXME: check to see if there is a matching name */
        if (new_name) {
            for (i = 0; i < sizeof(Object_Name[index]); i++) {
                Object_Name[index][i] = new_name[i];
                if (new_name[i] == 0) {
                    break;
                }
            }
        }
        else {
            for (i = 0; i < sizeof(Object_Name[index]); i++) {
                Object_Name[index][i] = 0;
            }
        }
    }

    return status;
}


static bool Multistate_Input_Object_Name_Write(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* char_string,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code)
{
    unsigned index = 0; /* offset from instance lookup */
    size_t length = 0;
    uint8_t encoding = 0;
    bool status = false;        /* return value */

    index = Multistate_Input_Instance_To_Index(object_instance);
    if (index < MAX_MULTISTATE_INPUTS) {
        length = characterstring_length(char_string);
        if (length <= sizeof(Object_Name[index])) {
            encoding = characterstring_encoding(char_string);
            if (encoding == CHARACTER_UTF8) {
                status =
                    characterstring_ansi_copy(Object_Name[index],
                        sizeof(Object_Name[index]), char_string);
                if (!status) {
                    *error_class = ERROR_CLASS_PROPERTY;
                    *error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            else {
                *error_class = ERROR_CLASS_PROPERTY;
                *error_code = ERROR_CODE_CHARACTER_SET_NOT_SUPPORTED;
            }
        }
        else {
            *error_class = ERROR_CLASS_PROPERTY;
            *error_code = ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
        }
    }

    return status;
}
#endif


static inline bool Multistate_Input_Reliability_Set(
    MULTISTATE_INPUT_DESCR* currentObject,
    BACNET_WRITE_PROPERTY_DATA* wp_data,
    BACNET_APPLICATION_DATA_VALUE* value)
{
    if (!WP_ValidateEnumTypeAndRange(wp_data, value, RELIABILITY_PROPRIETARY_MAX)) return false;

    bool status = WPValidateArgType(value, BACNET_APPLICATION_TAG_ENUMERATED,
        &wp_data->error_class, &wp_data->error_code);
    if (!status) return status;

    if (currentObject->multistateCommon.common.Out_Of_Service) {
        currentObject->multistateCommon.common.shadowReliability = (BACNET_RELIABILITY)value->type.Enumerated;
        return true;
    }
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    return false;
}


static char* Multistate_Input_State_Text_Get(
    MULTISTATE_INPUT_DESCR* currentObject,
    uint32_t state_index)
{
    // todo 2 - add bounds check here
    return currentObject->multistateCommon.State_Text[state_index - 1];
}


//bool Multistate_Input_State_Text_Set(
//    uint32_t object_instance,
//    uint32_t state_index,
//    char *new_name)
//{
//    unsigned index = 0; /* offset from instance lookup */
//    size_t i = 0;       /* loop counter */
//    bool status = false;        /* return value */
//
//    panic();
//    //index = Multistate_Input_Instance_To_Index(object_instance);
//    //if ((index < MAX_MULTISTATE_INPUTS) && (state_index > 0) &&
//    //    (state_index <= MULTISTATE_NUMBER_OF_STATES)) {
//    //    state_index--;
//    //    status = true;
//    //    if (new_name) {
//    //        for (i = 0; i < sizeof(State_Text[index][state_index]); i++) {
//    //            State_Text[index][state_index][i] = new_name[i];
//    //            if (new_name[i] == 0) {
//    //                break;
//    //            }
//    //        }
//    //    }
//    //    else {
//    //        for (i = 0; i < sizeof(State_Text[index][state_index]); i++) {
//    //            State_Text[index][state_index][i] = 0;
//    //        }
//    //    }
//    //}
//
//    return status;
//}


#if ( BACNET_SVC_COV_B == 1 )
// This function reports to the BACnet stack if there has/has not been a change to the Present Value or status flags
// therefore, unfortunately, we have to derive the pointers from the object_instance each time.
bool Multistate_Input_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t object_instance)
{
    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(
        pDev,
        object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    // COVs are generated by a) application logic  b) BACnet operation  c) Client side reads
    // and apply to changes in a) Present_Value, b) Status Flags (all of OOS, Flt, AL, Ovr) !

    return currentObject->multistateCommon.Changed ;

    // Modern implementations of change detection is event driven, and every time a Present Value is
    // updated (a fairly rare event, compared to change detection) the Changed flag will be updated
    // if necessary. Do not try and do any further checking, it is unnecessary, and also leads to
    // double COV events (a fail).
}


/**
 * For a given object instance-number, loads the value_list with the COV data.
 *
 * @param  object_instance - object-instance number of the object
 * @param  value_list - list of COV data
 *
 * @return  true if the value list is encoded
 */
bool Multistate_Input_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list)
{
    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(
        pDev,
        object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    BACNET_APPLICATION_DATA_VALUE tvalue;

    tvalue.tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
    tvalue.type.Unsigned_Int = Multistate_Input_Present_Value_Get(currentObject);

    return Common_Encode_Value_List(
        &currentObject->multistateCommon.common,
        isInAlarm(currentObject),
        isInFault(currentObject),
        isOutOfService(currentObject),
        &tvalue,
        value_list);
}


void Multistate_Input_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance)
{
    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(pDev, instance);
    if (currentObject == NULL) {
        panic();
        return;
    }
    currentObject->multistateCommon.Changed = false;
}
#endif // ( BACNET_SVC_COV_B == 1 )


static void Multistate_Input_Out_Of_Service_Set(
    MULTISTATE_INPUT_DESCR* currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->multistateCommon.common.Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->multistateCommon.Changed = true;
#endif

    currentObject->multistateCommon.common.Out_Of_Service = oos_flag;
}

static bool Multistate_Input_State_Text_Write(
    MULTISTATE_INPUT_DESCR* currentObject,
    uint32_t state_index,
    BACNET_CHARACTER_STRING* char_string,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code)
{
    unsigned index = 0;
    size_t length = 0;
    uint8_t encoding = 0;
    bool status = false;

    if (state_index-- < MULTISTATE_NUMBER_OF_STATES) {
        length = characterstring_length(char_string);
        if (length <= sizeof(currentObject->multistateCommon.State_Text[state_index])) {
            encoding = characterstring_encoding(char_string);
            if (encoding == CHARACTER_UTF8) {
                status =
                    characterstring_ansi_copy(currentObject->multistateCommon.State_Text[state_index],
                        sizeof(currentObject->multistateCommon.State_Text[state_index]), char_string);
                if (!status) {
                    *error_class = ERROR_CLASS_PROPERTY;
                    *error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            else {
                *error_class = ERROR_CLASS_PROPERTY;
                *error_code = ERROR_CODE_CHARACTER_SET_NOT_SUPPORTED;
            }
        }
        else {
            *error_class = ERROR_CLASS_PROPERTY;
            *error_code = ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
        }
    }
    else {
        *error_class = ERROR_CLASS_PROPERTY;
        *error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
    }
    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Multistate_Input_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata)
{
    int len;
    int apdu_len;   /* return value */
    // BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t* apdu;
    uint32_t present_value = 0;
    unsigned i = 0;
    uint32_t max_states = 0;
    bool state = false;
    time_t now;

#ifdef BAC_DEBUG
    if ((rpdata == NULL) ||
        (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }
#endif

    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(pDev, rpdata->object_instance);
    if (currentObject == NULL) {
        panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_MULTISTATE_INPUT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
        Multistate_Input_Object_Name(
            pDev,
            rpdata->object_instance,
            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_DESCRIPTION:
        apdu_len =
            encode_application_character_string(
                &apdu[0],
                BACnetObject_Description_Get(&currentObject->multistateCommon.common));
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(
                &apdu[0],
                OBJECT_MULTISTATE_INPUT);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_unsigned(
                apdu,
                Multistate_Input_Present_Value_Get(currentObject));
        break;

    case PROP_STATUS_FLAGS:
        apdu_len =
            encode_status_flags(
                apdu,
                isInAlarm(currentObject),
                isInFault(currentObject),
                isOutOfService(currentObject));
        break;

    case PROP_EVENT_STATE:
#if ( INTRINSIC_REPORTING_B == 1 )
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->multistateCommon.common.Event_State);
#else
        apdu_len =
            encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
#endif
        break;

#if ( BACNET_PROTOCOL_REVISION >= 13 )
#if ( INTRINSIC_REPORTING_B == 1 )
    case PROP_EVENT_DETECTION_ENABLE:
        apdu_len =
            encode_application_boolean(&apdu[0], true);
        break;
#endif
#endif

    case PROP_RELIABILITY:
        apdu_len =
            encode_application_enumerated(
                &apdu[0],
                Multistate_Input_Reliability_Get(currentObject));
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len =
            encode_application_boolean(
                apdu,
                isOutOfService(currentObject));
        break;

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_TIME_DELAY:
        apdu_len =
            encode_application_unsigned(&apdu[0], currentObject->multistateCommon.common.eventCommon.Time_Delay);
        break;

    case PROP_NOTIFICATION_CLASS:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                currentObject->multistateCommon.common.eventCommon.Notification_Class);
        break;

    case PROP_EVENT_ENABLE:
        apdu_len = encode_application_bitstring3(
            apdu,
            (currentObject->multistateCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false,
            (currentObject->multistateCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false,
            (currentObject->multistateCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        break;

    case PROP_ACKED_TRANSITIONS:
        apdu_len = encode_application_bitstring3(
            apdu,
            currentObject->multistateCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_OFFNORMAL].bIsAcked,
            currentObject->multistateCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked,
            currentObject->multistateCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);
        break;

    case PROP_NOTIFY_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->multistateCommon.common.eventCommon.Notify_Type ? NOTIFY_EVENT : NOTIFY_ALARM);
        break;

    case PROP_EVENT_TIME_STAMPS:
        apdu_len = encode_application_event_time_stamps(apdu, rpdata, currentObject->multistateCommon.common.eventCommon.Event_Time_Stamps);
        break;

#endif  // ( INTRINSIC_REPORTING_B == 1 )

	case PROP_NUMBER_OF_STATES:
		apdu_len =
			encode_application_unsigned(
				apdu,
				Multistate_Input_Max_States(rpdata->object_instance));
		break;

	case PROP_STATE_TEXT:
		if (rpdata->array_index == 0) {
			/* Array element zero is the number of elements in the array */
			apdu_len =
				encode_application_unsigned(&apdu[0],
					Multistate_Input_Max_States(rpdata->object_instance));
		}
		else if (rpdata->array_index == BACNET_ARRAY_ALL) {
			/* if no index was specified, then try to encode the entire list */
			/* into one packet. */
			// todo
			max_states =
				Multistate_Input_Max_States(rpdata->object_instance);
			apdu_len = 0;
			for (i = 1; i <= max_states; i++) {
				characterstring_init_ansi(&char_string,
					Multistate_Input_State_Text_Get(
						currentObject,
						i));
				len =
					encode_application_character_string(&apdu[apdu_len],
						&char_string);
				/* add it if we have room */
				if ((apdu_len + len) < pDev->datalink->max_lpdu ) {
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
		else {
			max_states =
				Multistate_Input_Max_States(rpdata->object_instance);
			if (rpdata->array_index <= max_states) {
				characterstring_init_ansi(&char_string,
					Multistate_Input_State_Text_Get(
						currentObject,
						rpdata->array_index));
				apdu_len =
					encode_application_character_string(&apdu[0],
						&char_string);
			}
			else {
				rpdata->error_class = ERROR_CLASS_PROPERTY;
				rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
				apdu_len = BACNET_STATUS_ERROR;
			}
		}
		break;

	case PROP_PROPERTY_LIST:
		apdu_len = property_list_encode(
			rpdata,
			Properties_Required,
			Properties_Optional,
			Properties_Proprietary);
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
#if (INTRINSIC_REPORTING_B == 1)
		(rpdata->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
		(rpdata->object_property != PROP_STATE_TEXT) &&
		(rpdata->array_index != BACNET_ARRAY_ALL)) {
		rpdata->error_class = ERROR_CLASS_PROPERTY;
		rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
		apdu_len = BACNET_STATUS_ERROR;
	}

	return apdu_len;
}


/* returns true if successful */
bool Multistate_Input_Write_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data)
{
    bool status = false;        /* return value */
    int len;
    BACNET_APPLICATION_DATA_VALUE value;
    uint32_t max_states;
    uint32_t array_index;
    uint element_len;

    /* decode some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
            wp_data->application_data_len, &value);

    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    /*  only array properties can have array options */
    if ((wp_data->array_index != BACNET_ARRAY_ALL) &&

#if (INTRINSIC_REPORTING_B == 1)
        (wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
        (wp_data->object_property != PROP_STATE_TEXT) &&
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(pDev, wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        status = Multistate_Input_Present_Value_Set(
            currentObject,
            wp_data,
            &value);
        break;

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Multistate_Input_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_RELIABILITY:
        status =
            Common_Reliability_Set(
                &currentObject->multistateCommon.common,
                wp_data,
                &value);
        break;

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_TIME_DELAY:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->multistateCommon.common.eventCommon.Time_Delay = value.type.Unsigned_Int;
            currentObject->multistateCommon.common.eventCommon.Remaining_Time_Delay = currentObject->multistateCommon.common.eventCommon.Time_Delay;
        }
        break;

    case PROP_NOTIFICATION_CLASS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->multistateCommon.common.eventCommon.Notification_Class = value.type.Unsigned_Int;
        }
        break;

    case PROP_EVENT_ENABLE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            if (value.type.Bit_String.bits_used == 3) {
                currentObject->multistateCommon.common.eventCommon.Event_Enable = value.type.Bit_String.value[0];
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                status = false;
            }
        }
        break;

    case PROP_NOTIFY_TYPE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            switch ((BACNET_NOTIFY_TYPE)value.type.Enumerated) {
            case NOTIFY_EVENT:
            case NOTIFY_ALARM:
                currentObject->multistateCommon.common.eventCommon.Notify_Type = (BACNET_NOTIFY_TYPE)value.type.Enumerated;
                break;
            default:
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                status = false;
                break;
            }
        }
        break;

#endif // ( INTRINSIC_REPORTING_B == 1 )

#if ( INTRINSIC_REPORTING_B == 1 && BACNET_PROTOCOL_REVISION >= 13 )
    case PROP_EVENT_DETECTION_ENABLE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            status = false;
        }
        break;
#endif

    case PROP_STATE_TEXT:
        if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
            if (wp_data->array_index == 0) {
                /* Array element zero is the number of
                   elements in the array.  We have a fixed
                   size array, so we are read-only. */
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
            else if (wp_data->array_index == BACNET_ARRAY_ALL) {
				// if the client is to write the whole array, then may shorten or lengthen the original.
				// if the server can accomodate this, fine, if the server cannot accomodate this, then it will have to
				// send an error of some sort... todo ekh 2 research error message
                // we must also persist state text changes...
                max_states =
                    Multistate_Input_Max_States(wp_data->object_instance);
                array_index = 1;
                element_len = len;
                do {
                    if (element_len) {
                        status =
                            Multistate_Input_State_Text_Write(
                                currentObject,
                                array_index,
                                &value.type.Character_String,
                                &wp_data->error_class, &wp_data->error_code);
                    }
                    max_states--;
                    array_index++;
                    if (max_states) {
                        element_len =
                            bacapp_decode_application_data(&wp_data->
                                application_data[len],
                                wp_data->application_data_len - len, &value);
                        if (element_len < 0) {
                            wp_data->error_class = ERROR_CLASS_PROPERTY;
                            wp_data->error_code =
                                ERROR_CODE_VALUE_OUT_OF_RANGE;
                            break;
                        }
                        len += element_len;
                    }
                } while (max_states);
            }
            else {
                max_states =
                    Multistate_Input_Max_States(wp_data->object_instance);
                if (wp_data->array_index <= max_states) {
                    status =
                        Multistate_Input_State_Text_Write(
                            currentObject,
                            wp_data->array_index,
                            &value.type.Character_String,
                            &wp_data->error_class, &wp_data->error_code);
                }
                else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                }
            }
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;

    case PROP_PROPERTY_LIST:
    case PROP_DESCRIPTION:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_ACKED_TRANSITIONS:
    case PROP_EVENT_TIME_STAMPS:
#endif
    case PROP_NUMBER_OF_STATES:
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


#if (INTRINSIC_REPORTING_B == 1)

void Multistate_Input_Intrinsic_Reporting(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    MULTISTATE_INPUT_DESCR* currentObject;

    currentObject = Multistate_Input_Instance_To_Object(pDev, object_instance);
    if (currentObject == NULL) {
        panic();
        return;
    }

    Common_Multistate_Intrinsic_Reporting(
        &currentObject->multistateCommon.common,
        &currentObject->multistateCommon.common.eventCommon);
}


int Multistate_Input_Event_Information(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA* getevent_data)
{
    MULTISTATE_INPUT_DESCR* currentObject = (MULTISTATE_INPUT_DESCR*)Generic_Index_To_Object(&pDev->MSI_Descriptor_List, index);
    if (currentObject == NULL) return -1;

	return Common_Event_Information(pDev, &currentObject->multistateCommon.common, OBJECT_MULTISTATE_INPUT, getevent_data);
}


int Multistate_Input_Alarm_Ack(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_ALARM_ACK_DATA* alarmack_data,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code)
{

    MULTISTATE_INPUT_DESCR* currentObject = Multistate_Input_Instance_To_Object(pDev, alarmack_data->eventObjectIdentifier.instance);
    if (currentObject == NULL) {
        *error_class = ERROR_CLASS_OBJECT;
        *error_code = ERROR_CODE_UNKNOWN_OBJECT;
        panic();
        return -1;
    }

    if (currentObject == NULL) {
        panic();
        return -1;
    }

    return Common_Alarm_Ack(&currentObject->multistateCommon.common, alarmack_data, error_class, error_code);
}


#if ( BACNET_PROTOCOL_REVISION < 13 )
int Multistate_Input_Alarm_Summary(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA* getalarm_data)
{

    if (index >= multistateInputs.size()) return -1;  // So there is no exception below
    MultistateInputObject* currentObject = static_cast<MultistateInputObject*> (multistateInputs.at(index));
    if (currentObject == NULL) return -1;

    if ((currentObject->Event_State != EVENT_STATE_NORMAL) &&
        (currentObject->Notify_Type == NOTIFY_ALARM)) {
        getalarm_data->objectIdentifier.type = OBJECT_MULTISTATE_INPUT;
        getalarm_data->objectIdentifier.instance = currentObject->instance; // currentInstance;
        getalarm_data->alarmState = currentObject->Event_State;
        bitstring_init(&getalarm_data->acknowledgedTransitions);
        bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
            TRANSITION_TO_OFFNORMAL,
            currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked);
        bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
            TRANSITION_TO_FAULT,
            currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked);
        bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
            TRANSITION_TO_NORMAL,
            currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);

        return 1;   /* active alarm */
    }
    else {
        return 0;    /* no active alarm at this index */
    }
    else
        return -1;      /* end of list  */
}
#endif // ( BACNET_PROTOCOL_REVISION < 13 )

#endif /* (INTRINSIC_REPORTING_B == 1) */


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE* pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS* pErrorClass,
    BACNET_ERROR_CODE* pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}


void testMultistateInput(
    Test* pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint16_t len_value = 0;
    uint8_t tag_number = 0;
    uint32_t decoded_instance = 0;
    uint16_t decoded_type = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Multistate_Input_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_MULTISTATE_INPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Multistate_Input_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);
}


int main(
    void)
{
    Test* pTest;
    bool rc;

    pTest = ct_create("BACnet Multistate Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testMultistateInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_XXX_XXX */

#endif // if (BACNET_USE_OBJECT_XXX_XXX == 1 )
