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
*********************************************************************/

/* Binary Input Objects customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "rp.h"
#include "wp.h"
#include "cov.h"
#include "config.h"     /* the custom stuff */
#include "bi.h"
#include "handlers.h"

#ifndef MAX_BINARY_INPUTS
#define MAX_BINARY_INPUTS 5
#endif

/* stores the current value */
static BACNET_BINARY_PV Present_Value[MAX_BINARY_INPUTS];
/* out of service decouples physical input from Present_Value */
static bool Out_Of_Service[MAX_BINARY_INPUTS];
/* Change of Value flag */
static bool Change_Of_Value[MAX_BINARY_INPUTS];
/* Polarity of Input */
static BACNET_POLARITY Polarity[MAX_BINARY_INPUTS];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Input_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    -1
};

static const int Binary_Input_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Binary_Input_Properties_Proprietary[] = {
    -1
};

void Binary_Input_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Binary_Input_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Binary_Input_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Binary_Input_Properties_Proprietary;
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Input_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_BINARY_INPUTS) {
        return true;
    }

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Input_Count(
    void)
{
    return MAX_BINARY_INPUTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Input_Index_To_Instance(
    unsigned index)
{
    return index;
}

void Binary_Input_Init(
    void)
{
    static bool initialized = false;
    unsigned i;

    if (!initialized) {
        initialized = true;

        /* initialize all the values */
        for (i = 0; i < MAX_BINARY_INPUTS; i++) {
            Present_Value[i] = BINARY_INACTIVE;
        }
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Input_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_BINARY_INPUTS;

    if (object_instance < MAX_BINARY_INPUTS) {
        index = object_instance;
    }

    return index;
}

BACNET_BINARY_PV Binary_Input_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        value = Present_Value[index];
        if (Polarity[index] != POLARITY_NORMAL) {
            if (value == BINARY_INACTIVE) {
                value = BINARY_ACTIVE;
            } else {
                value = BINARY_INACTIVE;
            }
        }
    }

    return value;
}

bool Binary_Input_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        value = Out_Of_Service[index];
    }

    return value;
}


#if ( BACNET_SVC_COV_B == 1 )

bool Binary_Input_Change_Of_Value(
    uint32_t object_instance)
{
    bool status = false;
    unsigned index;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        status = Change_Of_Value[index];
    }

    return status;
}

void Binary_Input_Change_Of_Value_Clear(
    uint32_t object_instance)
{
    unsigned index;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        Change_Of_Value[index] = false;
    }

    return;
}

/**
 * For a given object instance-number, loads the value_list with the COV data.
 *
 * @param  object_instance - object-instance number of the object
 * @param  value_list - list of COV data
 *
 * @return  true if the value list is encoded
 */
bool Binary_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
        value_list->value.next = NULL;
        value_list->value.type.Enumerated =
            Binary_Input_Present_Value(object_instance);
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
            STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OVERRIDDEN, false);
        if (Binary_Input_Out_Of_Service(object_instance)) {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                STATUS_FLAG_OUT_OF_SERVICE, true);
        } else {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                STATUS_FLAG_OUT_OF_SERVICE, false);
        }
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
        status = true;
    }

    return status;
}
#endif // ( BACNET_SVC_COV_B == 1 )


bool Binary_Input_Present_Value_Set(
    uint32_t object_instance,
    BACNET_BINARY_PV value)
{
    unsigned index = 0;
    bool status = false;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        if (Polarity[index] != POLARITY_NORMAL) {
            if (value == BINARY_INACTIVE) {
                value = BINARY_ACTIVE;
            } else {
                value = BINARY_INACTIVE;
            }
        }
        if (Present_Value[index] != value) {
            Change_Of_Value[index] = true;
        }
        Present_Value[index] = value;
        status = true;
    }

    return status;
}

void Binary_Input_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value)
{
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        if (Out_Of_Service[index] != value) {
            Change_Of_Value[index] = true;
        }
        Out_Of_Service[index] = value;
    }

    return;
}

bool Binary_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;
    unsigned index ;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        sprintf(text_string, "BINARY INPUT %lu",
                (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

BACNET_POLARITY Binary_Input_Polarity(
    uint32_t object_instance)
{
    BACNET_POLARITY polarity = POLARITY_NORMAL;
    unsigned index ;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        polarity = Polarity[index];
    }

    return polarity;
}

bool Binary_Input_Polarity_Set(
    uint32_t object_instance,
    BACNET_POLARITY polarity)
{
    bool status = false;
    unsigned index ;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        // todo1 is there a COV issue when polarity changes??
        Polarity[index] = polarity;
    }

    return status;
}

