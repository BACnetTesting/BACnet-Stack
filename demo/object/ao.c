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
*********************************************************************/

/* Analog Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "wp.h"
#include "ao.h"
#include "handlers.h"

#ifndef MAX_ANALOG_OUTPUTS
#define MAX_ANALOG_OUTPUTS 4
#endif

/* we choose to have a NULL level in our system represented by */
/* a particular value.  When the priorities are not in use, they */
/* will be relinquished (i.e. set to the NULL level). */
#define AO_LEVEL_NULL 255
/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define AO_RELINQUISH_DEFAULT 0
/* Here is our Priority Array.  They are supposed to be Real, but */
/* we don't have that kind of memory, so we will use a single byte */
/* and load a Real for returning the value when asked. */
static uint8_t Analog_Output_Level[MAX_ANALOG_OUTPUTS][BACNET_MAX_PRIORITY];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
static bool Out_Of_Service[MAX_ANALOG_OUTPUTS];

/* we need to have our arrays initialized before answering any calls */
static bool Analog_Output_Initialized = false;

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_UNITS,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1
};

static const int Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_RELIABILITY,

#if ( BACNET_SVC_COV_B == 1 )
    PROP_COV_INCREMENT,
#endif

#if (INTRINSIC_REPORTING == 1)
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
#endif
    -1
};

static const int Properties_Proprietary[] = {
    -1
};

void Analog_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


void Analog_Output_Init(
    void)
{
    unsigned i, j;

    if (!Analog_Output_Initialized) {
        Analog_Output_Initialized = true;

        /* initialize all the analog output priority arrays to NULL */
        for (i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
            for (j = 0; j < BACNET_MAX_PRIORITY; j++) {
                Analog_Output_Level[i][j] = AO_LEVEL_NULL;
            }
        }
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Analog_Output_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_ANALOG_OUTPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Analog_Output_Count(
    void)
{
    return MAX_ANALOG_OUTPUTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Analog_Output_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Analog_Output_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_ANALOG_OUTPUTS;

    if (object_instance < MAX_ANALOG_OUTPUTS)
        index = object_instance;

    return index;
}

float Analog_Output_Present_Value(
    uint32_t object_instance)
{
    float value = AO_RELINQUISH_DEFAULT;
    unsigned index = 0;
    unsigned i = 0;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Analog_Output_Level[index][i] != AO_LEVEL_NULL) {
                value = Analog_Output_Level[index][i];
                break;
            }
        }
    }

    return value;
}

unsigned Analog_Output_Present_Value_Priority(
    uint32_t object_instance)
{
    unsigned index = 0; /* instance to index conversion */
    unsigned i = 0;     /* loop counter */
    unsigned priority = 0;      /* return value */

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Analog_Output_Level[index][i] != AO_LEVEL_NULL) {
                priority = i + 1;
                break;
            }
        }
    }

    return priority;
}

bool Analog_Output_Present_Value_Set(
    uint32_t object_instance,
    float value,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */ ) &&
            (value >= 0.0) && (value <= 100.0)) {
            Analog_Output_Level[index][priority - 1] = (uint8_t) value;
            /* Note: you could set the physical output here to the next
               highest priority, or to the relinquish default if no
               priorities are set.
               However, if Out of Service is TRUE, then don't set the
               physical output.  This comment may apply to the
               main loop (i.e. check out of service before changing output) */
            status = true;
        }
    }

    return status;
}

bool Analog_Output_Present_Value_Relinquish(
    uint32_t object_instance,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */ )) {
            Analog_Output_Level[index][priority - 1] = AO_LEVEL_NULL;
            /* Note: you could set the physical output here to the next
               highest priority, or to the relinquish default if no
               priorities are set.
               However, if Out of Service is TRUE, then don't set the
               physical output.  This comment may apply to the
               main loop (i.e. check out of service before changing output) */
            status = true;
        }
    }

    return status;
}

