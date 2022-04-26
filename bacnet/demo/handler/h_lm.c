/**************************************************************************
*
* Copyright (C) 2016 ConnectEx, Inc. <info@connect-ex.com>
*
* Permission is hereby granted, to whom a copy of this software and 
* associated documentation files (the "Software") is provided by ConnectEx, Inc.
* to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, subject to
* the following conditions:
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

/*

2016.03.22	EKH		AddListElement
	This file has been created to support the AddListElement and RemoveListElement
	services and the supporting code for these services by ConnectEx, Inc.
	Questions regarding this can be directed to: info@connect-ex.com

*/



#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "bacdevobjpropref.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "reject.h"
#include "listmanip.h"
/* device object has custom handler for all objects */
#include "device.h"
#include "handlers.h"

/** @file h_lm.c  Handles List Manipulation requests. */


/** Handler for a AddListElement Service request.
 * @ingroup DMLM
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 *   - if the response would be too large
 * - the result from Device_Add_List_Element(), if it succeeds
 * - an Error if Device_Add_List_Element() fails
 *   or there isn't enough room in the APDU to fit the data.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
#if 0
void handler_add_list_element(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_LIST_MANIPULATION_DATA lmdata ;
    int len = 0;
    int pdu_len = 0;
    int apdu_len;
    int npdu_len;
    BACNET_NPCI_DATA npci_data;
    bool error = true;  /* assume that there is an error */
    int bytes_sent = 0;
    //BACNET_GLOBAL_ADDRESS my_address;

    /* configure default error code as an abort since it is common */
    lmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npdu_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &src->bacnetPath->adr, NULL,
                        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len = BACNET_STATUS_ABORT;
#if PRINT_ENABLED
        fprintf(stderr, "ALE: Segmented message.  Sending Abort!\n");
#endif
        goto LM_FAILURE;
    }
    len = lm_decode_service_request(service_request, service_len, &lmdata);
    if (len <= 0) {
        /* bad decoding - skip to error/reject/abort handling */
        error = true;
#if PRINT_ENABLED
        fprintf(stderr, "ALE: Bad Encoding.\n");
#endif
        goto LM_FAILURE;
    }
    /* Test for case of indefinite Device object instance */
    if ((lmdata.object_type == OBJECT_DEVICE) &&
        (lmdata.object_instance == BACNET_MAX_INSTANCE)) {
        lmdata.object_instance = Device_Object_Instance_Number(pDev);
    }

    /* let device handle service request */
    if (Device_Add_List_Element(pDev, &lmdata)) {
        /* expect a closing tag immediately after list elements in application data */
        if (decode_is_closing_tag_number(lmdata.application_data +
                                         lmdata.application_data_len, 3)) {
            apdu_len =
                encode_simple_ack(&Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id, SERVICE_CONFIRMED_ADD_LIST_ELEMENT);
#if PRINT_ENABLED
            fprintf(stderr, "ALE: Sending Ack!\n");
#endif
            error = false;
        } else {
#if PRINT_ENABLED
            fprintf(stderr, "ALE: expected closing tag.\n");
#endif
            lmdata.error_code = ERROR_CODE_REJECT_INVALID_TAG;
            len = BACNET_STATUS_REJECT;
            error = true;
        }
    } else {
#if PRINT_ENABLED
        fprintf(stderr, "ALE: Device_Add_List_Element() error.\n");
#endif
        len = BACNET_STATUS_ERROR;
        error = true;
    }

LM_FAILURE:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id,
                                  abort_convert_error_code(lmdata.error_code), true);
#if PRINT_ENABLED
            fprintf(stderr, "ALE: Sending Abort!\n");
#endif
        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len =
                bacerror_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_ADD_LIST_ELEMENT,
                                     lmdata.error_class, lmdata.error_code);
            apdu_len +=
                encode_application_unsigned(&Handler_Transmit_Buffer[npdu_len +
                                            apdu_len], lmdata.first_failed_element + 1);
#if PRINT_ENABLED
            fprintf(stderr, "ALE: Sending Error!\n");
#endif
        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                   service_data->invoke_id,
                                   reject_convert_error_code(lmdata.error_code));
#if PRINT_ENABLED
            fprintf(stderr, "ALE: Sending Reject!\n");
