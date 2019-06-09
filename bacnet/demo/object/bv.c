/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"     /* the custom stuff */

#if (BACNET_USE_OBJECT_BINARY_VALUE == 1 )

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "debug.h"
#include "wp.h"
#include "rp.h"
#include "handlers.h"

#include "bv.h"
#include "bitsDebug.h"
#include "llist.h"
#include "emm.h"
#include "BACnetObject.h"

LLIST_HDR BV_Descriptor_List;

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    PROP_RELIABILITY,

#if ( BACNET_SVC_COV_B == 1 )
    PROP_COV_INCREMENT,
#endif

#if (INTRINSIC_REPORTING_B == 1)
    PROP_TIME_DELAY,
    PROP_NOTIFICATION_CLASS,
    //todo2 - what about event_detection_enable?
    PROP_HIGH_LIMIT,
    PROP_LOW_LIMIT,
    PROP_DEADBAND,
    PROP_LIMIT_ENABLE,
    PROP_EVENT_ENABLE,
    PROP_ACKED_TRANSITIONS,
    PROP_NOTIFY_TYPE,
    PROP_EVENT_TIME_STAMPS,
#if ( BACNET_PROTOCOL_REVISION >= 13 )
    PROP_EVENT_DETECTION_ENABLE,
#endif
#endif
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


bool Binary_Value_Create(
    const uint32_t instance,
    const char *name)
{
    BINARY_VALUE_DESCR *currentObject = (BINARY_VALUE_DESCR *)emm_scalloc('a', sizeof(BINARY_VALUE_DESCR));
    if (currentObject == NULL) {
        panic();
        return false;
    }
    if (!ll_Enqueue(&BV_Descriptor_List, currentObject)) {
        panic();
        return false;
    }

    Generic_Object_Init(&currentObject->common, instance, name);

    currentObject->Present_Value = BINARY_ACTIVE;
    currentObject->Out_Of_Service = false;
    currentObject->Reliability = RELIABILITY_NO_FAULT_DETECTED;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->Prior_Value = BINARY_ACTIVE;
    currentObject->Changed = false;
#endif

#if (INTRINSIC_REPORTING_B == 1)
    currentObject->Event_State = EVENT_STATE_NORMAL;
    /* notification class not connected */
    currentObject->Notification_Class = BACNET_MAX_INSTANCE;
    /* initialize Event time stamps using wildcards
    and set Acked_transitions */
    for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
        datetime_wildcard_set(&currentObject->Event_Time_Stamps[j]);
        currentObject->Acked_Transitions[j].bIsAcked = true;
    }
#endif

    return true;
}


void Binary_Value_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    if (pRequired) {
        // Do NOT be tempted to use property_list_required() - that is for supporting epics.c, and perhaps other Client operations, only
        *pRequired = Properties_Required;
    }
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// Gets called once for each device

