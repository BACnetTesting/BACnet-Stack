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

#ifndef COV_H
#define COV_H

#include <stdint.h>
#include <stdbool.h>
#include "bacapp.h"
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/bits/util/BACnetObject.h"

enum COV_STATE {
    COV_STATE_IDLE = 0,
    COV_STATE_MARK,
    COV_STATE_CLEAR,
    COV_STATE_FREE,
    COV_STATE_SEND
}  ;

typedef struct BACnet_COV_Address {
    bool valid : 1;
    BACNET_ROUTE destRoute;
} BACNET_COV_ADDRESS;

/* note: This COV service only monitors the properties
of an object that have been specified in the standard.  */
typedef struct BACnet_COV_Subscription_Flags {
    bool valid : 1;
    bool issueConfirmedNotifications : 1; /* optional */
    bool send_requested : 1;
} BACNET_COV_SUBSCRIPTION_FLAGS;

typedef struct {
    BACNET_COV_SUBSCRIPTION_FLAGS flag;
    int8_t dest_index;      // Has to be signed, tested for < 0 in places
    uint8_t invokeID;       /* for confirmed COV */
    uint32_t subscriberProcessIdentifier;
    uint32_t lifetime;  /* optional */
    uint32_t lifetimeSet;   /* when the lifetime was set (persistence) */
    BACNET_GLOBAL_ADDRESS srcRoute;     /* added for (persistence) */
    BACNET_MAC_ADDRESS localMac;        /* added for (persistence) */
    BPT_TYPE portType;                  /* added for (persistence) */
    BACNET_OBJECT_ID monitoredObjectIdentifier;
} BACNET_COV_SUBSCRIPTION ;

typedef struct BACnet_COV_Data {
    uint32_t subscriberProcessIdentifier;
    uint32_t initiatingDeviceIdentifier;
    BACNET_OBJECT_ID monitoredObjectIdentifier;
    uint32_t timeRemaining;     /* seconds */
    /* simple linked list of values */
    BACNET_PROPERTY_VALUE *listOfValues;
} BACNET_COV_DATA;

struct BACnet_Subscribe_COV_Data;
typedef struct BACnet_Subscribe_COV_Data {
    uint32_t subscriberProcessIdentifier;
    BACNET_OBJECT_ID monitoredObjectIdentifier;
    bool cancellationRequest;			/* true if this is a cancellation request */
    bool issueConfirmedNotifications;   /* optional */
    uint32_t lifetime;					/* seconds, optional 2^32 seconds = 136 years */
    BACNET_PROPERTY_REFERENCE monitoredProperty;
    bool covIncrementPresent;			/* true if present */
    float covIncrement;					/* optional */
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
    struct BACnet_Subscribe_COV_Data *next;
} BACNET_SUBSCRIBE_COV_DATA;

#define TEXT_BPD_TOPIC  "bpd/0/./fb/dev/"   // instance ID follows at [5]

int ucov_notify_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu_len,
    BACNET_COV_DATA * data);

int ucov_notify_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_COV_DATA * data);

int ucov_notify_send(
    uint8_t * apdu,
    uint16_t max_apdu_len,
    BACNET_COV_DATA * data);

int ccov_notify_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu_len,
    uint8_t invoke_id,
    BACNET_COV_DATA * data);

int ccov_notify_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_COV_DATA * data);

/* common for both confirmed and unconfirmed */
int cov_notify_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_COV_DATA * data);

int cov_subscribe_property_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_SUBSCRIBE_COV_DATA * data);

int cov_subscribe_property_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu_len,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data);

int cov_subscribe_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_SUBSCRIBE_COV_DATA * data);

int cov_subscribe_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu_len,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data);

void cov_data_value_list_link(
    BACNET_COV_DATA *data,
    BACNET_PROPERTY_VALUE *value_list,
    uint16_t count);

bool Common_Encode_Value_List(
    BACNET_OBJECT* currentObject,
    bool isInAlarm, bool isInFault, bool isOutOfService,
    BACNET_APPLICATION_DATA_VALUE* presentValue,
    BACNET_PROPERTY_VALUE* value_list);

bool cov_subscribe(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE *srcRoute,
    BACNET_SUBSCRIBE_COV_DATA * cov_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code);

#ifdef TEST
#include "ctest.h"
void testCOVNotify(
    Test * pTest);
void testCOVSubscribeProperty(
    Test * pTest);
void testCOVSubscribe(
    Test * pTest);
#endif

/** @defgroup DSCOV Data Sharing - Change of Value Service (DS-COV)
 * @ingroup DataShare
 * 13.1 Change of Value Reporting <br>
 * Change of value (COV) reporting allows a COV-client to subscribe with a
 * COV-server, on a permanent or temporary basis, to receive reports of some
 * changes of value of some referenced property based on fixed criteria.
 * If an object provides COV reporting, then changes of value of any
 * subscribed-to properties of the object, in some cases based on programmable
 * increments, trigger COV notifications to be sent to subscribing clients.
 * Typically, COV notifications are sent to supervisory programs in COV-client
 * devices or to operators or logging devices. Any object, proprietary or
 * standard, may support COV reporting at the implementor's option.
 */
#endif
