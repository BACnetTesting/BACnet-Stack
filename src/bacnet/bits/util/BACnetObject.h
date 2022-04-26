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

#pragma once

#include <stdint.h>
#include "eLib/util/llist.h"
//#include "bacstr.h"
//// #include "bacApp.h"
#include "bacnet/wp.h"
#include "bacnet/rp.h"
// #include "bacnet/basic/object/device.h"


#if (INTRINSIC_REPORTING_B == 1)

/* Indicates whether the transaction has been confirmed */
typedef struct Acked_info {
	bool bIsAcked;  /* true when transitions is acked */
	BACNET_DATE_TIME Time_Stamp;    /* time stamp of when a alarm was generated */
} ACKED_INFO;

/* Information needed to send AckNotification */
typedef struct Ack_Notification {
	bool                bSendAckNotify;    /* true if need to send AckNotification */
	BACNET_EVENT_STATE  EventState;
} ACK_NOTIFICATION;

#endif


#if (INTRINSIC_REPORTING_B == 1)

typedef struct
{
	unsigned			Event_Enable : 3;
	uint32_t			Time_Delay;
	uint32_t			Notification_Class;
	BACNET_NOTIFY_TYPE	Notify_Type;
	ACKED_INFO			Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
	BACNET_DATE_TIME	Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
	/* time to generate event notification */
	uint32_t			Remaining_Time_Delay;
	/* AckNotification informations */
	ACK_NOTIFICATION	Ack_notify_data;
} EVENT_COMMON;
#endif


typedef struct
{
	LLIST_LB                llist;                      // must be first

    DEVICE_OBJECT_DATA      *pDev2;                      // the Virtual Device that this object belongs to
    
	BACNET_OBJECT_TYPE		objectType;
	uint32_t                objectInstance;
	BACNET_CHARACTER_STRING objectName;
	BACNET_CHARACTER_STRING description;

	bool Out_Of_Service;
	BACNET_RELIABILITY reliability;
	BACNET_RELIABILITY shadowReliability;
	BACNET_EVENT_STATE Event_State;						// Event_State is a required property, INTRINSIC_REPORTING selected or not...

#if ( BACNET_SVC_COV_B == 1 )
    bool priorOutOfService;
#endif

#if (INTRINSIC_REPORTING_B == 1)
	EVENT_COMMON	eventCommon;
#endif
    
    // 'real' is BACnet terminology for single precision 'float'. 
	// 2021-11-09 These functions to migrate to the terminology shown below
// renamed StackUpdate_real float (*GetPresentValue_real)(const void *ptrToBACnetObject);
    bool (*SetPresentValue_real)(void* ptrToBACnetObject, const float value);
    uint32_t (*GetPresentValue_unsigned)(const void* ptrToBACnetObject);
    bool (*SetPresentValue_unsigned)(void* ptrToBACnetObject, const uint32_t value);
    bool (*SetPresentValue_adv)(void* ptrToBACnetObject, const BACNET_APPLICATION_DATA_VALUE *value);
    bool (*GetPresentValue_boolean)(const void* ptrToBACnetObject);
    bool (*SetPresentValue_boolean)(void* ptrToBACnetObject, const bool value);

	/*
		Data operations.    Many TBD

		Server-side Write:  e.g. BACnet Client (Workstation) writes to our BACnet stack
		Server-side Read    e.g. BACnet Client (Workstation) reads from our BACnet stack
		Client-side Write   our BACnet stack writes to another box
		Client-side Read    our BACNet stack reads from another box
		Internal Write      our BACnet stack writes to our application (or hardware)
		Internal Update     our BACnet stack reads from our application (or hardware) (may be a null operation)
		Application Write   our Application writes to our BACnet stack
		Application Read    our Application reads from our BACnet stack
	*/
	void (*InternalUpdate_real) (float *value);
	void (*InternalUpdate_Reliability) (BACNET_RELIABILITY* reliability);

    void *userData;

} BACNET_OBJECT;


BACNET_OBJECT* Generic_Instance_To_Object(
	const LLIST_HDR* objectHdr,
	const uint32_t object_instance);

bool Generic_Instance_To_Object_Name(
	LLIST_HDR* objectHdr,
	uint32_t objectInstance,
	BACNET_CHARACTER_STRING* object_name);

void Generic_Object_Init(
	BACNET_OBJECT* bacnetObject,
	const BACNET_OBJECT_TYPE	objectType,
	const uint32_t				objectInstance,
	const char* objectName);

uint32_t Generic_Index_To_Instance(
	LLIST_HDR* objectHdr,
	uint32_t objectIndex);

BACNET_OBJECT* Generic_Index_To_Object(
	LLIST_HDR* objectHdr,
	uint32_t objectIndex);

BACNET_OBJECT* BACnet_Find_Object_from_DevPtr(
    const DEVICE_OBJECT_DATA* pDev,
    const BACNET_OBJECT_TYPE objType,
    const uint32_t objInstance);

BACNET_OBJECT* BACnet_Find_Object(
    const uint32_t deviceInstance, 
    const BACNET_OBJECT_TYPE objectType, 
    const uint32_t objectInstance);

BACNET_CHARACTER_STRING* BACnetObject_Description_Get(
	BACNET_OBJECT* bacnetCommon);

int encode_application_bitstring3(uint8_t* apdu, const bool one, const bool two, const bool three);

int encode_application_bitstring2(uint8_t* apdu, const bool one, const bool two);

int encode_status_flags(uint8_t* apdu,
	bool alarm,
	bool fault,
	bool oos);

int encode_application_event_time_stamps(
	uint8_t* apdu,
	BACNET_READ_PROPERTY_DATA* rpdata,
	BACNET_DATE_TIME* Event_TimeStamps);

bool WP_ValidateTagType(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	BACNET_APPLICATION_TAG expectedTag);

bool WP_ValidateEnumType(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue);

bool WP_ValidateRangeReal(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	double low,
	double high);

bool WP_ValidateRangeUnsignedInt(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	uint32_t low,
	uint32_t high);

bool WP_ValidateEnumTypeAndRange(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	uint16_t    maxRange);

bool WP_StorePropertyFloat(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	float* tValue);

bool WP_StorePropertyEnumRanged(
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* pValue,
	uint16_t    maxRange,
	int* tValue);

bool Common_Reliability_Set(
	BACNET_OBJECT* currentObject,
	BACNET_WRITE_PROPERTY_DATA* wp_data,
	BACNET_APPLICATION_DATA_VALUE* value);

BACNET_RELIABILITY Common_Reliability_Get(
    BACNET_OBJECT* currentObject);

