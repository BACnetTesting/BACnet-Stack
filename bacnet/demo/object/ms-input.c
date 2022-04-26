/****************************************************************************************
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
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
*  2018.07.04 EKH Diffed in hints from Binary Value for future reference
*
****************************************************************************************/

#include "configProj.h"     /* the custom stuff */

#if (BACNET_USE_OBJECT_MULTISTATE_INPUT == 1)

#include "handlers.h"
#include "ms-input.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"
// #include "proplist.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "device.h"
#endif

static LLIST_HDR MSI_Descriptor_List;

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
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
#endif
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Multistate_Input_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// Gets called once for each device
// This only gets called once on startup, and has to initialize for ALL virtual devices. Todo 2 - rename to differentiate!
void Multistate_Input_Init(
    void)
{
    ll_Init(&MSI_Descriptor_List, 100);

#if (INTRINSIC_REPORTING_B == 1)
    /* Set handler for GetAlarmSummary Service */
    // Deprecated since Rev 13
    /* Set handler for GetAlarmSummary Service */
    //handler_get_alarm_summary_set(OBJECT_ANALOG_INPUT,
    //    Analog_Input_Alarm_Summary);

#endif
}


bool Multistate_Input_Create(
    const uint32_t instance,
    const char *name)
{
    MULTISTATE_INPUT_DESCR *currentObject = (MULTISTATE_INPUT_DESCR *)emm_scalloc('a', sizeof(MULTISTATE_INPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&MSI_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    // Note that our structure is 0 initialized by calloc, so no zeroing operations are _required_.
    // Some are here just for clarity.
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = false;
    currentObject->Prior_Value = 0;
#endif

#if (INTRINSIC_REPORTING_B == 1)
    // todo 2 there is commonality here too that needs consolidation, optimization
    currentObject->Event_State = EVENT_STATE_NORMAL;
    /* notification class not connected */
    currentObject->Notification_Class = BACNET_MAX_INSTANCE;

    currentObject->Notify_Type = NOTIFY_ALARM;
    currentObject->Ack_notify_data.EventState = EVENT_STATE_NORMAL;

    /* initialize Event time stamps using wildcards
       and set Acked_transitions */
    for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
        datetime_wildcard_set(&currentObject->Event_Time_Stamps[j]);
        currentObject->Acked_Transitions[j].bIsAcked = true;
    }
#endif

    strcpy(currentObject->State_Text[0], "Off");
    strcpy(currentObject->State_Text[1], "On");

    return true;
}


bool Multistate_Input_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&MSI_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Multistate_Input_Count(
    void)
{
    return MSI_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Multistate_Input_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&MSI_Descriptor_List, index);
}


static inline bool isOutOfService(MULTISTATE_INPUT_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static inline bool isInAlarm(MULTISTATE_INPUT_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
}


static BACNET_RELIABILITY Multistate_Input_Reliability_Get(
    MULTISTATE_INPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadowReliability ;
    }

    // In this reference stack, nobody ever actually sets reliability, we expect the Application to do so (along with PV).
    return currentObject->Reliability;
}


static bool isInFault(
    MULTISTATE_INPUT_DESCR *currentObject)
{
    return (Multistate_Input_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


MULTISTATE_INPUT_DESCR *Multistate_Input_Instance_To_Object(
    uint32_t object_instance)
{
    return (MULTISTATE_INPUT_DESCR *)Generic_Instance_To_Object(&MSI_Descriptor_List, object_instance);
}


uint32_t Multistate_Input_Present_Value(
    MULTISTATE_INPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadow_Present_Value;
    }
    return currentObject->Present_Value;
}


//bool Multistate_Input_Present_Value_Set(
//    uint32_t object_instance,
//    uint32_t value)
//{
//    bool status = false;
//
//    return status;
//}


//void Multistate_Input_Out_Of_Service_Set(
//    MULTISTATE_INPUT_DESCR *currentObject,
//    bool value)
//{
//    currentObject->Out_Of_Service = value;
//}


#if ( BACNET_SVC_COV_B == 1 )
static void Multistate_Input_COV_Detect_PV_Change(
    MULTISTATE_INPUT_DESCR *currentObject,
    uint value)
{
    if ( value != currentObject->Prior_Value ) {
        currentObject->Changed = true;
        currentObject->Prior_Value = value;
    }
}
#endif


static bool Multistate_Input_Present_Value_Set(
    MULTISTATE_INPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    bool status = WPValidateArgType(
        value,
        BACNET_APPLICATION_TAG_UNSIGNED_INT,
        &wp_data->error_class, &wp_data->error_code);

    if (!status) return false;

    if (!currentObject->Out_Of_Service) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    }

#if ( BACNET_SVC_COV_B == 1 )
    Multistate_Input_COV_Detect_PV_Change(currentObject, value->type.Unsigned_Int);
#endif

    if (currentObject->Out_Of_Service) {
        currentObject->shadow_Present_Value = (uint16_t)value->type.Unsigned_Int;
    }
    else {
        currentObject->Present_Value = (uint16_t)value->type.Unsigned_Int;
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
    uint32_t object_instance,
    BACNET_CHARACTER_STRING *object_name)
{
    return Generic_Instance_To_Object_Name(&MSI_Descriptor_List, object_instance, object_name);
}


#if 0
/* note: the object name must be unique within this device */
bool Multistate_Input_Name_Set(
    uint32_t object_instance,
    char *new_name)
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
    BACNET_CHARACTER_STRING * char_string,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
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
    MULTISTATE_INPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    if (!WP_ValidateEnumTypeAndRange(wp_data, value, RELIABILITY_PROPRIETARY_MAX)) return false;  // todo 1 BTC range test opportunity

    bool status = WPValidateArgType(value, BACNET_APPLICATION_TAG_ENUMERATED,
        &wp_data->error_class, &wp_data->error_code);
    if (!status) return status;

    if (currentObject->Out_Of_Service) {
        currentObject->shadowReliability = (BACNET_RELIABILITY) value->type.Enumerated;
        return true;
    }
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    return false;
}


