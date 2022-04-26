/****************************************************************************************
*
*   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.If not, see <http://www.gnu.org/licenses/>.
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
