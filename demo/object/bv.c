/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"     /* the custom stuff */

#if (BACNET_USE_OBJECT_BINARY_VALUE == 1 )

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "debug.h"
#include "wp.h"
#include "rp.h"
#include "handlers.h"

#include "bv.h"
#include "bitsDebug.h"


/* These three arrays are used by the ReadPropertyMultiple handler */
static const BACNET_PROPERTY_ID Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    PROP_RELIABILITY,
    MAX_BACNET_PROPERTY_ID
};

static const BACNET_PROPERTY_ID Properties_Proprietary[] = {
    MAX_BACNET_PROPERTY_ID
};

std::vector<BACnetObject *> binaryValues;


bool Binary_Value_Create(
    const uint32_t instance,
    const std::string& nameRoot)
{
    std::string name = nameRoot;

    BinaryValueObject *newObject = new BinaryValueObject(
        instance,
        name,
        name);

    binaryValues.push_back(newObject);
    return true;
}


void Binary_Value_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary)
{
    if (pRequired) {
        // Do NOT be tempted to use property_list_required() - that is for supporting epics.c, and perhaps other Client operations, only
        *pRequired = Properties_Required;
    }
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;
}


// once per device
void Binary_Value_Init(
    void)
{
}


void BinaryValueObject::Init(void)
{
    //    unsigned i;
}
    

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Value_Valid_Instance(
    uint32_t object_instance)
{
    return BACnetObject::Valid_Instance(&binaryValues, object_instance);
}


/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Value_Count(
    void)
{
    return binaryValues.size();
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Value_Index_To_Instance(
    unsigned index)
{
    // todo1 - can we catch out-of-bounds?
    return  binaryValues[index]->instance;
}


/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
int Binary_Value_Instance_To_Index(
    uint32_t object_instance)
{
    for (size_t i = 0; i < binaryValues.size(); i++) {
        if (object_instance == binaryValues[i]->instance) {
            return i;
        }
    }
    // don't panic here, it is valid in some circumstances (e.g. Who-Has) to NOT find the index
    return -1;
}


BinaryValueObject *Binary_Value_Instance_To_Object(
    uint32_t object_instance)
{
    return static_cast<BinaryValueObject *> (BACnetObject::Instance_To_Object(&binaryValues, object_instance));
}


void Binary_Value_Update (
		const uint32_t instance,
		const bool value )
{
	BinaryValueObject *bacnetObject = Binary_Value_Instance_To_Object ( instance ) ;
	if ( bacnetObject == NULL )
	{
		panic();
		return;
	}
	bacnetObject->Present_Value = ( value ) ? BINARY_ACTIVE : BINARY_INACTIVE ;
}


BACNET_BINARY_PV Binary_Value_Present_Value(
    BinaryValueObject *currentObject)
{
    return currentObject->Present_Value;

#if 0
    // todo, fix the priority array access
    BACNET_BINARY_PV value = RELINQUISH_DEFAULT_BINARY;
    unsigned index = 0;
    unsigned i = 0;

    index = Binary_Value_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_VALUES) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Binary_Value_Level[index][i] != BINARY_NULL) {
                value = Binary_Value_Level[index][i];
                break;
            }
        }
    }

    return value;
#endif
}


// todo 3 move to the generic module
BACNET_BINARY_PV SweepToPresentValue(BinaryValueObject *currentObject)
{
    BACNET_BINARY_PV tvalue = RELINQUISH_DEFAULT_BINARY ;

    for (int i = 0; i < BACNET_MAX_PRIORITY; i++) {
        if ( currentObject->prioritySet[i]) {
            tvalue = currentObject->priorityValues[i];
            break;
        }
    }

    return tvalue;
}


bool Binary_Value_Present_Value_Set(
    BinaryValueObject *currentObject,
    BACnet_Write_Property_Data *wp_data,
    BACNET_APPLICATION_DATA_VALUE *value )
{
    uint8_t priority = wp_data->priority;
    // float currentPV = currentObject->Present_Value;

    /*  BTC todo - 19.2.3 When a write to a commandable property occurs at any priority, the specified value or relinquish (NULL) is always written to
        the appropriate slot in the priority table, regardless of any minimum on or off times.
        Actually: NOT ALLOWED to write NULL to 6
        */

    if (value->tag == BACNET_APPLICATION_TAG_ENUMERATED) {
        /* Command priority 6 is reserved for use by Minimum On/Off
        algorithm and may not be used for other purposes in any
        object. */
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */)) {

            // new as of 2016.09.20 
            currentObject->priorityValues[priority - 1] = (BACNET_BINARY_PV) value->type.Enumerated;
            currentObject->prioritySet[priority - 1] = true;
            SweepToPresentValue( currentObject );
        } else if (priority == 6) {
            /* Command priority 6 is reserved for use by Minimum On/Off
            algorithm and may not be used for other purposes in any
            object. */
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        } else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        }
    } else {
        // This is the relinquish case (If the value is a NULL )
        bool status =
            WPValidateArgType( value, BACNET_APPLICATION_TAG_NULL,
                               &wp_data->error_class, &wp_data->error_code);
        if (status) {
            if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                // We are writing a NULL to the priority array...
                currentObject->prioritySet[priority-1] = false;
                SweepToPresentValue(currentObject);
                // todo1 transfer to application layer here
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                return false;
            }
        }
    }
    return true;
}


bool Binary_Value_Present_Value_Relinquish(
    uint32_t object_instance,
    unsigned priority)
{

    return true ;
}