#endif
        }
    }
    pdu_len = npdu_len + apdu_len;
        
    src->portParams->SendPdu(src->portParams, pDev, &src->bacnetPath->localMac, &npci_data, &Handler_Transmit_Buffer[0],
                          pdu_len);

    return;
}

/** Handler for a RemoveListElement Service request.
* @ingroup DMLM
* This handler will be invoked by apdu_handler() if it has been enabled
* by a call to apdu_set_confirmed_handler().
* This handler builds a response packet, which is
* - an Abort if
*   - the message is segmented
*   - if decoding fails
*   - if the response would be too large
* - the result from Device_Remove_List_Element(), if it succeeds
* - an Error if Device_Remove_List_Element() fails
*   or there isn't enough room in the APDU to fit the data.
*
* @param service_request [in] The contents of the service request.
* @param service_len [in] The length of the service_request.
* @param src [in] BACNET_ADDRESS of the source of the message
* @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
*                          decoded from the APDU header of this message.
*/
void handler_remove_list_element(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_LIST_MANIPULATION_DATA lmdata;
    int len = 0;
    int pdu_len = 0;
    int apdu_len = -1;
    int npdu_len;
    BACNET_NPCI_DATA npci_data;
    bool error = true;  /* assume that there is an error */
    int bytes_sent = 0;
    //BACNET_GLOBAL_ADDRESS my_address;

    /* configure default error code as an abort since it is common */
    lmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npdu_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &src->bacnetPath->adr, NULL,
                        &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len = BACNET_STATUS_ABORT;
#if PRINT_ENABLED
        fprintf(stderr, "RLE: Segmented message.  Sending Abort!\n");
#endif
        goto LM_FAILURE;
    }
    len = lm_decode_service_request(service_request, service_len, &lmdata);
    if (len <= 0) {
        /* bad decoding - skip to error/reject/abort handling */
        error = true;
#if PRINT_ENABLED
        fprintf(stderr, "RLE: Bad Encoding.\n");
#endif
        goto LM_FAILURE;
    }
    /* Test for case of indefinite Device object instance */
    if ((lmdata.object_type == OBJECT_DEVICE) &&
        (lmdata.object_instance == BACNET_MAX_INSTANCE)) {
        lmdata.object_instance = Device_Object_Instance_Number(pDev);
    }
    /* let device handle service request */
    if (Device_Remove_List_Element(pDev, &lmdata)) {
        /* expect a closing tag immediately after list elements in application data */
        if (decode_is_closing_tag_number(lmdata.application_data +
                                         lmdata.application_data_len, 3)) {
            apdu_len =
                encode_simple_ack(&Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id,
                                  SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT);
#if PRINT_ENABLED
            fprintf(stderr, "RLE: Sending Ack!\n");
#endif
            error = false;
        } else {
#if PRINT_ENABLED
            fprintf(stderr, "RLE: expected closing tag.\n");
#endif
            lmdata.error_code = ERROR_CODE_REJECT_INVALID_TAG;
            len = BACNET_STATUS_REJECT;
            error = true;
        }
    } else {
#if PRINT_ENABLED
        fprintf(stderr, "RLE: Device_Remove_List_Element() error.\n");
#endif
        len = BACNET_STATUS_ERROR;
        error = true;
    }

LM_FAILURE:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id,
                                  abort_convert_error_code(lmdata.error_code), true);
#if PRINT_ENABLED
            fprintf(stderr, "RLE: Sending Abort!\n");
#endif
        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len =
                bacerror_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT,
                                     lmdata.error_class, lmdata.error_code);
            apdu_len +=
                encode_application_unsigned(&Handler_Transmit_Buffer[npdu_len +
                                            apdu_len], lmdata.first_failed_element + 1);
#if PRINT_ENABLED
            fprintf(stderr, "RLE: Sending Error!\n");
#endif
        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
                                   service_data->invoke_id,
                                   reject_convert_error_code(lmdata.error_code));
#if PRINT_ENABLED
            fprintf(stderr, "RLE: Sending Reject!\n");
#endif
        }
    }
    pdu_len = npdu_len + apdu_len;
    src->portParams->SendPdu(src->portParams, pDev, &src->bacnetPath->localMac, &npci_data, &Handler_Transmit_Buffer[0],
                          pdu_len);
}

#endif