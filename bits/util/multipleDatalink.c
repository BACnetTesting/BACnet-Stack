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

// #include "datalink.h"
#include "emm.h"
#include "ese.h"
#include "logging.h"
// #include "configParams.h"
#include "bitsUtil.h"
#include "multipleDatalink.h"

DLCB *alloc_dlcb_sys(char typ, bool isResponse, const BACNET_ROUTE *route)
{
    DLCB *dlcb = (DLCB *)emm_dmalloc(typ, sizeof(DLCB));
    if (dlcb == NULL) return NULL;

    dlcb->Handler_Transmit_Buffer = (uint8_t *)emm_dmalloc(typ, route->portParams->max_lpdu);
    if (dlcb->Handler_Transmit_Buffer == NULL)
    {
        emm_free(dlcb);
        return NULL;
    }

#if ( BAC_DEBUG == 1 )
    dlcb->signature = 'd';
#endif

    bacnet_route_copy(&dlcb->route, route);
    dlcb->source = typ;            // 'a'pp or 'm'stp (in future, ethernet(s), other mstp(s) wifi
    dlcb->lpduMax = route->portParams->max_lpdu;
    dlcb->optr = 0;
    // memset ( &dlcb->npciData, 0, sizeof ( dlcb->npciData ));
    // no. route is passed to us as a parameter bacnet_route_clear( &dlcb->route ) ;
    dlcb->expectingReply = isResponse ;

    return dlcb;
}


static DLCB *dlcb_clone_common(const DLCB *dlcb, uint16_t len)
{
#if (BAC_DEBUG == 1 )
    dlcb_check(dlcb);
#endif

    DLCB *newdlcb = (DLCB *)emm_dmalloc('k', sizeof(DLCB));
    if (newdlcb == NULL) return NULL;

    newdlcb->Handler_Transmit_Buffer = (uint8_t *)emm_dmalloc('h', len);
    if (newdlcb->Handler_Transmit_Buffer == NULL)
    {
        emm_free(newdlcb);
        return NULL;
    }

#if ( BAC_DEBUG == 1 )
    newdlcb->signature = dlcb->signature;
#else
#error
#endif

    bacnet_route_copy(&newdlcb->route, &dlcb->route);
    // bacnet_mac_copy ( &newdlcb->phyDest, &dlcb->phyDest) ;
    // newdlcb->npciData = dlcb->npciData ;
    newdlcb->source = dlcb->source;
    newdlcb->expectingReply = dlcb->expectingReply;

    return newdlcb;
}


DLCB *dlcb_clone_with_copy_of_buffer(const DLCB *dlcb, const uint16_t len, const uint8_t *buffer)
{
    DLCB *newdlcb = dlcb_clone_common(dlcb, len);
    if (newdlcb == NULL) return NULL;

    newdlcb->lpduMax = len;
    newdlcb->optr = len;
    memcpy(newdlcb->Handler_Transmit_Buffer, buffer, len);  // todo3, start using safebuffers, with associated lengths
    return newdlcb;
}


DLCB *dlcb_clone_deep(const DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    if (dlcb->lpduMax > 1500)
    {
        panic();
        return NULL;
    }
    if (dlcb->optr == 0)
    {
        panic();
        return NULL;
    }
#else
#error
#endif

    DLCB *newdlcb = dlcb_clone_common(dlcb, dlcb->lpduMax);
    if (newdlcb == NULL) return NULL;

    newdlcb->lpduMax = dlcb->lpduMax;
    newdlcb->optr = dlcb->optr;
    memcpy(newdlcb->Handler_Transmit_Buffer, dlcb->Handler_Transmit_Buffer, dlcb->lpduMax);
    return newdlcb;
}

#if ( BAC_DEBUG == 1 )
bool dlcb_check(const DLCB *dlcb)
{
    if (dlcb->signature != 'd')
    {
        if (dlcb->signature == 'Z') {
            // DLCB is already freed!
            ese_enqueue(ese008_08_duplicate_free);
            panic();
        }
        else {
            // probably a bad pointer to block
            ese_enqueue(ese007_07_bad_malloc_free);
            panic();
        }
        return false;
    }
    return true;
}
#else
#error
#endif

void dlcb_free(DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    if (!dlcb_check(dlcb)) return;
    ((DLCB *)dlcb)->signature = 'Z';
#else
#error
#endif // DEBUG

    emm_free(dlcb->Handler_Transmit_Buffer);
    emm_free((void *)dlcb);
}


