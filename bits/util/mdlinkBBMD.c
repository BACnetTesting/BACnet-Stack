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

#ifdef _MSC_VER
#include <process.h>
#endif

#include "bip.h"
#include "handlers.h"
#include "bitsDebug.h"
#include "multipleDatalink.h"
#include "BACnetToString.h"

extern PORT_SUPPORT *datalinkSupportHead;

// extern DEVICE_OBJECT_DATA applicationDevice;

#if 0
// this functionality is reserved for non-routing model.
// it has been polluted by a routing attempt, clean it up when
// next looking at multiple Datalink model.
#ifdef _MSC_VER
static void DatalinkListen_BBMD(void *pArgs)
#else
static void* DatalinkListen_BBMD(void *pArgs)
#endif
{
    PORT_SUPPORT *datalink = (PORT_SUPPORT *) pArgs;

    while (true) {

        BACNET_ROUTE src;         /* address where message came from */
        static uint8_t rxBuf[MAX_LPDU_IP];

        bacnet_route_clear(&src);

        // Just a reminder, much activity can go on in ReceiveMPDU, e.g. BBMD handling, that results in this function returning
        // a length of 0, perfectly normal operation.
        int pdu_len = bbmd_receive(datalink, &src.bacnetPath.localMac, rxBuf, sizeof(rxBuf) );

        /* process */
        if (pdu_len) {

            ROUTER_MSG rtmsg;
            RX_DETAILS rxDetails;

            src.portParams = datalink;

            rxDetails.npdu = rxBuf;
            rxDetails.npdu_len = pdu_len;
            rxDetails.block_valid = 'R';
            // contained in src rxDetails.portParams = rp->port_support; // more duplication..

            rtmsg.rxDetails = &rxDetails;
            rtmsg.sourceRouterPort = rp;

            bacnet_address_clear(&rxDetails.srcPath.glAdr);

            int bpdu_offset = npci_decode(rxBuf, &rtmsg.dest,
                &rxDetails.srcPath.glAdr, &rtmsg.npdu_data);

            rtmsg.bpdu_len = pdu_len - bpdu_offset;
            rtmsg.bpdu = &rxBuf[bpdu_offset];

            // a bit of duplication here
            // rtmsg.src = rxDetails.srcPath.glAdr;

            // todo1 - deadly embrace in linux
            // LockTransaction(stackLock);
            handle_npdu_router(&rtmsg);
            // UnlockTransaction(stackLock);


        }
    }
}
#endif


PORT_SUPPORT *Init_Datalink_BBMD(const char *ifName, const uint16_t localIPport )
{
    PORT_SUPPORT *ps = datalink_initCommon2(ifName, BPT_BBMD, MAX_LPDU_IP);
    if (ps == NULL) {
        return ps;
    }

    // dlenv_init();
    ps->SendPdu = bbmd_send_npdu;
    ps->ReceiveMPDU = bbmd_receive;
    ps->get_MAC_address = bip_get_MAC_address;
    ps->datalink.bipParams.nwoPort = htons(localIPport);

    bool stat = StringTo_IPaddr((struct in_addr *) &ps->datalink.bipParams.nwoLocal_addr, ifName);
    // note if parse fails, we can still possibly operate with address 0 (any), so just note it and continue
    if (!stat) {
        if (strlen(ifName)) {
            printf("Could not establish IP address for %s\r\n", ifName);
        }
    }

    ps->datalink.bipParams.BVLC_NAT_Handling = false;
    if (!bip_init(ps, ps->ifName)) {
        return NULL ;
    }

    bbmd_clear_bdt_local(ps);   // at present, all PORT_SUPPORTS have FD, BBMD tables. todo4 eliminate unused instances. esp MSTP?
    bbmd_clear_fdt_local(ps);

#if 0
    // this is for non-routing, routing will run its own thread
#ifdef _MSC_VER
    uintptr_t rcode;
    rcode = _beginthread(DatalinkListen_BBMD, 0, ps);
    if (rcode == -1L) {
        syslog(LOG_ERR, "Failed to create thread");
    }
#else
    int rcode;
    pthread_t threadvar;
    rcode = pthread_create(&threadvar, NULL,
        (void *(*)(void *)) DatalinkListen_BBMD, datalink);
    if (rcode != 0) {
        log_printf("Failed to create thread");
    }
    // so we don't have to wait for the thread to complete before exiting main()
    pthread_detach(threadvar);
#endif
#endif // 0

    LinkListAppend((void **)&datalinkSupportHead, ps);

    return ps;
}
