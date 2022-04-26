/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include <stdint.h>

#include "configProj.h"

#include "bacnet/bacdef.h"
#include "bacnet/iam.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/object/device.h"

/** @file h_iam.c  Handles I-Am requests. */

/** Handler for I-Am responses.
 * Will add the responder to our cache, or update its binding.
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_PATH of the message's source.
 */
#if (BACNET_CLIENT == 1)
void handler_i_am_add(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src)
{
    int len ;
    uint32_t device_id ;
    uint16_t max_apdu ;
    int segmentation ;
    uint16_t vendor_id ;

    (void) service_len;
    len =
        iam_decode_service_request(service_request, &device_id, &max_apdu,
                                   &segmentation, &vendor_id);

    dbMessage(DBD_ALL, DB_NOTE, "Received I-Am message %d", device_id );

    if (len != -1) {
        
        address_add(src->portParams, device_id, max_apdu, &src->bacnetPath );
    }
    else {
        dbMessage(DBD_ALL, DB_ERROR, "Unable to decode I-Am message");
    }
}
#endif

/** Handler for I-Am responses (older binding-update-only version).
 * Will update the responder's binding, but if already in our cache.
 * @note This handler is deprecated, in favor of handler_i_am_add().
 *
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_PATH of the message's source.
 */
#if 0
void handler_i_am_bind(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t *service_request,
    uint16_t service_len,
    BACNET_ROUTE * srcRoute)
{
    int len = 0;
    uint32_t device_id = 0;
    uint16_t max_apdu ;
    int segmentation = 0;
    uint16_t vendor_id = 0;

    (void) service_len;
    (void)pDev;
    len =
        iam_decode_service_request(service_request, &device_id, &max_apdu,
                                   &segmentation, &vendor_id);
    if (len > 0) {
        /* only add address if requested to bind */
        address_add_binding(device_id, max_apdu, srcRoute);
    }
}
#endif