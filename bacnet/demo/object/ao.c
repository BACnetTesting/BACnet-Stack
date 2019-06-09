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
****************************************************************************************/

#include "configProj.h"     /* the custom stuff */

#if (BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )

#include "handlers.h"
#include "ao.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "device.h"
#endif

#include "handlers.h"
#include "emm.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "device.h"
#endif

#if 0
#if 0
#ifndef MAX_ANALOG_OUTPUTS
#define MAX_ANALOG_OUTPUTS 4
#endif

/* we choose to have a NULL level in our system represented by */
/* a particular value.  When the priorities are not in use, they */
/* will be relinquished (i.e. set to the NULL level). */
#define AO_LEVEL_NULL 255
/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
/* Here is our Priority Array.  They are supposed to be Real, but */
/* we don't have that kind of memory, so we will use a single byte */
/* and load a Real for returning the value when asked. */
static uint8_t Analog_Output_Level[MAX_ANALOG_OUTPUTS][BACNET_MAX_PRIORITY];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
static bool Out_Of_Service[MAX_ANALOG_OUTPUTS];

/* we need to have our arrays initialized before answering any calls */
static bool Analog_Output_Initialized = false;
#endif
#endif // 0


LLIST_HDR AO_Descriptor_List;

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
    PROP_HIGH_LIMIT,
    PROP_LOW_LIMIT,
    PROP_DEADBAND,
    PROP_LIMIT_ENABLE,
#endif

#if (BACNET_SVC_COV_B_AO == 1)
    // not fully supported yet, todo...
    PROP_COV_INCREMENT,
