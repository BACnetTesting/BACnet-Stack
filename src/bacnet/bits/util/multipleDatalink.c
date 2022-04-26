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

#include <stdint.h>
#include "configProj.h"
#include "eLib/util/emm.h"
#include "eLib/util/ese.h"
#include "bacnet/bits/util/multipleDatalink.h"
#include "bacnet/datalink/bip.h"
#include "bacnet/bacaddr.h"

#if ( BAC_DEBUG == 1 )
#define SIGNATURE_ALLOC     'a'
#endif

DLCB *dlcb_alloc (char type, uint16_t bufSize)
{
    // large one first
    uint8_t *buf = (uint8_t *)emm_dmalloc(typ, bufSize);
    if (buf == NULL) return NULL ;

    DLCB *dlcb = (DLCB *)emm_dcalloc(typ, sizeof(DLCB));
    if (dlcb == NULL) {
        emm_free(buf);
        return NULL ;
    }

    dlcb->Handler_Transmit_Buffer = buf;

#if ( BAC_DEBUG == 1 )
    dlcb->signature = SIGNATURE_ALLOC ;
    bacnet_path_clear(&dlcb->bacnetPath);
#endif

    dlcb->source = type;    // 'a'pp or 'm'stp (in future, ethernet(s), other mstp(s) wifi
    dlcb->lpduMax = bufSize;

    return dlcb;
}


DLCB *dlcb_alloc_with_buffer_clone(uint16_t len, uint8_t *buffer) 
{
    DLCB *dlcb = dlcb_alloc('?', len);
    if (dlcb == NULL) return NULL;

    memcpy(dlcb->Handler_Transmit_Buffer, buffer, len);
    // for a clone, we want to preserve the optr
    dlcb->optr = len;

    return dlcb;
}


DLCB *dlcb_sys_alloc(char typ, bool isResponse, const BACNET_PATH *bacnetPath, const uint16_t size )
{
    DLCB *dlcb = dlcb_alloc ( typ, size );
    if (dlcb == NULL) return NULL;

    bacnet_path_copy(&dlcb->bacnetPath, bacnetPath);
    dlcb->expectingReply = isResponse ;

    return dlcb;
}


static DLCB *dlcb_clone_common(const DLCB *dlcb, uint16_t len)
{
#if (BAC_DEBUG == 1 )
    dlcb_check(dlcb);
#endif

    DLCB *newdlcb = dlcb_alloc('k', len);
    if (newdlcb == NULL) return NULL;

    bacnet_path_copy(&newdlcb->bacnetPath, &dlcb->bacnetPath);
    newdlcb->source = dlcb->source;
    newdlcb->expectingReply = dlcb->expectingReply;
    newdlcb->isBroadcastWhoIs = dlcb->isBroadcastWhoIs;

    return newdlcb;
}


DLCB *dlcb_clone_with_copy_of_buffer(const DLCB *dlcb, const uint16_t len, const uint8_t *buffer)
{
    DLCB *newdlcb = dlcb_clone_common(dlcb, len);
    if (newdlcb == NULL) return NULL;

    newdlcb->optr = len;
    memcpy(newdlcb->Handler_Transmit_Buffer, buffer, len);  // todo 0, start using safebuffers, with associated lengths

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

    newdlcb->optr = dlcb->optr;
    memcpy(newdlcb->Handler_Transmit_Buffer, dlcb->Handler_Transmit_Buffer, dlcb->lpduMax);

    return newdlcb;
}

