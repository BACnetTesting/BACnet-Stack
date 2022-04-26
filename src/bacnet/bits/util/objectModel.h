/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
* 
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
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

#pragma once

#include <stdint.h>
#include "bacnet/bacenum.h"
#include "bacnet/basic/object/device.h"

#if 0
void BACnet_Create_Object(
    const uint32_t					deviceInstance,
    const BACNET_OBJECT_TYPE		objectType,
    const uint32_t					objectInstance,
    const char 						*name,
    const char 						*description,
    const BACNET_ENGINEERING_UNITS 	units);
#endif

bool MultiState_Input_Create(
    DEVICE_OBJECT_DATA *pDev,
    const uint32_t instance,
    const char* name,
    const void *userObjectData
);

bool Analog_Input_Create(
    DEVICE_OBJECT_DATA *pDev,
    const uint32_t instance,
    const char *name,
    const BACNET_ENGINEERING_UNITS units,
    const void *userObjectData
);