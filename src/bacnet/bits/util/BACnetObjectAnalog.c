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

#include "bacnet/bits/util/BACnetObjectAnalog.h"
#include "appApi.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#endif

static float Get_PV_real(const void *ptrToBACnetObject )
{
	BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;

	return currentObject->Present_Value;
}


static bool Set_PV_real(void* ptrToBACnetObject, const float value )
{
    BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;
    
    currentObject->Present_Value = value;
    return true; 
}

static uint Get_PV_unsigned(const void *ptrToBACnetObject )
{
	BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;

	return (uint) currentObject->Present_Value;
}


static bool Set_PV_unsigned(void* ptrToBACnetObject, const uint value )
{
    BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;
    
    currentObject->Present_Value = (float) value;

    return true; 
}

static bool Get_PV_boolean(const void* ptrToBACnetObject)
{
    BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;

    return (bool)currentObject->Present_Value;
}


static bool Set_PV_boolean(void* ptrToBACnetObject, const bool value)
{
    BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;

    currentObject->Present_Value = value;

    return true;
}

static bool Set_PV_adv(void* ptrToBACnetObject, const BACNET_APPLICATION_DATA_VALUE *value)
{
    BACnetAnalogObject* currentObject = (BACnetAnalogObject*)ptrToBACnetObject;

    double tval = BACapi_Get_adv_double(value);

    if (tval != currentObject->Present_Value)
    {
        dbMessage(DBD_Application, DB_UNUSUAL_TRAFFIC, "Value change %f to %f", currentObject->Present_Value, tval);
    }

    currentObject->Present_Value = (float)  tval ;

    return true;
}


void BACnetAnalogObject_Init(
	BACnetAnalogObject			*currentObject,
	const	BACNET_OBJECT_TYPE	objectType,
	const   uint32_t			objectInstance,
	const   char						*objectName,
	const   BACNET_ENGINEERING_UNITS	units)
{
	Generic_Object_Init(
		&currentObject->common,
		objectType,
		objectInstance,
		objectName);

    // 2021-11-09 below are initialized in Getneri_Object_Init()
	//currentObject->common.GetPresentValue_real = Get_PV_real;
 //   currentObject->common.SetPresentValue_real = Set_PV_real;
	//currentObject->common.GetPresentValue_unsigned = Get_PV_unsigned;
 //   currentObject->common.SetPresentValue_unsigned = Set_PV_unsigned;
 //   currentObject->common.GetPresentValue_boolean = Get_PV_boolean;
 //   currentObject->common.SetPresentValue_boolean = Set_PV_boolean;
 //   currentObject->common.SetPresentValue_adv = Set_PV_adv;

	currentObject->Present_Value = 0.0f;

    currentObject->Units = units;

    characterstring_init_ansi(&currentObject->deviceType, "Default 01");

#if (INTRINSIC_REPORTING_B == 1)

	currentObject->eventCommonAnalog.High_Limit = 100.0;
	currentObject->eventCommonAnalog.Low_Limit = 0.0;
	currentObject->eventCommonAnalog.Deadband = 1.0;
	currentObject->eventCommonAnalog.Limit_Enable = 0;

	/* initialize Event time stamps using wildcards and set Acked_transitions */
	for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
		datetime_wildcard_set(&currentObject->common.eventCommon.Event_Time_Stamps[j]);
		currentObject->common.eventCommon.Acked_Transitions[j].bIsAcked = true;
	}

#endif  // (INTRINSIC_REPORTING_B == 1)

#if ( BACNET_USE_MAX_MIN_RESOLUTION_PRESENT_VALUE == 1 )
    currentObject->max_pres_value = 100000.0;
    currentObject->min_pres_value = -100000.0;
    currentObject->resolution = 0.001;
#endif

#if ( BACNET_SVC_COV_B == 1 )
	currentObject->Changed = false;
	currentObject->Prior_Value = 0.0f;
	currentObject->COV_Increment = 1.0f;
#endif  // ( BACNET_SVC_COV_B == 1 )

}


#if (INTRINSIC_REPORTING_B == 1)

