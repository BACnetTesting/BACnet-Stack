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

#include "bacnet/bits/util/BACnetObjectBinary.h"
#include "appApi.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#endif

static float Get_PV_real(const void *ptrToBACnetObject )
{
	BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;

	return (float) currentObject->Present_Value;
}


static bool Set_PV_real(void* ptrToBACnetObject, const float value )
{
    BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;
  
    currentObject->Present_Value = (BACNET_BINARY_PV) (uint) value;
    return true; 
}

static uint Get_PV_unsigned(const void *ptrToBACnetObject )
{
	BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;

	return (uint) currentObject->Present_Value;
}


static bool Set_PV_unsigned(void* ptrToBACnetObject, const uint value )
{
    BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;
    
    currentObject->Present_Value = (BACNET_BINARY_PV) value;

    return true; 
}

static bool Get_PV_boolean(const void* ptrToBACnetObject)
{
    BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;

    return (bool)currentObject->Present_Value;
}


static bool Set_PV_boolean(void* ptrToBACnetObject, const bool value)
{
    BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;

    currentObject->Present_Value = (BACNET_BINARY_PV) value;

    return true;
}

static bool Set_PV_adv(void* ptrToBACnetObject, const BACNET_APPLICATION_DATA_VALUE *value)
{
    BACnetBinaryObject* currentObject = (BACnetBinaryObject*)ptrToBACnetObject;

    currentObject->Present_Value = (BACNET_BINARY_PV) BACapi_Get_adv_unsigned ( value ) ;

    return true;
}


void BACnetBinaryObject_Init(
    BACnetBinaryObject			*currentObject,
    const	BACNET_OBJECT_TYPE	objectType,
	const   uint32_t			objectInstance,
	const   char				*objectName)
{
	Generic_Object_Init(
        &currentObject->common,
        objectType,
		objectInstance,
		objectName);

    // 2021-11-09 below are initialized in Getneri_Object_Init()
 //   currentObject->common.GetPresentValue_real = Get_PV_real;
 //   currentObject->common.SetPresentValue_real = Set_PV_real;
	//currentObject->common.GetPresentValue_unsigned = Get_PV_unsigned;
 //   currentObject->common.SetPresentValue_unsigned = Set_PV_unsigned;
 //   currentObject->common.GetPresentValue_boolean = Get_PV_boolean;
 //   currentObject->common.SetPresentValue_boolean = Set_PV_boolean;
 //   currentObject->common.SetPresentValue_adv = Set_PV_adv;

	currentObject->Present_Value = BINARY_INACTIVE ;

#if (INTRINSIC_REPORTING_B == 1)

	/* initialize Event time stamps using wildcards and set Acked_transitions */
	for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
		datetime_wildcard_set(&currentObject->common.eventCommon.Event_Time_Stamps[j]);
		currentObject->common.eventCommon.Acked_Transitions[j].bIsAcked = true;
	}

#endif  // (INTRINSIC_REPORTING_B == 1)

#if ( BACNET_SVC_COV_B == 1 )
	currentObject->Changed = false;
	currentObject->Prior_Value = BINARY_INACTIVE;
#endif  // ( BACNET_SVC_COV_B == 1 )

}


#if (INTRINSIC_REPORTING_B == 1)

void Common_Binary_Intrinsic_Reporting(
    BACNET_OBJECT* currentObject,
    EVENT_COMMON* eventCommon)
{
        // todo 2 - implement
}

#endif  // (INTRINSIC_REPORTING_B == 1)

