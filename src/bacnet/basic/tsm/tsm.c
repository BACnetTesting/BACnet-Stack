/****************************************************************************************

    Copyright (C) 2005 Steve Karg
    Corrections by Ferran Arumi, 2007, Barcelona, Spain

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

#include "configProj.h"

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )

#include <stddef.h>
// use emm.h and emm_malloc, etc. #include <malloc.h>
#include "bits.h"

#include "apdu.h"
#include "bacnet/bacdef.h"
#include "bacdcode.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/binding/address.h"
#include "bactext.h"
#include "osLayer.h"
#include "eLib/util/emm.h"
#include "bacaddr.h"

/** @file tsm.c  BACnet Transaction State Machine operations  */

#if (MAX_TSM_TRANSACTIONS)
/* Really only needed for segmented messages */
/* and a little for sending confirmed messages */
/* If we are only a server and only initiate broadcasts, */
/* then we don't need a TSM layer. */

/* FIXME: not coded for segmentation */

/* declare space for the TSM transactions, and set it up in the init. */
/* table rules: an Invoke ID = 0 is an unused spot in the table */
// moved to DEVICE_OBJECT_DATA static BACNET_TSM_DATA TSM_List[MAX_TSM_TRANSACTIONS];

/* invoke ID for incrementing between subsequent calls. */
// moved to DEVICE_OBJECT_DATA static uint8_t Current_Invoke_ID = 1;

static tsm_timeout_function Timeout_Function;

static bits_mutex_define(tsmSema);

#if defined ( _MSC_VER  ) || defined ( __GNUC__ )
void print_tsm_cache(DEVICE_OBJECT_DATA *pDev)
{
    char tbuf[100];
    printf("\nTSM cache:");
    printf("\n   IID   Tmr Retry  State  Dest");
    for (int i = 0; i < MAX_TSM_TRANSACTIONS; i++)
    {
        printf("\n   %3d  %4u   %d       %02x  %s",
            pDev->TSM_List[i].InvokeID,
            pDev->TSM_List[i].RequestTimer,
            pDev->TSM_List[i].RetryCount,
            pDev->TSM_List[i].state,
            bactext_bacnet_path(tbuf, &pDev->TSM_List[i].dlcb2->bacnetPath)
        );
    }
}
#endif


void tsm_init(
    DEVICE_OBJECT_DATA *pDev)
{
    bits_mutex_init(tsmSema);

	// we do this check because there are various options that need the TSM, and we cannot predict if 0, 1 or more
	// of these services will be enabled, so this function may be called multiple times, but we only want to malloc once
	if (pDev->TSM_List != NULL)
	{
        return;
	}

    pDev->Current_Invoke_ID = 1;

    pDev->TSM_List = (BACNET_TSM_DATA *) emm_calloc(sizeof(BACNET_TSM_DATA) * MAX_TSM_TRANSACTIONS);
    if (pDev->TSM_List == NULL)
    {
        panic();
        // and we cannot continue in this condition
        exit(-1);
    }
}


void tsm_deinit(
    DEVICE_OBJECT_DATA *pDev)
{
    emm_free( pDev->TSM_List);
}


void tsm_set_timeout_handler(
    tsm_timeout_function pFunction)
{
    Timeout_Function = pFunction;
}

/* returns MAX_TSM_TRANSACTIONS if not found */
static uint8_t tsm_find_invokeID_index(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID)
{
    unsigned i ;     /* counter */
    uint8_t index = MAX_TSM_TRANSACTIONS;       /* return value */

    if (invokeID == 0)
    {
        // do not panic, on startup the invoke ID in use is 0, and we don't want to use this, so 
        // just say that it is not in use.
        // 2019.12.21 EKH: On the other hand, panic, we should never use IID 0 -> reserved for special use
        // The startup case mentioned above was in error, and I had to initialize Current_Invoke_ID to 1 to avoid.
        panic();
        return MAX_TSM_TRANSACTIONS ;
    }

    bits_mutex_lock(tsmSema);

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (pDev->TSM_List[i].InvokeID == invokeID) {
            index = (uint8_t)i;
            break;
        }
    }

    bits_mutex_unlock(tsmSema);

    return index;
}

