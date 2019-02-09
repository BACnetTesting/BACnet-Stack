/**************************************************************************
*
* Permission is hereby granted, free of charge,
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

/*

2016.03.22	EKH		AddListElement / RemoveListElement
	This file has been created to support the AddListElement and RemoveListElement
	services and the supporting code for these services by ConnectEx, Inc.
	Questions regarding this can be directed to: info@connect-ex.com

*/



#ifndef LIST_MANIPULATION_H
#define LIST_MANIPULATION_H

#include <stdint.h>
// #include <stdbool.h>
#include "bacdef.h"

typedef struct BACnet_list_manipulation_data {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    BACNET_PROPERTY_ID object_property;
    uint8_t *application_data;
    int application_data_len;			/* actual length of decoded list of elements */
    int application_max_data_len;		/* maximum length of encoded list of elements */
    unsigned first_failed_element;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
} BACNET_LIST_MANIPULATION_DATA;

/* encode service */
int lm_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_LIST_MANIPULATION_DATA * lm_data);

/* decode the service request only */
int lm_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_LIST_MANIPULATION_DATA * lm_data);

#ifdef TEST
#include "ctest.h"
int lm_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_LIST_MANIPULATION_DATA * lm_data);

void test_ListManipulation(
    Test * pTest);
#endif

/** @defgroup DMLM Device Management-List Manipulation (DM-LM)
 * @ingroup ???
 *
 */
#endif