PORT_SUPPORT *datalinkSupportHead;

PORT_SUPPORT *datalink_initCommon2(const char *adapter, const BPT_TYPE rpt, const uint16_t maxLPDU )
{
    static uint8_t datalinkId;

    PORT_SUPPORT *ps = (PORT_SUPPORT *)emm_scalloc('b', sizeof(PORT_SUPPORT));
    if (ps == NULL) {
        panic();
        return ps;
    }
    ps->datalinkId = ++datalinkId;
    ps->portType = rpt;
    ps->max_lpdu = maxLPDU;

    ps->ifName = (const char *)malloc(strlen(adapter) + 1);
    strcpy(
        (char *)ps->ifName,
        adapter);

    return ps;
}


#if 0
PORT_SUPPORT *InitDatalink(const char *adapter, const BPT_TYPE rpt, const uint16_t ipPort)
{
    PORT_SUPPORT *ps = datalink_initCommon2(adapter, rpt, MAX_LPDU_IP );
    if (ps == NULL) {
        return ps;
    }

    switch (rpt)
    {
    default:
        panic();
        emm_free(ps);
        return NULL ;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case BPT_FD:
        ps->datalink.bipParams.BVLC_NAT_Handling = false;
        ps->SendPdu = fd_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        if ( ! bip_init(ps, ps->ifName ) )
        {
            return NULL ;
        }
        break;
#endif

    case BPT_BIP:
        ps->SendPdu = bip_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        if (!bip_init(ps, ps->ifName))
        {
            return NULL ;
        }
        break;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case BPT_BBMD:
        if ( ! Init_Datalink_BBMD(ps, ipPort ) )
        {
            return NULL ;
        }
        break;

    case BPT_NAT:
        ps->SendPdu = bbmd_send_npdu;
        ps->ReceiveMPDU = bbmd_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        ps->datalink.bipParams.BVLC_NAT_Handling = true ;
        if (!bip_init(ps, ps->ifName))
        {
            return NULL;
    }
        break;
#endif

#if 0   // todo
    case DL_MSTP:
        ps->SendPdu = dlmstp_send_pdu;
        ps->ReceiveMPDU = dlmstp_receive_pdu;
        ps->get_MAC_address = dlmstp_get_MAC_address;
        ps->max_apdu = MAX_APDU_MSTP;
        dlmstp_init(ps, "COM15");              // todo ks - same as above
        break;
#endif

    }


#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    // need to clear BDT after initializing the port, since we need to insert the port address
    bbmd_clear_bdt_local(ps);   // at present, all PORT_SUPPORTS have FD, BBMD tables. todo4 eliminate unused instances. esp MSTP?
    bbmd_clear_fdt_local(ps);
#endif

    LinkListAppend((void **)&datalinkSupportHead, ps);     // Regular port structures

    return ps ;
}
#endif

void datalink_cleanup(void)
{
    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next)
    {
        switch (ps->portType)
        {
        case BPT_BIP:
            panic();
            // bip_cleanup(ps);
            break;
        case BPT_BBMD:
            panic();
            // bip_cleanup(ps);
            break;
        default:
            panic();
            break;
        }
    }
}


// Do not blindly remove due to persistence, this is still used by Linux command line utils
// But it cannot be used for Full Routing (only one port)
//void SetConfigDefaults(ConfigType *config)
//{
//#if defined ( _MSC_VER )
//    config->ifName = "192.168.1.101";
//#elif defined ( __GNUC__ )
//// for vmware    config->ifName = "ens33";
//    config->ifName = "wlan0"; // "eth0";                    // for Laptop
//#else
//#error - set for your platform
//#endif
//
//    // Regular test BACnet Port (ad-hoc local network)
//    config->localBACnetPort = 0xc200;
//    config->localNetworkNumber = 0xd200;
//
//    // For BBMD
//    config->localBBMDbacnetPort = 0xc201;
//    config->localBBMDnetworkNumber = 0xd201;
//
//    // For BBMD with NAT
//    config->globalBACnetPort = 57809;           // Port forwarded from Public IP #2
//    config->globalNetworkNumber = 0xd202;
//    config->natRouting = true;
//
//    config->virtualNetworkNumber = 0xd203;
//}