static uint8_t tsm_find_first_free_index(
    DEVICE_OBJECT_DATA *pDev)
{
    unsigned i;                                 /* counter */
    uint8_t index = MAX_TSM_TRANSACTIONS;       /* return value */

    bits_mutex_lock(tsmSema);

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (pDev->TSM_List[i].InvokeID == 0) {
            index = (uint8_t)i;
            break;
        }
    }

    bits_mutex_unlock(tsmSema);

    return index;
}


bool tsm_transaction_available(
    DEVICE_OBJECT_DATA *pDev)
{
    bool status = false;        /* return value */
    unsigned i ;

    bits_mutex_lock(tsmSema);

    for (i=0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (pDev->TSM_List[i].InvokeID == 0) {
            /* one is available! */
            status = true;
            break;
        }
    }

    bits_mutex_unlock(tsmSema);

    return status;
}

uint8_t tsm_transaction_idle_count(
    DEVICE_OBJECT_DATA *pDev)
{
    uint8_t count = 0;  /* return value */
    unsigned i;

    bits_mutex_lock(tsmSema);

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if ((pDev->TSM_List[i].InvokeID == 0) &&
            (pDev->TSM_List[i].state == TSM_STATE_IDLE)) {
            /* one is available! */
            count++;
        }
    }

    bits_mutex_unlock(tsmSema);

    return count;
}

/* gets the next free invokeID,
   and reserves a spot in the table
   returns 0 if none are available */
static uint8_t tsm_find_free_invokeID(
    DEVICE_OBJECT_DATA *pDev,
    bool autoclear)
{
    uint8_t index;
    uint8_t invokeID = 0;
    bool found = false;

    bits_mutex_lock(tsmSema);

    /* is there even space available? */
    if (tsm_transaction_available(pDev)) {
        while (!found) {
            index = tsm_find_invokeID_index(pDev, pDev->Current_Invoke_ID);
            if (index == MAX_TSM_TRANSACTIONS) {
                /* Not found, so this invokeID is not used */
                found = true;
                /* set this id into the table */
                index = tsm_find_first_free_index(pDev);
                if (index != MAX_TSM_TRANSACTIONS) {

                    dbMessage(DBD_ClientSide, DB_NORMAL_TRAFFIC, "Found IID: %p, %03d", pDev, pDev->Current_Invoke_ID);

                    pDev->TSM_List[index].InvokeID = invokeID = pDev->Current_Invoke_ID;
                    pDev->TSM_List[index].state = TSM_STATE_IDLE;
                    pDev->TSM_List[index].RequestTimer = apdu_timeout();
                    pDev->TSM_List[index].autoClear = autoclear;
                    /* update for the next call or check */
                    pDev->Current_Invoke_ID++;
                    /* skip zero - we treat that internally as invalid or no free */
                    if (pDev->Current_Invoke_ID == 0) {
                        pDev->Current_Invoke_ID = 1;
                    }
                }
            }
            else {
                /* found! This invokeID is already used */
                /* try next one */
                pDev->Current_Invoke_ID++;
                /* skip zero - we treat that internally as invalid or no free */
                if (pDev->Current_Invoke_ID == 0) {
                    pDev->Current_Invoke_ID = 1;
                }
            }
        }
    }
    bits_mutex_unlock(tsmSema);
    
    return invokeID;
}


/* gets the next free invokeID,
   and reserves a spot in the table
   returns 0 if none are available */
uint8_t tsm_next_free_invokeID(
    DEVICE_OBJECT_DATA *pDev)
{
    return tsm_find_free_invokeID(pDev, false);
}


// Karg has no way of clearing a failed notification... todo 3
uint8_t tsm_next_free_invokeID_autoclear(
    DEVICE_OBJECT_DATA *pDev)
{
    return tsm_find_free_invokeID(pDev, true);
}

#if ( BAC_DEBUG_todo == 1 ) 
extern ROUTER_PORT *headRouterPort;
#endif

