/****************************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*   This program is free software : you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.

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

#include "bitsDebug.h"
#include "btaDebug.h"
#include "emm.h"
#include "ese.h"
#include "llist.h"
#include "usbd_cdc_if.h"

typedef struct
{
    LLIST_LB    ll_lb;          // must be first
    uint8_t     *payload;
    uint16_t    length;
} btaLB;


static LLIST_HDR btaOutputQueueCB ;

bool BTA_DatalinkInit( void )
{
    // using newlib nano, 100 crashes (hardfaults)
    // 20 ok, and now I start seeing traffic, but I still block queue
    // 40 drops messages when under pressure, going for 50
    ll_Init(&btaOutputQueueCB, 50);
    return true;
}


// this function does nothing but send the payload and frees allocated
// memory - no formatting at all

void SendBTApayload(uint8_t *payload, const int sendlength)
{
    if (!BTA_Ready()) {
        emm_free(payload);
        return;
    }

    uint8_t *stringPayload = emm_smalloc('1', MX_BTA_BUFFER);
    btaLB *btalb = (btaLB *) emm_smalloc('a', sizeof(btaLB)) ;
    if ( ! emm_check_alloc_two( stringPayload, btalb) ) {
        emm_free(payload);
        return;
    }

    memcpy(stringPayload, "<F-", 3);
    uint16_t    oPtr = 3;
    // CDC_Transmit_FS( "<F-", 3 );

    uint8_t hexa[3];
    for (int i = 0; i < sendlength; i++) {
        hexa[0] = ToHexadecimal (payload[i] >> 4 );
        hexa[1] = ToHexadecimal(payload[i] & 0x0F);
        hexa[2] = ' ';
        // CDC_Transmit_FS( hexa, 3);
        memcpy(&stringPayload[oPtr], hexa, 3);
        oPtr += 3;
        if (oPtr > MX_BTA_BUFFER - 3 ) {
            emm_free_three(payload, stringPayload, btalb );
            ese_enqueue_once(ese032_BTA_output_length_exceeded);
            return;
        }
    }

    memcpy(&stringPayload[oPtr], "\r\n", 2);
    oPtr += 2;

    btalb->payload = stringPayload;
    btalb->length = oPtr;

    if (!ll_Enqueue(&btaOutputQueueCB, btalb)) {
        emm_free_three(payload, stringPayload, btalb);
        ese_enqueue_once(ese033_BTA_output_queue_full);
        return;
    }

    // CDC_Transmit_FS("\r\n", 2);
    emm_free(payload);
}



void  BTA_DatalinkIdle(void)
{
    if (!CDC_Transmit_Ready()) return;

    if (!ll_GetCount(&btaOutputQueueCB)) return;

    // we can send 
    btaLB *btalb = ll_Dequeue(&btaOutputQueueCB);

    CDC_Transmit_FS(btalb->payload, btalb->length);

    emm_free_two(btalb->payload, btalb);
}
