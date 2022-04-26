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

#ifndef TSM_H
#define TSM_H

#include <stdint.h>
#include "bacnet/bacdef.h"
#include "bacnet/npdu.h"
//#include "datalink.h"
#include "bacnet/bits/util/multipleDatalink.h"

typedef struct devObj_s DEVICE_OBJECT_DATA ;

/* note: TSM functionality is optional - only needed if we are
   doing client requests */
#if (!MAX_TSM_TRANSACTIONS)
#define tsm_free_invoke_id(x) (void)x;
#else
typedef enum {
    TSM_STATE_IDLE,
    TSM_STATE_AWAIT_CONFIRMATION,
    TSM_STATE_AWAIT_RESPONSE,
    TSM_STATE_SEGMENTED_REQUEST,
    TSM_STATE_SEGMENTED_CONFIRMATION
} BACNET_TSM_STATE;

/* 5.4.1 Variables And Parameters */
/* The following variables are defined for each instance of  */
/* Transaction State Machine: */
typedef struct BACnet_TSM_Data {
    /* used to count APDU retries */
    uint8_t RetryCount;

    // variables below are to support PDU segmentation, something we choose not to do at this time.
    /* used to count segment retries */
    /*uint8_t SegmentRetryCount;  */
    /* used to control APDU retries and the acceptance of server replies */
    /*bool SentAllSegments;  */
    /* stores the sequence number of the last segment received in order */
    /*uint8_t LastSequenceNumber; */
    /* stores the sequence number of the first segment of */
    /* a sequence of segments that fill a window */
    /*uint8_t InitialSequenceNumber; */
    /* stores the current window size */
    /*uint8_t ActualWindowSize; */
    /* stores the window size proposed by the segment sender */
    /*uint8_t ProposedWindowSize;  */
    /*  used to perform timeout on PDU segments */
    /*uint8_t SegmentTimer; */

    /* used to perform timeout on Confirmed Requests */
    /* in milliseconds */
    uint16_t RequestTimer;
    /* unique id */
    uint8_t InvokeID;
    /* state that the TSM is in */
    BACNET_TSM_STATE state;

    BACNET_MAC_ADDRESS *ourMAC;      // in order to support virtual devices, we need to make a copy of the local MAC of each device -> we can't depend on the (virtual) datalink to report the correct one

    DLCB *dlcb2;

    uint8_t autoClear:1 ;

} BACNET_TSM_DATA;

typedef void(
    *tsm_timeout_function) (
        DEVICE_OBJECT_DATA *pDev,
        uint8_t invoke_id);

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )

void tsm_init(
    DEVICE_OBJECT_DATA *pDev);

void tsm_deinit(
    DEVICE_OBJECT_DATA *pDev);

#endif

void tsm_set_timeout_handler(
    tsm_timeout_function pFunction);

bool tsm_transaction_available(
    DEVICE_OBJECT_DATA *pDev);

uint8_t tsm_transaction_idle_count(
    DEVICE_OBJECT_DATA *pDev);

void tsm_timer_milliseconds(
    DEVICE_OBJECT_DATA *pDev,
    uint16_t milliseconds);

/* free the invoke ID when the reply comes back */
bool tsm_free_invoke_id(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID);

/* use these in tandem */
uint8_t tsm_next_free_invokeID(
    DEVICE_OBJECT_DATA *pDev);

uint8_t tsm_next_free_invokeID_autoclear(
    DEVICE_OBJECT_DATA *pDev);

/* returns the same invoke ID that was given */
void tsm_set_confirmed_unsegmented_transaction(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID,
    BACNET_MAC_ADDRESS *ourMAC,
    DLCB *dlcb );

/* returns true if transaction is found */
bool tsm_get_transaction_pdu(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID,
    DLCB **dlcb );

bool is_tsm_invoke_id_free(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID);

bool tsm_invoke_id_failed(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID);

/* define out any functions necessary for compile */
#endif
#endif