static char *Multistate_Input_State_Text_Get(
    MULTISTATE_INPUT_DESCR *currentObject,
    uint32_t state_index)
{
    // todo 2 - add bounds check here, BTC
    return currentObject->State_Text[state_index-1];
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
    const uint32_t object_instance)
{
    MULTISTATE_INPUT_DESCR *currentObject = Multistate_Input_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }
    return currentObject->Changed;
}


void Multistate_Input_Change_Of_Value_Clear(
    const uint32_t instance)
{
    MULTISTATE_INPUT_DESCR *currentObject = Multistate_Input_Instance_To_Object(instance);
    if (currentObject == NULL) {
        panic();
        return;
    }
    currentObject->Changed = false;
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
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    MULTISTATE_INPUT_DESCR *currentObject = Multistate_Input_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
        value_list->value.type.Unsigned_Int =
            Multistate_Input_Present_Value(currentObject);
        value_list->value.next = NULL;
        value_list->priority = BACNET_NO_PRIORITY;
        value_list = value_list->next;
    }
    if (value_list) {
        value_list->propertyIdentifier = PROP_STATUS_FLAGS;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_BIT_STRING;
        value_list->value.next = NULL;
        bitstring_init(&value_list->value.type.Bit_String);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_IN_ALARM, isInAlarm(currentObject));
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_FAULT, isInFault(currentObject));
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OVERRIDDEN, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OUT_OF_SERVICE, isOutOfService(currentObject));
        value_list->value.next = NULL;
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
        status = true;
    }
    return status;
}
#endif // ( BACNET_SVC_COV_B == 1 )


static void Multistate_Input_Out_Of_Service_Set(
    MULTISTATE_INPUT_DESCR *currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = true;
#endif

    currentObject->Out_Of_Service = oos_flag;
}


