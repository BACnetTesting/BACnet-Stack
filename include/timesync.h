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

    Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

    This file contains changes made by BACnet Interoperability Testing
    Services, Inc. These changes are subject to the permissions,
    warranty terms and limitations above.
    For more information: info@bac-test.com
    For access to source code:  info@bac-test.com
            or      www.github.com/bacnettesting/bacnet-stack

####COPYRIGHTEND####
*********************************************************************/
#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"

struct BACnet_Recipient_List;
typedef struct BACnet_Recipient_List {
    /*
       BACnetRecipient ::= CHOICE {
       device [0] BACnetObjectIdentifier,
       address [1] BACnetAddress
       }
     */
    uint8_t tag;
    union {
        BACNET_OBJECT_ID device;
        BACNET_ADDRESS address;
    } type;
    /* simple linked list */
    struct BACnet_Recipient_List *next;
} BACNET_RECIPIENT_LIST;

    /* encode service */
    int timesync_utc_encode_apdu(
        uint8_t * apdu,
        BACNET_DATE * my_date,
        BACNET_TIME * my_time);
    int timesync_encode_apdu(
        uint8_t * apdu,
        BACNET_DATE * my_date,
    BACNET_TIME * my_time);
int timesync_encode_apdu_service(
    uint8_t * apdu,
    BACNET_UNCONFIRMED_SERVICE service,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time);
/* decode the service request only */
int timesync_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time);
int timesync_utc_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time);
int timesync_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time);

int timesync_encode_timesync_recipients(
    uint8_t * apdu,
    unsigned max_apdu,
    BACNET_RECIPIENT_LIST * recipient);
int timesync_decode_timesync_recipients(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_RECIPIENT_LIST * recipient);

#ifdef TEST
#include "ctest.h"
void testTimeSync(
    Test * pTest);
#endif

#endif /* TIMESYNC_H */
