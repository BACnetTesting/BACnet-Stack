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

#if ( BACDL_BIP == 1 )

#include "bacnet/datalink/bip.h"
#include "eLib/util/eLibDebug.h"
#include "multipleDatalink.h"
#include "bitsUtil.h"
#include "bacnet/bits/util/BACnetToString.h"

#if ( BITS_ROUTER_LAYER == 0 )
#include "bacnet/basic/npdu/h_npdu.h"
#endif

extern PORT_SUPPORT *datalinkSupportHead;

static void Datalink_Maintenance_IP(PORT_SUPPORT *datalink)
{
    // this is being called every second... (todo 4 - allow each datalink to set it's own interval)
    IP_Datalink_Watch_IP_Address( datalink);
}


PORT_SUPPORT *Init_Datalink_IP(const char *ifName, const uint16_t localIPport )
{
    PORT_SUPPORT *ps = datalink_initCommon(ifName, BPT_BIP );
    if (ps == NULL) {
        return ps;
    }

    ps->SendPdu = bip_send_npdu;
    ps->ReceiveMPDU = bip_receive;
    ps->get_MAC_address = bip_get_MAC_address;
    ps->Datalink_Maintenance = Datalink_Maintenance_IP;
    ps->isActive = bits_Datalink_isActive_IP;
    ps->max_lpdu = MAX_LPDU_IP;
    ps->max_apdu = MAX_APDU_IP;
    ps->datalink.bipParams.nwoPort = htons(localIPport);

#ifdef OS_LAYER_WIN
    bool stat = StringTo_IPaddr((struct in_addr *)&ps->datalink.bipParams.nwoLocal_addr, ifName);
    // note if parse fails, we can still possibly operate with address 0 (any), so just note it and continue
    if (!stat) {
        if (strlen(ifName)) {
            printf("Could not establish IP address for %s\r\n", ifName);
        }
    }
#endif

    if (!bip_init(ps, ps->ifName)) {
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