static bool Multistate_Input_State_Text_Write(
    MULTISTATE_INPUT_DESCR *currentObject,
    uint32_t state_index,
    BACNET_CHARACTER_STRING * char_string,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    unsigned index = 0; /* offset from instance lookup */
    size_t length = 0;
    uint8_t encoding = 0;
    bool status = false;        /* return value */

    if ( state_index-- < MULTISTATE_NUMBER_OF_STATES ) {
        length = characterstring_length(char_string);
        if (length <= sizeof(currentObject->State_Text[state_index])) {
            encoding = characterstring_encoding(char_string);
            if (encoding == CHARACTER_UTF8) {
                status =
                    characterstring_ansi_copy(currentObject->State_Text[state_index],
                        sizeof(currentObject->State_Text[state_index]), char_string);
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
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int len;
    int apdu_len;   /* return value */
    // BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;
    uint32_t present_value = 0;
    unsigned i = 0;
    uint32_t max_states = 0;
    bool state = false;

#ifdef BAC_DEBUG
    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }
#endif

    MULTISTATE_INPUT_DESCR *currentObject = Multistate_Input_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_MULTI_STATE_INPUT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
        Multistate_Input_Object_Name(rpdata->object_instance,
            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_DESCRIPTION:
        apdu_len =
            encode_application_character_string(&apdu[0], BACnetObject_Description_Get( &currentObject->common ));
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(
                apdu,
                OBJECT_MULTI_STATE_INPUT);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_unsigned(
                apdu,
                Multistate_Input_Present_Value(currentObject));
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
                currentObject->Event_State);
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
            encode_application_enumerated(&apdu[0],
                currentObject->Reliability);
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
            encode_application_unsigned(&apdu[0], currentObject->Time_Delay);
        break;

    case PROP_NOTIFICATION_CLASS:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                currentObject->Notification_Class);
        break;

    case PROP_EVENT_ENABLE:
        apdu_len = encode_application_bitstring3(
            apdu,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        break;

    case PROP_ACKED_TRANSITIONS:
        apdu_len = encode_application_bitstring3(
            apdu,
            currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].bIsAcked,
            currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked,
            currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);
        break;

    case PROP_NOTIFY_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->Notify_Type ? NOTIFY_EVENT : NOTIFY_ALARM);
        break;

    case PROP_EVENT_TIME_STAMPS:
        /* Array element zero is the number of elements in the array */
        // todo3 - this can be consolidated across multiple objects
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
                        &currentObject->Event_Time_Stamps[i].date);
                len +=
                    encode_application_time(&apdu[apdu_len + len],
                        &currentObject->Event_Time_Stamps[i].time);
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
                    &currentObject->Event_Time_Stamps[rpdata->array_index-1].date);
            apdu_len +=
                encode_application_time(&apdu[apdu_len],
                    &currentObject->Event_Time_Stamps[rpdata->array_index-1].time);
            apdu_len +=
                encode_closing_tag(&apdu[apdu_len], TIME_STAMP_DATETIME);
        }
        else {
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
            apdu_len = BACNET_STATUS_ERROR;
        }
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
            max_states =
                Multistate_Input_Max_States(rpdata->object_instance);
            apdu_len = 0;
            for (i = 1; i <= max_states; i++) {
                characterstring_init_ansi(&char_string,
                    Multistate_Input_State_Text_Get(
                        currentObject,
                        i));
                /* FIXME: this might go beyond MAX_LPDU_IP length! */
                len =
                    encode_application_character_string(&apdu[apdu_len],
                        &char_string);
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
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len;
    int element_len;
    BACNET_APPLICATION_DATA_VALUE value;
    uint32_t max_states;
    uint32_t array_index;
    // BACNET_OBJECT_TYPE object_type;
//    uint32_t object_instance;

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

    MULTISTATE_INPUT_DESCR *currentObject = Multistate_Input_Instance_To_Object(wp_data->object_instance);
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
            Multistate_Input_Reliability_Set(
                currentObject,
                wp_data,
                &value);
        break;

#if 0
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            status =
                Multistate_Input_Present_Value_Set
                (wp_data->object_instance, value.type.Unsigned_Int);
            if (!status) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;
#endif

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_TIME_DELAY:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->Time_Delay = value.type.Unsigned_Int;
            currentObject->Remaining_Time_Delay = currentObject->Time_Delay;
        }
        break;

    case PROP_NOTIFICATION_CLASS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->Notification_Class = value.type.Unsigned_Int;
        }
        break;

    case PROP_HIGH_LIMIT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_SIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->High_Limit = value.type.Signed_Int;
        }
        break;

    case PROP_LOW_LIMIT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_SIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->Low_Limit = value.type.Signed_Int;
        }
        break;

    case PROP_DEADBAND:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_SIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->Deadband = value.type.Signed_Int;
        }
        break;

    case PROP_LIMIT_ENABLE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            if (value.type.Bit_String.bits_used == 2) {
                currentObject->Limit_Enable = value.type.Bit_String.value[0];
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                status = false;
            }
        }
        break;

    case PROP_EVENT_ENABLE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            if (value.type.Bit_String.bits_used == 3) {
                currentObject->Event_Enable = value.type.Bit_String.value[0];
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
                currentObject->Notify_Type = (BACNET_NOTIFY_TYPE)value.type.Enumerated;
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


#if 0
        // nowhere else do we allow writing of object name, putting this on ice..
    case PROP_OBJECT_NAME:
        if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
            /* All the object names in a device must be unique */
            if (Device_Valid_Object_Name(&value.type.Character_String,
                &object_type, &object_instance)) {
                if ((object_type == wp_data->object_type) &&
                    (object_instance == wp_data->object_instance)) {
                    /* writing same name to same object */
                    status = true;
                }
                else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_DUPLICATE_NAME;
                }
            }
            else {
                status = 
                    Multistate_Input_Object_Name_Write(wp_data->
                        object_instance, &value.type.Character_String,
                        &wp_data->error_class, &wp_data->error_code);
            }
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        }
        break;

    case PROP_DESCRIPTION:
        if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
            status =
                Multistate_Input_Description_Write(wp_data->
                    object_instance, &value.type.Character_String,
                    &wp_data->error_class, &wp_data->error_code);
        }
        else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
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

        // BTC todo1 - missing case not detected? case PROP_PROPERTY_LIST:
        // commenting out for the test
    // case PROP_PROPERTY_LIST:
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



#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"


bool Device_Valid_Object_Name(
    BACNET_CHARACTER_STRING * object_name,
    BACNET_OBJECT_TYPE *object_type,
    uint32_t * object_instance)
{
    return true;
}

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}


void testMultistateInput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Multistate_Input_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_MULTI_STATE_INPUT;
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

#ifdef TEST_MULTISTATE_INPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Multi-state Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testMultistateInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_MULTISTATE_INPUT == 1 )