/* note: the object name must be unique within this device */
bool Binary_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    int index;
    bool status = false;

    index = Binary_Value_Instance_To_Index(object_instance);
    if (index >= 0) {
        characterstring_init_ansi(object_name, static_cast<BinaryValueObject *>(binaryValues[index])->name.c_str());
    }

    return status;
}


bool Binary_Value_Out_Of_Service(
    BACnetObject *currentObject)
{
    return currentObject->Out_Of_Service;
}

void Binary_Value_Out_Of_Service_Set(
    BinaryValueObject *currentObject,
    const bool value)
{
    if (currentObject->Out_Of_Service != value) {
        currentObject->Changed = true;
    }
    currentObject->Out_Of_Service = value;
}

/* return apdu length, or BACNET_STATUS_ERROR on error */
int Binary_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BinaryValueObject *currentObject;
    uint8_t *apdu = NULL;

    // todo1 const BACNET_PROPERTY_ID *pRequired = NULL, *pOptional = NULL, *pProprietary = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    currentObject = Binary_Value_Instance_To_Object(rpdata->object_instance);
    if (currentObject == NULL) {
        // todo1 panic();
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0], OBJECT_BINARY_VALUE,
                rpdata->object_instance);
        break;
        /* note: Name and Description don't have to be the same.
           You could make Description writable and different */
    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
        Binary_Value_Object_Name(rpdata->object_instance, &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;

    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0], OBJECT_BINARY_VALUE);
        break;

    case PROP_PRESENT_VALUE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                Binary_Value_Present_Value(currentObject));
        break;

    case PROP_STATUS_FLAGS:
        bitstring_init(&bit_string);
        bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
            Binary_Value_Out_Of_Service(currentObject));
        apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
        break;

    case PROP_EVENT_STATE:
        apdu_len =
            encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
        break;

    case PROP_RELIABILITY:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                currentObject->Reliability);
        break;

    case PROP_OUT_OF_SERVICE:
        apdu_len = encode_application_boolean(&apdu[0],
            Binary_Value_Out_Of_Service(currentObject));
        break;
#if 0
    case PROP_PRIORITY_ARRAY:
        /* Array element zero is the number of elements in the array */
        if (rpdata->array_index == 0)
            apdu_len =
            encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
        /* if no index was specified, then try to encode the entire list */
        /* into one packet. */
        else if (rpdata->array_index == BACNET_ARRAY_ALL) {
            object_index =
                Binary_Value_Instance_To_Index(rpdata->object_instance);
            for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                /* FIXME: check if we have room before adding it to APDU */
                if (Binary_Value_Level[object_index][i] == BINARY_NULL)
                    len = encode_application_null(&apdu[apdu_len]);
                else {
                    present_value = Binary_Value_Level[object_index][i];
                    len =
                        encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                }
                /* add it if we have room */
                if ((apdu_len + len) < MAX_APDU)
                    apdu_len += len;
                else {
                    rpdata->error_code =
                        ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                    apdu_len = BACNET_STATUS_ABORT;
                    break;
                }
            }
        }
        else {
            object_index =
                Binary_Value_Instance_To_Index(rpdata->object_instance);
            if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                if (Binary_Value_Level[object_index][rpdata->array_index]
                    == BINARY_NULL)
                    apdu_len = encode_application_null(&apdu[apdu_len]);
                else {
                    present_value = Binary_Value_Level[object_index]
                        [rpdata->array_index];
                    apdu_len =
                        encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                }
            }
            else {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
        }
        break;
    case PROP_RELINQUISH_DEFAULT:
        present_value = RELINQUISH_DEFAULT;
        apdu_len = encode_application_enumerated(&apdu[0], present_value);
        break;
#endif

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Binary_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    // uint8_t priority;
    // uint32_t level;
    BACNET_APPLICATION_DATA_VALUE value;
    BinaryValueObject *currentObject;

    /* decode the some of the request */
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

    /*  only array properties can have array options */
    if ((wp_data->array_index != BACNET_ARRAY_ALL) &&
#if (INTRINSIC_REPORTING_BV_B == 1)
        (wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
#endif
        (wp_data->object_property != PROP_PROPERTY_LIST)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    currentObject = Binary_Value_Instance_To_Object(wp_data->object_instance);
    if (currentObject == NULL) {
        // todo1 panic();
        return false;
    }

    switch (wp_data->object_property) {

    case PROP_PRESENT_VALUE:
        if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                status =
                    Binary_Value_Present_Value_Set( 
                        currentObject,
                        wp_data, 
                        &value );

                if (wp_data->priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else if (!status) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    status =
                        Binary_Value_Present_Value_Relinquish
                        (wp_data->object_instance, wp_data->priority);
                    if (!status) {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            }
            break;

    case PROP_OUT_OF_SERVICE:
        status =
            WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
        if (status) {
            Binary_Value_Out_Of_Service_Set(currentObject,
                value.type.Boolean);
        }
        break;
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:
    case PROP_PRIORITY_ARRAY:
    case PROP_RELINQUISH_DEFAULT:
    case PROP_PROPERTY_LIST:
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


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testBinary_Value(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Binary_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Value_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

}

#ifdef TEST_BINARY_VALUE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary_Value", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinary_Value);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_VALUE */
#endif /* TEST */

#endif // if (BACNET_USE_OBJECT_BINARY_VALUE == 1 )
