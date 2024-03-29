/**************************************************************************
 * @file
 * @author Steve Karg
 * @date 2019
 * @brief Header file for a basic APDU Handler
 *
 * @section LICENSE
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

#ifndef BACNET_APDU_HANDLER_H
#define BACNET_APDU_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include "bacnet/bacnet_stack_exports.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacenum.h"
#include "bacnet/apdu.h"
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/basic/object/device.h"

#ifdef __cplusplus_disable
extern "C" {
#endif /* __cplusplus_disable */

/* generic unconfirmed function handler */
/* Suitable to handle the following services: */
/* I_Am, Who_Is, Unconfirmed_COV_Notification, I_Have, */
/* Unconfirmed_Event_Notification, Unconfirmed_Private_Transfer, */
/* Unconfirmed_Text_Message, Time_Synchronization, Who_Has, */
/* UTC_Time_Synchronization */
typedef void(
    *unconfirmed_function) (
        DEVICE_OBJECT_DATA *pDev,
        uint8_t * service_request,
        uint16_t len,
        BACNET_ROUTE * src);

/* generic confirmed function handler */
/* Suitable to handle the following services: */
/* Acknowledge_Alarm, Confirmed_COV_Notification, */
/* Confirmed_Event_Notification, Get_Alarm_Summary, */
/* Get_Enrollment_Summary_Handler, Get_Event_Information, */
/* Subscribe_COV_Handler, Subscribe_COV_Property, */
/* Life_Safety_Operation, Atomic_Read_File, */
/* Confirmed_Atomic_Write_File, Add_List_Element, */
/* Remove_List_Element, Create_Object_Handler, */
/* Delete_Object_Handler, Read_Property, */
/* Read_Property_Conditional, Read_Property_Multiple, Read_Range, */
/* Write_Property, Write_Property_Multiple, */
/* Device_Communication_Control, Confirmed_Private_Transfer, */
/* Confirmed_Text_Message, Reinitialize_Device, */
/* VT_Open, VT_Close, VT_Data_Handler, */
/* Authenticate, Request_Key */
typedef void(
    *confirmed_function) (
        DEVICE_OBJECT_DATA *pDev,
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ROUTE * srcRoute,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);


// Note: None of the message functions have any concept of a pDev (that information cannot
// be extracted from a BACnet message, so the following ack handlers
// cannot ever be called using a pDev, do not be led astry by temptation!!

/* generic confirmed simple ack function handler */
typedef void(
    *confirmed_simple_ack_function) (
        // no pDev in ack, see above
        BACNET_ROUTE * src,
        uint8_t invoke_id);

/* generic confirmed ack function handler */
typedef void(
    *confirmed_ack_function) (
        // no pDev in ack, see above
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ROUTE * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

/* generic error reply function */
#if ( BACNET_CLIENT == 1 )
typedef void(
    *error_function) (
        // no pDev in ack, see above
        BACNET_ROUTE * src,
        uint8_t invoke_id,
        BACNET_ERROR_CLASS error_class,
        BACNET_ERROR_CODE error_code);

/* generic abort reply function */
typedef void(
    *abort_function) (
        // no pDev in ack, see above
        BACNET_ROUTE * src,
        uint8_t invoke_id,
        BACNET_ABORT_REASON abort_reason,
        bool server);

/* generic reject reply function */
typedef void(
    *reject_function) (
        // no pDev in ack, see above
        BACNET_ROUTE * src,
        uint8_t invoke_id,
        BACNET_REJECT_REASON reject_reason);

void apdu_set_confirmed_ack_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    confirmed_ack_function pFunction);

void apdu_set_confirmed_simple_ack_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    confirmed_simple_ack_function pFunction);
#endif

/* configure reject for confirmed services that are not supported */
    BACNET_STACK_EXPORT
    void apdu_set_unrecognized_service_handler_handler(
        confirmed_function pFunction);

    BACNET_STACK_EXPORT
    void apdu_set_confirmed_handler(
        BACNET_CONFIRMED_SERVICE service_choice,
        confirmed_function pFunction);

    BACNET_STACK_EXPORT
    void apdu_set_unconfirmed_handler(
        BACNET_UNCONFIRMED_SERVICE service_choice,
        unconfirmed_function pFunction);

/* returns true if the service is supported by a handler */
    BACNET_STACK_EXPORT
    bool apdu_service_supported(
        BACNET_SERVICES_SUPPORTED service_supported);

///* Function to translate a SERVICE_SUPPORTED_ enum to its SERVICE_CONFIRMED_
// *  or SERVICE_UNCONFIRMED_ index.
// */
//    BACNET_STACK_EXPORT
//    bool apdu_service_supported_to_index(
//        BACNET_SERVICES_SUPPORTED service_supported,
//        uint16_t * index,
//        bool * bIsConfirmed);


#if ( BACNET_CLIENT == 1 )
void apdu_set_error_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    error_function pFunction);

void apdu_set_abort_handler(
    abort_function pFunction);

void apdu_set_reject_handler(
    reject_function pFunction);
#endif

uint16_t apdu_decode_confirmed_service_request(
    uint8_t * apdu, /* APDU data */
    uint16_t apdu_len,
    BACNET_CONFIRMED_SERVICE_DATA * service_data,
    BACNET_CONFIRMED_SERVICE * service_choice,
    uint8_t ** service_request,
    uint16_t * service_request_len);

uint16_t apdu_timeout(
    void);

void apdu_timeout_set(
    uint16_t value);

uint8_t apdu_retries(
    void);

void apdu_retries_set(
    uint8_t value);

void apdu_handler(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE *src,      /* source address */
    uint8_t *apdu,          /* APDU data */
    uint16_t pdu_len);      /* for confirmed messages */

#ifdef __cplusplus_disable
}
#endif /* __cplusplus_disable */
#endif