void Common_Analog_Intrinsic_Reporting(
    BACNET_OBJECT* currentObject,
    EVENT_COMMON* eventCommon,
    EVENT_COMMON_ANALOG* eventCommonAnalog)
{
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    BACNET_EVENT_STATE FromState;
    BACNET_EVENT_STATE ToState;
    float ExceededLimit;
    float PresentVal;
    bool SendNotify = false;

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
    if (!eventCommonAnalog->Limit_Enable) {
        /* 13.3.6 (c) If pCurrentState is HIGH_LIMIT, and the HighLimitEnable flag of pLimitEnable is FALSE, then indicate a
        transition to the NORMAL event state. (etc) */
        currentObject->Event_State = EVENT_STATE_NORMAL;
        return;    /* limits are not configured */
    }

    if (eventCommon->Ack_notify_data.bSendAckNotify) {
        /* clean bSendAckNotify flag */
        eventCommon->Ack_notify_data.bSendAckNotify = false;
        /* copy toState */
        ToState = eventCommon->Ack_notify_data.EventState;
        FromState = eventCommon->Ack_notify_data.EventState;  // not actually used, just to eliminate warnings

        characterstring_init_ansi(&msgText, "AckNotification");

        /* Notify Type */
        event_data.notifyType = NOTIFY_ACK_NOTIFICATION;

        /* Send EventNotification. */
        SendNotify = true;
    }
    else {
        /* actual Present_Value */
        PresentVal = currentObject->GetPresentValue_float(currentObject);
        FromState = currentObject->Event_State;
        switch (currentObject->Event_State) {
        case EVENT_STATE_NORMAL:

            // todo 2 - review
            //if (>FaultStatus) {
            //    currentObject->Event_State = EVENT_STATE_FAULT;
            //    break;
            //}

            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the High_Limit for a minimum
               period of time, specified in the Time_Delay property, and
               (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-OFFNORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal > eventCommonAnalog->High_Limit) &&
                ((eventCommonAnalog->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE) &&
                    ((eventCommon->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!eventCommon->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_HIGH_LIMIT;
                }
                else {
                    eventCommon->Remaining_Time_Delay--;
                }
                break;
            }

            /* A TO-OFFNORMAL event is generated under these conditions:
               (a) the Present_Value must exceed the Low_Limit plus the Deadband
               for a minimum period of time, specified in the Time_Delay property, and
               (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
               (c) the TO-NORMAL flag must be set in the Event_Enable property. */
            if ((PresentVal < eventCommonAnalog->Low_Limit) &&
                ((eventCommonAnalog->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //    ((currentObject->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                //        EVENT_ENABLE_TO_OFFNORMAL)) {
                if (!eventCommon->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_LOW_LIMIT;
                }
                else {
                    eventCommon->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            eventCommon->Remaining_Time_Delay = eventCommon->Time_Delay;
            break;

        case EVENT_STATE_HIGH_LIMIT:

            // todo 2 - review
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
            if ((PresentVal < eventCommonAnalog->High_Limit - eventCommonAnalog->Deadband)
                && ((eventCommonAnalog->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                    EVENT_HIGH_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //   ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                //        EVENT_ENABLE_TO_NORMAL)) {
                if (!eventCommon->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                }
                else {
                    eventCommon->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            eventCommon->Remaining_Time_Delay = eventCommon->Time_Delay;
            break;

        case EVENT_STATE_LOW_LIMIT:

            // todo 2 - review
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
            if ((PresentVal > eventCommonAnalog->Low_Limit + eventCommonAnalog->Deadband)
                && ((eventCommonAnalog->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                    EVENT_LOW_LIMIT_ENABLE)) {
                // not true - EVENT_ENABLE only affects distribution of the event, not logic processing!
                //    ((currentObject->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                //        EVENT_ENABLE_TO_NORMAL)) {
                if (!eventCommon->Remaining_Time_Delay) {
                    currentObject->Event_State = EVENT_STATE_NORMAL;
                }
                else {
                    eventCommon->Remaining_Time_Delay--;
                }
                break;
            }
            /* value of the object is still in the same event state */
            eventCommon->Remaining_Time_Delay = eventCommon->Time_Delay;
            break;

            // todo 2 - review
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

               // todo 1 : so much in common between AI and AV here...  consolidate?

            switch (ToState) {
            case EVENT_STATE_HIGH_LIMIT:
                Device_getCurrentDateTime(&eventCommon->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL]);
                if ((eventCommon->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) == EVENT_ENABLE_TO_OFFNORMAL) {
                    ExceededLimit = eventCommonAnalog->High_Limit;
                    characterstring_init_ansi(&msgText, "Goes to high limit");
                    SendNotify = true;
                }
                break;

            case EVENT_STATE_LOW_LIMIT:
                Device_getCurrentDateTime(&eventCommon->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL]);
                if ((eventCommon->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) == EVENT_ENABLE_TO_OFFNORMAL) {
                    ExceededLimit = eventCommonAnalog->Low_Limit;
                    characterstring_init_ansi(&msgText, "Goes to low limit");
                    SendNotify = true;
                }
                break;

            case EVENT_STATE_NORMAL:
                Device_getCurrentDateTime(&eventCommon->Event_Time_Stamps[TRANSITION_TO_NORMAL]);
                if ((eventCommon->Event_Enable & EVENT_ENABLE_TO_NORMAL) == EVENT_ENABLE_TO_NORMAL) {
                    if (FromState == EVENT_STATE_HIGH_LIMIT) {
                        ExceededLimit = eventCommonAnalog->High_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from high limit");
                    }
                    else {
                        ExceededLimit = eventCommonAnalog->Low_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from low limit");
                    }
                    break;

            case EVENT_STATE_FAULT:
                Device_getCurrentDateTime(&eventCommon->Event_Time_Stamps[TRANSITION_TO_FAULT]);
                if ((eventCommon->Event_Enable & EVENT_ENABLE_TO_FAULT) == EVENT_ENABLE_TO_FAULT) {
                    ExceededLimit = 999.9f;
                    characterstring_init_ansi(&msgText, "New Fault Condition");
                    SendNotify = true;
                }
                break;

            default:
                panic();
                ExceededLimit = 0;
                break;
                }   /* switch (ToState) */

                /* Notify Type */
                event_data.notifyType = eventCommon->Notify_Type;

                /* Send EventNotification. */
                // see logic above SendNotify = true;
            }
        }


        if (SendNotify) {
            /* Event Object Identifier */
            event_data.eventObjectIdentifier.type = currentObject->objectType;
            event_data.eventObjectIdentifier.instance = currentObject->objectInstance;

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
            event_data.notificationClass = eventCommon->Notification_Class;

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
                    eventCommonAnalog->Deadband;
                /* Limit that was exceeded. */
                event_data.notificationParams.outOfRange.exceededLimit =
                    ExceededLimit;
            }

            /* add data from notification class */
            panic();
            // need to get pDev in here // todo 1 ekh
            // Notification_Class_common_reporting_function(&event_data);

            /* Ack required */
            if ((event_data.notifyType != NOTIFY_ACK_NOTIFICATION) &&
                (event_data.ackRequired == true)) {
                switch (event_data.toState) {
                case EVENT_STATE_OFFNORMAL:
                case EVENT_STATE_HIGH_LIMIT:
                case EVENT_STATE_LOW_LIMIT:
                    eventCommon->Acked_Transitions[TRANSITION_TO_OFFNORMAL].bIsAcked = false;
                    eventCommon->Acked_Transitions[TRANSITION_TO_OFFNORMAL].Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_FAULT:                                         // todo3 - we don't have a fault condition. Review.
                    eventCommon->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked = false;
                    eventCommon->Acked_Transitions[TRANSITION_TO_FAULT].Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_NORMAL:
                    eventCommon->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked = false;
                    eventCommon->Acked_Transitions[TRANSITION_TO_NORMAL].Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;

                default:
                    panic();
                    // note: we are not supporting FAULT state (requires reliability flag. not sure if we want that at this time.) todo
                }
            }
        }
    }
}

#endif  // (INTRINSIC_REPORTING_B == 1)

