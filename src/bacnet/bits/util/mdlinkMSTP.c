/****************************************************************************************
 *
 *   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 *   GNU Lesser General Public License for more details.
 *
 *   As a special exception, if other files instantiate templates or
 *   use macros or inline functions from this file, or you compile
 *   this file and link it with other works to produce a work based
 *   on this file, this file does not by itself cause the resulting
 *   work to be covered by the GNU General Public License. However
 *   the source code for this file must still be made available in
 *   accordance with section (3) of the GNU General Public License.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *   For more information : info@bac-test.com
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#include "configProj.h"

#if ( BACDL_MSTP == 1 )

#include "multipleDatalink.h"
#include "bacnet/datalink/dlmstp.h"

extern PORT_SUPPORT *datalinkSupportHead;

static void Datalink_Maintenance_MSTP(PORT_SUPPORT *datalink)
{
    // this is being called every second... (todo 4 - allow each datalink to set it's own interval)
}


PORT_SUPPORT *Init_Datalink_MSTP(const char *ifName, const uint16_t localMAC)
{
    PORT_SUPPORT *ps = datalink_initCommon(ifName, BPT_MSTP );
    if (ps == NULL) {
        return ps;
    }

    ps->SendPdu = dlmstp_send_npdu;
    ps->ReceiveMPDU = dlmstp_receive_npdu;
    ps->get_MAC_address = dlmstp_get_MAC_address;
    ps->Datalink_Maintenance = Datalink_Maintenance_MSTP;
    ps->isActive = bits_Datalink_isActive_IP;
    ps->max_lpdu = MAX_LPDU_MSTP;
    ps->max_apdu = MAX_APDU_MSTP;
    if (!dlmstp_init(ps, ps->ifName))
    {
        datalink_destroyCommon(ps);
        return NULL;
    }

    // todo 0 - check < 127, check max_masters
    ps->datalink.mstpParams.mstpPort.This_Station = (uint8_t) localMAC;

    ll_Init(&ps->datalink.mstpParams.mstpPort.outputQueueMSTP, 128);

#if ( BITS_ROUTER_LAYER == 0 )
    // The router layer has its own datalink handling functions, so if router layer enabled, we do not
    // intialize the datalink threads..
    Datalink_Initialize_Thread(ps);
    // todo 2 - cleanup on failure, return null
#endif

    return ps;
}

#endif // BACDL_*