/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object already exists, and has been bounds checked */
int Binary_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu = NULL;
    bool state = false;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_BINARY_INPUT,
                rpdata->object_instance);
            break;

        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            /* note: object name must be unique in our device */
            Binary_Input_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_INPUT);
            break;

        case PROP_PRESENT_VALUE:
            /* note: you need to look up the actual value */
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Binary_Input_Present_Value(rpdata->object_instance));
            break;

        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            state = Binary_Input_Out_Of_Service(rpdata->object_instance);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, state);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;

    case PROP_RELIABILITY:
// todo 1	    apdu_len = encode_application_enumerated(&apdu[0], GetReliability(&livedata->objectTypeDescriptor));
        break;

        case PROP_OUT_OF_SERVICE:
            state = Binary_Input_Out_Of_Service(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_POLARITY:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Binary_Input_Polarity(rpdata->object_instance));
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Binary_Input_Write_Property(
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
    if (wp_data->array_index != BACNET_ARRAY_ALL) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (value.type.Enumerated <= MAX_BINARY_PV) {
                    Binary_Input_Present_Value_Set(wp_data->object_instance,
                        (BACNET_BINARY_PV) value.type.Enumerated);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Binary_Input_Out_Of_Service_Set(wp_data->object_instance,
                    value.type.Boolean);
            }
            break;

        case PROP_POLARITY:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (value.type.Enumerated < MAX_POLARITY) {
                    Binary_Input_Polarity_Set(wp_data->object_instance,
                        (BACNET_POLARITY) value.type.Enumerated);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;

        case PROP_DESCRIPTION:
        case PROP_EVENT_STATE:
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_PROPERTY_LIST:
        case PROP_RELIABILITY:
        case PROP_STATUS_FLAGS:
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

void Binary_Input_Intrinsic_Reporting(
    uint32_t object_instance)
{
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    BINARY_INPUT_DESCR *currentObject;
    // unsigned int object_index;
    BACNET_EVENT_STATE FromState;
    BACNET_EVENT_STATE ToState;
    BACNET_BINARY_PV PresentVal;
    bool SendNotify = false;

    currentObject = Find_bacnet_BI_object(object_instance);
    if ( currentObject == NULL) {
        panic();
        return;
    }
    
    // todo 2 - Implement Event Detection Enable?

    if (currentObject->Ack_notify_data.bSendAckNotify) {
        /* clean bSendAckNotify flag */
        currentObject->Ack_notify_data.bSendAckNotify = false;
        /* copy toState */
        ToState = currentObject->Ack_notify_data.EventState;

        dbTraffic(DB_DEBUG, "Send Acknotification for (%s,%d).",
            bactext_object_type_name(OBJECT_BINARY_INPUT), object_instance);

        characterstring_init_ansi(&msgText, "AckNotification");

        /* Notify Type */
        event_data.notifyType = NOTIFY_ACK_NOTIFICATION;

        /* Send EventNotification. */
        SendNotify = true;
    }
    else {
        /* actual Present_Value */

        // PresentVal = ((ram->PresentValue >> gBinaryInput[object_instance].bit) & 1) ? BINARY_ACTIVE : BINARY_INACTIVE;

        PresentVal = Binary_Input_Present_Value(object_instance, nv, ram);
        FromState = currentObject->Event_State;
        switch (currentObject->Event_State) {
            /*  If pCurrentState is NORMAL, and pMonitoredValue is equal to any of the values contained in pAlarmValues
                for pTimeDelay, then indicate a transition to the OFFNORMAL event state.
            */
        case EVENT_STATE_NORMAL:
            if (PresentVal == FIXED_ALARM_VALUE) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_OFFNORMAL;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }

            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->pBI->Time_Delay;
            break;

        case EVENT_STATE_OFFNORMAL:
            if (PresentVal != FIXED_ALARM_VALUE) {
                if (!currentObject->Remaining_Time_Delay)
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                else
                    currentObject->Remaining_Time_Delay--;
                break;
            }
            /* value of the object is still in the same event state */
            currentObject->Remaining_Time_Delay = currentObject->pBI->Time_Delay;
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
            case EVENT_STATE_NORMAL:
                characterstring_init_ansi(&msgText,
                    "Back to normal state");
                break;

            case EVENT_STATE_OFFNORMAL:
                characterstring_init_ansi(&msgText,
                    "To offnormal state");
                break;

            default:
                panic();
                break;
            }   /* switch (ToState) */

            dbTraffic(DB_DEBUG, "Event_State for (%s,%d) goes from %s to %s.",
                bactext_object_type_name(OBJECT_BINARY_INPUT), object_instance,
                bactext_event_state_name(FromState),
                bactext_event_state_name(ToState));

            /* Notify Type */
            event_data.notifyType = currentObject->pBI->Notify_Type;

            /* Send EventNotification. */
            SendNotify = true;
        }
    }


    if (SendNotify) {
        /* Event Object Identifier */
        event_data.eventObjectIdentifier.type = OBJECT_BINARY_INPUT;
        event_data.eventObjectIdentifier.instance = object_instance;

        /* Time Stamp */
        event_data.timeStamp.tag = TIME_STAMP_DATETIME;
        Device_getCurrentDateTime(&event_data.timeStamp.value.dateTime);

        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            /* fill Event_Time_Stamps */
            switch (ToState) {
            case EVENT_STATE_OFFNORMAL:
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

            case EVENT_STATE_HIGH_LIMIT:
            case EVENT_STATE_LOW_LIMIT:
                panic();
                break;
            }
        }

        /* Notification Class */
        event_data.notificationClass = currentObject->pBI->Notification_Class;

        // todo2  - there is no check of the event_enable T,T,T flags! we are sending events even if they are not enabled!

        /* Event Type */
        event_data.eventType = EVENT_CHANGE_OF_STATE;

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
            event_data.notificationParams.changeOfState.newState.state.state = ToState;
            event_data.notificationParams.changeOfState.newState.tag = STATE;   // todo2 - understand this some more - why no equivalent with AI?
            /* Status_Flags of the referenced object. */
            bitstring_init(&event_data.notificationParams.changeOfState.
                statusFlags);
            bitstring_set_bit(&event_data.notificationParams.changeOfState.
                statusFlags, STATUS_FLAG_IN_ALARM,
                currentObject->Event_State ? true : false);
            bitstring_set_bit(&event_data.notificationParams.changeOfState.
                statusFlags, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&event_data.notificationParams.changeOfState.
                statusFlags, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&event_data.notificationParams.changeOfState.
                statusFlags, STATUS_FLAG_OUT_OF_SERVICE,
                    IsOutOfService( nv ) ) ;
        }

        /* add data from notification class */
        Notification_Class_common_reporting_function(&event_data);

        /* Ack required */
        if ((event_data.notifyType != NOTIFY_ACK_NOTIFICATION) &&
            (event_data.ackRequired == true)) {
            switch (event_data.toState) {
            case EVENT_STATE_OFFNORMAL:
                currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                    bIsAcked = false;
                currentObject->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
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


int Binary_Input_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data)
{
    bool IsNotAckedTransitions;
    bool IsActiveEvent;
    int i;

    BINARY_INPUT_DESCR *currentObject ;

    /* check index */
    if (index < MAX_BINARY_INPUTS) {
        /* Event_State not equal to NORMAL */
        currentObject = &gBinaryInput[index];

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
        getevent_data->objectIdentifier.type = OBJECT_BINARY_INPUT;
        getevent_data->objectIdentifier.instance =
            Binary_Input_Index_To_Instance(index);
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
        getevent_data->notifyType = currentObject->pBI->Notify_Type;
        /* Event Enable */
        bitstring_init(&getevent_data->eventEnable);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_OFFNORMAL,
            (currentObject->pBI->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_FAULT,
            (currentObject->pBI->Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_NORMAL,
            (currentObject->pBI->Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        /* Event Priorities */
        Notification_Class_Get_Priorities(currentObject->pBI->Notification_Class,
            getevent_data->eventPriorities);

        return 1;       /* active event */
    }
    else {
        return 0;       /* no active event at this index */
    }
}


// todo 3 - these functions are essentially identical between object types, consolidate 
int Binary_Input_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CLASS *error_class,
    BACNET_ERROR_CODE * error_code)
{
    BINARY_INPUT_DESCR *currentObject = Find_bacnet_BI_object(alarmack_data->eventObjectIdentifier.instance);
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

void testBinaryInput(
    Test * pTest)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;

    Binary_Input_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_INPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Input_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_BINARY_INPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinaryInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_INPUT */
#endif /* TEST */
