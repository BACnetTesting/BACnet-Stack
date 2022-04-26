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

#if ( FOREIGN_DEVICE_CLIENT == 1 )

#include "bacnet/datalink/bip.h"
#include "multipleDatalink.h"
#include "bacnet/bits/util/bitsUtil.h"
#include "bacnet/bacaddr.h"
#include "bacnet/basic/bbmd/h_bbmd.h"
#include "eLib/util/eLibUtil.h"

extern PORT_SUPPORT *datalinkSupportHead;

// This is awkward. We _only_ use the listener threads for command line utilities (virtual routers and routers have their devices
// attached to routerPorts, and command line utilities do not have routerPorts. Only datalinks. In addition, router applications
// do not use these datalinklisten threads, they have their own routerportlisten threads.
// these items should be moved out so as not to confuse up router applications, but as of now, for expedience, I am leaving them
// as baggage in router applications
DEVICE_OBJECT_DATA *ourDeviceForCommandLine;

// NOTE! These datalink threads are NOT used for any router applications, but need to remain
// in place for possible use by command line tools.

static void DatalinkListen_FD(void *pArgs)
{
    BACNET_MAC_ADDRESS srcMac;         /* address where message came from */
    static uint8_t Rx_Buf[MAX_LPDU_IP];
    PORT_SUPPORT *datalink = (PORT_SUPPORT *) pArgs;

    while (true) {
        /* returns 0 bytes on timeout */
        int pdu_len = datalink->ReceiveMPDU(datalink, &srcMac, &Rx_Buf[0], datalink->max_lpdu );

        /* process */
        if (pdu_len) {
            RX_DETAILS rxDetails;
            rxDetails.npdu = Rx_Buf;
            rxDetails.npdu_len = (uint16_t) pdu_len;
            rxDetails.portParams = datalink;
            bacnet_mac_copy(&rxDetails.srcPath.localMac, &srcMac);

            panic();
            // npdu_handler_datalink(&rxDetails, ourDeviceForCommandLine );
        }

        msSleep(100);
    }
}


static void KeepAlive_FD(void *pArgs)
{
    PORT_SUPPORT *datalink = (PORT_SUPPORT *) pArgs;
	uint16_t keepalive = 50;

    while (true) {
        bvlc_register_with_bbmd(datalink, keepalive );
        msSleep( (uint16_t) (( keepalive - 10) * 1000 ));
        // no action to be taken on 'successful completion' ?
    }
}


PORT_SUPPORT *Init_Datalink_FD(const char *ifName, const uint16_t remoteIPpo)
{
    PORT_SUPPORT *ps = datalink_initCommon(ifName, BPT_FD );
    if (ps == NULL) {
        return ps;
    }

    ps->datalink.bipParams.fd_ipep.sin_addr.s_addr = inet_addr(remoteName);
    ps->datalink.bipParams.fd_ipep.sin_port = htons(remoteIPport);


    ps->datalink.bipParams.BVLC_NAT_Handling = false;
    ps->max_lpdu = MAX_LPDU_IP;
    ps->max_apdu = MAX_APDU_IP;
    ps->datalink.bipParams.nwoPort = htons(ipPort);
    ps->SendPdu = fd_send_npdu;
    ps->ReceiveMPDU = bip_receive;
    ps->get_MAC_address = bip_get_MAC_address;
    ps->datalink.bipParams.nwoPort = 0 ;                // FD uses ephemeral local port
    ps->datalink.bipParams.BVLC_NAT_Handling = false;
    if (!bip_init(ps, ps->ifName)) {
        return NULL ;
    }

#ifdef _MSC_VER
    uintptr_t rcode;
    rcode = _beginthread(DatalinkListen_FD, 0, ps);
    if (rcode == -1L) {
        dbMessage(DBD_ALL, DB_ERROR, "Failed to create thread");
    }
    
    rcode = _beginthread(KeepAlive_FD, 0, ps);
    if (rcode == -1L) {
        dbMessage(DBD_ALL, DB_ERROR, "Failed to create thread");
    }
#else
    bitsCreateThread(DatalinkListen_FD, NULL);
    bitsCreateThread(KeepAlive_FD, NULL );
#endif

    LinkListAppend((void **)&datalinkSupportHead, ps);     // Regular port structures

    return ps;
}

#endif // #if ( FOREIGN_DEVICE_CLIENT == 1 )