#endif

    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Analog_Output_Property_Lists(
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
void Analog_Output_Init(
    void)
{
    ll_Init(&AO_Descriptor_List, 100);

#if (INTRINSIC_REPORTING_B == 1)

    /* Set handler for GetEventInformation function */
    handler_get_event_information_set(
        OBJECT_ANALOG_OUTPUT,
        Analog_Output_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
    handler_alarm_ack_set(
        OBJECT_ANALOG_OUTPUT,
        Analog_Output_Alarm_Ack);

    /* Set handler for GetAlarmSummary Service */
    // Deprecated since Rev 13
    /* Set handler for GetAlarmSummary Service */
    //handler_get_alarm_summary_set(OBJECT_ANALOG_OUTPUT,
    //    Analog_Output_Alarm_Summary);

#endif
}


bool Analog_Output_Create(
    const uint32_t instance,
    const char *name)
{
    ANALOG_OUTPUT_DESCR *currentObject = (ANALOG_OUTPUT_DESCR *)emm_scalloc('a', sizeof(ANALOG_OUTPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&AO_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    // Note that our structure is 0 initialized by calloc, so no zeroing operations are _required_.
    // Some are here just for clarity.
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;
    currentObject->Units = UNITS_PERCENT;

#if ( BACNET_SVC_COV_B_AO == 1 )
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


bool Analog_Output_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&AO_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Analog_Output_Count(
    void)
{
    return AO_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Analog_Output_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&AO_Descriptor_List, index);
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


static inline bool isOutOfService(ANALOG_OUTPUT_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static inline bool isInAlarm(ANALOG_OUTPUT_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
}


static BACNET_RELIABILITY Reliability_Get(
    ANALOG_OUTPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadowReliability ;
    }

    // In this reference stack, nobody ever actually sets reliability, we expect the Application to do so (along with PV).
    return currentObject->Reliability;
}


static bool isInFault(
    ANALOG_OUTPUT_DESCR *currentObject)
{
    return (Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


ANALOG_OUTPUT_DESCR *Analog_Output_Instance_To_Object(
    uint32_t object_instance)
{
    return (ANALOG_OUTPUT_DESCR *)Generic_Instance_To_Object(&AO_Descriptor_List, object_instance);
}


static float Analog_Output_Present_Value_Get(
    ANALOG_OUTPUT_DESCR *currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->shadow_Present_Value;
    }
    return currentObject->Present_Value;
}


#if ( BACNET_SVC_COV_B == 1 )
static void Analog_Output_COV_Detect_PV_Change(
    ANALOG_OUTPUT_DESCR *currentObject,
    float value)
{
    if (fabs(value - currentObject->Prior_Value) >= currentObject->COV_Increment) {
        currentObject->Changed = true;
        currentObject->Prior_Value = value;
    }
}
#endif


// Signature used by device mux
/* note: the object name must be unique within this device */
bool Analog_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name(&AO_Descriptor_List, object_instance, object_name);
}


// todo 3 move to the generic module
// Starting with the Relinquish Default, process through the Priority Array, overwriting with highest priority
// and place the result in either the physical output register, or the shadow value, depending on the Out_of_Service flag.
// If COV enabled, detect a COV change.
static void SweepToPresentValue(
    ANALOG_OUTPUT_DESCR *currentObject)
{
    float tvalue = currentObject->Relinquish_Default;
    int i;
    for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
        if (currentObject->priorityFlags & BIT(i)) {
            tvalue = currentObject->priorityArray[i];
            break;
        }
    }

#if ( BACNET_SVC_COV_B == 1 )
    Analog_Output_COV_Detect_PV_Change(currentObject, tvalue);
#endif

    if (currentObject->Out_Of_Service) {
        currentObject->shadow_Present_Value = tvalue;
    }
    else {
        currentObject->Present_Value = tvalue;
    }
}


float Analog_Output_Present_Value_from_Instance(
    const uint32_t instance)
{
    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(instance);
    if (currentObject == NULL) {
        panic();
        return 0.0f;
    }
    if (currentObject->Out_Of_Service) {
        return currentObject->shadow_Present_Value ;
    }
    else {
        return currentObject->Present_Value ;
    }
}


//unsigned Analog_Output_Present_Value_Priority(
//    uint32_t object_instance)
//{
//    unsigned index = 0; /* instance to index conversion */
//    unsigned i = 0;     /* loop counter */
//    unsigned priority = 0;      /* return value */
//
//    //index = Analog_Output_Instance_To_Index(object_instance);
//    //if (index < MAX_ANALOG_OUTPUTS) {
//    //    for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
//    //        if (Analog_Output_Level[index][i] != AO_LEVEL_NULL) {
//    //            priority = i + 1;
//    //            break;
//    //        }
//    //    }
//    //}
//    panic();
//
//    return priority;
//}


#if ( BACNET_SVC_COV_B == 1 )

// This function reports to the BACnet stack if there has/has not been a change to the Present Value or status flags
// therefore, unfortunately, we have to derive the pointers from the object_instance each time.
bool Analog_Output_Change_Of_Value(
    const uint32_t object_instance)
{

    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(object_instance);
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
        float tempPV = Analog_Output_Present_Value_Get(currentObject);
        // todo 0 Binary_Value_COV_Detect_PV_Change(currentObject,  currentObject->shadow_Present_Value, tempPV);
        Analog_Output_COV_Detect_PV_Change(currentObject, tempPV);
    }

    return currentObject->Changed;
}


/**
 * For a given object instance-number, loads the value_list with the COV data.
 *
 * @param  object_instance - object-instance number of the object
 * @param  value_list - list of COV data
 *
 * @return  true if the value list is encoded
 */
bool Analog_Output_Encode_Value_List(
    const uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(object_instance);
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
            Analog_Output_Present_Value_Get(currentObject);
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


bool Analog_Output_Present_Value_Set(
    ANALOG_OUTPUT_DESCR *currentObject,
    float value,
    unsigned priority)
{
    bool status = false;

    if (priority && (priority <= BACNET_MAX_PRIORITY) &&
        (priority != 6 /* reserved */) &&
        (value >= 0.0) && (value <= 100.0)) {
        currentObject->priorityArray[priority - 1] = value;
        currentObject->priorityFlags |= BIT(priority - 1);
        status = true;
        // todo, if low priority written, confirm that the COV not triggered
        SweepToPresentValue(currentObject);
    }
    return status;
}


bool Analog_Output_Relinquish_Default_Set(
    ANALOG_OUTPUT_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    if (!WP_ValidateTagType(wp_data, value, BACNET_APPLICATION_TAG_REAL)) return false;
    // if range properties (etc) exist check valid ranges here
    if (!WP_ValidateRangeReal(wp_data, value, 0.0, 100.0)) return false;
    currentObject->Relinquish_Default = value->type.Real;
    SweepToPresentValue(currentObject);
    return true;
}

static bool Analog_Output_Present_Value_Relinquish(
    ANALOG_OUTPUT_DESCR *currentObject,
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


#if ( BACNET_SVC_COV_B_AO == 1 )

void Analog_Output_Change_Of_Value_Clear(
    const uint32_t instance)
{
    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(instance);
    if (currentObject == NULL) {
        panic();
        return;
    }
    currentObject->Changed = false;
}



void Analog_Output_COV_Increment_Set(
    ANALOG_OUTPUT_DESCR *currentObject,
    float value)
{
    currentObject->COV_Increment = value;
    if (fabs(Analog_Output_Present_Value_Get(currentObject) - currentObject->Prior_Value) > currentObject->COV_Increment) {
        currentObject->Changed = true;
    }
}
#endif // ( BACNET_SVC_COV_B_AO == 1 )


static void Analog_Output_Out_Of_Service_Set(
    ANALOG_OUTPUT_DESCR *currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B_AO == 1 )
    currentObject->Changed = true;
#endif

    currentObject->Out_Of_Service = oos_flag;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Analog_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;   /* return value */
    // BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;

#if (INTRINSIC_REPORTING_B2 == 1)
    unsigned int i = 0;
    int len = 0;
#endif

#ifdef BAC_DEBUG
    if ((rpdata == NULL) ||
        (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }
#endif

    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_ANALOG_OUTPUT,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
        Analog_Output_Object_Name(
            rpdata->object_instance,
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
                OBJECT_ANALOG_OUTPUT);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_real(
                apdu,
                Analog_Output_Present_Value_Get(currentObject));
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

    case PROP_UNITS:
        apdu_len =
            encode_application_enumerated(&apdu[0], currentObject->Units);
        break;

#if ( BACNET_SVC_COV_B_AO == 1 )
    case PROP_COV_INCREMENT:
        apdu_len = encode_application_real(&apdu[0],
            currentObject->COV_Increment);
        break;
#endif // ( BACNET_SVC_COV_B_AO == 1 )

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

#endif  // ( INTRINSIC_REPORTING_B == 1 )

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
                    len = encode_application_real(
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
                    apdu_len = encode_application_real(&apdu[apdu_len], currentObject->priorityArray[rpdata->array_index - 1]);
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
        apdu_len = encode_application_real(
            &apdu[0], 
            currentObject->Relinquish_Default);
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
        (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
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
bool Analog_Output_Write_Property(
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
        (wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        if (value.tag == BACNET_APPLICATION_TAG_REAL) {
            /* Command priority 6 is reserved for use by Minimum On/Off
               algorithm and may not be used for other purposes in any
               object. */
            status =
                Analog_Output_Present_Value_Set(
                    currentObject,
                    value.type.Real,
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
                    Analog_Output_Present_Value_Relinquish(currentObject, wp_data->priority);
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
            Analog_Output_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_UNITS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->Units = (BACNET_ENGINEERING_UNITS)value.type.Enumerated;
        }
        break;

#if ( BACNET_SVC_COV_B_AO == 1 )
    case PROP_COV_INCREMENT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Real >= 0.0) {
                Analog_Output_COV_Increment_Set(
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
#endif // ( BACNET_SVC_COV_B_AO == 1 )

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

    case PROP_RELINQUISH_DEFAULT:
        status = Analog_Output_Relinquish_Default_Set( 
            currentObject,
            wp_data,
            &value);
        break;

    // todo 0 why did BTC not detect missing property list here?
    case PROP_DESCRIPTION:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:
    case PROP_RELIABILITY:  // todo 0 - we need to be able to write reliability for regression testing.
#if (INTRINSIC_REPORTING_B == 1)
    case PROP_ACKED_TRANSITIONS:
    case PROP_EVENT_TIME_STAMPS:
#endif
    case PROP_PRIORITY_ARRAY:
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

void Analog_Output_Intrinsic_Reporting(
    uint32_t object_instance)
{
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    ANALOG_OUTPUT_DESCR *currentObject;
    BACNET_EVENT_STATE FromState;
    BACNET_EVENT_STATE ToState;
    float ExceededLimit;
    float PresentVal;
    bool SendNotify = false;


    currentObject = Analog_Output_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        panic();
        return;
    }

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
                    EVENT_HIGH_LIMIT_ENABLE) &&
                    ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
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
                    EVENT_LOW_LIMIT_ENABLE) &&
                    ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
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
                    EVENT_HIGH_LIMIT_ENABLE) &&
                    ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
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
                    EVENT_LOW_LIMIT_ENABLE) &&
                    ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
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
                ExceededLimit = currentObject->High_Limit;
                characterstring_init_ansi(&msgText, "Goes to high limit");
                break;

            case EVENT_STATE_LOW_LIMIT:
                ExceededLimit = currentObject->Low_Limit;
                characterstring_init_ansi(&msgText, "Goes to low limit");
                break;

            case EVENT_STATE_NORMAL:
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

            default:
                panic();
                ExceededLimit = 0;
                break;
            }   /* switch (ToState) */

            /* Notify Type */
            event_data.notifyType = currentObject->Notify_Type;

            /* Send EventNotification. */
            SendNotify = true;
        }
    }


    if (SendNotify) {
        /* Event Object Identifier */
        event_data.eventObjectIdentifier.type = OBJECT_ANALOG_OUTPUT;
        event_data.eventObjectIdentifier.instance = object_instance;

        /* Time Stamp */
        event_data.timeStamp.tag = TIME_STAMP_DATETIME;
        Device_getCurrentDateTime(&event_data.timeStamp.value.dateTime);

        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            /* fill Event_Time_Stamps */
            switch (ToState) {
            case EVENT_STATE_HIGH_LIMIT:
            case EVENT_STATE_LOW_LIMIT:
                currentObject->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL] =
                    event_data.timeStamp.value.dateTime;
                break;

            case EVENT_STATE_FAULT:
                currentObject->Event_Time_Stamps[TRANSITION_TO_FAULT] =
                    event_data.timeStamp.value.dateTime;
                break;

            case EVENT_STATE_NORMAL:
                currentObject->Event_Time_Stamps[TRANSITION_TO_NORMAL] =
                    event_data.timeStamp.value.dateTime;
                break;

                //todonext9 - what about tooffnormal??
            }
        }

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
#endif /* defined(INTRINSIC_REPORTING) */


#if (INTRINSIC_REPORTING_B == 1)
int Analog_Output_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data)
{
    bool IsNotAckedTransitions;
    bool IsActiveEvent;
    int i;

    ANALOG_OUTPUT_DESCR *currentObject = (ANALOG_OUTPUT_DESCR *)Generic_Index_To_Object(&AO_Descriptor_List, index);
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
int Analog_Output_Alarm_Ack(
    BACNET_ALARM_ACK_DATA *alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE *error_code)
{
    //     unsigned int object_index;

    ANALOG_OUTPUT_DESCR *currentObject = Analog_Output_Instance_To_Object(alarmack_data->eventObjectIdentifier.instance);
    if (currentObject == NULL) {
        *error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return -1;
    }

    switch (alarmack_data->eventStateAcked) {
    case EVENT_STATE_OFFNORMAL:
    case EVENT_STATE_HIGH_LIMIT:
    case EVENT_STATE_LOW_LIMIT:
        if (currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked == false) {
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
        }
        else {
            *error_code = ERROR_CODE_INVALID_EVENT_STATE;
            return -1;
        }
        break;

    case EVENT_STATE_FAULT:
        if (currentObject->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked ==
            false) {
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
        }
        else {
            *error_code = ERROR_CODE_INVALID_EVENT_STATE;
            return -1;
        }
        break;

    case EVENT_STATE_NORMAL:
        if (currentObject->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked ==
            false) {
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
        }
        else {
            *error_code = ERROR_CODE_INVALID_EVENT_STATE;
            return -1;
        }
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
int Analog_Output_Alarm_Summary(
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data)
{

    if (index >= analogValues.size()) return -1;  // So there is no exception below
    AnalogValueObject *currentObject = static_cast<AnalogValueObject *> (analogValues.at(index));
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


void testAnalogOutput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint32_t decoded_instance = 0;
    uint16_t decoded_type = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Analog_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_ANALOG_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Analog_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);
}

#ifdef TEST_ANALOG_OUTPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Analog Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAnalogOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ANALOG_INPUT */
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )
