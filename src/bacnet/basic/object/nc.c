/****************************************************************************************
*
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
*   2018.06.17 -    Attempting to write to Object_Name returned UNKNOWN_PROPERTY.
*                   Now returns WRITE_ACCESS_DENIED
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

#include <stdint.h>

#include "configProj.h" /* the custom stuff */

#if (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
// this should be set for Intrinsic Reporting or Alert Enrollment

#include "nc.h"
#include "emm.h"
#include "address.h"
#include "bacnet/basic/services.h"
#include "client.h"
#include "bacnet/basic/service/h_wp.h"

static int Notification_Class_decode_destination(
	uint8_t* application_data,
	int application_data_len,
	BACNET_DESTINATION* destination,
	BACNET_ERROR_CLASS* error_class,
	BACNET_ERROR_CODE* error_code);


/* These three arrays are used by the ReadPropertyMultiple handler */

static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_NOTIFICATION_CLASS,
    PROP_PRIORITY,
    PROP_ACK_REQUIRED,
    PROP_RECIPIENT_LIST,
	MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Optional[] = {
    PROP_DESCRIPTION,
	MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
	MAX_BACNET_PROPERTY_ID
};


void Notification_Class_Property_Lists(
	const BACNET_PROPERTY_ID** pRequired,
	const BACNET_PROPERTY_ID** pOptional,
	const BACNET_PROPERTY_ID** pProprietary)
{
    if (pRequired) {
		* pRequired = Properties_Required;
    }
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}



void Notification_Class_Init(
    void)
{
}


BACNET_OBJECT* Notification_Class_Create(
	DEVICE_OBJECT_DATA* pDev,
	const uint32_t objectInstance,
	const char* objectName)
{
	NOTIFICATION_CLASS_DESCR* currentObject = (NOTIFICATION_CLASS_DESCR*)emm_scalloc('a', sizeof(NOTIFICATION_CLASS_DESCR));
	if (currentObject == NULL) {
		panic();
		return NULL ;
	}

	Generic_Object_Init(
		&currentObject->common,
		OBJECT_NOTIFICATION_CLASS,
		objectInstance,
		objectName);

	if (!ll_Enqueue(&pDev->NC_Descriptor_List, currentObject)) {
		panic();
		return NULL ;
	}

	return (BACNET_OBJECT*)currentObject;
}


bool Notification_Class_Valid_Instance(
	DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance)
{
	if (Generic_Instance_To_Object(&pDev->NC_Descriptor_List, object_instance) != NULL) return true;
	return false;
}


/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Notification_Class_Count(
	DEVICE_OBJECT_DATA* pDev)
{
	return pDev->NC_Descriptor_List.count;
}


/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
// This is used by the Device Object Function Table. Must have this signature.
uint32_t Notification_Class_Index_To_Instance(
	DEVICE_OBJECT_DATA* pDev,
	unsigned index)
{
	return Generic_Index_To_Instance(&pDev->NC_Descriptor_List, index);
}




bool Notification_Class_Object_Name(
	DEVICE_OBJECT_DATA* pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
	return Generic_Instance_To_Object_Name(
		&pDev->NC_Descriptor_List,
		object_instance,
		object_name);
}


NOTIFICATION_CLASS_DESCR* Notification_Class_Instance_To_Object(
	DEVICE_OBJECT_DATA* pDev,
	uint32_t object_instance)
{
	return (NOTIFICATION_CLASS_DESCR*)Generic_Instance_To_Object(&pDev->NC_Descriptor_List, object_instance);
}




static int encode_recipient(
    uint8_t* apdu, 
    int size, 
    BACNET_DESTINATION* rcp)
{
	BACNET_OCTET_STRING octet_string;
	BACNET_BIT_STRING bit_string;
	int apdu_len = 0;

	// first make sure this list entry is active
	if (rcp->Recipient.RecipientType == RECIPIENT_TYPE_NOTINITIALIZED) return 0;
	// is there space for largest possible encoding?
	if (size < 32) return -1;
    
    /* Valid Days - BACnetDaysOfWeek - [bitstring] monday-sunday */
    bitstring_init(&bit_string);

    for (int i = 0; i < MAX_BACNET_DAYS_OF_WEEK; i++) {
		bitstring_set_bit(&bit_string, i, rcp->ValidDays & (1 << i));
	}
    
    apdu_len +=
        encode_application_bitstring(
            &apdu[apdu_len],
            &bit_string);

    /* From Time */
    apdu_len +=
        encode_application_time(
            &apdu[apdu_len],
            &rcp->FromTime);

    /* To Time */
    apdu_len +=
        encode_application_time(
            &apdu[apdu_len],
            &rcp->ToTime);

    /*
       BACnetRecipient ::= CHOICE {
       device [0] BACnetObjectIdentifier,
       address [1] BACnetAddress
       } */

       /* CHOICE - device [0] BACnetObjectIdentifier */
	if (rcp->Recipient.RecipientType == RECIPIENT_TYPE_DEVICE_INSTANCE) {
        apdu_len +=
            encode_context_object_id(&apdu[apdu_len], 0,
                OBJECT_DEVICE,
                rcp->Recipient._.DeviceIdentifier);
    }
    /* CHOICE - address [1] BACnetAddress */
	else if (rcp->Recipient.RecipientType == RECIPIENT_TYPE_ADDRESS) {
        /* opening tag 1 */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
        /* network-number Unsigned16, */
        apdu_len +=
            encode_application_unsigned(&apdu[apdu_len],
                rcp->Recipient._.Address.net);

		/* mac-address OCTET STRING */
		// EKH: Removed BACNET_PATH from Recipient, replaced with Global_Address
		octetstring_init(&octet_string,
			rcp->Recipient._.Address.mac.bytes,
			rcp->Recipient._.Address.mac.len);

        apdu_len +=
            encode_application_octet_string(&apdu[apdu_len],
                &octet_string);

        /* closing tag 1 */
        apdu_len += encode_closing_tag(&apdu[apdu_len], 1);

    }

    /* Process Identifier - Unsigned32 */
    apdu_len +=
        encode_application_unsigned(
            &apdu[apdu_len],
            rcp->ProcessIdentifier);

    /* Issue Confirmed Notifications - boolean */
    apdu_len +=
        encode_application_boolean(
            &apdu[apdu_len],
            rcp->ConfirmedNotify);

    /* Transitions - BACnet Event Transition Bits [bitstring] */
	bitstring_init(&bit_string);
	bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
		rcp->Transitions & TRANSITION_TO_OFFNORMAL_MASKED);
	bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
		rcp->Transitions & TRANSITION_TO_FAULT_MASKED);
	bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
		rcp->Transitions & TRANSITION_TO_NORMAL_MASKED);

    apdu_len +=
        encode_application_bitstring(
            &apdu[apdu_len],
            &bit_string);

	return apdu_len;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object already exists, and has been bounds checked */
int Notification_Class_Read_Property(
	DEVICE_OBJECT_DATA* pDev,
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    BACNET_CHARACTER_STRING char_string;
    BACNET_OCTET_STRING octet_string;
    BACNET_BIT_STRING bit_string;
    uint8_t *apdu;
    uint8_t u8Val;
    int len;
    int apdu_len = 0;   /* return value */

	const BACNET_PROPERTY_ID* pRequired = NULL, * pOptional = NULL, * pProprietary = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
		return BACNET_STATUS_ERROR;
	}


	NOTIFICATION_CLASS_DESCR* currentObject = Notification_Class_Instance_To_Object(
		pDev,
		rpdata->object_instance);

    if (currentObject == NULL)
    {
        rpdata->error_class = ERROR_CLASS_OBJECT;
        rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;

    switch (rpdata->object_property) {

    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                OBJECT_NOTIFICATION_CLASS,
                rpdata->object_instance);
        break;

    case PROP_OBJECT_NAME:
		Notification_Class_Object_Name(
			pDev,
			rpdata->object_instance,
            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

	case PROP_DESCRIPTION:
		apdu_len =
			encode_application_character_string(&apdu[0], BACnetObject_Description_Get(&currentObject->common));
		break;

    case PROP_OBJECT_TYPE:
        apdu_len =
			encode_application_enumerated(
				&apdu[0],
                OBJECT_NOTIFICATION_CLASS);
        break;

    case PROP_NOTIFICATION_CLASS:
		// todo BTC - this shall always equal object instance
        apdu_len +=
            encode_application_unsigned(&apdu[0], rpdata->object_instance);
        break;

    case PROP_PRIORITY:
        if (rpdata->array_index == 0) {
            apdu_len += encode_application_unsigned(&apdu[0], MAX_BACNET_EVENT_TRANSITION);
        }
        else {
            if (rpdata->array_index == BACNET_ARRAY_ALL) {
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                        currentObject->Priority[TRANSITION_TO_OFFNORMAL]);
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                        currentObject->Priority[TRANSITION_TO_FAULT]);
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                        currentObject->Priority[TRANSITION_TO_NORMAL]);
            }
            else if (rpdata->array_index <= MAX_BACNET_EVENT_TRANSITION) {
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                        currentObject->Priority[rpdata->array_index - 1]);
            }
            else {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = -1;
            }
        }
        break;

    case PROP_ACK_REQUIRED:
        u8Val = currentObject->Ack_Required;

        bitstring_init(&bit_string);
        bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
            (u8Val & TRANSITION_TO_OFFNORMAL_MASKED) ? true : false);
        bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
            (u8Val & TRANSITION_TO_FAULT_MASKED) ? true : false);
        bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
            (u8Val & TRANSITION_TO_NORMAL_MASKED) ? true : false);
        /* encode bitstring */
        apdu_len +=
            encode_application_bitstring(&apdu[apdu_len], &bit_string);
        break;

    case PROP_RECIPIENT_LIST:
        /* encode all entry of Recipient_List */
        for (int idx = 0; idx < NC_MAX_RECIPIENTS; idx++) {
			len = encode_recipient(
                        &apdu[apdu_len],
				        rpdata->application_data_len - apdu_len,
				        &currentObject->Recipient_List[idx]);
			if (len < 0) {
				rpdata->error_class = ERROR_CLASS_PROPERTY;
				rpdata->error_code = ERROR_CODE_ABORT_BUFFER_OVERFLOW;
				apdu_len = -1;
				break;
			}
			apdu_len += len;
        }
        break;

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) &&
        (rpdata->object_property != PROP_PRIORITY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}


