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

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )

#include "address.h"
//#include "datalink.h"
#include "dcc.h"
// #include "bacnet/basic/tsm/tsm.h"
#include "bacnet/wp.h"
//#include "debug.h"
#include "client.h"
#include "multipleDatalink.h"
#include "bitsRouter.h"

/** @file s_wp.c  Send a Write Property request. */

/** returns the invoke ID for confirmed request, or zero on failure */

uint8_t Send_Write_Property_Request_Data(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_PATH *dest,
    uint16_t max_apdu,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_ID object_property,
    uint8_t * application_data,
    int application_data_len,
    uint8_t priority,
    uint32_t array_index)
{
    //BACNET_PATH my_address;
    uint8_t invoke_id = 0;
    int len = 0;
    int pdu_len = 0;
    BACNET_WRITE_PROPERTY_DATA data;
    BACNET_NPCI_DATA npci_data;

    if (!dcc_communication_enabled(pDev)) {
        return 0;
    }

    DLCB *dlcb = alloc_dlcb_application('a', dest, pDev->datalink->max_lpdu );
    if (dlcb == NULL)
    {
        return 0;
    }

    /* is there a tsm available? */
    invoke_id = tsm_next_free_invokeID(routerApplicationEntity);
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        //datalink_get_my_address(&my_address);
        npdu_setup_npci_data(&npci_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(dlcb->Handler_Transmit_Buffer, &dlcb->bacnetPath.glAdr, NULL,
                &npci_data);
        /* encode the APDU portion of the packet */
        data.object_type = object_type;
        data.object_instance = object_instance;
        data.object_property = object_property;
        data.array_index = array_index;
        data.application_data_len = application_data_len;
        memcpy(&data.application_data[0], &application_data[0],
            application_data_len);
        data.priority = priority;

        len =
            wp_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], max_apdu, invoke_id,
                &data);

        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((uint16_t)pdu_len < max_apdu) {
            dlcb->optr = pdu_len;
            tsm_set_confirmed_unsegmented_transaction(pDev, invoke_id, NULL, dlcb);
            pDev->datalink->SendPdu(pDev->datalink, dlcb);

        }
        else {
            tsm_free_invoke_id(pDev, invoke_id);
            invoke_id = 0;
            dbMessage(DBD_ALL, DB_ERROR,
                "Failed to Send WriteProperty Request "
                "(exceeds destination maximum APDU)!\n");
        }
    }
    else
    {
        dlcb_free(dlcb); // todo 3 - check wpm for these frees too!
    }

    return invoke_id;
}


#if 0
/** Sends a Write Property request.
 * @ingroup DSWP
 *
 * @param device_id [in] ID of the destination device
 * @param object_type [in]  Type of the object whose property is to be written.
 * @param object_instance [in] Instance # of the object to be written.
 * @param object_property [in] Property to be written.
 * @param object_value [in] The value to be written to the property.
 * @param priority [in] Write priority of 1 (highest) to 16 (lowest)
 * @param array_index [in] Optional: if the Property is an array,
 *   - 0 for the array size
 *   - 1 to n for individual array members
 *   - BACNET_ARRAY_ALL (~0) for the array value to be ignored (not sent)
 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
 */
uint8_t Send_Write_Property_Request(
    PORT_SUPPORT *portParams,
    DEVICE_OBJECT_DATA *sendingDev,
    uint32_t device_id, /* destination device */
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_ID object_property,
    BACNET_APPLICATION_DATA_VALUE * object_value,
    uint8_t priority,
    uint32_t array_index)
{
    BACNET_ROUTE dest;
    uint16_t max_apdu;
    uint8_t invoke_id = 0;
    bool status;
    uint8_t application_data[MAX_LPDU_IP] = { 0 };
    int apdu_len = 0, len = 0;

    while (object_value) {
#if PRINT_ENABLED_DEBUG
        fprintf(stderr, "WriteProperty service: " "%s tag=%d\n",
            (object_value->context_specific ? "context" : "application"),
            (int)(object_value->
                context_specific ? object_value->context_tag : object_value->
                tag));
#endif
        len = bacapp_encode_data(&application_data[apdu_len], object_value);
        if ((len + apdu_len) < MAX_LPDU_IP) {
            apdu_len += len;
        }
        else {
            return 0;
        }
        object_value = object_value->next;
    }

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    if (status) {

        DLCB *dlcb = alloc_dlcb_application('r', &dest);
        if (dlcb == NULL)
        {
            panic();
            return 0;   // todo, make a real flag here.
        }

    invoke_id = Send_Write_Property_Request_Data(
        sendingDev, dlcb, max_apdu, object_type,
        object_instance, object_property,
        application_data, apdu_len,
        priority,
        array_index);

    }

    return invoke_id;
}

#endif // if 0
#endif
