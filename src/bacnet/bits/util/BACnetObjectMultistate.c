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

#include "configProj.h"

#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )

#include "BACnetObjectMultistate.h"

#if (INTRINSIC_REPORTING_B == 1)
#include "nc.h"
#endif

static float GetFloatPV(const void *ptrToBACnetObject )
{
	BACnetMultistateObject* currentObject = (BACnetMultistateObject*)ptrToBACnetObject;

	return (float) currentObject->Present_Value;
}


void BACnetMultistateObject_Init(
	BACnetMultistateObject			*currentObject,
	const	BACNET_OBJECT_TYPE	objectType,
	const   uint32_t			objectInstance,
	const   char						*objectName)
{
	Generic_Object_Init(
		&currentObject->common,
		objectType,
		objectInstance,
		objectName);

	// 2021-11-09 below are initialized in Getneri_Object_Init()
//	currentObject->common.GetPresentValue_real = GetFloatPV;


	currentObject->Present_Value = 1;

#if (INTRINSIC_REPORTING_B == 1)

	/* initialize Event time stamps using wildcards and set Acked_transitions */
	for (int j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
		datetime_wildcard_set(&currentObject->common.eventCommon.Event_Time_Stamps[j]);
		currentObject->common.eventCommon.Acked_Transitions[j].bIsAcked = true;
	}

#endif  // (INTRINSIC_REPORTING_B == 1)

#if ( BACNET_SVC_COV_B == 1 )
	currentObject->Changed = false;
	currentObject->Prior_Value = 1;
#endif  // ( BACNET_SVC_COV_B == 1 )

}


#if (INTRINSIC_REPORTING_B == 1)

void Common_Multistate_Intrinsic_Reporting(
    BACNET_OBJECT* currentObject,
    EVENT_COMMON* eventCommon)
{
}

#endif  // (INTRINSIC_REPORTING_B == 1)

#endif // #if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1) || (BACNET_USE_OBJECT_MULTISTATE_VALUE == 1 )