/* returns true if successful */
bool Notification_Class_Write_Property(
	DEVICE_OBJECT_DATA* pDev,
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	NOTIFICATION_CLASS_DESCR TmpNotify;
	BACNET_APPLICATION_DATA_VALUE value;
	uint8_t TmpPriority[MAX_BACNET_EVENT_TRANSITION];  /* BACnetARRAY[3] of Unsigned */
	bool status = false;
	int iOffset;
	uint8_t idx;
    int len;


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
    if ((wp_data->object_property != PROP_PRIORITY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

	NOTIFICATION_CLASS_DESCR* currentObject =
		Notification_Class_Instance_To_Object(
			pDev,
			wp_data->object_instance);

    if (currentObject == NULL) {
        // btc todo - when do we ever use this error code? wp_data->error_code = ERROR_CODE_NO_OBJECTS_OF_SPECIFIED_TYPE;
        // todo1 - does BTC detect this error??
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRIORITY:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

		if (status) {
			if (wp_data->array_index == 0) {
				wp_data->error_class = ERROR_CLASS_PROPERTY;
				wp_data->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
				status = false;
			}
			else if (wp_data->array_index == BACNET_ARRAY_ALL) {
				for (idx = 0; idx < MAX_BACNET_EVENT_TRANSITION; idx++)
				{
					len =
						bacapp_decode_application_data(&wp_data->
							application_data[iOffset], wp_data->application_data_len,
							&value);
					if ((len == 0) ||
						(value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT))
					{
						/* Bad decode, wrong tag or following required parameter missing */
						wp_data->error_class = ERROR_CLASS_PROPERTY;
						wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
						status = false;
						break;
					}
					if (value.type.Unsigned_Int > 255) {
						wp_data->error_class = ERROR_CLASS_PROPERTY;
						wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
						status = false;
						break;
					}
					TmpPriority[idx] = (uint8_t)value.type.Unsigned_Int;
					iOffset += len;
				}
				if (status == true) {
					for (idx = 0; idx < MAX_BACNET_EVENT_TRANSITION; idx++)
						currentObject->Priority[idx] = TmpPriority[idx];
				}
			}
			else if (wp_data->array_index <= MAX_BACNET_EVENT_TRANSITION) {
				if (value.type.Unsigned_Int > 255) {
					wp_data->error_class = ERROR_CLASS_PROPERTY;
					wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
					status = false;
				}
				else
					currentObject->Priority[wp_data->array_index - 1] =
					value.type.Unsigned_Int;
			}
			else {
				wp_data->error_class = ERROR_CLASS_PROPERTY;
				wp_data->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
				status = false;
			}
		}
		break;

    case PROP_ACK_REQUIRED:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

        if (status) {
			if (value.type.Bit_String.bits_used == MAX_BACNET_EVENT_TRANSITION) {
                currentObject->Ack_Required =
                    value.type.Bit_String.value[0];
            }
            else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
        }
        break;

    case PROP_RECIPIENT_LIST:

		// need to zero out all unused recipients, no matter the ultimate length of the written list cr3412341234124
		memset(&TmpNotify, 0x00, sizeof(NOTIFICATION_CLASS_DESCR));
		idx = 0;
		iOffset = 0;
		/* decode all packed */
		while (iOffset < wp_data->application_data_len) {

			/* Increasing element of list */
			if (idx < NC_MAX_RECIPIENTS) {
				len =
					Notification_Class_decode_destination(&wp_data->application_data[iOffset],
						wp_data->application_data_len - iOffset, &currentObject->Recipient_List[idx],
						&wp_data->error_class, &wp_data->error_code);

				/* check for error decoding list element(s) */
				if (len < 0) {
					/* error code/class already set in wp_data */
					return false;
				}
				/* store value */
				// EKH: Removed BACNET_PATH from Recipient, replaced with Global_Address
			}
			else {
				wp_data->error_class = ERROR_CLASS_RESOURCES;
				wp_data->error_code = ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
				return false;
			}
			/* increment offset into application data */
			iOffset += len;
			/* increment list element index */
			idx++;
		}



        /* Decoded all recipient list */
        /* copy elements from temporary object */
        for (idx = 0; idx < NC_MAX_RECIPIENTS; idx++) {
			BACNET_PATH src;
			uint16_t max_apdu;
			int32_t DeviceID;

			currentObject->Recipient_List[idx] =
				TmpNotify.Recipient_List[idx];

			if (currentObject->Recipient_List[idx].Recipient.
				RecipientType == RECIPIENT_TYPE_DEVICE_INSTANCE) {
				/* copy Device_ID */
				DeviceID =
					currentObject->Recipient_List[idx].Recipient._.
					DeviceIdentifier;
				address_bind_request(pDev->datalink, DeviceID, &max_apdu, &src);

			}
			else if (currentObject->Recipient_List[idx].Recipient.RecipientType == RECIPIENT_TYPE_ADDRESS) {
				/* copy Address */
				// src = currentObject->Recipient_List[idx].Recipient._.Address;
				// we only have the address, so we need to discover 1) the local mac (maybe) and 2) the instance associated with
				// that BACnet address. The following will do this for us. 
				//
				// 2017.03.05 Actually, in the address form of the recipient, we have NN and MAC of the recipient.
				// We need to discover the MAC address for the router to NN, Kargs implementation is going to fail
				// 7.3.2.21.3.6 Recipient_List Property Supports Network Address Recipients
				// 
				// todo 1 - not fully implemented by Karg. 
				/* address_bind_request(BACNET_MAX_INSTANCE, &max_apdu, &src); */

				panic();
				// only need to send this to unbound NNs..
				// Send_Who_Is_Router_To_Network(pDev, currentObject->Recipient_List[idx].Recipient._.Address.net);
			}
		}

		status = true;
		break;

    case PROP_DESCRIPTION:
    case PROP_NOTIFICATION_CLASS:
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
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


void Notification_Class_Get_Priorities(
	DEVICE_OBJECT_DATA* pDev,
	uint32_t objectInstance,
	uint32_t* pPriorityArray)
{
    int i;

	NOTIFICATION_CLASS_DESCR* currentObject =
		Notification_Class_Instance_To_Object(
			pDev,
			objectInstance);

	if (currentObject == NULL) {
		for (i = 0; i < MAX_BACNET_EVENT_TRANSITION; i++) {
			pPriorityArray[i] = 255;
		}
		return; /* unknown object */
	}

	for (i = 0; i < MAX_BACNET_EVENT_TRANSITION; i++) {
		pPriorityArray[i] = currentObject->Priority[i];
	}
}


static bool IsRecipientActive(
	BACNET_DESTINATION* pBacDest,
	uint8_t EventToState)
{
	BACNET_DATE_TIME DateTime;

	/* valid Transitions */
	switch (EventToState) {
	case EVENT_STATE_OFFNORMAL:
	case EVENT_STATE_HIGH_LIMIT:
	case EVENT_STATE_LOW_LIMIT:
		if (!(pBacDest->Transitions & TRANSITION_TO_OFFNORMAL_MASKED))
			return false;
		break;

	case EVENT_STATE_FAULT:
		if (!(pBacDest->Transitions & TRANSITION_TO_FAULT_MASKED))
			return false;
		break;

	case EVENT_STATE_NORMAL:
		if (!(pBacDest->Transitions & TRANSITION_TO_NORMAL_MASKED))
			return false;
		break;

	default:
		return false;       /* shouldn't happen */
	}

    /* get actual date and time */
    Device_getCurrentDateTime(&DateTime);

	/* valid Days */
	if (!((0x01 << (DateTime.date.wday - 1)) & pBacDest->ValidDays)) {
		return false;
	}

    /* valid FromTime */
    if (datetime_compare_time(&DateTime.time, &pBacDest->FromTime) < 0) {
        // printf("Not active 6\n\r");
        return false;
    }

    /* valid ToTime */
    if (datetime_compare_time(&pBacDest->ToTime, &DateTime.time) < 0) {
        // printf("Not active 7\n\r");
        return false;
    }

    return true;
}


void Notification_Class_common_reporting_function(
	DEVICE_OBJECT_DATA* pDev,
	BACNET_ROUTE* route,
	BACNET_EVENT_NOTIFICATION_DATA* event_data)
{
	/* Fill the parameters common for all types of events. */

	BACNET_DESTINATION* pBacDest;
	uint32_t notify_index;
	uint8_t index;

	NOTIFICATION_CLASS_DESCR* currentObject =
		Notification_Class_Instance_To_Object(
			pDev,
			event_data->notificationClass);


	if (currentObject == NULL) {
		panic();
		return;
	}


	/* Initiating Device Identifier */
	event_data->initiatingDeviceIdentifier.type = OBJECT_DEVICE;
	event_data->initiatingDeviceIdentifier.instance = pDev->bacObj.objectInstance;

    /* Priority and AckRequired */
    switch (event_data->toState) {
    case EVENT_STATE_NORMAL:
        event_data->priority =
            currentObject->Priority[TRANSITION_TO_NORMAL];
        event_data->ackRequired =
            (currentObject->
                Ack_Required & TRANSITION_TO_NORMAL_MASKED) ? true : false;
        break;

    case EVENT_STATE_FAULT:
        event_data->priority =
            currentObject->Priority[TRANSITION_TO_FAULT];
        event_data->ackRequired =
            (currentObject->
                Ack_Required & TRANSITION_TO_FAULT_MASKED) ? true : false;
        break;

    case EVENT_STATE_OFFNORMAL:
    case EVENT_STATE_HIGH_LIMIT:
    case EVENT_STATE_LOW_LIMIT:
        event_data->priority =
            currentObject->Priority[TRANSITION_TO_OFFNORMAL];
        event_data->ackRequired =
            (currentObject->Ack_Required & TRANSITION_TO_OFFNORMAL_MASKED)
            ? true : false;
        break;

    default:       /* shouldn't happen */
        break;
    }

	/* send notifications for active recipients */
	/* pointer to first recipient */
	pBacDest = &currentObject->Recipient_List[0];
	for (index = 0; index < NC_MAX_RECIPIENTS; index++, pBacDest++) {
		/* check if recipient is defined */
		if (pBacDest->Recipient.RecipientType == RECIPIENT_TYPE_NOTINITIALIZED) {
			break;    /* recipient isn't defined - end of list */
		}

		if (IsRecipientActive(pBacDest, event_data->toState) == true) {
			BACNET_PATH dest;
			uint32_t device_id;
			uint16_t max_apdu;

			/* Process Identifier */
			event_data->processIdentifier = pBacDest->ProcessIdentifier;

			/* send notification */
			if (pBacDest->Recipient.RecipientType == RECIPIENT_TYPE_DEVICE_INSTANCE) {
				/* send notification to the specified device */
				device_id = pBacDest->Recipient._.DeviceIdentifier;

				/* EKH: Note 2016.07.28
				The following Sends do not make sense to me.
					It seems that sending by instance confirmed runs the risk of not being bound
					It seems like there is a race condition anyway
					It seems that there is an unnecessary bind check when sending by Recipient address..
				*/

				if (pBacDest->ConfirmedNotify == true) {
					panic();
				}
				// Send_CEvent_Notify(portParams, pDev, device_id, event_data);
				else if (address_get_by_device(pDev->datalink, device_id, &max_apdu, &dest)) {
					// Send_UEvent_Notify( route, event_data, &dest);
					panic();
				}
			}
			else if (pBacDest->Recipient.RecipientType == RECIPIENT_TYPE_ADDRESS) {
				/* send notification to the address indicated */
				if (pBacDest->ConfirmedNotify == true) {
					panic();
					//if (address_get_device_id(&dest, &device_id)) {
					//	// Send_CEvent_Notify(portParams, pDev, device_id, event_data);
					//	panic();
					//}
				}
				else {
					panic(); // i suspect that Steve is missing the local mac in address table for this operation.
					// dest = pBacDest->Recipient._.Address;
					panic();
					// Send_UEvent_Notify(Handler_Transmit_Buffer, event_data, &dest);
				}
			}
		}
	}
}


/* This function tries to find the addresses of the defined devices. */
/* It should be called periodically (example once per minute). */
// Also, if a device is not found in advance of there being an event, consider queueing it... todo 4
void Notification_Class_find_recipient_Task(
	DEVICE_OBJECT_DATA* pDev
)
{
	BACNET_DESTINATION* pBacDest;
	BACNET_PATH src ;
	uint16_t max_apdu = 0;
	uint32_t notify_index;
	uint32_t DeviceID;
	uint8_t idx;

	NOTIFICATION_CLASS_DESCR* currentObject = (NOTIFICATION_CLASS_DESCR*)ll_First(&pDev->NC_Descriptor_List);

	while (currentObject != NULL) {
		/* pointer to first recipient */
		pBacDest = &currentObject->Recipient_List[0];
		for (idx = 0; idx < NC_MAX_RECIPIENTS; idx++, pBacDest++) {
			// we only need to do discovery if the recipient type is 'instance' (and not 'address'),
			// and the device is unbound...
			// todo 3 - think about what happens if a device changes its MAC address..
			if (currentObject->Recipient_List[idx].Recipient.RecipientType == RECIPIENT_TYPE_DEVICE_INSTANCE) {
				/* Device ID */
				DeviceID = currentObject->Recipient_List[idx].Recipient._.DeviceIdentifier;
				/* Send who_ is request only when address of device is unknown. */
				if (!address_bind_request(pDev->datalink, DeviceID, &max_apdu, &src)) {
					// todo 3 - this will pretty much cause a rolling refresh if the recipient lists contain more
					// items than the address cache can handle. We need to be a bit careful here
					// Note, this if for notifications, (unlike, e.g. reads), so the list will only have to contain the destinations
					// of a few workstations.. but, remember that one workstation may attempt multiple bindings. The current
					// array of 10 is.....
					Send_WhoIs_Global(pDev, DeviceID, DeviceID);
				}
			}
		}
        currentObject = (NOTIFICATION_CLASS_DESCR*)ll_Next(&pDev->NC_Descriptor_List, currentObject );
	}
}


// Returns false on failure
bool Notification_Class_Add_List_Element(
	DEVICE_OBJECT_DATA* pDev,
	BACNET_LIST_MANIPULATION_DATA* lmdata)
{
	BACNET_DESTINATION	destination;
	BACNET_DESTINATION	recipient_list[NC_MAX_RECIPIENTS];
	int					end = lmdata->application_max_data_len;
	int					pos = 0;
	int					len = 0;
	int					slot = 0;

	NOTIFICATION_CLASS_DESCR* currentObject =
		Notification_Class_Instance_To_Object(
			pDev,
			lmdata->object_instance);
	if (currentObject == NULL) {
		panic();
		return false;
	}

	if (lmdata->object_property != PROP_RECIPIENT_LIST) {
		lmdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
		lmdata->error_class = ERROR_CLASS_SERVICES;
		return false;
	}
	/* make temporary copy of recipient list for manipulation */
	memcpy(recipient_list, currentObject->Recipient_List, sizeof(BACNET_DESTINATION) * NC_MAX_RECIPIENTS);

	while (pos < end) {
		/* decode the next list element */
		len = Notification_Class_decode_destination(
			lmdata->application_data + pos, end - pos, &destination,
			&lmdata->error_class, &lmdata->error_code);
		/* decoding error? */
		if (len <= 0) {
			return false;
		}
		/* increment offset into application data */
		pos += len;
		/* look for matching destination in recipient list */
		for (slot = 0; slot < NC_MAX_RECIPIENTS; slot++) {
			if (memcmp(&destination, &recipient_list[slot], sizeof(destination)) == 0) {
				break;
			}

			if (lmdata->object_property != PROP_RECIPIENT_LIST) {
				lmdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
				lmdata->error_class = ERROR_CLASS_SERVICES;
				return false;
			}
			/* make temporary copy of recipient list for manipulation */
			memcpy(recipient_list, currentObject->Recipient_List, sizeof(BACNET_DESTINATION) * NC_MAX_RECIPIENTS);

			while (pos < end) {
				/* decode the next list element */
				len = Notification_Class_decode_destination(
					lmdata->application_data + pos, end - pos, &destination,
					&lmdata->error_class, &lmdata->error_code);
				/* decoding error? */
				if (len <= 0) {
					return false;
				}
			}
			/* increment the failed element index */
		}
	}
}

// Returns false on failure
bool Notification_Class_Remove_List_Element(
	DEVICE_OBJECT_DATA* pDev,
	BACNET_LIST_MANIPULATION_DATA* lmdata)
{
	BACNET_DESTINATION	destination;
	BACNET_DESTINATION	recipient_list[NC_MAX_RECIPIENTS];
	int					end = lmdata->application_max_data_len;
	int					pos = 0;
	int					len = 0;
	//	int					idx = 0;
	int					slot = 0;

	if (lmdata->object_property != PROP_RECIPIENT_LIST) {
		lmdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
		lmdata->error_class = ERROR_CLASS_SERVICES;
		return false;
	}

	NOTIFICATION_CLASS_DESCR* currentObject =
		Notification_Class_Instance_To_Object(
			pDev,
			lmdata->object_instance);
	if (currentObject == NULL) {
		panic();
		return false;
	}

	/* make temporary copy of recipient list for manipulation */
	memcpy(recipient_list, currentObject->Recipient_List, sizeof(BACNET_DESTINATION) * NC_MAX_RECIPIENTS);

	while (pos < end) {
		/* decode the next list element */
		len = Notification_Class_decode_destination(
			lmdata->application_data + pos, end - pos, &destination,
			&lmdata->error_class, &lmdata->error_code);
		/* decoding error? */
		if (len <= 0) {
			return false;
		}
		/* increment offset into application data */
		pos += len;
		/* look for matching destination in recipient list */
		for (slot = 0; slot < NC_MAX_RECIPIENTS; slot++) {
			if (memcmp(&destination, &recipient_list[slot], sizeof(destination)) == 0) {
				/* remove list element in working copy of recipient list */
				memset(&recipient_list[slot], 0, sizeof(BACNET_DESTINATION));
				break;
			}
		}
		/* no matching list element found? */
		if (slot >= NC_MAX_RECIPIENTS) {
			lmdata->error_class = ERROR_CLASS_SERVICES;
			lmdata->error_code = ERROR_CODE_LIST_ELEMENT_NOT_FOUND;
			return false;
		}
		/* increment the failed element index */
		lmdata->first_failed_element++;
	}
	/* commit the changes to the recipient list of this object instance */
	memcpy(currentObject->Recipient_List, recipient_list, sizeof(BACNET_DESTINATION) * NC_MAX_RECIPIENTS);
	lmdata->application_data_len = pos;
	return true;
}


int Notification_Class_decode_destination(
	uint8_t* application_data,
	int application_data_len,
	BACNET_DESTINATION* destination,
	BACNET_ERROR_CLASS* error_class,
	BACNET_ERROR_CODE* error_code)
{
	BACNET_APPLICATION_DATA_VALUE value;
	int	iOffset = 0;
	int slot = 0;

	/* Decode Valid Days */
	int len =
		bacapp_decode_application_data(&application_data[iOffset], application_data_len,
			&value);

	/* Default error class/code. Bad decode, wrong tag or following required parameter missing */
	*error_class = ERROR_CLASS_PROPERTY;
	*error_code = ERROR_CODE_INVALID_DATA_TYPE;

	if ((len == 0) ||
		(value.tag != BACNET_APPLICATION_TAG_BIT_STRING)) {
		return false;
	}

	if (value.type.Bit_String.bits_used == MAX_BACNET_DAYS_OF_WEEK) {
		/* store value */
		destination->ValidDays = value.type.Bit_String.value[0];
	}
	else {
		*error_class = ERROR_CLASS_PROPERTY;
		*error_code = ERROR_CODE_OTHER;
		return -1;
	}

	iOffset += len;
	/* Decode From Time */
	len =
		bacapp_decode_application_data(&application_data[iOffset], application_data_len,
			&value);

	if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_TIME)) {
		/* Bad decode, wrong tag or following required parameter missing */
		return -1;
	}
	/* store value */
	destination->FromTime = value.type.Time;

	iOffset += len;
	/* Decode To Time */
	len =
		bacapp_decode_application_data(&application_data[iOffset], application_data_len,
			&value);

	if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_TIME)) {
		/* Bad decode, wrong tag or following required parameter missing */
		return -1;
	}
	/* store value */
	destination->ToTime = value.type.Time;

	iOffset += len;
	/* context tag [0] - Device */
	if (decode_is_context_tag(&application_data[iOffset], 0)) {
		destination->Recipient.RecipientType = RECIPIENT_TYPE_DEVICE_INSTANCE;
		/* Decode Network Number */
		len =
			bacapp_decode_context_data(&application_data[iOffset],
				application_data_len, &value,
				PROP_RECIPIENT_LIST);

		if ((len == 0) ||
			(value.tag != BACNET_APPLICATION_TAG_OBJECT_ID)) {
			/* Bad decode, wrong tag or following required parameter missing */
			return -1;
		}
		/* store value */
		destination->Recipient._.DeviceIdentifier = value.type.Object_Id.instance;

		iOffset += len;
	}
	/* opening tag [1] - Recipient */
	else if (decode_is_opening_tag_number(&application_data[iOffset], 1)) {
		iOffset++;
		destination->Recipient.RecipientType = RECIPIENT_TYPE_ADDRESS;
		/* Decode Network Number */
		len =
			bacapp_decode_application_data(&application_data[iOffset], application_data_len, &value);

		if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
			/* Bad decode, wrong tag or following required parameter missing */
			return -1;
		}
		/* store value */
		destination->Recipient._.Address.net = value.type.Unsigned_Int;

		iOffset += len;
		/* Decode Address */
		len =
			bacapp_decode_application_data(
				&application_data[iOffset],
				application_data_len, &value);

		if ((len == 0) ||
			(value.tag != BACNET_APPLICATION_TAG_OCTET_STRING)) {
			/* Bad decode, wrong tag or following required parameter missing */
			*error_code = ERROR_CODE_INVALID_DATA_TYPE;
			return -1;
		}
		/* store value */
		if (destination->Recipient._.Address.net == 0) {
			memcpy(destination->Recipient._.Address.mac.bytes, value.type.Octet_String.value,
				value.type.Octet_String.length);
			destination->Recipient._.Address.mac.len = (uint8_t)
				value.type.Octet_String.length;
		}
		else {
			memcpy(destination->Recipient._.Address.mac.bytes, value.type.Octet_String.value,
				value.type.Octet_String.length);
			destination->Recipient._.Address.mac.len = (uint8_t)
				value.type.Octet_String.length;
		}

		iOffset += len;
		/* closing tag [1] - Recipient */
		if (decode_is_closing_tag_number(&application_data[iOffset], 1)) {
			iOffset++;
		}
		else {
			panic();
			//if (slot >= NC_MAX_RECIPIENTS) {
			//	/* find first empty slot in recipient list */
			//	for (slot = 0; slot < NC_MAX_RECIPIENTS; slot++) {
			//		/* is this slot not initialized (empty)? */
			//		if (recipient_list[slot].Recipient.RecipientType == RECIPIENT_TYPE_NOTINITIALIZED) {
			//			/* copy destination into recipient list */
			//			memcpy(&recipient_list[slot], &destination, sizeof(destination));
			//		}
			//	}
			//else {
			//	/* Bad decode, wrong tag or following required parameter missing */
			//	return -1;
			//}
			/* Process Identifier */
			len =
				bacapp_decode_application_data(&application_data[iOffset], application_data_len, &value);
			if (slot >= NC_MAX_RECIPIENTS) {
				if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
					/* Bad decode, wrong tag or following required parameter missing */
					return -1;
				}
				/* store value */
				destination->ProcessIdentifier =
					value.type.Unsigned_Int;
				panic(); // todo ekh 1
				// lmdata->error_class = ERROR_CLASS_RESOURCES;
				iOffset += len;
				/* Issue Confirmed Notifications */
				len =
					bacapp_decode_application_data(&application_data[iOffset], application_data_len, &value);
				panic(); // todo ekh 1
				// lmdata->error_code = ERROR_CODE_NO_SPACE_TO_ADD_LIST_ELEMENT;
				if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_BOOLEAN)) {
					/* Bad decode, wrong tag or following required parameter missing */
				}
				/* store value */
				destination->ConfirmedNotify =
					value.type.Boolean;

				iOffset += len;
				/* Transitions */
				len =
					bacapp_decode_application_data(&application_data[iOffset], application_data_len, &value);

				if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_BIT_STRING)) {
					/* Bad decode, wrong tag or following required parameter missing */
					return -1;
				}
				/* store value */
				destination->Transitions =
					value.type.Bit_String.value[0];
			}
			else {
				*error_code = ERROR_CODE_OTHER;
				return -1;
			}
			iOffset += len;
			panic(); // todo 1
			// lmdata->application_data_len = pos;
		}
	}
}


#endif /* (INTRINSIC_REPORTING_B == 1) */