#if ( BAC_DEBUG == 1 )
bool dlcb_check(const DLCB *dlcb)
{
    if (dlcb->signature != SIGNATURE_ALLOC )
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
#endif


void dlcb_free( const DLCB *dlcb)
{
#if (BAC_DEBUG == 1 )
    if (!dlcb_check(dlcb)) return;
    ((DLCB *)dlcb)->signature = 'Z';
#endif // BAC_DEBUG

    emm_free(dlcb->Handler_Transmit_Buffer);
    emm_free((void *)dlcb);
}


PORT_SUPPORT *datalinkSupportHead;

PORT_SUPPORT *datalink_initCommon(
	const char *adapter,
	const BPT_TYPE rpt)
{
    static uint8_t datalinkId;

    PORT_SUPPORT *ps = (PORT_SUPPORT *)emm_scalloc('b', sizeof(PORT_SUPPORT));
    if (ps == NULL) {
        panic();
        return ps;
    }
    ps->datalinkId = ++datalinkId;
    ps->portType = rpt;
    
    ps->ifName = (const char *)emm_scalloc('c', (uint16_t) strlen(adapter) + 1);
    if (ps == NULL) {
        panic();
        return ps;
    }
    
    strcpy( (char *)ps->ifName, adapter);

    // 2020-08-16 Moved to Init_Datalink_BBMD()
   // if (ps->portType == BPT_BBMD) {
   //     ps->datalink.bipParams.BBMD_Table = (BBMD_TABLE_ENTRY*)emm_calloc(1,sizeof(BBMD_TABLE_ENTRY) * MAX_BBMD_ENTRIES);
   //     ps->datalink.bipParams.FD_Table = (FD_TABLE_ENTRY*)emm_calloc(1,sizeof(FD_TABLE_ENTRY) * MAX_FD_ENTRIES);

   //     if (! emm_check_alloc_two(ps->datalink.bipParams.BBMD_Table, ps->datalink.bipParams.FD_Table)) {
			//// this is pretty fatal
   //         exit(-1) ;
   //     }
   // }
	
#if ( BACNET_CLIENT == 1 )
    // initialize binding table
    ps->Address_Cache = (ADDRESS_CACHE_ENTRY *) emm_calloc(sizeof(ADDRESS_CACHE_ENTRY) * MAX_ADDRESS_CACHE);
	if ( ps->Address_Cache == NULL ) {
		// we have fatal problem
		panic();
		exit(-1);
	}
	
    address_init(ps);
#endif

    return ps;
}


void datalink_destroyCommon(PORT_SUPPORT *datalink)
{
#if ( BACDL_BBMD == 1 )
    // this should move into mdlinkBBMD, but we need to create a destroy function per datalink first, in case some
    // other use of destroy is found...
    if (datalink->portType == BPT_BBMD) {
        emm_free(datalink->datalink.bipParams.BBMD_Table);
        emm_free(datalink->datalink.bipParams.FD_Table);
    }
#endif

#if ( BACNET_CLIENT == 1 )
    if ( datalink->Address_Cache )
	{
		emm_free(datalink->Address_Cache) ;
	}
#endif

    emm_free( (void *) datalink->ifName);
    emm_free(datalink);
}


#if ( BITS_ROUTER_LAYER == 0 )
    // we only use this init for non-routing applications

PORT_SUPPORT *InitDatalink(
    const BPT_TYPE rpt,
    const char *ifName,
    const uint16_t ipPort)
{
    // 2020-08-16 EKH - removed duplicate init
    //PORT_SUPPORT *ps = datalink_initCommon(adapter, rpt );
    //if (ps == NULL) {
    //    return ps;
    //}
    PORT_SUPPORT *ps ;

    switch (rpt)
    {
    default:
        panic();
        // emm_free(ps);
        return NULL;

#if ( BACDL_FD == 1)
    case BPT_FD:
        if (!Init_Datalink_FD(ps->ifName, ipPort))
        {
            return NULL;
        }
        break;
#endif

#if ( BACDL_FD == 1)
    case BPT_BIP:
        ps = Init_Datalink_IP(ifName, ipPort);
        break;
#endif

#if ( BACDL_BBMD == 1 )
    case BPT_BBMD:
        ps = Init_Datalink_BBMD(ifName, ipPort);
        break;
#endif

#if 0   // todo
    case DL_MSTP:
        ps->SendPdu = dlmstp_send_pdu;
        ps->ReceiveMPDU = dlmstp_receive_pdu;
        ps->get_MAC_address = dlmstp_get_MAC_address;
        ps->max_lpdu = MAX_LPDU_MSTP;
        ps->max_apdu = MAX_APDU_MSTP;
        dlmstp_init(ps, "COM15");              // todo ks - same as above
        break;
#endif

    }

    if (ps != NULL)
    {
        LinkListAppend((void **)&datalinkSupportHead, ps);
    }

    return ps;
}
#endif

// not useful
//PORT_SUPPORT *datalink_find(const uint8_t datalinkId)
//{
//    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next) {
//        if (ps->datalinkId == datalinkId) return ps;
//    }
//    panic();
//    return NULL;
//}


void datalink_cleanup(void)
{
    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next) {
        switch (ps->portType) {
        case BPT_BIP:
#if ( BACDL_BBMD == 1 )
        case BPT_BBMD:
#endif
#if ( BACDL_FD == 1 )
        case BPT_FD:
#endif
            bip_cleanup(ps);
            break;

        default:
            panic();
            break;
        }
    }
}


// This function was created to monitor the status of the underlying UDP interface in changing conditions (e.g. DHCP reallocation,
// cable disconnected, etc)
bool bits_Datalink_isActive_IP(PORT_SUPPORT *datalink)
{
#ifdef OS_LAYER_WIN
    // do we need to support this better in windows?
    return true;
#else
    return (datalink->datalink.bipParams.nwoLocal_addr != 0);
#endif
}


bool bits_Datalink_isActive_NoOp(PORT_SUPPORT *datalink)
{
    return true;
}


void IP_Datalink_Watch_IP_Address(PORT_SUPPORT *datalink)
{
    // get the latest parameters from the IP port, and if they are different, adopt them, and show a message
    IP_ADDR_PARAMS portIpParams = { 0 };
    // int rv;
    bool changed = false;

    bits_get_port_params(datalink, &portIpParams); 
    
    // obtained successfully. Or unsuccsessfully (in which case portIpParams will remain 0
    
    if(datalink->datalink.bipParams.nwoLocal_addr != portIpParams.local_address.s_addr)
    {
        char tbuf1[30];
        char tbuf2[30];
        
        // erp. a change...
        dbMessage(DBD_Config,
            DB_NOTE,
            "IP address on port %s changed from %s to %s",
            datalink->ifName,
            NwoIPAddrToString(tbuf1, datalink->datalink.bipParams.nwoLocal_addr),
            NwoIPAddrToString(tbuf2, portIpParams.local_address.s_addr));
        changed = true;
    }
     
    if (datalink->datalink.bipParams.nwoNetmask_addr != portIpParams.netmask.s_addr )
    {
        char tbuf1[30];
        char tbuf2[30];
        
        dbMessage(DBD_Config,
            DB_NOTE,
            "Netmask on port %s changed from %s to %s",
            datalink->ifName,
            NwoIPAddrToString(tbuf1, datalink->datalink.bipParams.nwoNetmask_addr),
            NwoIPAddrToString(tbuf2, portIpParams.netmask.s_addr));
        changed = true;
    }
     
    if (changed)
    {
        bits_set_port_params(datalink, &portIpParams);
    }
}


#if ( BITS_ROUTER_LAYER == 0 )

static void DatalinkListen(void *pArgs)
{
    PORT_SUPPORT *datalink = (PORT_SUPPORT *)pArgs;

    while (true)
    {

        BACNET_ROUTE srcRoute;         /* address where message came from */
        static uint8_t rxBuf[MAX_LPDU_IP];

        bacnet_route_clear(&srcRoute);

        /* returns 0 bytes on timeout */
        int pdu_len = datalink->ReceiveMPDU(datalink, &srcRoute.bacnetPath.localMac, rxBuf, sizeof(rxBuf));

        /* process */
        if (pdu_len)
        {

            srcRoute.portParams = datalink;

            npdu_handler2(
                &srcRoute,
                rxBuf,
                pdu_len);
        }
    }
}


void Datalink_Initialize_Thread(PORT_SUPPORT *datalink)
{
    bitsCreateThread(DatalinkListen, datalink);
}
#endif