void Binary_Value_Init(
    void)
{
    // unsigned i;
#if (INTRINSIC_REPORTING_B2 == 1)
    unsigned j;
#endif

    ll_Init(&BV_Descriptor_List, 100);

#if (INTRINSIC_REPORTING_B2 == 1)

    /* Set handler for GetEventInformation function */
    handler_get_event_information_set(OBJECT_ANALOG_INPUT,
        Analog_Input_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
    handler_alarm_ack_set(OBJECT_ANALOG_INPUT,
        Analog_Input_Alarm_Ack);

    /* Set handler for GetAlarmSummary Service */
	// Deprecated since Rev 13   
	/* Set handler for GetAlarmSummary Service */
    //handler_get_alarm_summary_set(OBJECT_ANALOG_INPUT,
    //    Analog_Input_Alarm_Summary);

#endif

}


bool Binary_Value_Valid_Instance(
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&BV_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Binary_Value_Count(
    void)
{
    return BV_Descriptor_List.count;
}


uint32_t Binary_Value_Index_To_Instance(
    unsigned index)
{
    return Generic_Index_To_Instance(&BV_Descriptor_List, index);
}


// This is a shitty function to have - use Generic_Instance_To_Object() for those functions that wanted this.. *
// int Binary_Value_Instance_To_Index(
//    uint32_t object_instance)
//{
//    unsigned index = MAX_ANALOG_INPUTS;
//
//    if (object_instance < MAX_ANALOG_INPUTS)
//        index = object_instance;
//
//    return index;
//}


static inline bool IsOutOfService(BINARY_VALUE_DESCR *currentObject)
{
    return currentObject->Out_Of_Service;
}


static inline bool IsInAlarm(BINARY_VALUE_DESCR *currentObject)
{
    return currentObject->Event_State != EVENT_STATE_NORMAL;
}


BINARY_VALUE_DESCR *Binary_Value_Instance_To_Object(
    uint32_t object_instance)
{
    return (BINARY_VALUE_DESCR *)Generic_Instance_To_Object(&BV_Descriptor_List, object_instance);
}


BACNET_BINARY_PV Binary_Value_Present_Value(
    BINARY_VALUE_DESCR *currentObject)
{
    return currentObject->Present_Value;
}


void Binary_Value_Update(
    const uint32_t instance,
    const bool value)
{
    BINARY_VALUE_DESCR *bacnetObject = Binary_Value_Instance_To_Object(instance);
    if (bacnetObject == NULL) {
        panic();
        return;
    }
    bacnetObject->Present_Value = (value) ? BINARY_ACTIVE : BINARY_INACTIVE;
}


#if ( BACNET_SVC_COV_B == 1 )
static void Binary_Value_COV_Detect_PV_Change(
    BINARY_VALUE_DESCR *currentObject,
    bool value)
{
    if (value != currentObject->Prior_Value) {
        currentObject->Prior_Value = value;
        // must be careful to never un-set changed here
        currentObject->Changed = true;
    }
}
#endif


// todo 3 move to the generic module
static void SweepToPresentValue(BINARY_VALUE_DESCR *currentObject)
{
    BACNET_BINARY_PV tvalue = RELINQUISH_DEFAULT_BINARY;
    int i;
    for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
        if (currentObject->priorityArrayFlags[i]) {
            tvalue = currentObject->priorityArrayValues[i];
            break;
        }
    }
    currentObject->Present_Value = tvalue;
}


static bool Binary_Value_Present_Value_Set(
    BINARY_VALUE_DESCR *currentObject,
    BACNET_WRITE_PROPERTY_DATA *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value)
{
    uint8_t priority = wp_data->priority;

    /*  BTC todo - 19.2.3 When a write to a commandable property occurs at any priority, the specified value or relinquish (NULL) is always written to
        the appropriate slot in the priority table, regardless of any minimum on or off times.
        Actually: NOT ALLOWED to write NULL to 6
        */

        // BTC todo - Karg allowed a write of NULL to present value, priority 6. 
        // btc todo - try write priority 17
    if (priority == 6 || priority > BACNET_MAX_PRIORITY) {
        /* Command priority 6 is reserved for use by Minimum On/Off
        algorithm and may not be used for other purposes in any
        object. */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        return false;
    }

    if (value->tag == BACNET_APPLICATION_TAG_ENUMERATED) {
        // new as of 2016.09.20 
        // todo 4 - use bit arrays to save space?
        currentObject->priorityArrayValues[priority - 1] = (BACNET_BINARY_PV)value->type.Enumerated;
        currentObject->priorityArrayFlags[priority - 1] = true;
        SweepToPresentValue(currentObject);
        if (value->type.Enumerated > 1) {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            return false ;
        }
    }
    else if (value->tag == BACNET_APPLICATION_TAG_NULL) {
        // This is the relinquish case (If the value is a NULL )
        // We are writing a NULL to the priority array...
        currentObject->priorityArrayFlags[priority - 1] = false;
        SweepToPresentValue(currentObject);
        // todo1 transfer to application layer here
    }
    else {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
        return false;
    }
    return true;
}


bool Binary_Value_Present_Value_Relinquish(
    uint32_t object_instance,
    unsigned priority)
{

    return true;
}


static inline bool Binary_Value_Reliability_Set(
    BINARY_VALUE_DESCR *currentObject,
    uint32_t reliability)
{
    if (currentObject->Out_Of_Service) {
        // the BACnet way
        currentObject->reliabilityShadowValue = (BACNET_RELIABILITY)reliability;
        return true;
    }
    // writes, when not OOS, are not allowed
    return false;
}


static BACNET_RELIABILITY Binary_Value_Reliability(
    BINARY_VALUE_DESCR *currentObject)
{
    if (IsOutOfService(currentObject)) {
        // the BACnet way
        return currentObject->reliabilityShadowValue;
    }

    // so in this embodiment, nobody gets to set reliability, we expect the Application to do so (along with PV)
    return currentObject->Reliability;
}


static bool isInFault(
    BINARY_VALUE_DESCR *currentObject)
{
    return (Binary_Value_Reliability(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


bool Binary_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    return Generic_Instance_To_Object_Name(&BV_Descriptor_List, object_instance, object_name);
}


#if ( BACNET_SVC_COV_B == 1 )

// This function reports to the BACnet stack if there has/has not been a change to the Present Value or status flags
// therefore, unfortunately, we have to derive the pointers from the object_instance each time.
bool Binary_Value_Change_Of_Value(
    const uint32_t object_instance)
{
    BINARY_VALUE_DESCR *currentObject = Binary_Value_Instance_To_Object(object_instance);
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
    bool currentOOS = currentObject->Out_Of_Service ;
    if ((bool)currentObject->prior_OOS != currentOOS) {
        currentObject->prior_OOS = currentOOS;
        currentObject->Changed = true;
    }

    // todo 2 - consider Ov and Flt flags here too!

    if (!currentOOS) {
        // Has the Application code changed the PV behind our back?
        BACNET_BINARY_PV tempPV = Binary_Value_Present_Value(currentObject);
        Binary_Value_COV_Detect_PV_Change(currentObject, tempPV);
    }

    return currentObject->Changed;
}


void Binary_Value_Change_Of_Value_Clear(
    uint32_t object_instance)
{
    BINARY_VALUE_DESCR *currentObject = Binary_Value_Instance_To_Object(object_instance);
    if (currentObject == NULL) return;
    currentObject->Changed = false;
}


/* returns true if value has changed */
bool Binary_Value_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    BINARY_VALUE_DESCR *currentObject = Binary_Value_Instance_To_Object(object_instance);
    if (currentObject == NULL) {
        return false;
    }

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
        value_list->value.type.Enumerated =
            Binary_Value_Present_Value(currentObject);
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
            STATUS_FLAG_IN_ALARM, IsInAlarm(currentObject));
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_FAULT, isInFault(currentObject));
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OVERRIDDEN, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OUT_OF_SERVICE, currentObject->Out_Of_Service );
        value_list->value.next = NULL;
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
    }
    status = Binary_Value_Change_Of_Value(object_instance);

    return status;
}
#endif // ( BACNET_SVC_COV_B == 1 )