void tsm_set_confirmed_unsegmented_transaction(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID,
    BACNET_MAC_ADDRESS *ourMAC,
    DLCB *dlcb
    )
{
    uint8_t index;

    bits_mutex_lock(tsmSema);

    if (invokeID) {
        index = tsm_find_invokeID_index(pDev, invokeID);
        if (index < MAX_TSM_TRANSACTIONS) {
            /* SendConfirmedUnsegmented */
            pDev->TSM_List[index].dlcb2 = dlcb_clone_deep(dlcb); // todo 3 - probably don't have to clone the _whole_ buffer, check optr?
            // if the dlcb copy was successful, complete the entries
            if (pDev->TSM_List[index].dlcb2 != NULL)
            {
                // we only ever do this for Virtual devices...
                pDev->TSM_List[index].ourMAC = ourMAC ;
                pDev->TSM_List[index].state = TSM_STATE_AWAIT_CONFIRMATION;
                pDev->TSM_List[index].RetryCount = 0;
                /* start the timer */
                pDev->TSM_List[index].RequestTimer = apdu_timeout();
            }
            else
            {
                dbMessage(DBD_ALL, DB_ERROR, "TSM: Out of TSM table space for this device");
            }
        }
    }

    bits_mutex_unlock(tsmSema);
}


/* used to retrieve the transaction payload */
/* if we wanted to find out what we sent (i.e. when we get an ack) */
bool tsm_get_transaction_pdu(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID,
    DLCB **dlcb)
{
    bits_mutex_lock(tsmSema);

    if (invokeID) {
        uint8_t index = tsm_find_invokeID_index(pDev, invokeID);
        /* how much checking is needed?  state?  dest match? just invokeID? */
        if (index < MAX_TSM_TRANSACTIONS) {
            /* FIXME: we may want to free the transaction so it doesn't timeout */
            /* retrieve the transaction */
            *dlcb = pDev->TSM_List[index].dlcb2;
            return true;
        }
    }

    bits_mutex_unlock(tsmSema);
    return false;
}


/* called once a millisecond or slower */
void tsm_timer_milliseconds(
    DEVICE_OBJECT_DATA *pDev,
    uint16_t milliseconds)
{
    unsigned i ;     /* counter */

    bits_mutex_lock(tsmSema);

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (pDev->TSM_List[i].state == TSM_STATE_AWAIT_CONFIRMATION) {

            dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC,
                "TSM: Awaiting Confirmation InvokeID: %d",
                    pDev->TSM_List[i].InvokeID );

            if (pDev->TSM_List[i].RequestTimer > milliseconds) {
                pDev->TSM_List[i].RequestTimer -= milliseconds;
            }
            else {
                pDev->TSM_List[i].RequestTimer = 0;
            }

            /* AWAIT_CONFIRMATION */
            if (pDev->TSM_List[i].RequestTimer == 0) {
                if (pDev->TSM_List[i].RetryCount <= apdu_retries()) {
                    // retry....

                    pDev->TSM_List[i].RequestTimer = apdu_timeout();
                    pDev->TSM_List[i].RetryCount++;

                    dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC,
                        "   TSM: Retry InvokeID: [%p] IID:%d, Retry#%d",
                        pDev,
                        pDev->TSM_List[i].InvokeID,
                        pDev->TSM_List[i].RetryCount);

                    // we are resending a Virtual Device? If so, we need to adjust the virtualized host MAC address accordingly
                    if (pDev->datalink->portType == BPT_VIRT)
                    {
                        if (pDev->TSM_List[i].ourMAC == NULL)
                        {
                            panic();
                            pDev->TSM_List[i].state = TSM_STATE_IDLE;
                            dlcb_free(pDev->TSM_List[i].dlcb2);
                            pDev->TSM_List[i].dlcb2 = NULL;
                            return;
                        }
                        pDev->datalink->localMAC = pDev->TSM_List[i].ourMAC;
                    }

                    // make a copy of the dclb (and its attachments)
                    DLCB *dlcb = dlcb_clone_deep(pDev->TSM_List[i].dlcb2);
                    if (dlcb != NULL)
                    {
                        pDev->datalink->SendPdu(pDev->datalink, dlcb);
                    }
                    // else, we may be able to alloc later, so leave this as a non-responded send for now (don't kill off further retries)
                }
                else {
                    /* note: the invoke id has not been cleared yet
                       and this indicates a failed message:
                       IDLE and a valid invoke id */
                    pDev->TSM_List[i].state = TSM_STATE_IDLE;

                    dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC,
                        "   TSM: Abandon InvokeID: %p %d",
                        pDev,
                        pDev->TSM_List[i].InvokeID);

                    // and free the copy we kept around for resending
                    dlcb_free(pDev->TSM_List[i].dlcb2);
                    pDev->TSM_List[i].dlcb2 = NULL;

                    if (pDev->TSM_List[i].autoClear)
                    {
                        // clear it forever
                        pDev->TSM_List[i].InvokeID = 0;
                        dbMessage(DBD_ClientSide, DB_ALWAYS, "Autofree? [%p]", pDev);
                    }

                    // EKH: I have made this mod. The Karg method requires the client to monitor the TSM progress.
                    // A better way would have been to callback when finally done... we can do that.
                    // see tsm_free_invoke_id() TSM_List[i].InvokeID = 0;

                    // 2017.02.20 Karg's latest now has a callback, but he does not use it, and anyway, he does not distinquish between a 'monitored'
                    // transaction e.g. ReadProperty, WriteProperty, and an Event Notification, which is why I added 'autoClear'
                    if (pDev->TSM_List[i].InvokeID != 0) {
                        if (Timeout_Function) {
                            Timeout_Function(pDev, pDev->TSM_List[i].InvokeID);
                        }
                    }
                }
            }
        }
    }
    bits_mutex_unlock(tsmSema);
}


