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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "configProj.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/whohas.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"
//#include "datalink.h"

/** @file h_whohas.c  Handles Who-Has requests. */

/** Local function which responds with either the requested object name
 *  or object ID, if the Device has a match.
 *  @param data [in] The decoded who-has payload from the request.
 */
static void match_name_or_object(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE *src,
    BACNET_WHO_HAS_DATA * data)
{
    BACNET_OBJECT_TYPE object_type ;
    uint32_t object_instance = 0;
    bool found = false;
    BACNET_CHARACTER_STRING object_name;

    /* do we have such an object?  If so, send an I-Have.
       note: we should have only 1 of such an object */
    if (data->is_object_name) {
        /* valid name in my device? */
        found =
            Device_Valid_Object_Name(pDev,&data->object.name, &object_type,
                &object_instance);
        if (found) {
            Send_I_Have(
                src,
                pDev,
                Device_Object_Instance_Number(pDev),
                (BACNET_OBJECT_TYPE) object_type, object_instance,
                &data->object.name);
        }
    } else {
        /* valid object_name copy in my device? */
        found =
            Device_Object_Name_Copy(pDev,
                (BACNET_OBJECT_TYPE) data->object.identifier.type,
                data->object.identifier.instance,
                &object_name);
        if (found) {
            Send_I_Have(
                src,
                pDev,
                Device_Object_Instance_Number(pDev),
                (BACNET_OBJECT_TYPE) data->object.identifier.type,
                data->object.identifier.instance, &object_name);
        }
    }
}


/** Handler for Who-Has requests, with broadcast I-Have response.
 * Will respond if the device Object ID matches, and we have
 * the Object or Object Name requested.
 *
 * @ingroup DMDOB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_GLOBAL_ADDRESS of the message's source.
 */
void handler_who_has(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * src)
{
    int len = 0;
    BACNET_WHO_HAS_DATA data;
    bool directed_to_me = false;

    len = whohas_decode_service_request(service_request, service_len, &data);
    if (len > 0) {
        if ((data.low_limit == -1) || (data.high_limit == -1))
            directed_to_me = true;
        else if ((Device_Object_Instance_Number(pDev) >= (uint32_t) data.low_limit)
                   && (Device_Object_Instance_Number(pDev) <= (uint32_t) data.high_limit))
            directed_to_me = true;
        if (directed_to_me) {
            match_name_or_object(pDev, src, &data);
        }
    }
}



