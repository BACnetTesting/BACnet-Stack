/****************************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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


#include "config.h"     /* the custom stuff */

// #if (BACNET_USE_OBJECT_ANALOG_INPUT == 1)

#include "handlers.h"
#include "ai.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"
#include "proplist.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "device.h"
#endif

static LLIST_HDR AI_Descriptor_List;

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_UNITS,
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
    PROP_HIGH_LIMIT,
    PROP_LOW_LIMIT,
    PROP_DEADBAND,
    PROP_LIMIT_ENABLE,
#endif

#if (BACNET_SVC_COV_B == 1)
    PROP_COV_INCREMENT,
#endif

    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    // Todo BTC - Sensitivity test for existence of 'karg stack'
    //9997,
    //9998,
    //9999,
    MAX_BACNET_PROPERTY_ID
};


void Analog_Input_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    // Do NOT be tempted to use property_list_required() - that is for supporting epics.c, and perhaps other Client operations, only
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// Gets called once for each device
// This only gets called once on startup, and has to initialize for ALL virtual devices. Todo 2 - rename to differentiate!
void Analog_Input_Init(
    void)
{
    ll_Init(&AI_Descriptor_List, 100);

#if (INTRINSIC_REPORTING_B == 1)

    /* Set handler for GetEventInformation function */
    handler_get_event_information_set(
        OBJECT_ANALOG_INPUT,
        Analog_Input_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
    handler_alarm_ack_set(
        OBJECT_ANALOG_INPUT,
        Analog_Input_Alarm_Ack);

    /* Set handler for GetAlarmSummary Service */
    // Deprecated since Rev 13
    /* Set handler for GetAlarmSummary Service */
    //handler_get_alarm_summary_set(OBJECT_ANALOG_INPUT,
    //    Analog_Input_Alarm_Summary);

#endif
}


bool Analog_Input_Create(
    const uint32_t instance,
    const char *name)
{
    ANALOG_INPUT_DESCR *currentObject = (ANALOG_INPUT_DESCR *)emm_scalloc('a', sizeof(ANALOG_INPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&AI_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    // Note that our structure is 0 initialized by calloc, so no zeroing operations are _required_.
    // Some are here just for clarity.
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;
    currentObject->Units = UNITS_PERCENT;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = false;
    currentObject->Prior_Value = 0.0f;
    currentObject->COV_Increment = 1.0f;
#endif

#if (INTRINSIC_REPORTING_B == 1)
    // todo 2 there is commonality here too that needs consolidation, optimization
    currentObject->Event_State = EVENT_STATE_NORMAL;
    /* notification class not connected */
    currentObject->Notification_Class = BACNET_MAX_INSTANCE;

    currentObject->Notify_Type = NOTIFY_ALARM;
    currentObject->Ack_notify_data.EventState = EVENT_STATE_NORMAL;
    currentObject->High_Limit = 100.0;
    currentObject->Low_Limit = 0.0;
    currentObject->Deadband = 1.0;
    currentObject->Limit_Enable = 0;

    /* initialize Event time stamps using wildcards
       and set Acked_transitions */
    for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
        datetime_wildcard_set(&currentObject->Event_Time_Stamps[j]);
        currentObject->Acked_Transitions[j].bIsAcked = true;
    }
#endif

    return true;
}


#if defined(INTRINSIC_REPORTING)
bool Analog_Input_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&AI_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Analog_Input_Count(
    void)
{
    return AI_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
// This is used by the Device Object Function Table. Must have this signature.
uint32_t Analog_Input_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&AI_Descriptor_List, index);
}


// This is a shitty function to have - use Generic_Instance_To_Object() for those functions that wanted this.. *
//unsigned Analog_Input_Instance_To_Index(
//    uint32_t object_instance)
//{
//    unsigned index = MAX_ANALOG_INPUTS;
//
//    if (object_instance < MAX_ANALOG_INPUTS)
//        index = object_instance;
//
//    return index;
//}


static inline bool isOutOfService(ANALOG_INPUT_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static inline bool isInAlarm(ANALOG_INPUT_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
}


static BACNET_RELIABILITY Analog_Input_Reliability_Get(
    ANALOG_INPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadowReliability ;
    }

    // In this reference stack, nobody ever actually sets reliability, we expect the Application to do so (along with PV).
    return currentObject->Reliability;
}


static bool isInFault(
    ANALOG_INPUT_DESCR *currentObject)
{
    return (Analog_Input_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


ANALOG_INPUT_DESCR *Analog_Input_Instance_To_Object(
    uint32_t object_instance)
{
    return (ANALOG_INPUT_DESCR *)Generic_Instance_To_Object(&AI_Descriptor_List, object_instance);
}


static float Analog_Input_Present_Value(
    ANALOG_INPUT_DESCR *currentObject)
{
 
    if (currentObject->Out_Of_Service) {
        return currentObject->shadow_Present_Value;
    }
    return currentObject->Present_Value;
}


char *Analog_Input_Description(
    uint32_t object_instance)
{
    unsigned index = 0; /* offset from instance lookup */
    char *pName = NULL; /* return value */

    //index = Multistate_Input_Instance_To_Index(object_instance);
    //if (index < MAX_MULTISTATE_INPUTS) {
    //    pName = Object_Description[index];
    //}

    return pName;
}


#if ( BACNET_SVC_COV_B == 1 )
static void Analog_Input_COV_Detect_PV_Change(
    ANALOG_INPUT_DESCR *currentObject,
    float value)
{
    if (fabs(value - currentObject->Prior_Value) >= currentObject->COV_Increment) {
        currentObject->Changed = true;
        currentObject->Prior_Value = value;
    }
}
#endif


static bool Analog_Input_Present_Value_Set(
    ANALOG_INPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    bool status = WPValidateArgType(
        value,
        BACNET_APPLICATION_TAG_REAL,
        &wp_data->error_class, &wp_data->error_code);
    if (!status) return false;

    if (!currentObject->Out_Of_Service) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    }

#if ( BACNET_SVC_COV_B == 1 )
    Analog_Input_COV_Detect_PV_Change(currentObject, value->type.Real);
#endif

    if (currentObject->Out_Of_Service) {
        currentObject->shadow_Present_Value = value->type.Real ;
    }
    else {
        currentObject->Present_Value = value->type.Real ;
    }
    return true;
}


//#if 0 // done below, properly
//static inline bool Analog_Input_Reliability_Set(
//    ANALOG_INPUT_DESCR *currentObject,
//    uint32_t reliability)
//{
//    if (currentObject->Out_Of_Service) {
//        // the BACnet way
//        currentObject->shadowReliability = (BACNET_RELIABILITY)reliability;
//        return true;
//    }
//    // writes, when not OOS, are not allowed
//    return false;
//}
//#endif


//static inline BACNET_RELIABILITY Analog_Input_Reliability_Get(
//    ANALOG_INPUT_DESCR *currentObject)
//{
//    if (currentObject->Out_Of_Service) {
//        return currentObject->shadowReliability ;
//    }
//        return currentObject->Reliability ;
//}


// Signature used by device mux
/* note: the object name must be unique within this device */
bool Analog_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name(
                &AI_Descriptor_List,
                object_instance, object_name);
}


void Analog_Input_Update(
    const uint32_t instance,
    const double value)
{
    ANALOG_INPUT_DESCR *bacnetObject = Analog_Input_Instance_To_Object(instance);
    if (bacnetObject == NULL) {
        panic();
        return;
    }
    bacnetObject->Present_Value = value;
}


double Analog_Input_Present_Value_from_Instance(
    const uint32_t instance)
{
    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(instance);
    if (currentObject == NULL) {
        panic();
        return 0.0;
    }
    return currentObject->Present_Value;
}


static inline bool Analog_Input_Reliability_Set(
    ANALOG_INPUT_DESCR *currentObject,
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


#if ( BACNET_SVC_COV_B == 1 )

// This function reports to the BACnet stack if there has/has not been a change to the Present Value or status flags
// therefore, unfortunately, we have to derive the pointers from the object_instance each time.
bool Analog_Input_Change_Of_Value(
    const uint32_t object_instance)
{
    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }
    return currentObject->Changed;
}


void Analog_Input_Change_Of_Value_Clear(
    const uint32_t instance)
{
    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(instance);
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
bool Analog_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;
    ANALOG_INPUT_DESCR *currentObject = Find_bacnet_AI_object(object_instance);
    if (currentObject == NULL)
    {
        return false;
    }

    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_REAL;
        value_list->value.type.Real =
            Analog_Input_Present_Value(currentObject);
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
    // nope. this needs to be true e.g. on initial subscription.
    // status = currentObject->Changed;

    return status;
}



// Not used anywhere, even in Karg Stack
//float Analog_Input_COV_Increment(
//    ANALOG_INPUT_DESCR *currentObject )
//{
//    int index;
//    float value = 0;
//
//    index = Analog_Input_Instance_To_Index(object_instance);
//    if (index >= 0) {
//        value = AI_Descr[index].COV_Increment;
//    }
//    else {
//        panic();
//    }
//
//    return value;
//}


void Analog_Input_COV_Increment_Set(
    ANALOG_INPUT_DESCR *currentObject,
    float value)
{
    currentObject->COV_Increment = value;
    if (fabs(Analog_Input_Present_Value(currentObject) - currentObject->Prior_Value) > currentObject->COV_Increment)         {
        currentObject->Changed = true;
    }
}
#endif // ( BACNET_SVC_COV_B == 1 )


static void Analog_Input_Out_Of_Service_Set(
    ANALOG_INPUT_DESCR *currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = true;
#endif

    currentObject->Out_Of_Service = oos_flag;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Analog_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;

#if (INTRINSIC_REPORTING_B == 1)
    unsigned int i = 0;
    int len = 0;
#endif

    if ((rpdata == NULL) ||
        (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }

    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_ANALOG_INPUT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
        Analog_Input_Object_Name(
            rpdata->object_instance,
            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_DESCRIPTION:
        characterstring_init_ansi(&char_string,
            Analog_Input_Description(rpdata->object_instance));
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(
                apdu,
                OBJECT_ANALOG_INPUT);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_real(
                apdu,
                Analog_Input_Present_Value(currentObject));
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
                Analog_Input_Reliability_Get(currentObject));
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len =
            encode_application_boolean(
                apdu,
                isOutOfService(currentObject));
        break;

    case PROP_UNITS:
        apdu_len =
            encode_application_enumerated(&apdu[0], currentObject->Units);
        break;

#if ( BACNET_SVC_COV_B == 1 )
    case PROP_COV_INCREMENT:
        apdu_len = encode_application_real(&apdu[0],
            currentObject->COV_Increment);
        break;
#endif // ( BACNET_SVC_COV_B == 1 )

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

    case PROP_HIGH_LIMIT:
        apdu_len =
            encode_application_real(&apdu[0], currentObject->High_Limit);
        break;

    case PROP_LOW_LIMIT:
        apdu_len = encode_application_real(&apdu[0], currentObject->Low_Limit);
        break;

    case PROP_DEADBAND:
        apdu_len = encode_application_real(&apdu[0], currentObject->Deadband);
        break;

    case PROP_LIMIT_ENABLE:
        apdu_len = encode_application_bitstring2(
            apdu,
            (currentObject->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ? true : false,
            (currentObject->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ? true : false);
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
                } else {
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
        (rpdata->object_property != PROP_PROPERTY_LIST) &&  // todo other project that passed btc did not have this. check it out
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
bool Analog_Input_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len;
    BACNET_APPLICATION_DATA_VALUE value;

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
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        status = Analog_Input_Present_Value_Set(
            currentObject,
            wp_data,
            &value);
        break;

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Analog_Input_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_RELIABILITY:
        status =
            Analog_Input_Reliability_Set(
                currentObject,
                wp_data,
                &value);
        break;

    case PROP_UNITS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->Units = (BACNET_ENGINEERING_UNITS)value.type.Enumerated;
        }
        break;

#if ( BACNET_SVC_COV_B == 1 )
    case PROP_COV_INCREMENT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Real >= 0.0) {
                Analog_Input_COV_Increment_Set(
                    currentObject,
                    value.type.Real);
            }
            else {
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;
#endif // ( BACNET_SVC_COV_B == 1 )

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
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->High_Limit = value.type.Real;
        }
        break;

    case PROP_LOW_LIMIT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->Low_Limit = value.type.Real;
        }
        break;

    case PROP_DEADBAND:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->Deadband = value.type.Real;
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

    case PROP_PROPERTY_LIST:
        // BTC todo1 - missing case not detected? case PROP_PROPERTY_LIST:
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

void Analog_Input_Intrinsic_Reporting(
    uint32_t object_instance)
{
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    ANALOG_INPUT_DESCR *currentObject;
    BACNET_EVENT_STATE FromState;
    BACNET_EVENT_STATE ToState;
    float ExceededLimit ;
    float PresentVal ;
    bool SendNotify = false;


    currentObject = Analog_Input_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return;
    }

    // todo 1
    /* Have to remove this limit_enable check, else we cannot process e.g. 13.3.6 (e) condition */

    /*
        13.2.2.1 Event-State-Detection State Machine

        If the Event_Detection_Enable property is FALSE, then this state machine is not evaluated. In this case, no transitions shall
        occur, Event_State shall be set to NORMAL, and Event_Time_Stamps, Event_Message_Texts and Acked_Transitions shall
        be set to their respective initial conditions.
    */


    // todo 1
    /* Have to remove this limit_enable check, else we cannot process e.g. 13.3.6 (e) condition */

    // (and (c))
    /* If pCurrentState is HIGH_LIMIT, and the HighLimitEnable flag of pLimitEnable is FALSE, then indicate a
        transition to the NORMAL event state */

        /* check limits */
            // todo, watch out, the logic below assumes both flags false, deal with the case where only one is false!
    if (!currentObject->Limit_Enable) {
        /* 13.3.6 (c) If pCurrentState is HIGH_LIMIT, and the HighLimitEnable flag of pLimitEnable is FALSE, then indicate a
        transition to the NORMAL event state. (etc) */
        // todo3 - BTC, we should examing both flags here.. Karg's logic is not complete here.
        currentObject->Event_State = EVENT_STATE_NORMAL;
        return;    /* limits are not configured */
    }

    if (currentObject->Ack_notify_data.bSendAckNotify) {
        /* clean bSendAckNotify flag */
        currentObject->Ack_notify_data.bSendAckNotify = false;
        /* copy toState */
        ToState = currentObject->Ack_notify_data.EventState;
        FromState = currentObject->Ack_notify_data.EventState;  // not actually used, just to eliminate warnings

        characterstring_init_ansi(&msgText, "AckNotification");

        /* Notify Type */
        event_data.notifyType = NOTIFY_ACK_NOTIFICATION;

        /* Send EventNotification. */
        SendNotify = true;
    }
    else {
        /* actual Present_Value */
        PresentVal = currentObject->Present_Value;
        FromState = currentObject->Event_State;

        switch (currentObject->Event_State) {
        case EVENT_STATE_NORMAL:

            // todo 0 - review
            //if (>FaultStatus) {
            //    currentObject->Event_State = EVENT_STATE_FAULT;
            //    break;
            //}

            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the High_Limit for a minimum
               period of time, specified in the Time_Delay property, and
               (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-OFFNORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal > currentObject->High_Limit) &&
                ((currentObject->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE)) {
                        // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                    //((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                    //    EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!currentObject->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_HIGH_LIMIT;
                }
                else {
                    currentObject->Remaining_Time_Delay--;
                }
                break;
            }

            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the Low_Limit plus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal < currentObject->Low_Limit) &&
                ((currentObject->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //    ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                //        EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!currentObject->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_LOW_LIMIT;
                }
                else {
                    currentObject->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->Time_Delay;
            break;

        case EVENT_STATE_HIGH_LIMIT:

            // todo 0 - review
            //if (>FaultStatus) {
            //    currentObject->Event_State = EVENT_STATE_FAULT;
            //    break;
            //}

            /* Once exceeded, the Present_Value must fall below the High_Limit minus
               the Deadband before a TO-NORMAL event is generated under these conditions:
               (a) the Present_Value must fall below the High_Limit minus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal < currentObject->High_Limit - currentObject->Deadband)
                && ((currentObject->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //   ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                //        EVENT_ENABLE_TO_NORMAL)) {
                if (!currentObject->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                }
                else {
                    currentObject->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->Time_Delay;
            break;

        case EVENT_STATE_LOW_LIMIT:

            // todo 0 - review
            //if (>FaultStatus) {
            //    currentObject->Event_State = EVENT_STATE_FAULT;
            //    break;
            //}

            /* Once the Present_Value has fallen below the Low_Limit,
               the Present_Value must exceed the Low_Limit plus the Deadband
               before a TO-NORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the Low_Limit plus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal > currentObject->Low_Limit + currentObject->Deadband)
                && ((currentObject->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //    ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                //        EVENT_ENABLE_TO_NORMAL)) {
                if (!currentObject->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                }
                else {
                    currentObject->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->Time_Delay;
            break;

            // todo 0 - review
            //if (>FaultStatus) {
            //    currentObject->Event_State = EVENT_STATE_FAULT;
            //    break;
            //}

        default:
            panic();
            return; /* shouldn't happen */
        }       /* switch (FromState) */

        ToState = currentObject->Event_State;

        if (FromState != ToState) {
            /* Event_State has changed.
               Need to fill only the basic parameters of this type of event.
               Other parameters will be filled in common function. */

               // todo1 soooooo much in common between AI and AV here...  consolidate?

            switch (ToState) {
            case EVENT_STATE_HIGH_LIMIT:
                Device_getCurrentDateTime(&currentObject->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL]);
                if ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) == EVENT_ENABLE_TO_OFFNORMAL) {
                    ExceededLimit = currentObject->High_Limit;
                    characterstring_init_ansi(&msgText, "Goes to high limit");
                    SendNotify = true;
                }
                break;

            case EVENT_STATE_LOW_LIMIT:
                Device_getCurrentDateTime(&currentObject->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL]);
                if ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) == EVENT_ENABLE_TO_OFFNORMAL) {
                    ExceededLimit = currentObject->Low_Limit;
                    characterstring_init_ansi(&msgText, "Goes to low limit");
                    SendNotify = true;
                }
                break;

            case EVENT_STATE_NORMAL:
                Device_getCurrentDateTime(&currentObject->Event_Time_Stamps[TRANSITION_TO_NORMAL]);
                if ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) == EVENT_ENABLE_TO_NORMAL) {
                    if (FromState == EVENT_STATE_HIGH_LIMIT) {
                        ExceededLimit = currentObject->High_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from high limit");
                    }
                    else {
                        ExceededLimit = currentObject->Low_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from low limit");
                    }
                    break;

            case EVENT_STATE_FAULT:
                Device_getCurrentDateTime(&currentObject->Event_Time_Stamps[TRANSITION_TO_FAULT]);
                if ((currentObject->Event_Enable & EVENT_ENABLE_TO_FAULT) == EVENT_ENABLE_TO_FAULT) {
                    ExceededLimit = 999.9f ;
                    characterstring_init_ansi(&msgText, "New Fault Condition");
                    SendNotify = true;
                }
                break;

            default:
                panic();
                ExceededLimit = 0;
                break;
            }   /* switch (ToState) */

                dbTraffic(DBD_ALL, DB_BTC_ERROR, "Event_State for (%s,%d) goes from %s to %s.",
                    bactext_object_type_name(OBJECT_ANALOG_INPUT), object_instance,
                    bactext_event_state_name(FromState),
                    bactext_event_state_name(ToState));

                /* Notify Type */
                event_data.notifyType = currentObject->Notify_Type;

                /* Send EventNotification. */
                // see logic above SendNotify = true;
            }
        }


        if (SendNotify) {
            /* Event Object Identifier */
            event_data.eventObjectIdentifier.type = OBJECT_ANALOG_INPUT;
            event_data.eventObjectIdentifier.instance = object_instance;

            // This logic has been moved to the proper place above. todo 3 remove comments when reviewed and tested.
            ///* Time Stamp */
            //event_data.timeStamp.tag = TIME_STAMP_DATETIME;
            //Device_getCurrentDateTime(&event_data.timeStamp.value.dateTime);

            //if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            //    /* fill Event_Time_Stamps */
            //    switch (ToState) {
            //    case EVENT_STATE_HIGH_LIMIT:
            //    case EVENT_STATE_LOW_LIMIT:
            //            CurrentAI->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL] =
            //            event_data.timeStamp.value.dateTime;
            //        break;

            //    case EVENT_STATE_FAULT:
            //            CurrentAI->Event_Time_Stamps[TRANSITION_TO_FAULT] =
            //            event_data.timeStamp.value.dateTime;
            //        break;

            //    case EVENT_STATE_NORMAL:
            //            CurrentAI->Event_Time_Stamps[TRANSITION_TO_NORMAL] =
            //            event_data.timeStamp.value.dateTime;
            //        break;

            //    case EVENT_STATE_OFFNORMAL:
            //        panic();
            //        break;
            //    }
            //}

        /* Notification Class */
        event_data.notificationClass = currentObject->Notification_Class;

        // todo2  - there is no check of the event_enable T,T,T flags! we are sending events even if they are not enabled!

        /* Event Type */
        event_data.eventType = EVENT_OUT_OF_RANGE;

        /* Message Text */
        event_data.messageText = &msgText;

        /* Notify Type */
        /* filled before */

        /* From State */
        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            event_data.fromState = FromState;
        }

        /* To State */
        event_data.toState = currentObject->Event_State;

        /* Event Values */
        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            /* Value that exceeded a limit. */
            event_data.notificationParams.outOfRange.exceedingValue =
                PresentVal;
            /* Status_Flags of the referenced object. */
            bitstring_init(&event_data.notificationParams.outOfRange.
                statusFlags);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_IN_ALARM,
                currentObject->Event_State ? true : false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_OUT_OF_SERVICE,
                currentObject->Out_Of_Service);
            /* Deadband used for limit checking. */
            event_data.notificationParams.outOfRange.deadband =
                currentObject->Deadband;
            /* Limit that was exceeded. */
            event_data.notificationParams.outOfRange.exceededLimit =
                ExceededLimit;
        }

        /* add data from notification class */
        Notification_Class_common_reporting_function(&event_data);

        /* Ack required */
        if ((event_data.notifyType != NOTIFY_ACK_NOTIFICATION) &&
            (event_data.ackRequired == true)) {
            switch (event_data.toState) {
            case EVENT_STATE_OFFNORMAL:
            case EVENT_STATE_HIGH_LIMIT:
            case EVENT_STATE_LOW_LIMIT:
                currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                    bIsAcked = false;
                currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                    Time_Stamp = event_data.timeStamp.value.dateTime;
                break;

            case EVENT_STATE_FAULT:                                         // todo3 - we don't have a fault condition. Review.
                currentObject->Acked_Transitions[TRANSITION_TO_FAULT].
                    bIsAcked = false;
                currentObject->Acked_Transitions[TRANSITION_TO_FAULT].
                    Time_Stamp = event_data.timeStamp.value.dateTime;
                break;

            case EVENT_STATE_NORMAL:
                currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].
                    bIsAcked = false;
                currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].
                    Time_Stamp = event_data.timeStamp.value.dateTime;
                break;

            default:
                panic();
                // note: we are not supporting FAULT state (requires reliability flag. not sure if we want that at this time.) todo2
                }
            }
        }
    }
}
#endif /* defined(INTRINSIC_REPORTING) */


#if (INTRINSIC_REPORTING_B == 1)
int Analog_Input_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data)
{
    bool IsNotAckedTransitions;
    bool IsActiveEvent;
    int i;

    ANALOG_INPUT_DESCR *currentObject = (ANALOG_INPUT_DESCR *)Generic_Index_To_Object(&AI_Descriptor_List, index);
    if (currentObject == NULL) return -1;

    /* check index */
        /* Event_State not equal to NORMAL */
    IsActiveEvent = (currentObject->Event_State != EVENT_STATE_NORMAL);

    /* Acked_Transitions property, which has at least one of the bits
       (TO-OFFNORMAL, TO-FAULT, TONORMAL) set to FALSE. */
    IsNotAckedTransitions =
        (currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked ==
            false) | (currentObject->Acked_Transitions[TRANSITION_TO_FAULT].
                bIsAcked ==
                false) | (currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].
                    bIsAcked == false);

    if ((IsActiveEvent) || (IsNotAckedTransitions)) {
        /* Object Identifier */
        getevent_data->objectIdentifier.type = OBJECT_ANALOG_INPUT;
        getevent_data->objectIdentifier.instance = currentObject->common.objectInstance;

        /* Event State */
        getevent_data->eventState = currentObject->Event_State;
        /* Acknowledged Transitions */
        bitstring_init(&getevent_data->acknowledgedTransitions);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_OFFNORMAL,
            currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_FAULT,
            currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_NORMAL,
            currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);
        /* Event Time Stamps */
        for (i = 0; i < 3; i++) {
            getevent_data->eventTimeStamps[i].tag = TIME_STAMP_DATETIME;
            getevent_data->eventTimeStamps[i].value.dateTime =
                currentObject->Event_Time_Stamps[i];
        }
        /* Notify Type */
        getevent_data->notifyType = currentObject->Notify_Type;
        /* Event Enable */
        bitstring_init(&getevent_data->eventEnable);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_OFFNORMAL,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_FAULT,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_NORMAL,
            (currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        /* Event Priorities */
        Notification_Class_Get_Priorities(
            currentObject->Notification_Class,
            getevent_data->eventPriorities);

        return 1;       /* active event */
    }
    else {
        return 0;    /* no active event at this index */
    }
}


/* return +1 if alarm was acknowledged
   return -1 if any error occurred
   return -2 abort */
int Analog_Input_Alarm_Ack(
    BACNET_ALARM_ACK_DATA *alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE *error_code)
{
    //     unsigned int object_index;

    ANALOG_INPUT_DESCR *currentObject = Analog_Input_Instance_To_Object(alarmack_data->eventObjectIdentifier.instance);
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

    // preset default for rest
    *error_class = ERROR_CLASS_SERVICES;

    if (alarmack_data->eventStateAcked != currentObject->Event_State) {
        *error_code = ERROR_CODE_INVALID_EVENT_STATE;
        return -1;
    }

    switch (alarmack_data->eventStateAcked) {
    case EVENT_STATE_OFFNORMAL:
    case EVENT_STATE_HIGH_LIMIT:
    case EVENT_STATE_LOW_LIMIT:
        // 2017.05.03 - EKH: I took this check out since it is legitimate to have multiple
        // acks for any given alarm.. in fact BTL failed us for responding with the original ERROR_CODE_INVALID_EVENT_STATE.
        // I did replace this with the check "alarmack_data->eventStateAcked != currentObject->Event_State" for anticipated
        // other upcoming BTL tests
        //if (currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
        //    bIsAcked == false) {
        if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }
        if (datetime_compare(&currentObject->
            Acked_Transitions[TRANSITION_TO_OFFNORMAL].Time_Stamp,
            &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }

        /* FIXME: Send ack notification */
        currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked = true;
        //}
        //else {
            //*error_code = ERROR_CODE_INVALID_EVENT_STATE;
            //return -1;
        //}
        break;

    case EVENT_STATE_FAULT:
        //if (currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked ==
        //    false) {
        if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }
        if (datetime_compare(&currentObject->
            Acked_Transitions[TRANSITION_TO_FAULT].Time_Stamp,
            &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }

        /* FIXME: Send ack notification */
        currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked =
            true;
        //}
        //else {
        //    *error_code = ERROR_CODE_INVALID_EVENT_STATE;
        //    return -1;
        //}
        break;

    case EVENT_STATE_NORMAL:
        //if (currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked ==
        //    false) {
        if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }
        if (datetime_compare(&currentObject->
            Acked_Transitions[TRANSITION_TO_NORMAL].Time_Stamp,
            &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
            *error_code = ERROR_CODE_INVALID_TIME_STAMP;
            return -1;
        }

        /* FIXME: Send ack notification */
        currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked =
            true;
        //}
        //else {
        //    *error_code = ERROR_CODE_INVALID_EVENT_STATE;
        //    return -1;
        //}
        break;

    default:
        return -2;
    }
    currentObject->Ack_notify_data.bSendAckNotify = true;
    currentObject->Ack_notify_data.EventState = alarmack_data->eventStateAcked;

    return 1;
}


