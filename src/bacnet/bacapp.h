/**************************************************************************
 *
 * Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef BACAPP_H
#define BACAPP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bacnet/bacdef.h"
#include "bacstr.h"
#include "datetime.h"
#if defined (BACAPP_LIGHTING_COMMAND)
#include "lighting.h"
#endif
#if defined (BACAPP_DEVICE_OBJECT_PROP_REF)
#include "bacdevobjpropref.h"
#endif


struct BACnet_Application_Data_Value;

typedef struct BACnet_Application_Data_Value {

    bool context_specific;          /* true if context specific data */
    uint8_t context_tag;            /* only used for context specific data */
    BACNET_APPLICATION_TAG tag;     /* application tag data type */

    union {
        /* NULL - not needed as it is encoded in the tag alone */
#if defined (BACAPP_BOOLEAN)
        bool Boolean;
#endif
#if defined (BACAPP_UNSIGNED)
        BACNET_UNSIGNED_INTEGER Unsigned_Int;
#endif
#if defined (BACAPP_SIGNED)
        int32_t Signed_Int;
#endif
#if defined (BACAPP_REAL)
        float Real;
#endif
#if defined (BACAPP_DOUBLE)
        double Double;
#endif
#if defined (BACAPP_OCTET_STRING)
        BACNET_OCTET_STRING Octet_String;
#endif
#if defined (BACAPP_CHARACTER_STRING)
        BACNET_CHARACTER_STRING Character_String;
#endif
#if defined (BACAPP_BIT_STRING)
        BACNET_BIT_STRING Bit_String;
#endif
#if defined (BACAPP_ENUMERATED)
        uint32_t Enumerated;
#endif
#if defined (BACAPP_DATE)
        BACNET_DATE Date;
#endif
#if defined (BACAPP_TIME)
        BACNET_TIME Time;
#endif
#if defined (BACAPP_OBJECT_ID)
        BACNET_OBJECT_ID Object_Id;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
        BACNET_LIGHTING_COMMAND Lighting_Command;
#endif
#if defined (BACAPP_DEVICE_OBJECT_PROP_REF)
        BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE
            Device_Object_Property_Reference;
#endif
    } type;
    /* simple linked list if needed */
    struct BACnet_Application_Data_Value *next;
} BACNET_APPLICATION_DATA_VALUE;


struct BACnet_Access_Error;
typedef struct BACnet_Access_Error {
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
} BACNET_ACCESS_ERROR;


struct BACnet_Property_Reference;
typedef struct BACnet_Property_Reference {
    BACNET_PROPERTY_ID propertyIdentifier;
    /* optional array index */
    BACNET_ARRAY_INDEX propertyArrayIndex;
    /* either value or error, but not both.
       Use NULL value to indicate error */
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_ACCESS_ERROR error;
    /* simple linked list */
    struct BACnet_Property_Reference *next;
} BACNET_PROPERTY_REFERENCE;

struct BACnet_Property_Value;
typedef struct BACnet_Property_Value {
    BACNET_PROPERTY_ID propertyIdentifier;
    BACNET_ARRAY_INDEX propertyArrayIndex;
    BACNET_APPLICATION_DATA_VALUE value;
    uint8_t priority;
    /* simple linked list */
    struct BACnet_Property_Value *next;
} BACNET_PROPERTY_VALUE;

/* used for printing values */
struct BACnet_Object_Property_Value;
typedef struct BACnet_Object_Property_Value {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    BACNET_PROPERTY_ID object_property;
    BACNET_ARRAY_INDEX array_index;
    BACNET_APPLICATION_DATA_VALUE *value;
} BACNET_OBJECT_PROPERTY_VALUE;

int bacapp_encode_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value);
    
BACNET_STACK_EXPORT
int bacapp_decode_data(
    const uint8_t * apdu,
    uint8_t tag_data_type,
    uint32_t len_value_type,
    BACNET_APPLICATION_DATA_VALUE * value);

int bacapp_decode_application_data(
    const uint8_t * apdu,
    const uint16_t max_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value);

bool bacapp_decode_application_data_safe(
    uint8_t * new_apdu,
    uint32_t new_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value);

int bacapp_encode_application_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value);

int bacapp_decode_context_data(
    const uint8_t * apdu,
    const uint16_t max_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_PROPERTY_ID property);

int bacapp_encode_context_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_PROPERTY_ID property);

int bacapp_encode_context_data_value(
    uint8_t * apdu,
    uint8_t context_tag_number,
    BACNET_APPLICATION_DATA_VALUE * value);

BACNET_APPLICATION_TAG bacapp_context_tag_type(
    BACNET_PROPERTY_ID property,
    uint8_t tag_number);

bool bacapp_copy(
    BACNET_APPLICATION_DATA_VALUE * dest_value,
    BACNET_APPLICATION_DATA_VALUE * src_value);

/* returns the length of data between an opening tag and a closing tag.
   Expects that the first octet contain the opening tag.
   Include a value property identifier for context specific data
   such as the value received in a WriteProperty request */
int bacapp_data_len(
    const uint8_t * apdu,
    const uint16_t max_apdu_len,
    BACNET_PROPERTY_ID property);
    
BACNET_STACK_EXPORT
int bacapp_decode_data_len(
    uint8_t tag_data_type,
    uint32_t len_value_type);
    
BACNET_STACK_EXPORT
int bacapp_decode_application_data_len(
    const uint8_t * apdu,
    const uint16_t max_apdu_len);
    
BACNET_STACK_EXPORT
int bacapp_decode_context_data_len(
    const uint8_t * apdu,
    const uint16_t max_apdu_len,
    BACNET_PROPERTY_ID property);

// todo 3 remove 
#define BACAPP_PRINT_ENABLED    1
#define BACAPP_SNPRINTF_ENABLED 1

#if ( BACAPP_PRINT_ENABLED != 1 )
#if PRINT_ENABLED || defined TEST
#define BACAPP_PRINT_ENABLED    1
#define BACAPP_SNPRINTF_ENABLED
#endif
#endif

#ifdef BACAPP_SNPRINTF_ENABLED
int bacapp_snprintf_value(
    char *str,
    uint16_t str_len,
    BACNET_OBJECT_PROPERTY_VALUE * object_value);
#endif

#ifdef BACAPP_PRINT_ENABLED
    BACNET_STACK_EXPORT
    bool bacapp_parse_application_data(
        BACNET_APPLICATION_TAG tag_number,
        const char *argv,
        BACNET_APPLICATION_DATA_VALUE * value);
        
    BACNET_STACK_EXPORT
    bool bacapp_print_value(
        FILE * stream,
        BACNET_OBJECT_PROPERTY_VALUE * value);
#else
/* Provide harmless return values */
#define bacapp_parse_application_data(x,y,z)   false
#define bacapp_print_value(x,y) 			   false
#endif

#ifdef BAC_TEST
#include "ctest.h"
#include "bacnet/datetime.h"
    BACNET_STACK_EXPORT
    bool bacapp_same_value(
        BACNET_APPLICATION_DATA_VALUE * value,
        BACNET_APPLICATION_DATA_VALUE * test_value);

    BACNET_STACK_EXPORT
    void testBACnetApplicationDataLength(
        Test * pTest);
    BACNET_STACK_EXPORT
    void testBACnetApplicationData(
        Test * pTest);
#endif

#endif