/* note: the object name must be unique within this device */
bool Analog_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;

    if (object_instance < MAX_ANALOG_OUTPUTS) {
        sprintf(text_string, "ANALOG OUTPUT %lu",
            (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


bool Analog_Output_Out_Of_Service(
    uint32_t instance)
{
    unsigned index = 0;
    bool oos_flag = false;

    index = Analog_Output_Instance_To_Index(instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        oos_flag = Out_Of_Service[index];
    }

    return oos_flag;
}

void Analog_Output_Out_Of_Service_Set(
    uint32_t instance,
    bool oos_flag)
{
    unsigned index = 0;

    index = Analog_Output_Instance_To_Index(instance);
    if (index < MAX_ANALOG_OUTPUTS) {
        Out_Of_Service[index] = oos_flag;
    }
}

/* return apdu length, or BACNET_STATUS_ERROR on error */
int Analog_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    float real_value = (float) 1.414;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return BACNET_STATUS_ERROR;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_ANALOG_OUTPUT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Analog_Output_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_OUTPUT);
            break;

        case PROP_PRESENT_VALUE:
            real_value = Analog_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            state = Analog_Output_Out_Of_Service(rpdata->object_instance);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, state);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            state = Analog_Output_Out_Of_Service(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_UNITS:
            apdu_len = encode_application_enumerated(&apdu[0], UNITS_PERCENT);
            break;
        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0)
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                object_index =
                    Analog_Output_Instance_To_Index(rpdata->object_instance);
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    if (Analog_Output_Level[object_index][i] == AO_LEVEL_NULL)
                        len = encode_application_null(&apdu[apdu_len]);
                    else {
                        real_value = Analog_Output_Level[object_index][i];
                        len =
                            encode_application_real(&apdu[apdu_len],
                            real_value);
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
            } else {
                object_index =
                    Analog_Output_Instance_To_Index(rpdata->object_instance);
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    if (Analog_Output_Level[object_index][rpdata->array_index -
                            1] == AO_LEVEL_NULL)
                        apdu_len = encode_application_null(&apdu[0]);
                    else {
                        real_value = Analog_Output_Level[object_index]
                            [rpdata->array_index - 1];
                        apdu_len =
                            encode_application_real(&apdu[0], real_value);
                    }
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_RELINQUISH_DEFAULT:
            real_value = AO_RELINQUISH_DEFAULT;
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
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
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
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
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                status =
                    Analog_Output_Present_Value_Set(wp_data->object_instance,
                    value.type.Real, wp_data->priority);
                if (wp_data->priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else if (!status) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    status =
                        Analog_Output_Present_Value_Relinquish
                        (wp_data->object_instance, wp_data->priority);
                    if (!status) {
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
                Analog_Output_Out_Of_Service_Set(wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
    case PROP_RELIABILITY:
        case PROP_UNITS:
#if ( INTRINSIC_REPORTING == 1 )
    case PROP_ACKED_TRANSITIONS:
    case PROP_EVENT_TIME_STAMPS:
#endif
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
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


#if (INTRINSIC_REPORTING == 1)

void Analog_Output_Intrinsic_Reporting(
    uint32_t object_instance)
{
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    ANALOG_OUTPUT_DESCR *currentObject;
    // unsigned int object_index;
    BACNET_EVENT_STATE FromState;
    BACNET_EVENT_STATE ToState;
    float ExceededLimit = 0.0f;
    float PresentVal = 0.0f;
    bool SendNotify = false;

    currentObject = Find_bacnet_AO_object(object_instance);
    if ( currentObject == NULL) {
        panic();
        return;
    }


    /* check limits */
    if (!currentObject->pAO->Limit_Enable)
    {
        /* 13.3.6 (c) If pCurrentState is HIGH_LIMIT, and the HighLimitEnable flag of pLimitEnable is FALSE, then indicate a
        transition to the NORMAL event state. (etc) */
        // todo3 - BTC, we should examing both flags here.. Karg's logic is not complete here.
        currentObject->Event_State = EVENT_STATE_NORMAL;
        return; /* limits are not configured */
    }


    if (currentObject->Ack_notify_data.bSendAckNotify) {
        /* clean bSendAckNotify flag */
        currentObject->Ack_notify_data.bSendAckNotify = false;
        /* copy toState */
        ToState = currentObject->Ack_notify_data.EventState;

        dbTraffic(DB_DEBUG, "Send Acknotification for (%s,%d).",
            bactext_object_type_name(OBJECT_ANALOG_OUTPUT), object_instance);

        characterstring_init_ansi(&msgText, "AckNotification");

        /* Notify Type */
        event_data.notifyType = NOTIFY_ACK_NOTIFICATION;

        /* Send EventNotification. */
        SendNotify = true;
    }
    else {
        /* actual Present_Value */
        PresentVal = Analog_Output_Present_Value(currentObject, ram);
        FromState = currentObject->Event_State;
        switch (currentObject->Event_State) {
        case EVENT_STATE_NORMAL:
            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the High_Limit for a minimum
               period of time, specified in the Time_Delay property, and
               (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-OFFNORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal > currentObject->pAO->High_Limit) &&
                ((currentObject->pAO->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE) &&
                    ((currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_HIGH_LIMIT;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }

            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the Low_Limit plus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal < currentObject->pAO->Low_Limit) &&
                ((currentObject->pAO->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE) &&
                    ((currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_LOW_LIMIT;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->pAO->Time_Delay;
            break;

        case EVENT_STATE_HIGH_LIMIT:
            /* Once exceeded, the Present_Value must fall below the High_Limit minus
               the Deadband before a TO-NORMAL event is generated under these conditions:
               (a) the Present_Value must fall below the High_Limit minus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal < currentObject->pAO->High_Limit - currentObject->pAO->Deadband)
                && ((currentObject->pAO->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE) &&
                    ((currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->pAO->Time_Delay;
            break;

        case EVENT_STATE_LOW_LIMIT:
            /* Once the Present_Value has fallen below the Low_Limit,
               the Present_Value must exceed the Low_Limit plus the Deadband
               before a TO-NORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the Low_Limit plus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal > currentObject->pAO->Low_Limit + currentObject->pAO->Deadband)
                && ((currentObject->pAO->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE) &&
                    ((currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->pAO->Time_Delay;
            break;

        default:
            return; /* shouldn't happen */
        }       /* switch (FromState) */

        ToState = currentObject->Event_State;

        if (FromState != ToState) {
            /* Event_State has changed.
               Need to fill only the basic parameters of this type of event.
               Other parameters will be filled in common function. */

            switch (ToState) {
            case EVENT_STATE_HIGH_LIMIT:
                ExceededLimit = currentObject->pAO->High_Limit;
                characterstring_init_ansi(&msgText, "Goes to high limit");
                break;

            case EVENT_STATE_LOW_LIMIT:
                ExceededLimit = currentObject->pAO->Low_Limit;
                characterstring_init_ansi(&msgText, "Goes to low limit");
                break;

            case EVENT_STATE_NORMAL:
                if (FromState == EVENT_STATE_HIGH_LIMIT) {
                    ExceededLimit = currentObject->pAO->High_Limit;
                    characterstring_init_ansi(&msgText,
                        "Back to normal state from high limit");
                }
                else {
                    ExceededLimit = currentObject->pAO->Low_Limit;
                    characterstring_init_ansi(&msgText,
                        "Back to normal state from low limit");
                }
                break;

            default:
                ExceededLimit = 0;
                break;
            }   /* switch (ToState) */

            dbTraffic(DB_DEBUG, "Event_State for (%s,%d) goes from %s to %s.",
                bactext_object_type_name(OBJECT_ANALOG_OUTPUT), object_instance,
                bactext_event_state_name(FromState),
                bactext_event_state_name(ToState));

            /* Notify Type */
            event_data.notifyType = currentObject->pAO->Notify_Type;

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

            case EVENT_STATE_OFFNORMAL:
                panic();
                break;
            }
        }

        /* Notification Class */
        event_data.notificationClass = currentObject->pAO->Notification_Class;

        // todo2  - there is no check of the event_enable T,T,T flags! we are sending events even if they are not enabled!

        /* Event Type */
        event_data.eventType = EVENT_OUT_OF_RANGE;

        /* Message Text */
        event_data.messageText = &msgText;

        /* Notify Type */
        /* filled before */

        /* From State */
        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION)
            event_data.fromState = FromState;

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
                    IsOutOfService( nv ) ) ;

            /* Deadband used for limit checking. */
            event_data.notificationParams.outOfRange.deadband =
                currentObject->pAO->Deadband;
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


#if (INTRINSIC_REPORTING == 1)

int Analog_Output_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data)
{
    bool IsNotAckedTransitions;
    bool IsActiveEvent;
    int i;

    ANALOG_OUTPUT_DESCR *currentObject ;

    /* check index */
    if (index < MAX_ANALOG_OUTPUTS) {
        /* Event_State not equal to NORMAL */
        currentObject = &gAnalogOutput[index];

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
    }
    else {
        return -1;      /* end of list  */
    }

    if ((IsActiveEvent) || (IsNotAckedTransitions)) {
        /* Object Identifier */
        getevent_data->objectIdentifier.type = OBJECT_ANALOG_OUTPUT;
        getevent_data->objectIdentifier.instance =
            Analog_Output_Index_To_Instance(index);
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
        getevent_data->notifyType = currentObject->pAO->Notify_Type;
        /* Event Enable */
        bitstring_init(&getevent_data->eventEnable);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_OFFNORMAL,
            (currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_FAULT,
            (currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_NORMAL,
            (currentObject->pAO->Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        /* Event Priorities */
        Notification_Class_Get_Priorities(currentObject->pAO->Notification_Class,
            getevent_data->eventPriorities);

        return 1;       /* active event */
    }
    else {
        return 0;       /* no active event at this index */
    }
}


// todo 3 - these functions are essentially identical between object types, consolidate 
int Analog_Output_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE * error_code)
{
    ANALOG_OUTPUT_DESCR *currentObject = Find_bacnet_AO_object(alarmack_data->eventObjectIdentifier.instance);
    if (currentObject == NULL) {
        panic();
        *error_class = ERROR_CLASS_OBJECT;
        *error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return -1;
    }

    // preset default for rest
    *error_class = ERROR_CLASS_SERVICES;

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



#endif // defined(INTRINSIC_REPORTING_todo1)
#endif /* defined(INTRINSIC_REPORTING) */



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

    return;
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
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ANALOG_INPUT */
#endif /* TEST */
