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
*
*****************************************************************************************
*
*   Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.
*
*   July 1, 2017    BITS    Modifications to this file have been made in compliance
*                           with original licensing.
*
*   This file contains changes made by BACnet Interoperability Testing
*   Services, Inc. These changes are subject to the permissions,
*   warranty terms and limitations above.
*   For more information: info@bac-test.com
*   For access to source code:  info@bac-test.com
*          or      www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#ifdef _MSC_VER
#include <process.h>
#endif
  
#include "datalink.h"
#include "logging.h"
#include "handlers.h"
#include "dlenv.h"
#include "bitsDebug.h"
#include "ese.h"
#include "emm.h"

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
 * @param npci_data [in] The NPDU header (Network) information.
 * @param pdu [in] Buffer of data to be sent - may be null.
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
int (
    *datalink_send_pdu) (
        BACNET_PATH * dest,
        BACNET_NPCI_DATA * npci_data,
        DLCB *dlcb);

uint16_t(*datalink_receive) (BACNET_PATH * src, uint8_t * pdu,
    uint16_t max_pdu, unsigned timeout);
#endif

/** Function template to close the DataLink services and perform any cleanup.
 * @ingroup DLTemplates
 */
//void(
//    *datalink_cleanup) (
//        void)
//{
//}

#if 0
void (
    *datalink_get_broadcast_address) (
        BACNET_GLOBAL_ADDRESS * dest);

void (
    *datalink_get_my_address) (
        BACNET_GLOBAL_ADDRESS * my_address);

#endif

