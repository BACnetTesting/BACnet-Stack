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

#include "configParams.h"
#include "datalink.h"
#include "emm.h"
#include "ese.h"
#include "logging.h"
#include "bitsUtil.h"
#include "bitsDatalink.h"
#include "handlers.h"

#if 0
DLCB *alloc_dlcb_sys(char tag, bool isResponse, uint8_t macAddress )
{
    DLCB *dlcb = (DLCB *) emm_dmalloc(tag, sizeof(DLCB));
    if (dlcb == NULL) return NULL;

    dlcb->Handler_Transmit_Buffer = (uint8_t *)emm_dmalloc(tag, MAX_NPDU);  // todo 2 - we can use portParams max_apdu here already
    if (dlcb->Handler_Transmit_Buffer == NULL)
    {
        emm_free(dlcb);
        return NULL;
    }

#if ( BAC_DEBUG == 1 )
    dlcb->signature = 'd';
#endif

    // dlcb->portParams = portParams;
    dlcb->isDERresponse = isResponse ;
    dlcb->lpduMax = MAX_NPDU;
    dlcb->optr = 0 ;
    // memset ( &dlcb->npciData2, 0, sizeof ( dlcb->npciData2 ));
    // bacnet_mac_clear ( &dlcb->phyDest ) ;
    dlcb->phyDest = macAddress;
    return dlcb;
}
#endif
 
#if 0
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
#endif

    bacnet_route_copy(&newdlcb->route, &dlcb->route);
    // bacnet_mac_copy ( &newdlcb->phyDest, &dlcb->phyDest) ;
    // newdlcb->npciData = dlcb->npciData ;
    newdlcb->source = dlcb->source;
    newdlcb->expectingReply = dlcb->expectingReply;

    return newdlcb;
}
#endif

#if 0
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
#endif

    DLCB *newdlcb = dlcb_clone_common(dlcb, dlcb->lpduMax);
    if (newdlcb == NULL) return NULL;

    newdlcb->lpduMax = dlcb->lpduMax;
    newdlcb->optr = dlcb->optr;
    memcpy(newdlcb->Handler_Transmit_Buffer, dlcb->Handler_Transmit_Buffer, dlcb->lpduMax);
    return newdlcb;
}
#endif

#if ( BAC_DEBUG == 1 )
bool dlcb_check(const DLCB *dlcb)
{
    if (dlcb->signature != 'd')
    {
        if ( dlcb->signature == 'Z' ) {
          // DLCB is already freed!
          ese_enqueue(ese008_08_duplicate_free) ;
          panic();
        }
        else {
          // probably a bad pointer to block
          ese_enqueue(ese007_07_bad_malloc_free) ;
        }
        return false ;
    }
    return true ;
}
#endif


void dlcb_free( const DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    if (!dlcb_check(dlcb)) return;
    ((DLCB *)dlcb)->signature = 'Z';
#endif // BAC_DEBUG

    // todo2 - add debug check that dlcb not already freed
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
    if (ps == NULL) {
        panic();
        return ps;
    }
    
    strcpy(
        (char *)ps->ifName,
        adapter);

    // see notes about initialization - change to one of our link-lists some day. cr2341234134141
    // ps->datalinkDevices = new std::vector<VirtualDeviceInfo *>();
    
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

void bitsDatalink_cleanup(void)
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


void bitsDatalink_tick(void) 
{
    BACNET_ADDRESS src;         /* address where message came from */
    unsigned timeout = 100;		/* milliseconds */
    static uint8_t Rx_Buf[MAX_MPDU];

    /* returns 0 bytes on timeout */
    int pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

    /* process */
    if (pdu_len) {
        npdu_handler(&src, &Rx_Buf[0], pdu_len);
    }
}
