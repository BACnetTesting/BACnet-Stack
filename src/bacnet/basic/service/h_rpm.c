/**************************************************************************
 *
 * Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
 * Inspired by John Stachler <John.Stachler@lennoxind.com>
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
#include "bacnet/memcopy.h"
#include "bacnet/apdu.h"
#include "bacnet/abort.h"
#include "bacnet/reject.h"
#include "bacnet/bacerror.h"
#include "bacnet/rpm.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/bacenum.h"

/** @file h_rpm.c  Handles Read Property Multiple requests. */

static uint8_t Temp_Buf[MAX_LPDU_IP] = { 0 };		// todo 3 - avoid this static allocation

static BACNET_PROPERTY_ID RPM_Object_Property(
    struct special_property_list_t *pPropertyList,
    BACNET_PROPERTY_ID special_property,
    unsigned index)
{
    int property = -1;  /* return value */
    unsigned required, optional, proprietary;

    required = pPropertyList->Required.count;
    optional = pPropertyList->Optional.count;
    proprietary = pPropertyList->Proprietary.count;
    if (special_property == PROP_ALL) {
        if (index < required) {
            property = pPropertyList->Required.pList[index];
        }
        else if (index < (required + optional)) {
            index -= required;
            property = pPropertyList->Optional.pList[index];
        }
        else if (index < (required + optional + proprietary)) {
            index -= (required + optional);
            property = pPropertyList->Proprietary.pList[index];
        }
    }
    else if (special_property == PROP_REQUIRED) {
        if (index < required) {
            property = pPropertyList->Required.pList[index];
        }
    }
    else if (special_property == PROP_OPTIONAL) {
        if (index < optional) {
            property = pPropertyList->Optional.pList[index];
        }
    }

    return (BACNET_PROPERTY_ID) property;
}

static unsigned RPM_Object_Property_Count(
    struct special_property_list_t *pPropertyList,
    BACNET_PROPERTY_ID special_property)
{
    unsigned count = 0; /* return value */

    if (special_property == PROP_ALL) {
        count =
            pPropertyList->Required.count + pPropertyList->Optional.count +
            pPropertyList->Proprietary.count;
    }
    else if (special_property == PROP_REQUIRED) {
        count = pPropertyList->Required.count;
    }
    else if (special_property == PROP_OPTIONAL) {
        count = pPropertyList->Optional.count;
    }

    return count;
}

/** Encode the RPM property returning the length of the encoding,
   or 0 if there is no room to fit the encoding.  */
static int RPM_Encode_Property(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * apdu,
    uint16_t offset,
    uint16_t max_apdu,
    BACNET_RPM_DATA * rpmdata)
{
    int len;
    uint16_t copy_len ;
    int apdu_len ;
    BACNET_READ_PROPERTY_DATA rpdata;

    len =
        rpm_ack_encode_apdu_object_property(&Temp_Buf[0],
            rpmdata->object_property, rpmdata->array_index);
    copy_len = memcopy(&apdu[0], &Temp_Buf[0], offset, len, max_apdu);
    if (copy_len == 0) {
        rpmdata->error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
        return BACNET_STATUS_ABORT;
    }
    apdu_len = len;

    rpdata.error_class = ERROR_CLASS_OBJECT;
    rpdata.error_code = ERROR_CODE_UNKNOWN_OBJECT;
    rpdata.object_type = rpmdata->object_type;
    rpdata.object_instance = rpmdata->object_instance;
    rpdata.object_property = rpmdata->object_property;
    rpdata.array_index = rpmdata->array_index;
    rpdata.application_data = &Temp_Buf[0];
    rpdata.application_data_len = sizeof(Temp_Buf);

    len = Device_Read_Property(pDev, &rpdata);
    if (len < 0) {
        if ((len == BACNET_STATUS_ABORT) || (len == BACNET_STATUS_REJECT)) {
            rpmdata->error_code = rpdata.error_code;
            /* pass along aborts and rejects for now */
            return len; /* Ie, Abort */
        }
        /* error was returned - encode that for the response */
        len =
            rpm_ack_encode_apdu_object_property_error(&Temp_Buf[0],
                rpdata.error_class, rpdata.error_code);
        copy_len =
            memcopy(&apdu[0], &Temp_Buf[0], offset + apdu_len, len, max_apdu);

        if (copy_len == 0) {
            rpmdata->error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
            return BACNET_STATUS_ABORT;
        }
    }
    else if ((offset + apdu_len + 1 + len + 1) < max_apdu) {
        /* enough room to fit the property value and tags */
        len =
            rpm_ack_encode_apdu_object_property_value(&apdu[offset + apdu_len],
                &Temp_Buf[0], len);
    }
    else {
        /* not enough room - abort! */
        rpmdata->error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
        return BACNET_STATUS_ABORT;
    }
    apdu_len += len;

    return apdu_len;
}