/* frees the invokeID and sets its state to IDLE */
// returns true if one was indeed found (If there is a very late response / duplicate response), 
// there may no longer be a TSM entry.
bool tsm_free_invoke_id(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID)
{
    uint8_t index;

    bits_mutex_lock(tsmSema);

    index = tsm_find_invokeID_index(pDev, invokeID);
    if (index < MAX_TSM_TRANSACTIONS) {

        pDev->TSM_List[index].state = TSM_STATE_IDLE;
        pDev->TSM_List[index].InvokeID = 0;

        // sometimes Invoke Ids are assigned and rejected before DLCB assigned. (eg, when out of mem when trying to create DLCB)
        if (pDev->TSM_List[index].dlcb2 != NULL)
        {
            dlcb_free(pDev->TSM_List[index].dlcb2);
        }
        pDev->TSM_List[index].dlcb2 = NULL;
    }
    bits_mutex_unlock(tsmSema);

    return (index < MAX_TSM_TRANSACTIONS);
}


/** Check if the invoke ID has been made free by the Transaction State Machine.
 * @param invokeID [in] The invokeID to be checked, normally of last message sent.
 * @return True if it is free (done with), False if still pending in the TSM.
 */
bool is_tsm_invoke_id_free(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID)
{
    bool status = true;
    uint8_t index;

    bits_mutex_lock(tsmSema);

    index = tsm_find_invokeID_index(pDev, invokeID);
    if (index < MAX_TSM_TRANSACTIONS) {
        // no! this is just a check!! dlcb_free(TSM_List[index].dlcb2);
        status = false;
    }

    bits_mutex_unlock(tsmSema);

    return status;
}

/** See if we failed get a confirmation for the message associated
 *  with this invoke ID.
 * @param invokeID [in] The invokeID to be checked, normally of last message sent.
 * @return True if already failed, False if done or segmented or still waiting
 *         for a confirmation.
 */
bool tsm_invoke_id_failed(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t invokeID)
{
    bool status = false;
    uint8_t index;

    bits_mutex_lock(tsmSema);

    index = tsm_find_invokeID_index(pDev, invokeID);
    if (index < MAX_TSM_TRANSACTIONS) {
        /* a valid invoke ID and the state is IDLE is a
           message that failed to confirm */
        if (pDev->TSM_List[index].state == TSM_STATE_IDLE)
            status = true;
    }
    else {
        // EKH: If the invokeId could not be found, that too would be a failure, right?
        status = true;
    }

    bits_mutex_unlock(tsmSema);

    return status;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* flag to send an I-Am */
bool I_Am_Request = true;

/* dummy function stubs */
int datalink_send_pdu(
    BACNET_PATH * dest,
    BACNET_NPCI_DATA * npci_data,
    uint8_t * pdu,
    unsigned pdu_len)
{
    (void)dest;
    (void)npci_data;
    (void)pdu;
    (void)pdu_len;

    return 0;
}

/* dummy function stubs */
void datalink_get_broadcast_address(
    BACNET_PATH * dest)
{
    (void)dest;
}

void testTSM(
    Test * pTest)
{
    /* FIXME: add some unit testing... */
}

#ifdef TEST_TSM
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet TSM", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testTSM);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_TSM */
#endif /* TEST */
#endif /* MAX_TSM_TRANSACTIONS */

#endif // ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )