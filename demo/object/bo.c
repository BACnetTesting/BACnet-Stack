/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include "config.h"     /* the custom stuff */

#if (BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )

#include "rp.h"
#include "wp.h"
#include "bo.h"
#include "handlers.h"
#include "BACnetObjectBinary.h"
#include "bitsDebug.h"
#include "emm.h"
#include "datalink.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "device.h"
#endif

static LLIST_HDR BO_Descriptor_List;

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_POLARITY,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
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
    PROP_FEEDBACK_VALUE,
#endif
    PROP_ACTIVE_TEXT,
    PROP_INACTIVE_TEXT,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Binary_Output_Property_Lists(
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
void Binary_Output_Init(
    void)
{
    ll_Init(&BO_Descriptor_List, 100);

#if (INTRINSIC_REPORTING_B == 1)

    /* Set handler for GetEventInformation function */
//    handler_get_event_information_set(
//        OBJECT_ANALOG_INPUT,
//        Binary_Output_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
//    handler_alarm_ack_set(
//        OBJECT_ANALOG_INPUT,
//        Binary_Output_Alarm_Ack);

#endif
}


bool Binary_Output_Create(
    const uint32_t instance,
    const char *name)
{
    BINARY_OUTPUT_DESCR *currentObject = (BINARY_OUTPUT_DESCR *)emm_scalloc('a', sizeof(BINARY_OUTPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&BO_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    currentObject->Out_Of_Service = false;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = false;
    currentObject->Prior_Value = BINARY_INACTIVE;
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

    return true;
}


bool Binary_Output_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&BO_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Binary_Output_Count(
    void)
{
    return BO_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Binary_Output_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&BO_Descriptor_List, index);
}


static inline bool isOutOfService(BINARY_OUTPUT_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static int isInAlarm(BINARY_OUTPUT_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
}


static inline BACNET_RELIABILITY Binary_Output_Reliability_Get(
    BINARY_OUTPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadowReliability ;
    }

    // In this reference stack, nobody ever actually sets reliability, we expect the Application to do so (along with PV).
    return currentObject->Reliability;
}


static bool isInFault(
    BINARY_OUTPUT_DESCR *currentObject)
{
    return (Binary_Output_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


BINARY_OUTPUT_DESCR *Binary_Output_Instance_To_Object(
    uint32_t object_instance)
{
    return (BINARY_OUTPUT_DESCR *)Generic_Instance_To_Object(&BO_Descriptor_List, object_instance);
}


BACNET_BINARY_PV Binary_Output_Present_Value_Get(
    BINARY_OUTPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadow_Present_Value;
    }
    return currentObject->Present_Value;
}


#if 0
// Signature used by device mux
bool Binary_Output_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    panic();
    //index = Binary_Output_Instance_To_Index(object_instance);
    //if (index < MAX_BINARY_OUTPUTS) {
    //    value = Out_Of_Service[index];
    //}

    return value;
}
#endif


#if ( BACNET_SVC_COV_B == 1 )
static void Binary_Output_COV_Detect_PV_Change(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_BINARY_PV value)
{
    if ( value != currentObject->Prior_Value ) {
        currentObject->Changed = true;
        currentObject->Prior_Value = value;
    }
}
#endif



// Signature used by device mux
/* note: the object name must be unique within this device */
bool Binary_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name(&BO_Descriptor_List, object_instance, object_name);
}


// todo 3 move to the generic module
// Starting with the Relinquish Default, process through the Priority Array, overwriting with highest priority
// and place the result in either the physical output register, or the shadow value, depending on the Out_of_Service flag.
// If COV enabled, detect a COV change.
static void SweepToPresentValue(BINARY_OUTPUT_DESCR *currentObject)
{
    BACNET_BINARY_PV tvalue = currentObject->Relinquish_Default;
    int i;
    for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
        if (currentObject->priorityFlags & BIT(i)) {
            tvalue = currentObject->priorityArray[i];
            break;
        }
    }

#if ( BACNET_SVC_COV_B == 1 )
    Binary_Output_COV_Detect_PV_Change(currentObject, tvalue);
#endif

    if (currentObject->Out_Of_Service) {
        currentObject->shadow_Present_Value = tvalue;
    }
    else {
        currentObject->Present_Value = tvalue;
    }
}


bool Binary_Output_Present_Value_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_BINARY_PV value,
    unsigned priority)
{
    bool status = false;

    if (priority && (priority <= BACNET_MAX_PRIORITY) &&
        (priority != 6 /* reserved */) &&
        (value <= MAX_BINARY_PV)) {
        currentObject->priorityArray[priority - 1] = value;
        currentObject->priorityFlags |= BIT(priority - 1);
        status = true;
        SweepToPresentValue(currentObject);
    }
    return status;
}


static inline bool Binary_Output_Reliability_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    if (!WP_ValidateEnumTypeAndRange(wp_data, value, RELIABILITY_PROPRIETARY_MAX)) return false;

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
bool Binary_Output_Change_Of_Value(
    const uint32_t object_instance)
{

    BINARY_OUTPUT_DESCR *currentObject = Binary_Output_Instance_To_Object(object_instance);
    if (currentObject == NULL) return false;

    // COVs are generated by a) application logic  b) BACnet operation
    // and apply to changes in a) Present_Value, b) Status Flags (all of OOS, Flt, AL, Ovr) !

    // and anticipating that someone else may trigger ->changed, and also accommodating the fact we are in a loop....

    // I would like to believe that we can do some of this homework event-based, and although
    // there is already some logic done elsewhere (when BACnet makes changes, Intrinsic Alarms)
    // we can hope one day the App layer doing the evaluation for e.g. PV, Fault etc.
    if (currentObject->Changed) return true;


    // Also, there may be MANY subscriptions watching this object, so be sure that the 
    // ->changed flag persists once set, until explicitly cleared by xxx_Change_Of_Value_Clear()

    // did the application layer change OOS behind our back?
    bool currentOOS = currentObject->Out_Of_Service;
    if ((bool)currentObject->prior_OOS != currentOOS) {
        currentObject->prior_OOS = currentOOS;
        currentObject->Changed = true;
    }

    // todo 2 - consider Ov and Flt flags here too!

    if (!currentOOS) {
        // Has the Application code changed the PV behind our back?
        BACNET_BINARY_PV tempPV = Binary_Output_Present_Value_Get(currentObject);
        // todo 0 Binary_Value_COV_Detect_PV_Change(currentObject,  currentObject->shadow_Present_Value, tempPV);
        Binary_Output_COV_Detect_PV_Change(currentObject, tempPV);
    }

    return currentObject->Changed;
}


void Binary_Output_Change_Of_Value_Clear(
    const uint32_t instance)
{
    BINARY_OUTPUT_DESCR *currentObject = Binary_Output_Instance_To_Object(instance);
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
bool Binary_Output_Encode_Value_List(
    const uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    BINARY_OUTPUT_DESCR *currentObject = Binary_Output_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
        value_list->value.type.Enumerated =
            Binary_Output_Present_Value_Get(currentObject);
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

#endif // ( BACNET_SVC_COV_B == 1 )


bool Binary_Output_Relinquish_Default_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    if (!WP_ValidateTagType(wp_data, value, BACNET_APPLICATION_TAG_ENUMERATED)) return false;
    // if range properties (etc) exist check valid ranges here
    if (!WP_ValidateEnumTypeAndRange(wp_data, value, 1)) return false;
    currentObject->Relinquish_Default = (BACNET_BINARY_PV) value->type.Enumerated;
    SweepToPresentValue(currentObject);
    return true;
}

static bool Binary_Output_Present_Value_Relinquish(
    BINARY_OUTPUT_DESCR *currentObject,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    if (priority && (priority <= BACNET_MAX_PRIORITY) &&
        (priority != 6 /* reserved */)) {
        currentObject->priorityFlags &= ~(1 << (priority - 1));
        SweepToPresentValue(currentObject);
        status = true;
    }

    return status;
}


static void Binary_Output_Out_Of_Service_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Changed = true;
#endif

    currentObject->Out_Of_Service = oos_flag;
}


bool Binary_Output_Polarity_Set(
    BINARY_OUTPUT_DESCR *currentObject,
    BACNET_POLARITY polarity)
{
    bool status = false;
    unsigned index = 0;

    currentObject->polarity = polarity ;
    // do a sweep? COV?? Alarm evaluation?? todo 0
    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Binary_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    // BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;
    BACNET_BINARY_PV present_value = BINARY_INACTIVE;
    // no, it is now live BACNET_POLARITY polarity = POLARITY_NORMAL;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;

#ifdef BAC_DEBUG
    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }
#endif

    BINARY_OUTPUT_DESCR *currentObject = Binary_Output_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_BINARY_OUTPUT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
        Binary_Output_Object_Name(
            rpdata->object_instance,
            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(
                apdu,
                OBJECT_BINARY_OUTPUT);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_enumerated(
                apdu,
                Binary_Output_Present_Value_Get(currentObject));
        break;

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_FEEDBACK_VALUE:
        apdu_len =
            encode_application_enumerated(
                apdu,
                currentObject->feedbackValue );
        break;
#endif

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
                Binary_Output_Reliability_Get(currentObject));
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
        apdu_len = encode_application_event_time_stamps ( apdu, rpdata, currentObject->Event_Time_Stamps ) ;
        break;
#endif

    case PROP_POLARITY:
        apdu_len = encode_application_enumerated(&apdu[0], currentObject->polarity );
        break;

    case PROP_PRIORITY_ARRAY:
        /* Array element zero is the number of elements in the array */
        if (rpdata->array_index == 0)
            apdu_len = encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
        /* if no index was specified, then try to encode the entire list */
        /* into one packet. */
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            unsigned i, len;
            for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                /* FIXME: check if we have room before adding it to APDU */

                if (currentObject->priorityFlags & (1 << i))
                    len = encode_application_enumerated(
                        &apdu[apdu_len],
                        currentObject->priorityArray[i]);
                else {
                    len = encode_application_null(&apdu[apdu_len]);
                }

                /* add it if we have room */
                if ((apdu_len + len) < MAX_APDU) {
                    apdu_len += len;
                }
                else {
                    rpdata->error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                    apdu_len = BACNET_STATUS_ABORT;
                    break;
                }
            }
        }
        else {
            if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                if (currentObject->priorityFlags & (1 << (rpdata->array_index - 1))) {
                    apdu_len = encode_application_enumerated(&apdu[apdu_len], currentObject->priorityArray[rpdata->array_index - 1]);
                }
                else {
                    apdu_len = encode_application_null(&apdu[apdu_len]);
                }
            }
            else {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
        }
        break;

    case PROP_RELINQUISH_DEFAULT:
        apdu_len = encode_application_enumerated(
            &apdu[0], 
            currentObject->Relinquish_Default);
        break;

    case PROP_ACTIVE_TEXT:
        characterstring_init_ansi(&char_string, "Active");
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_INACTIVE_TEXT:
        characterstring_init_ansi(&char_string, "Inactive");
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_PROPERTY_LIST:
        apdu_len = property_list_encode(
            rpdata,
            Properties_Required,
            Properties_Optional,
            Properties_Proprietary);
        break;

#if (INTRINSIC_REPORTING_B == 0)
    case PROP_FEEDBACK_VALUE:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
#endif

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) &&
        (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        // btc todo1 - BTC missed this? (rpdata->object_property != PROP_PROPERTY_LIST) &&
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
bool Binary_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
    unsigned int priority = 0;
    BACNET_BINARY_PV level = BINARY_NULL;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    BINARY_OUTPUT_DESCR *currentObject;

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
        (wp_data->object_property != PROP_PRIORITY_ARRAY) &&
#if (INTRINSIC_REPORTING_B == 1)
        (wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    currentObject = Binary_Output_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
            /* Command priority 6 is reserved for use by Minimum On/Off
               algorithm and may not be used for other purposes in any
               object. */
            status =
                Binary_Output_Present_Value_Set(
                    currentObject,
                    (BACNET_BINARY_PV) value.type.Enumerated,
                    wp_data->priority);
            if (wp_data->priority == 6) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
            else if (!status) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        else {
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
            if (status) {
                status =
                    Binary_Output_Present_Value_Relinquish(currentObject, wp_data->priority);
                if (!status) {
                    // todo 0 - what is this bogus (?) error condition?
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
        }
        break;

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Binary_Output_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_RELIABILITY:
        status =
            Binary_Output_Reliability_Set(
                currentObject,
                wp_data,
                &value);
        break;

    case PROP_POLARITY:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Enumerated < MAX_POLARITY) {
                Binary_Output_Polarity_Set(currentObject,
                    (BACNET_POLARITY) value.type.Enumerated);
            } else {
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;

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

    case PROP_RELINQUISH_DEFAULT:
        status = Binary_Output_Relinquish_Default_Set( 
            currentObject,
            wp_data,
            &value);
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
    case PROP_ACTIVE_TEXT:
    case PROP_INACTIVE_TEXT:
    case PROP_PRIORITY_ARRAY:
    case PROP_FEEDBACK_VALUE:
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

void testBinaryOutput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint32_t decoded_instance = 0;
    uint16_t decoded_type = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Binary_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);
}

#ifdef TEST_BINARY_OUTPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinaryOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_INPUT */
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_BINARY_OUTPUT == 1 )
