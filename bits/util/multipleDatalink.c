/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#ifdef _MSC_VER
#include <process.h>
#endif

#include "datalink.h"
#include "emm.h"
#include "ese.h"
#include "logging.h"
#include "configParams.h"
#include "bitsUtil.h"

DLCB *alloc_dlcb_sys(char typ, const BACNET_ROUTE *route)
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
    dlcb->expectingReply = false;

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
    memcpy(newdlcb->Handler_Transmit_Buffer, buffer, len);	// todo3, start using safebuffers, with associated lengths
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

void dlcb_free(const DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    dlcb_check(dlcb);
    ((DLCB *)dlcb)->signature = 'Z';
#else 
#error
#endif // DEBUG

    emm_free(dlcb->Handler_Transmit_Buffer);
    emm_free((void *)dlcb);
}





PORT_SUPPORT *datalinkSupportHead;
// todo remove everywhere? extern ConfigType config;

bool InitDatalink(PORT_SUPPORT *ps, const char *adapter, const PF_TYPE rpt, const uint16_t ipPort)
{
    static uint8_t portId;

    ps->port_id = ++portId;
    ps->portType = rpt;


    ps->ifName = (const char *) malloc ( strlen(adapter) + 1 ) ;
    strcpy (
    		(char *) ps->ifName,
			adapter ) ;

    switch (rpt)
    {
    default:
        panic();
        //        emm_free(ps);
        return false;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case PF_FD:
        ps->datalink.bipParams.BVLC_NAT_Handling = false;
        ps->SendPdu = fd_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        ps->max_lpdu = MAX_LPDU_IP;
        if ( bip_init(ps, ps->ifName ) < 0 )
        {
        	return false ;
        }
        // Init_Router_Thread( RouterListenIP, rp );
        break;
#endif

    case PF_BIP:
        ps->SendPdu = bip_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        ps->max_lpdu = MAX_LPDU_IP;
        if ( bip_init(ps, ps->ifName ) < 0 )
        {
        	return false ;
        }
        // Init_Router_Thread(RouterListenIP, rp);
        break;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case PF_BBMD:
        ps->SendPdu = bbmd_send_npdu;
        ps->ReceiveMPDU = bbmd_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        ps->datalink.bipParams.BVLC_NAT_Handling = false;
        ps->max_lpdu = MAX_LPDU_IP;
        if ( bip_init(ps, ps->ifName ) < 0 )
        {
        	return false ;
        }
        log_puts("BBMD Enabled");
//         Init_Router_Thread(RouterListenIP, rp);
        break;

    case PF_NAT:
        ps->SendPdu = bbmd_send_npdu;
        ps->ReceiveMPDU = bbmd_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->datalink.bipParams.nwoPort = htons(ipPort);
        ps->datalink.bipParams.BVLC_NAT_Handling = true ;
        ps->max_lpdu = MAX_LPDU_IP;
        if (bip_init(ps, ps->ifName) < 0)
        {
            return false;
    }
        log_puts("BBMD with NAT Enabled");
        //         Init_Router_Thread(RouterListenIP, rp);
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

    return true;
}


void datalink_cleanup(void)
{
    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next)
    {
        switch (ps->portType)
        {
        case PF_BIP:
            bip_cleanup(ps);
            break;
        case PF_BBMD:
            bip_cleanup(ps);
            break;
        default:
            panic();
            break;
        }
    }
}

//void SetConfigDefaults(ConfigType *config)
//{
//#if defined ( _MSC_VER )
//	config->ifName = "192.168.1.101";
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
