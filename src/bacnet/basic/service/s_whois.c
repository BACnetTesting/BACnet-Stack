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

//#include "datalink.h"
#include "bacnet/dcc.h"
#include "bacnet/whois.h"
//#include "debug.h"
#include "bacnet/bacaddr.h"

/** @file s_whois.c  Send a Who-Is request. */

extern volatile struct  mstp_port_struct_t *tx_mstp_port;

/** Send a Who-Is request to a remote network for a specific device, a range,
 * or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param target_address [in] BACnet address of target router
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_To_Network(
    DEVICE_OBJECT_DATA *sendingDev,
    BACNET_ROUTE * dest,
    int32_t low_limit,
    int32_t high_limit)
{
    int len ;
    int pdu_len ;
    BACNET_NPCI_DATA npci_data;

	DLCB *dlcb = alloc_dlcb_response('t', &dest->bacnetPath, sendingDev->datalink->max_lpdu);
	if (dlcb == NULL) return;

    //datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu( dlcb->Handler_Transmit_Buffer, &dest->bacnetPath.glAdr,
        NULL, &npci_data);
    /* encode the APDU portion of the packet */
    len =
        whois_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len], low_limit,
                          high_limit);
    pdu_len += len;

    dbMessage(DBD_ALL, DB_NOTE, "Sending who-is %d:%d to dl:%d", low_limit, high_limit, sendingDev->datalink->datalinkId);

    dlcb->optr = pdu_len;
    sendingDev->datalink->SendPdu(sendingDev->datalink, dlcb);

}

/** Send a global Who-Is request for a specific device, a range, or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */

void Send_WhoIs_Global(
    DEVICE_OBJECT_DATA *sendingDev,
    const int32_t low_limit,
    const int32_t high_limit)
{
    BACNET_ROUTE dest;

    if (!dcc_communication_enabled(sendingDev))
        return;

    dest.portParams = sendingDev->datalink;

    /* Who-Is is a global broadcast */
    bacnet_path_set_broadcast_global(&dest.bacnetPath );

    Send_WhoIs_To_Network(sendingDev, &dest, low_limit, high_limit);
}

/** Send a local Who-Is request for a specific device, a range, or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_Local(
    PORT_SUPPORT  *portParams,
    DEVICE_OBJECT_DATA *sendingDev,
    int32_t low_limit,
    int32_t high_limit)
{
    BACNET_ROUTE dest;

    if (!dcc_communication_enabled(sendingDev))
        return;

    dest.portParams = portParams;
    bacnet_path_set_broadcast_local(&dest.bacnetPath );

    Send_WhoIs_To_Network(sendingDev, &dest, low_limit, high_limit);
}

/** Send a Who-Is request to a remote network for a specific device, a range,
 * or any device.
 * @ingroup DMDDB
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param target_address [in] BACnet address of target router
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_Remote(
    PORT_SUPPORT  *portParams,
    DEVICE_OBJECT_DATA *sendingDev,
    BACNET_PATH * target_address,
    int32_t low_limit,
    int32_t high_limit)
{
    BACNET_ROUTE dest;

    if (!dcc_communication_enabled(sendingDev))
        return;

    dest.portParams = portParams;
    bacnet_path_copy(&dest.bacnetPath, target_address);

    Send_WhoIs_To_Network( sendingDev, &dest, low_limit, high_limit);
}

/** Send a global Who-Is request for a specific device, a range, or any device.
 * @ingroup DMDDB
 * This was the original Who-Is broadcast but the code was moved to the more
 * descriptive Send_WhoIs_Global when Send_WhoIs_Local and Send_WhoIsRemote was
 * added.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
//void Send_WhoIs(
//    PORT_SUPPORT  *portParams,
//    DEVICE_OBJECT_DATA *pDev,
//    int32_t low_limit,
//    int32_t high_limit)
//{
//    Send_WhoIs_Global( pDev, low_limit, high_limit);
//}
