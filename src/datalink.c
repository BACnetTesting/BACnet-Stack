/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
//#include "ethernet.h"
#include "bip.h"
#include "bvlc.h"
//#include "arcnet.h"
#include "dlmstp.h"
#include "datalink.h"
//#include <string.h>
#include "emm.h"
#include "CEDebug.h"

/** @file datalink.c  Optional run-time assignment of datalink transport */

#if defined(BACDL_ALL) || defined FOR_DOXYGEN
/* Function pointers - point to your datalink */

/** Function template to Initialize the DataLink services at the given interface.
 * @ingroup DLTemplates
 *
 * @note For Linux, ifname is eth0, ath0, arc0, ttyS0, and others.
         For Windows, ifname is the COM port or dotted ip address of the interface.

 * @param ifname [in] The named interface to use for the network layer.
 * @return True if the interface is successfully initialized,
 *         else False if the initialization fails.
 */
bool(*datalink_init) (char *ifname);

/** Function template to send a packet via the DataLink.
 * @ingroup DLTemplates
 *
 * @param dest [in] Destination address.
 * @param npdu_data [in] The NPDU header (Network) information.
 * @param pdu [in] Buffer of data to be sent - may be null.
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
int (
    *datalink_send_pdu) (
    BACNET_PATH * dest,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * pdu,
    unsigned pdu_len);

uint16_t(*datalink_receive) (BACNET_PATH * src, uint8_t * pdu,
    uint16_t max_pdu, unsigned timeout);

/** Function template to close the DataLink services and perform any cleanup.
 * @ingroup DLTemplates
 */
void (
    *datalink_cleanup) (
    void);

void (
    *datalink_get_broadcast_address) (
    BACNET_PATH * dest);

void (
    *datalink_get_my_address) (
    BACNET_PATH * my_address);

#endif


DLCB *alloc_dlcb_sys(char typ, const DLINK_SUPPORT *portParams)
{
    DLCB *dlcb = (DLCB *)emm_dmalloc(typ, sizeof(DLCB));
    if (dlcb == NULL) return NULL;

    dlcb->Handler_Transmit_Buffer = (uint8_t *)emm_dmalloc(typ, portParams->max_apdu );
    if (dlcb->Handler_Transmit_Buffer == NULL)
    {
        emm_free(dlcb);
        return NULL;
    }

#if ( BAC_DEBUG == 1 )
    dlcb->signature = 'd';
#endif

    dlcb->portParams = portParams;
    dlcb->source = typ;            // 'a'pp or 'm'stp (in future, ethernet(s), other mstp(s) wifi
    dlcb->bufSize = portParams->max_apdu ;
    return dlcb;
}


DLCB *dlcb_deep_clone(const DLCB *dlcb)
{
#if ( BAC_DEBUG == 1 ) && ! defined ( _MSC_VER )
    if (dlcb->signature != 'd')
    {
        SendBTApanicMessage("mxxxx - Bad signature 2");
        return NULL;
    }
#endif

    DLCB *newdlcb = (DLCB *)emm_dmalloc('k', sizeof(DLCB));
    if (newdlcb == NULL) return NULL;

    newdlcb->Handler_Transmit_Buffer = (uint8_t *)emm_dmalloc('h', dlcb->bufSize);
    if (newdlcb->Handler_Transmit_Buffer == NULL)
    {
        emm_free(newdlcb);
        return NULL;
    }

#if ( BAC_DEBUG == 1 )
    newdlcb->signature = 'd';
#endif

    newdlcb->portParams = dlcb->portParams;
    newdlcb->bufSize = dlcb->bufSize;
    memcpy(newdlcb->Handler_Transmit_Buffer, dlcb->Handler_Transmit_Buffer, dlcb->bufSize);	// todo3, start using safebuffers, with associated lengths
    return newdlcb;
}


void dlcb_free(const DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    if (dlcb->signature != 'd')
    {
        // bad ptr/already free
        panic();
        return;
    }
    ((DLCB *)dlcb)->signature = 'Z';
#endif // BAC_DEBUG

    // todo2 - add debug check that dlcb not already freed
    emm_free(dlcb->Handler_Transmit_Buffer);
    emm_free((void *) dlcb);
}


DLINK_SUPPORT *datalinkSupportHead;

DLINK_SUPPORT *InitDatalink(DL_TYPE rpt, uint16_t ipPort )
{
    static uint8_t portId;

    DLINK_SUPPORT *ps = (DLINK_SUPPORT *)emm_dmalloc('b', sizeof(DLINK_SUPPORT));
    if (ps == NULL) return NULL;

    ps->port_id2 = ++portId;

    ps->portType = rpt;

    switch (rpt)
    {
    default:
        panic();
        emm_free(ps);
        return NULL;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case DL_FD:
        ps->bipParams.BVLC_NAT_Handling = false;
        ps->SendPdu = fd_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->bipParams.nwoPort = htons(ipPort);
        ps->max_apdu = MAX_MPDU_IP;
        bip_init(ps, "192.168.1.101" );     // todo ks - karg only allows one interface at a time. We need a method to set multiple... I will think of something todo ekh
        break;
#endif

    case DL_BIP:
        ps->SendPdu = bip_send_npdu;
        ps->ReceiveMPDU = bip_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->bipParams.nwoPort = htons(ipPort);
        ps->max_apdu = MAX_MPDU_IP;
        bip_init(ps, "192.168.1.101");
        break;

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    case DL_BBMD:
        ps->SendPdu = bbmd_send_npdu;
        ps->ReceiveMPDU = bbmd_receive;
        ps->get_MAC_address = bip_get_MAC_address;
        ps->bipParams.nwoPort = htons( ipPort ) ;
        ps->bipParams.BVLC_NAT_Handling = false;
        ps->max_apdu = MAX_MPDU_IP;
        bip_init (ps, "192.168.1.101");
        break;
#endif

#if 0
    case DL_MSTP:
        ps->SendPdu = dlmstp_send_pdu;
        ps->ReceiveMPDU = dlmstp_receive_pdu;  
        ps->get_MAC_address = dlmstp_get_MAC_address;
        ps->max_apdu = MAX_APDU_MSTP;
        dlmstp_init(ps, "COM15" );              // todo ks - same as above
        break;
#endif

    }


#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    // need to clear BDT after initializing the port, since we need to insert the port address
    bbmd_clear_bdt_local(ps);   // at present, all PORT_SUPPORTS have FD, BBMD tables. todo4 eliminate unused instances. esp MSTP?
    bbmd_clear_fdt_local(ps);
#endif

    LinkListAppend((void **)&datalinkSupportHead, ps);     // Regular port structures

    return ps;
}

void datalink_cleanup(void)
{
    for (DLINK_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (DLINK_SUPPORT *)ps->llist.next)
    {
        panic();
        // ps->
    }
}