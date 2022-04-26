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

#if ( BACDL_ETHERNET == 1 )

#include "eLib/util/eLibUtil.h"
#include "eLib/util/eLibDebug.h"
#include "eLib/util/emm.h"
#include "multipleDatalink.h"
#include "bitsUtil.h"
#include "bacnet/bits/util/BACnetToString.h"
#include "bacnet/datalink/ethernet.h"

#if ( BITS_ROUTER_LAYER == 0 )
#include "bacnet/basic/npdu/h_npdu.h"
#endif

extern PORT_SUPPORT *datalinkSupportHead;

static void Datalink_Ethernet_Maintenance(PORT_SUPPORT *datalink)
{
    // this is being called every second... (todo 4 - allow each datalink to set it's own interval)
}


PORT_SUPPORT *Init_Datalink_Ethernet(const char *ifName )
{
    PORT_SUPPORT *ps = datalink_initCommon(ifName, BPT_Ethernet );
    if (ps == NULL) {
        return ps;
    }

    ps->SendPdu = ethernet_send_npdu;
    ps->ReceiveMPDU = ethernet_receive;
    ps->get_MAC_address = ethernet_get_MAC_address;
    ps->Datalink_Maintenance = Datalink_Ethernet_Maintenance;
    ps->isActive = bits_Datalink_isActive_NoOp;
    ps->localMAC = (BACNET_MAC_ADDRESS *) emm_calloc(1, sizeof(BACNET_MAC_ADDRESS));
    if (!ps->localMAC)
    {
        emm_free(ps);
        return NULL;
    }
    ps->max_lpdu = MAX_LPDU_ETHERNET;
    ps->max_apdu = MAX_APDU_ETHERNET;
    if (!ethernet_init(ps, ps->ifName)) {
        datalink_destroyCommon(ps);
        return NULL ;
    }

#if ( BITS_ROUTER_LAYER == 0 )
    // The router layer has its own datalink handling functions, so if router layer enabled, we do not
    // intialize the datalink threads..
    Datalink_Initialize_Thread(ps);
    // todo 2 - cleanup on failure, return null
#endif

    // 2020-08-16 EKH: This is done at the calling function (and should be)
    // LinkListAppend((void **)&datalinkSupportHead, ps);

    return ps;
}

#endif // BACDL_*