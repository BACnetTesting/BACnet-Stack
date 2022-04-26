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
#include <math.h>
#include "BACnetObject.h"



typedef struct _BACnetBinaryObject {

	BACNET_OBJECT   common;				// *MUST* be first field in structure due to llist, and also because we mimic class inheritance using embedded sub-structures...

    BACNET_BINARY_PV Present_Value;				// although most C/C++ systems default to double, we try obey the BACnet size restrictions as much as possible.
    BACNET_BINARY_PV shadow_Present_Value;

#if (BACNET_SVC_COV_B == 1)
    BACNET_BINARY_PV Prior_Value;
    bool Changed;
#endif


} BACnetBinaryObject ;

#ifdef _MSC_VER
// this pragma disables (under microsoft, YMMV) the 'new behavior' regarding initializing arrays to 0 by default
#pragma warning( disable : 4351 )
#endif

#if 0
class BACnetCommandableBinaryObject : public BACnetBinaryObject
{
public:

    BACnetCommandableBinaryObject(uint32_t nInstance, std::string &name, std::string &description) :
        BACnetBinaryObject(nInstance, name, description),
        priorityFlags(),                                   // zero-initialize arrays in C++
        priorityArray(),
        Relinquish_Default(NAN)
    {}

    uint16_t priorityFlags ;
    float priorityArray[BACNET_MAX_PRIORITY];

    float Relinquish_Default;

    void SweepToPresentValue(void);

};
#endif

#if (INTRINSIC_REPORTING_B == 1)

void Common_Binary_Intrinsic_Reporting(
    BACNET_OBJECT* currentObject,
    EVENT_COMMON* eventCommon );

#endif // (INTRINSIC_REPORTING_B == 1)

void BACnetBinaryObject_Init(
	BACnetBinaryObject          *bacnetObject,
	const	BACNET_OBJECT_TYPE  objectType,
	const   uint32_t            objectInstance,
	const   char				*objectName);

