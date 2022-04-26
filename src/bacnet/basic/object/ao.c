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
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#include "configProj.h"     /* the project/application specific settings */

#if (BACNET_USE_OBJECT_ANALOG_OUTPUT == 1 )

// #include "bacnet/datalink/bip.h"
#include "bacnet/basic/services.h"
#include "ao.h"
#include "eLib/util/eLibDebug.h"
#include "eLib/util/llist.h"
#include "eLib/util/emm.h"
#include "device.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/bits.h"
#include "bacnet/basic/service/h_wp.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "bactext.h"
#include "eventCommon.h"
#endif

extern bool (*bits_ServerSide_Write_Output_Fn) (const BACNET_OBJECT * currentObject, const int priority, const BACNET_APPLICATION_DATA_VALUE* value);

/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_STATUS_FLAGS,
    PROP_OUT_OF_SERVICE,
    PROP_PRESENT_VALUE,
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

#if (BACNET_SVC_COV_B == 1)
    PROP_COV_INCREMENT,
#endif

    MAX_BACNET_PROPERTY_ID
};


static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};


void Analog_Output_Property_Lists(
    const BACNET_PROPERTY_ID** pRequired,
    const BACNET_PROPERTY_ID** pOptional,
    const BACNET_PROPERTY_ID** pProprietary)
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
// This only gets called once on startup, and has to initialize for ALL virtual devices.
void Analog_Output_Init(
    void)
{

#if (INTRINSIC_REPORTING_B == 1)
    /* Set handler for GetEventInformation function */
    handler_get_event_information_set(
        OBJECT_ANALOG_OUTPUT,
        Analog_Output_Event_Information);

    /* Set handler for AcknowledgeAlarm function */
    handler_alarm_ack_set(
        OBJECT_ANALOG_OUTPUT,
        Analog_Output_Alarm_Ack);

#if ( BACNET_PROTOCOL_REVISION < 13 )
    /* Set handler for GetAlarmSummary Service */
    // Deprecated since Rev 13
    /* Set handler for GetAlarmSummary Service */
    handler_get_alarm_summary_set(
        OBJECT_ANALOG_OUTPUT,
        Analog_Output_Alarm_Summary);
#endif // ( BACNET_PROTOCOL_REVISION < 13 )

#endif
}

BACNET_OBJECT* Analog_Output_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance,
    const char* name,
    const BACNET_ENGINEERING_UNITS units)
{
    if (Analog_Output_Valid_Instance(pDev, instance))
    {
        dbMessage(DBD_Config, DB_ERROR, "Attempting to create a duplicate Analog Output AO:%05u", instance);
        return NULL;
    }

    BACNET_CHARACTER_STRING tname;
    characterstring_init_ansi(&tname, name);
    BACNET_OBJECT_TYPE foundType;
    uint32_t foundInstance;
    if (Device_Valid_Object_Name(pDev, &tname, &foundType, &foundInstance))
    {
        dbMessage(DBD_Config, DB_ERROR, "Attempting to create a duplicate name of Analog Output AO:%s", name);
        return NULL;
    }

    ANALOG_OUTPUT_DESCR* currentObject = (ANALOG_OUTPUT_DESCR*)emm_scalloc('a', sizeof(ANALOG_OUTPUT_DESCR));
    if (currentObject == NULL) {
        panic();
        return NULL;
    }

    currentObject->priorityFlags = 0;

    for (int i = 0; i < BACNET_MAX_PRIORITY; i++) {
        currentObject->priorityArray[i] = 0.0;
    }

    if (!ll_Enqueue(&pDev->AO_Descriptor_List, currentObject)) {
        panic();
        return NULL;
    }

    BACnetAnalogObject_Init(&currentObject->analogCommon, OBJECT_ANALOG_OUTPUT, instance, name, units);

    return (BACNET_OBJECT*)currentObject;
}


bool Analog_Output_Valid_Instance(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    if (Generic_Instance_To_Object(&pDev->AO_Descriptor_List, object_instance) != NULL) return true;
    return false;
}


unsigned Analog_Output_Count(
    DEVICE_OBJECT_DATA* pDev)
{
    return pDev->AO_Descriptor_List.count;
}


// This is used by the Device Object Function Table. Must have this signature.
uint32_t Analog_Output_Index_To_Instance(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index)
{
    // todo 3 - can we catch out-of-bounds?
    return Generic_Index_To_Instance(&pDev->AO_Descriptor_List, index);
}


