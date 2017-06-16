/**************************************************************************
*
* Copyright (C) 2016 ConnectEx, Inc. <info@connect-ex.com>
*
* Permission is hereby granted, free of charge, to Schneider Electric,
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

/*

2016.03.22	EKH		AddListElement / RemoveListElement
	This file has been created to support the AddListElement and RemoveListElement
	services and the supporting code for these services by ConnectEx, Inc.
	Questions regarding this can be directed to: info@connect-ex.com

*/

#include <stdint.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "listmanip.h"

/** @file lm.c  Encode/Decode List Manipulation APDUs */
#if BACNET_SVC_LM_A
/* encode service */
int lm_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_REINITIALIZED_STATE state,
    BACNET_CHARACTER_STRING * password)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_REINITIALIZE_DEVICE;
        apdu_len = 4;
        len = encode_context_enumerated(&apdu[apdu_len], 0, state);
        apdu_len += len;
        /* optional password */
        if (password) {
            /* FIXME: must be at least 1 character, limited to 20 characters */
            len =
                encode_context_character_string(&apdu[apdu_len], 1, password);
            apdu_len += len;
        }
    }

    return apdu_len;
}
#endif

/* decode the service request only */
int lm_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_LIST_MANIPULATION_DATA * lmdata)
{
    unsigned len = 0;
    uint8_t tag_number;
    uint32_t type;      /* for decoding */// uint16_t failed under linux
    uint32_t value;

    /* check for value pointers */
    if (apdu_len) {
        /* Tag 0: Object ID          */
        if (!decode_is_context_tag(&apdu[len++], 0)) {
            lmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        len +=
            decode_object_id(&apdu[len], &lmdata->object_type,
                             &lmdata->object_instance);
        /* Tag 1: Property ID */
        len += decode_tag_number_and_value(&apdu[len], &tag_number, &type);
        if (tag_number != 1) {
            lmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        len += decode_enumerated(&apdu[len], type, &value);
        lmdata->object_property = (BACNET_PROPERTY_ID) value;
        /* opening tag */
        if (!decode_is_opening_tag_number(&apdu[len++], 3)) {
            lmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        lmdata->application_data = &apdu[len];
        lmdata->application_max_data_len = apdu_len - len - 1;  /* closing tag is 1 byte */
        //len += lmdata->application_data_len;

    }

    return (int) len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int lm_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_REINITIALIZED_STATE * state,
    BACNET_CHARACTER_STRING * password)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu) {
        return -1;
    }
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST) {
        return -1;
    }
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_REINITIALIZE_DEVICE) {
        return -1;
    }
    offset = 4;

    if (apdu_len > offset) {
        len =
            rd_decode_service_request(&apdu[offset], apdu_len - offset, state,
                                      password);
    }

    return len;
}

void test_ReinitializeDevice(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    BACNET_REINITIALIZED_STATE state;
    BACNET_REINITIALIZED_STATE test_state;
    BACNET_CHARACTER_STRING password;
    BACNET_CHARACTER_STRING test_password;

    state = BACNET_REINIT_WARMSTART;
    characterstring_init_ansi(&password, "John 3:16");
    len = rd_encode_apdu(&apdu[0], invoke_id, state, &password);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        rd_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_state,
                       &test_password);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_state == state);
    ct_test(pTest, characterstring_same(&test_password, &password));

    return;
}

#ifdef TEST_REINITIALIZE_DEVICE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet ReinitializeDevice", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, test_ReinitializeDevice);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_REINITIALIZE_DEVICE */
#endif /* TEST */