static void Binary_Value_Out_Of_Service_Set(
    BINARY_VALUE_DESCR *currentObject,
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
int Binary_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BINARY_VALUE_DESCR *currentObject;
    uint8_t *apdu;

#if (INTRINSIC_REPORTING_B == 1)
    unsigned int i = 0;
    int len = 0;
#endif

    const BACNET_PROPERTY_ID *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }

    currentObject = Binary_Value_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0], OBJECT_BINARY_VALUE,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
        Binary_Value_Object_Name(rpdata->object_instance, &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0], OBJECT_BINARY_VALUE);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                Binary_Value_Present_Value(currentObject));
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

#if ( BACNET_PROTOCOL_REVISION >= 14 )
#if ( INTRINSIC_REPORTING_B == 1 )
    case PROP_EVENT_DETECTION_ENABLE:
        apdu_len =
            encode_application_enumerated(&apdu[0], true);
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
            encode_application_boolean(&apdu[0],
                currentObject->Out_Of_Service);
        break;
#if 0
    case PROP_PRIORITY_ARRAY:
        /* Array element zero is the number of elements in the array */
        if (rpdata->array_index == 0)
            apdu_len =
            encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
        /* if no index was specified, then try to encode the entire list */
        /* into one packet. */
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            object_index =
                Binary_Value_Instance_To_Index(rpdata->object_instance);
            for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                /* FIXME: check if we have room before adding it to APDU */
                if (Binary_Value_Level[object_index][i] == BINARY_NULL)
                    len = encode_application_null(&apdu[apdu_len]);
                else {
                    present_value = Binary_Value_Level[object_index][i];
                    len =
                        encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                }
                /* add it if we have room */
                if ((apdu_len + len) < MAX_APDU)
                    apdu_len += len;
                else {
                    rpdata->error_code =
                        ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                    apdu_len = BACNET_STATUS_ABORT;
                    break;
                }
            }
        }
        else {
            object_index =
                Binary_Value_Instance_To_Index(rpdata->object_instance);
            if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                if (Binary_Value_Level[object_index][rpdata->array_index]
                    == BINARY_NULL)
                    apdu_len = encode_application_null(&apdu[apdu_len]);
                else {
                    present_value = Binary_Value_Level[object_index]
                        [rpdata->array_index];
                    apdu_len =
                        encode_application_enumerated(&apdu[apdu_len],
                            present_value);
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
        present_value = RELINQUISH_DEFAULT;
        apdu_len = encode_application_enumerated(&apdu[0], present_value);
        break;
#endif

    case PROP_PROPERTY_LIST:
        Binary_Value_Property_Lists(&pRequired, &pOptional, &pProprietary);
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
bool Binary_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len;
    BACNET_APPLICATION_DATA_VALUE value;
    BINARY_VALUE_DESCR *currentObject;

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

    currentObject = Binary_Value_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        // todo1 panic();
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        status = Binary_Value_Present_Value_Set(currentObject, wp_data, &value);
        break;

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Binary_Value_Out_Of_Service_Set(
                currentObject,
                value.type.Boolean);
        }
        break;

    case PROP_ACTIVE_TEXT:
    case PROP_DESCRIPTION:
    case PROP_INACTIVE_TEXT:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:
    case PROP_RELIABILITY:
    case PROP_PRIORITY_ARRAY:
    case PROP_RELINQUISH_DEFAULT:
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

void testBinary_Value(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint32_t decoded_instance = 0;
    uint16_t decoded_type = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Binary_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Value_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

}

#ifdef TEST_BINARY_VALUE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary_Value", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinary_Value);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_VALUE */
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_BINARY_VALUE == 1 )