static inline bool isOutOfService(
    ANALOG_OUTPUT_DESCR* currentObject)
{
    return currentObject->analogCommon.common.Out_Of_Service;
}


static inline bool isInAlarm(
    ANALOG_OUTPUT_DESCR* currentObject)
{
    return currentObject->analogCommon.common.Event_State != EVENT_STATE_NORMAL;
}


static BACNET_RELIABILITY Analog_Output_Reliability_Get(
    ANALOG_OUTPUT_DESCR* currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->analogCommon.common.shadowReliability;
    }

    return Common_Reliability_Get(&currentObject->analogCommon.common);
}


static bool isInFault(
    ANALOG_OUTPUT_DESCR* currentObject)
{
    return (Analog_Output_Reliability_Get(currentObject) != RELIABILITY_NO_FAULT_DETECTED);
}


ANALOG_OUTPUT_DESCR* Analog_Output_Instance_To_Object(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    return (ANALOG_OUTPUT_DESCR*)Generic_Instance_To_Object(&pDev->AO_Descriptor_List, object_instance);
}


float Analog_Output_Present_Value_Get(
    ANALOG_OUTPUT_DESCR* currentObject)
{
    if (isOutOfService(currentObject)) {
        return currentObject->analogCommon.shadow_Present_Value;
    }
    return currentObject->analogCommon.Present_Value;
}


#if ( BACNET_SVC_COV_B == 1 )
static void Analog_Output_COV_Detect_PV_Change(
    ANALOG_OUTPUT_DESCR* currentObject,
    float newValue)
{
    if (fabs(newValue - currentObject->analogCommon.Prior_Value) >= currentObject->analogCommon.COV_Increment) {

        dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, "COV: Detected change [%s] [%4.2f] -> [%4.2f], Increment [%4.2f]",
            BACnet_ObjectID_ToString(currentObject->analogCommon.common.objectType, currentObject->analogCommon.common.objectInstance),
            currentObject->analogCommon.Prior_Value,
            newValue,
            currentObject->analogCommon.COV_Increment
        );

        currentObject->analogCommon.Changed = true;
        currentObject->analogCommon.Prior_Value = newValue;
    }
}
#endif


// Starting with the Relinquish Default, process through the Priority Array, overwriting with highest priority
// and place the result in either the physical output register, or the shadow value, depending on the Out_of_Service flag.
// If COV enabled, detect a COV change.

static void SweepToPresentValue(
    ANALOG_OUTPUT_DESCR* currentObject)
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

    if (currentObject->analogCommon.common.Out_Of_Service) {
        currentObject->analogCommon.shadow_Present_Value = tvalue;
    }
    else {
        currentObject->analogCommon.Present_Value = tvalue;
    }
}


static bool Analog_Output_Present_Value_Relinquish(
    ANALOG_OUTPUT_DESCR * currentObject,
    unsigned priority)
{
    bool status = true;
    BACNET_APPLICATION_DATA_VALUE value;
    value.tag = BACNET_APPLICATION_TAG_NULL;
    if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != BACNET_MIN_ON_OFF_PRIORITY /* reserved */)) {
        currentObject->priorityFlags &= ~(1 << (priority - 1));
        SweepToPresentValue(currentObject);
    } else {
        status = false;
    }
    return status;
}