/** Handler for a ReadPropertyMultiple Service request.
 * @ingroup DSRPM
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 *   - if the response would be too large
 * - the result from each included read request, if it succeeds
 * - an Error if processing fails for all, or individual errors if only some fail,
 *   or there isn't enough room in the APDU to fit the data.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_PATH of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_read_property_multiple(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ROUTE * srcRoute,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    uint16_t copy_len = 0;
    uint16_t decode_len = 0;
    int pdu_len = 0;
    BACNET_NPCI_DATA npci_data;
    BACNET_RPM_DATA rpmdata;
    int apdu_len = 0;
    int npdu_len = 0;
    int error = 0;

    DLCB *dlcb = alloc_dlcb_response('j', &srcRoute->bacnetPath, pDev->datalink->max_lpdu);
    if (dlcb == NULL) return;

    /* configure default error code as an abort since it is common */
    rpmdata.error_class = ERROR_CLASS_OBJECT;
    rpmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;

    /* encode the NPDU portion of the packet */
    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &srcRoute->bacnetPath.glAdr, NULL,
        &npci_data);
    if (service_data->segmented_message) {
        rpmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
        error = BACNET_STATUS_ABORT;

        dbMessage( DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Segmented message. Sending Abort!\r\n");

        goto RPM_FAILURE;
    }
    /* decode apdu request & encode apdu reply
       encode complex ack, invoke id, service choice */
    apdu_len =
        rpm_ack_encode_apdu_init(&dlcb->Handler_Transmit_Buffer[npdu_len],
        service_data->invoke_id);
    for (;;) {
        /* Start by looking for an object ID */
        len =
            rpm_decode_object_id(&service_request[decode_len],
                service_len - decode_len, &rpmdata);
        if (len >= 0) {
            /* Got one so skip to next stage */
            decode_len += len;
        }
        else {
            /* bad encoding - skip to error/reject/abort handling */
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Bad Encoding.\n");
            error = len;
            goto RPM_FAILURE;
        }

        /* Test for case of indefinite Device object instance */
        if ((rpmdata.object_type == OBJECT_DEVICE) &&
            (rpmdata.object_instance == BACNET_MAX_INSTANCE)) {
            rpmdata.object_instance = Device_Object_Instance_Number(pDev);
        }

        /* Stick this object id into the reply - if it will fit */
        len = rpm_ack_encode_apdu_object_begin(&Temp_Buf[0], &rpmdata);
        copy_len =
            memcopy(&dlcb->Handler_Transmit_Buffer[npdu_len], &Temp_Buf[0], apdu_len,
                len, pDev->datalink->max_apdu);
        if (copy_len == 0) {
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Response too big!\r\n");
            rpmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
            error = BACNET_STATUS_ABORT;
            goto RPM_FAILURE;
        }

        apdu_len += copy_len;
        /* do each property of this object of the RPM request */
        for (;;) {
            /* Fetch a property */
            len =
                rpm_decode_object_property(&service_request[decode_len],
                    service_len - decode_len, &rpmdata);
            if (len < 0) {
                /* bad encoding - skip to error/reject/abort handling */
                dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Bad Encoding.\n");
                error = len;
                goto RPM_FAILURE;
            }
            decode_len += len;
            /* handle the special properties */
            if ((rpmdata.object_property == PROP_ALL) ||
                (rpmdata.object_property == PROP_REQUIRED) ||
                (rpmdata.object_property == PROP_OPTIONAL)) {
                struct special_property_list_t property_list;
                unsigned property_count = 0;
                unsigned index = 0;
                BACNET_PROPERTY_ID special_object_property;

                if (rpmdata.array_index != BACNET_ARRAY_ALL) {
                    /*  No array index options for this special property.
                       Encode error for this object property response */
                    len =
                        rpm_ack_encode_apdu_object_property(&Temp_Buf[0],
                            rpmdata.object_property, rpmdata.array_index);
                    copy_len =
                        memcopy(&dlcb->Handler_Transmit_Buffer[npdu_len],
                            &Temp_Buf[0], apdu_len, len, pDev->datalink->max_apdu);  // todo 2 - not safe!
                    if (copy_len == 0) {
                        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
                            "RPM: Too full to encode property!\r\n");
                        rpmdata.error_code =
                            ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                        error = BACNET_STATUS_ABORT;
                        goto RPM_FAILURE;
                    }
                    apdu_len += len;
                    len =
                        rpm_ack_encode_apdu_object_property_error(&Temp_Buf[0],
                            ERROR_CLASS_PROPERTY,
                            ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY);
                    copy_len =
                        memcopy(&dlcb->Handler_Transmit_Buffer[npdu_len],
                            &Temp_Buf[0], apdu_len, len, pDev->datalink->max_apdu);  // todo 2 - not safe!
                    if (copy_len == 0) {
                        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Too full to encode error!\r\n");
                        rpmdata.error_code =
                            ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                        error = BACNET_STATUS_ABORT;
                        goto RPM_FAILURE;
                    }
                    apdu_len += len;
                }
                else {
                    special_object_property = rpmdata.object_property;
                    Device_Objects_Property_List(rpmdata.object_type,
                        rpmdata.object_instance,
                        &property_list);
                    property_count =
                        RPM_Object_Property_Count(&property_list,
                            special_object_property);
                    if (property_count == 0) {
                        /* this only happens with the OPTIONAL property */
                        /* 135-2016bl-2. Clarify ReadPropertyMultiple   */
                        /* handle the error code - but use the special property */
                        len =
                            RPM_Encode_Property(pDev, &dlcb->Handler_Transmit_Buffer
                                [npdu_len], (uint16_t)apdu_len, pDev->datalink->max_apdu,
                                &rpmdata);
                        if (len > 0) {
                            apdu_len += len;
                        }
                        else {
                            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
                                "RPM: Too full for special property!\r\n");
                            error = len;
                            goto RPM_FAILURE;
                        }
                    }
                    else {
                        for (index = 0; index < property_count; index++) {
                            rpmdata.object_property =
                                RPM_Object_Property(&property_list,
                                    special_object_property, index);
                            len =
                                RPM_Encode_Property(
                                    pDev,
                                    &dlcb->Handler_Transmit_Buffer
                                    [npdu_len], (uint16_t)apdu_len, pDev->datalink->max_apdu,
                                    &rpmdata);
                            if (len > 0) {
                                apdu_len += len;
                            }
                            else {
                                dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
                                    "RPM: Too full for property!\r\n");
                                error = len;
                                goto RPM_FAILURE;
                            }
                        }
                    }
                }
            }
            else {
                /* handle an individual property */
                len =
                    RPM_Encode_Property(
                        pDev,
                        &dlcb->Handler_Transmit_Buffer[npdu_len],
                        (uint16_t)apdu_len, pDev->datalink->max_apdu, &rpmdata);
                if (len > 0) {
                    apdu_len += len;
                }
                else {
                    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR,
                        "RPM: Too full for individual property!\r\n");
                    error = len;
                    goto RPM_FAILURE;
                }
            }
            if (decode_is_closing_tag_number(&service_request[decode_len], 1)) {
                /* Reached end of property list so cap the result list */
                decode_len++;
                len = rpm_encode_apdu_object_end(&Temp_Buf[0]);
                copy_len =
                    memcopy(&dlcb->Handler_Transmit_Buffer[npdu_len], &Temp_Buf[0],
                        apdu_len, len, pDev->datalink->max_apdu);
                if (copy_len == 0) {
                    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Too full to encode object end!\r\n");
                    rpmdata.error_code =
                        ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                    error = BACNET_STATUS_ABORT;
                    goto RPM_FAILURE;
                }
                else {
                    apdu_len += copy_len;
                }
                break;  /* finished with this property list */
            }
        }
        if (decode_len >= service_len) {
            /* Reached the end so finish up */
            break;
        }
    }

    if (apdu_len > service_data->max_resp) {
        /* too big for the sender - send an abort */
        rpmdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
        error = BACNET_STATUS_ABORT;

        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Message too large.  Sending Abort!\n");

        goto RPM_FAILURE;
    }

RPM_FAILURE:
    if (error) {
        if (error == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id,
                abort_convert_error_code(rpmdata.error_code), true);
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Sending Abort!\n");
        }
        else if (error == BACNET_STATUS_ERROR) {
            apdu_len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id, SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
                rpmdata.error_class, rpmdata.error_code);
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Sending Error!\n");
        }
        else if (error == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                service_data->invoke_id,
                reject_convert_error_code(rpmdata.error_code));
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "RPM: Sending Reject!\n");
        }
    }

    pdu_len = apdu_len + npdu_len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);
}
