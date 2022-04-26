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

#ifdef __GNUC__
#include <math.h>       // For NAN
#endif
#include "configProj.h"

#include "bacnet/bacenum.h"
#include "stdint.h"
#include <string>
#include <cmath>
#include "BACnetObject.h"
#include "bacnet/basic/object/nc.h"

#define RELINQUISH_DEFAULT_MULTITSTATE  1

#if (INTRINSIC_REPORTING_B == 1)

// EVENT_COMMON_objecttype not required for Binary or Multistate objects
#endif


typedef struct _BACnetMultistateObject {
	BACNET_OBJECT   common;				// *MUST* be first field in structure due to llist, and also because we mimic class inheritance using embedded sub-structures...
    uint16_t Present_Value;
    uint16_t shadow_Present_Value;

#if (BACNET_SVC_COV_B == 1)
    uint16_t Prior_Value;
    bool Changed;
#endif
    
    char State_Text[MULTISTATE_NUMBER_OF_STATES][64];
    
    // event state is always required, intrinsic reporting or not...
    BACNET_EVENT_STATE Event_State;
    
#if (INTRINSIC_REPORTING_B == 1)
#endif

} BACnetMultistateObject ;


#ifdef _MSC_VER
// this pragma disables (under microsoft, YMMV) the 'new behavior' regarding initializing arrays to 0 by default
#pragma warning( disable : 4351 )
#endif


void BACnetMultistateObject_Init(
	BACnetMultistateObject          *bacnetObject,
	const	BACNET_OBJECT_TYPE  objectType,
	const   uint32_t            objectInstance,
	const   char						*objectName);


#if (INTRINSIC_REPORTING_B == 1)

void Common_Multistate_Intrinsic_Reporting(
    BACNET_OBJECT* currentObject,
    EVENT_COMMON* eventCommon);

#endif // (INTRINSIC_REPORTING_B == 1)

