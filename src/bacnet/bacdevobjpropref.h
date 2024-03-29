/**************************************************************************
 *
 * Copyright (C) 2008 John Minack
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
 *   July 1, 2017    BITS    Modifications to this file have been made in compliance
 *                           with original licensing.
 *
 *   This file contains changes made by BACnet Interoperability Testing
 *   Services, Inc. These changes are subject to the permissions,
 *   warranty terms and limitations above.
 *   For more information: info@bac-test.com
 *   For access to source code:  info@bac-test.com
 *          or      www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#ifndef _BAC_DEV_PROP_REF_H_
#define _BAC_DEV_PROP_REF_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacenum.h"

typedef struct BACnetDeviceObjectPropertyReference {
    BACNET_OBJECT_ID objectIdentifier;
    BACNET_PROPERTY_ID propertyIdentifier;
    uint32_t arrayIndex;                                    // Optional - if omitted and target is an array, then whole array is addressed
    BACNET_OBJECT_ID deviceIdentifier;                      // Optional - only if target object is in another device. 
} BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE;

/** BACnetDeviceObjectReference structure.
 * If the optional deviceIdentifier is not provided, then this refers
 * to an object inside this Device.
 */
typedef struct BACnetDeviceObjectReference {
    BACNET_OBJECT_ID deviceIdentifier;         /**< Optional, for external device. */
    BACNET_OBJECT_ID objectIdentifier;
} BACNET_DEVICE_OBJECT_REFERENCE;



int bacapp_encode_device_obj_property_ref(
    uint8_t *apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *value);

int bacapp_encode_context_device_obj_property_ref(
    uint8_t *apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *value);

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1) || (BACNET_USE_OBJECT_TRENDLOG == 1) || (BACNET_USE_OBJECT_SCHEDULE == 1)
int bacapp_decode_device_obj_property_ref(
    uint8_t *apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *value);
#endif

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT == 1) || (BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1)
int bacapp_decode_context_device_obj_property_ref(
    uint8_t *apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *value);
#endif

int bacapp_encode_device_obj_ref(
    uint8_t *apdu,
    BACNET_DEVICE_OBJECT_REFERENCE *value);

int bacapp_encode_context_device_obj_ref(
    uint8_t *apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_REFERENCE *value);

int bacapp_decode_device_obj_ref(
    uint8_t *apdu,
    BACNET_DEVICE_OBJECT_REFERENCE *value);

int bacapp_decode_context_device_obj_ref(
    uint8_t *apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_REFERENCE *value);

#ifdef TEST
#include "ctest.h"
    void testBACnetDeviceObjectPropertyReference(
        Test * pTest);
#endif

#endif