#if 0
// deprecated since rev 13
// BTC-todo 2 - does not mean we dont have to test it!
int Analog_Input_Alarm_Summary(
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data)
{

    if (index >= analogInputs.size()) return -1;  // So there is no exception below
    AnalogInputObject *currentObject = static_cast<AnalogInputObject *> (analogInputs.at(index));
    if (currentObject == NULL) return -1;

    /* check index */
        /* Event_State is not equal to NORMAL  and
           Notify_Type property value is ALARM */
    if ((currentObject->Event_State != EVENT_STATE_NORMAL) &&
        (currentObject->Notify_Type == NOTIFY_ALARM)) {
        /* Object Identifier */
        getalarm_data->objectIdentifier.type = OBJECT_ANALOG_INPUT;
        getalarm_data->objectIdentifier.instance = currentObject->instance; // currentInstance;

        /* Alarm State */
        getalarm_data->alarmState = currentObject->Event_State;
        /* Acknowledged Transitions */
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
#endif // 0

#endif /* (INTRINSIC_REPORTING_B == 1) */


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    bool bResult;

    /*
     * start out assuming success and only set up error
     * response if validation fails.
     */
    bResult = true;
    if (pValue->tag != ucExpectedTag) {
        bResult = false;
        *pErrorClass = ERROR_CLASS_PROPERTY;
        *pErrorCode = ERROR_CODE_INVALID_DATA_TYPE;
    }

    return (bResult);
}

void testAnalogInput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint32_t decoded_instance = 0;
    uint16_t decoded_type = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Analog_Input_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_ANALOG_INPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Analog_Input_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);
}

#ifdef TEST_ANALOG_INPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Analog Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAnalogInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ANALOG_INPUT */
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_ANALOG_INPUT == 1 )