static bool Analog_Output_Present_Value_Set(
    ANALOG_OUTPUT_DESCR* currentObject,
    BACNET_WRITE_PROPERTY_DATA* wp_data,
    BACNET_APPLICATION_DATA_VALUE* value)
{
    bool status = false;

    if (wp_data->priority == BACNET_MIN_ON_OFF_PRIORITY || wp_data->priority == BACNET_NO_PRIORITY || wp_data->priority > BACNET_MAX_PRIORITY)
    {
        /* Command priority 6 is reserved for use by Minimum On/Off
           algorithm and may not be used for other purposes in any
           object. */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    } else {

        // todo 0 - this mechanism is broken. It does not handle relinquishes propertly. Update ASAP!

        if (bits_ServerSide_Write_Output_Fn(&currentObject->analogCommon.common, wp_data->priority, value)) {
            // Not a virtual object, must be an actual BACnet device.
            if (value->tag == BACNET_APPLICATION_TAG_REAL) {
                currentObject->priorityArray[wp_data->priority - 1] = value->type.Real;
                currentObject->priorityFlags |= BIT(wp_data->priority - 1);
                status = true;
                // todo, if low priority written, confirm that the COV not triggered. BTL pre-testing should uncover
                SweepToPresentValue(currentObject);
            } else {
                status =
                    WPValidateArgType(value, BACNET_APPLICATION_TAG_NULL,
                        &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    status =
                        Analog_Output_Present_Value_Relinquish(currentObject, wp_data->priority);
                    if (!status) {
                        // todo 1 - Should be write_access_denied? BTL pre-testing will uncover
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
                // COV detection done in 'sweep' function
            }
        } else {
            // Virtual device.  If it's an actual BACnet device, we'll be back here for the physical object soon
            // to update that instance, then we'll update the mapped device from the published fb on the physical device.
            status = true;
        }
    }
    return status;
}


// Signature used by device mux
/* note: the object name must be unique within this device - todo 3 - run a check? */
bool Analog_Output_Object_Name(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING* object_name)
{
    return Generic_Instance_To_Object_Name(
        &pDev->AO_Descriptor_List,
        object_instance,
        object_name);
}


float Analog_Output_Present_Value_from_Instance(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance)
{
    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(pDev, instance);
    if (currentObject == NULL) {
        panic();
        return 0.0;
    }
    if (currentObject->analogCommon.common.Out_Of_Service) {
        return currentObject->analogCommon.shadow_Present_Value;
    }
    else {
        return currentObject->analogCommon.Present_Value;
    }
}


bool Analog_Output_Relinquish_Default_Set(
    ANALOG_OUTPUT_DESCR* currentObject,
    BACNET_WRITE_PROPERTY_DATA* wp_data,
    BACNET_APPLICATION_DATA_VALUE* value)
{
    bool status = false;
    if (WP_ValidateTagType(wp_data, value, BACNET_APPLICATION_TAG_REAL)) {
        currentObject->Relinquish_Default = value->type.Real;
        SweepToPresentValue(currentObject);
        status = true;
    }
    return status;
}


bool Analog_Output_Reliability_Set(
    ANALOG_OUTPUT_DESCR* currentObject,
    BACNET_WRITE_PROPERTY_DATA* wp_data,
    BACNET_APPLICATION_DATA_VALUE* value)
{
    if (!WP_ValidateEnumTypeAndRange(wp_data, value, RELIABILITY_PROPRIETARY_MAX)) return false;

    bool status = WPValidateArgType(value, BACNET_APPLICATION_TAG_ENUMERATED,
        &wp_data->error_class, &wp_data->error_code);
    if (!status) return status;

    if (currentObject->analogCommon.common.Out_Of_Service) {
        currentObject->analogCommon.common.shadowReliability = (BACNET_RELIABILITY)value->type.Enumerated;
        return true;
    }
    wp_data->error_class = ERROR_CLASS_PROPERTY;
    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
    return false;
}


#if ( BACNET_SVC_COV_B == 1 )
bool Analog_Output_Change_Of_Value(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t object_instance)
{
    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(
        pDev,
        object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    // COVs are generated by a) application logic  b) BACnet operation  c) Client side reads
    // and apply to changes in a) Present_Value, b) Status Flags (all of OOS, Flt, AL, Ovr) !

    return currentObject->analogCommon.Changed ;

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
bool Analog_Output_Encode_Value_List(
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE* value_list)
{
    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(
        pDev,
        object_instance);
    if (currentObject == NULL) {
        panic();
        return false;
    }

    BACNET_APPLICATION_DATA_VALUE tvalue;

    tvalue.tag = BACNET_APPLICATION_TAG_REAL;
    tvalue.type.Real = Analog_Output_Present_Value_Get(currentObject);

    return Common_Encode_Value_List(
        &currentObject->analogCommon.common,
        isInAlarm(currentObject),
        isInFault(currentObject),
        isOutOfService(currentObject),
        &tvalue,
        value_list);
}


void Analog_Output_Change_Of_Value_Clear(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t instance)
{
    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(pDev, instance);
    if (currentObject == NULL) {
        panic();
        return;
    }
    currentObject->analogCommon.Changed = false;
}


void Analog_Output_COV_Increment_Set(
    ANALOG_OUTPUT_DESCR* currentObject,
    float value)
{
    currentObject->analogCommon.COV_Increment = value;
    if (fabs(Analog_Output_Present_Value_Get(currentObject) - currentObject->analogCommon.Prior_Value) > currentObject->analogCommon.COV_Increment) {
        currentObject->analogCommon.Changed = true;
    }
}
#endif // ( BACNET_SVC_COV_B == 1 )


static void Analog_Output_Out_Of_Service_Set(
    ANALOG_OUTPUT_DESCR* currentObject,
    const bool oos_flag)
{
    // Is there actually a change? If not, then we don't have to do anything.
    if (currentObject->analogCommon.common.Out_Of_Service == oos_flag) return;

#if ( BACNET_SVC_COV_B == 1 )
    currentObject->analogCommon.Changed = true;
#endif

    currentObject->analogCommon.common.Out_Of_Service = oos_flag;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
int Analog_Output_Read_Property(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA* rpdata)
{
    int apdu_len;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    uint8_t* apdu;

#if (INTRINSIC_REPORTING_B == 1)
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

    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(pDev, rpdata->object_instance);
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
                BACnetObject_Description_Get(&currentObject->analogCommon.common));
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
#if (INTRINSIC_REPORTING_B == 1)
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->analogCommon.common.Event_State);
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
                Analog_Output_Reliability_Get(currentObject));
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len =
            encode_application_boolean(
                apdu,
                isOutOfService(currentObject));
        break;

    case PROP_UNITS:
        apdu_len =
            encode_application_enumerated(&apdu[0], currentObject->analogCommon.Units);
        break;

#if ( BACNET_SVC_COV_B == 1 )
    case PROP_COV_INCREMENT:
        apdu_len = encode_application_real(&apdu[0],
            currentObject->analogCommon.COV_Increment);
        break;
#endif // ( BACNET_SVC_COV_B == 1 )

#if (INTRINSIC_REPORTING_B == 1)
    case PROP_TIME_DELAY:
        apdu_len =
            encode_application_unsigned(&apdu[0], currentObject->analogCommon.common.eventCommon.Time_Delay);
        break;

    case PROP_NOTIFICATION_CLASS:
        apdu_len =
            encode_application_unsigned(&apdu[0],
                currentObject->analogCommon.common.eventCommon.Notification_Class);
        break;

    case PROP_HIGH_LIMIT:
        apdu_len =
            encode_application_real(&apdu[0], currentObject->analogCommon.eventCommonAnalog.High_Limit);
        break;

    case PROP_LOW_LIMIT:
        apdu_len = encode_application_real(&apdu[0], currentObject->analogCommon.eventCommonAnalog.Low_Limit);
        break;

    case PROP_DEADBAND:
        apdu_len = encode_application_real(&apdu[0], currentObject->analogCommon.eventCommonAnalog.Deadband);
        break;

    case PROP_LIMIT_ENABLE:
        apdu_len = encode_application_bitstring2(
            apdu,
            (currentObject->analogCommon.eventCommonAnalog.Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ? true : false,
            (currentObject->analogCommon.eventCommonAnalog.Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ? true : false);
        break;

    case PROP_EVENT_ENABLE:
        apdu_len = encode_application_bitstring3(
            apdu,
            (currentObject->analogCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false,
            (currentObject->analogCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false,
            (currentObject->analogCommon.common.eventCommon.Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        break;

    case PROP_ACKED_TRANSITIONS:
        apdu_len = encode_application_bitstring3(
            apdu,
            currentObject->analogCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_OFFNORMAL].bIsAcked,
            currentObject->analogCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked,
            currentObject->analogCommon.common.eventCommon.Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);
        break;

    case PROP_NOTIFY_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->analogCommon.common.eventCommon.Notify_Type ? NOTIFY_EVENT : NOTIFY_ALARM);
        break;

    case PROP_EVENT_TIME_STAMPS:
        apdu_len = encode_application_event_time_stamps(apdu, rpdata, currentObject->analogCommon.common.eventCommon.Event_Time_Stamps);
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
            apdu_len = 0;
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
                    apdu_len = encode_application_real(&apdu[0], currentObject->priorityArray[rpdata->array_index - 1]);
                }
                else {
                    apdu_len = encode_application_null(&apdu[0]);
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
    DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA* wp_data)
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

    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(pDev, wp_data->object_instance);
    if (currentObject == NULL) {
        panic(); // this should never happen, doesnt the kernel pre-check existence?
        wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        status = Analog_Output_Present_Value_Set(
            currentObject,
            wp_data,
            &value);
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

    case PROP_RELIABILITY:
        status =
            Common_Reliability_Set(
                &currentObject->analogCommon.common,
                wp_data,
                &value);
        break;

    case PROP_UNITS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->analogCommon.Units = (BACNET_ENGINEERING_UNITS)value.type.Enumerated;
        }
        break;

#if ( BACNET_SVC_COV_B == 1 )
    case PROP_COV_INCREMENT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (value.type.Real >= 0.0) {
                Analog_Output_COV_Increment_Set(
                    currentObject,
                    value.type.Real);
                pDev->covPersist = true;
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
            currentObject->analogCommon.common.eventCommon.Time_Delay = value.type.Unsigned_Int;
            currentObject->analogCommon.common.eventCommon.Remaining_Time_Delay = currentObject->analogCommon.common.eventCommon.Time_Delay;
        }
        break;

    case PROP_NOTIFICATION_CLASS:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->analogCommon.common.eventCommon.Notification_Class = value.type.Unsigned_Int;
        }
        break;

    case PROP_HIGH_LIMIT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->analogCommon.eventCommonAnalog.High_Limit = value.type.Real;
        }
        break;

    case PROP_LOW_LIMIT:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            currentObject->analogCommon.eventCommonAnalog.Low_Limit = value.type.Real;
        }
        break;

    case PROP_DEADBAND:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            currentObject->analogCommon.eventCommonAnalog.Deadband = value.type.Real;
        }
        break;

    case PROP_LIMIT_ENABLE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
            if (value.type.Bit_String.bits_used == 2) {
                currentObject->analogCommon.eventCommonAnalog.Limit_Enable = value.type.Bit_String.value[0];
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
                currentObject->analogCommon.common.eventCommon.Event_Enable = value.type.Bit_String.value[0];
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
                currentObject->analogCommon.common.eventCommon.Notify_Type = (BACNET_NOTIFY_TYPE)value.type.Enumerated;
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
    DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
    ANALOG_OUTPUT_DESCR* currentObject;

    currentObject = Analog_Output_Instance_To_Object(pDev, object_instance);
    if (currentObject == NULL) {
        panic();
        return;
    }

    Common_Analog_Intrinsic_Reporting(
        &currentObject->analogCommon.common,
        &currentObject->analogCommon.common.eventCommon,
        &currentObject->analogCommon.eventCommonAnalog);
}


int Analog_Output_Event_Information(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA* getevent_data)
{
    ANALOG_OUTPUT_DESCR* currentObject = (ANALOG_OUTPUT_DESCR*)Generic_Index_To_Object(&pDev->AO_Descriptor_List, index);
    if (currentObject == NULL) return -1;

	return Common_Event_Information(pDev, &currentObject->analogCommon.common, OBJECT_ANALOG_OUTPUT, getevent_data );
}



/* return +1 if alarm was acknowledged
   return -1 if any error occurred
   return -2 abort */
int Analog_Output_Alarm_Ack(
    DEVICE_OBJECT_DATA* pDev,
    BACNET_ALARM_ACK_DATA* alarmack_data,
    BACNET_ERROR_CLASS* error_class,
    BACNET_ERROR_CODE* error_code)
{

    ANALOG_OUTPUT_DESCR* currentObject = Analog_Output_Instance_To_Object(pDev, alarmack_data->eventObjectIdentifier.instance);
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

    return Common_Alarm_Ack(&currentObject->analogCommon.common, alarmack_data, error_class, error_code);
}


#if ( BACNET_PROTOCOL_REVISION < 13 )
// deprecated since rev 13
int Analog_Output_Alarm_Summary(
    DEVICE_OBJECT_DATA* pDev,
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA* getalarm_data)
{
    if (index >= analogOutputs.size()) return -1;  // So there is no exception below
    AnalogOutputObject* currentObject = static_cast<AnalogOutputObject*> (analogOutputs.at(index));
    if (currentObject == NULL) return -1;

    /* check index */
        /* Event_State is not equal to NORMAL  and
           Notify_Type property value is ALARM */
    if ((currentObject->Event_State != EVENT_STATE_NORMAL) &&
        (currentObject->Notify_Type == NOTIFY_ALARM)) {
        /* Object Identifier */
        getalarm_data->objectIdentifier.type = OBJECT_ANALOG_OUTPUT;
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
    Test* pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint16_t len_value = 0;
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


int main(
    void)
{
    Test* pTest;
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
#endif /* TEST_XXX_XXX */

#endif // if (BACNET_USE_OBJECT_XXX_XXX == 1 )
