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

#include "configProj.h"
#include "bacnet/whois.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"

/** @file h_whois.c  Handles Who-Is requests. */

/** Handler for Who-Is requests, with broadcast I-Am response.
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source that the
 *                 response will be sent back to.
 */
void handler_who_is_unicast(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE *src)
{
    int len ;
    int32_t low_limit ;
    int32_t high_limit ;

    len =
        whois_decode_service_request(service_request, service_len, &low_limit,
            &high_limit);

    if (len == 0) {
        /* If no limits, then always respond */
        Send_I_Am_Unicast(pDev, src);
    }
    else if (len != BACNET_STATUS_ERROR)
    {

        /* is my Device Instance within the limits? */
        if ((Device_Object_Instance_Number(pDev) >= (uint32_t)low_limit) &&
            (Device_Object_Instance_Number(pDev) <= (uint32_t)high_limit))
        {
            Send_I_Am_Unicast(pDev, src);
        }

#ifdef AUTOCREATE_SITE
        if (high_limit == low_limit)
        {
            // for AutoSite, if this is not me, and I am the router entity, then
            // check to see if the virtual device exists or not, and if not, create one
            DEVICE_OBJECT_DATA* pDev = Device_Find_Device(high_limit);
            if (pDev == NULL)
            {
                char tbufname[100];
                char tbufdesc[100];
                sprintf(tbufname, "AutoDev:%d", high_limit);
                sprintf(tbufdesc, "Automatically created Dev:%-07d", high_limit);
                printf("%s\n", tbufdesc);
                Create_Device_Virtual(PORTID_VIRT, high_limit, tbufname, tbufdesc, BACNET_VENDOR_ID, BACNET_VENDOR_NAME );

            }
        }
#endif

    }
}

