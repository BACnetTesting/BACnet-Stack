/**************************************************************************
 *
 * Copyright (C) 2007-2008 Steve Karg <skarg@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "configProj.h"

#if ( BACNET_SVC_COV_B == 1 )

#include "bacnet/bacdef.h"
#include "bacerror.h"
#include "bacdcode.h"
#include "bacaddr.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "reject.h"
#include "cov.h"
#include "bacnet/basic/tsm/tsm.h"
#include "dcc.h"
#if PRINT_ENABLED
#include "bactext.h"
#endif
/* demo objects */
#include "device.h"
#include "bacnet/basic/services.h"

#include "bacnet/bits/util/BACnetToString.h"
#include "eLib/util/emm.h"
#include "bitsRouter.h"
#include "eLib/util/eLibDebug.h"

/* COV persistence details */
#include "../../bits/ipc/cJSON/cJSON.h"
#include "bactext.h"
#include "ai.h"
#include "ao.h"
#include "av.h"
/* */

/** @file h_cov.c  Handles Change of Value (COV) services. */

#ifndef MAX_COV_SUBCRIPTIONS
#define MAX_COV_SUBCRIPTIONS 1000
#endif


#ifndef MAX_COV_ADDRESSES
#define MAX_COV_ADDRESSES 64
#endif

extern uint16_t Num_Managed_Devices;

// It is OK for us to have a single COV_Addresses table - we are behind a common router after all...
static BACNET_COV_ADDRESS COV_Addresses[MAX_COV_ADDRESSES];

#if ( MAX_COV_ADDRESSES > 127 )
#error This array is indexed by a int8_t, which may be a problem on small devices.
// Change the use of the index if more than 127 addresses are required.
#endif

/**
 * Gets the address from the list of COV addresses
 *
 * @param  index - offset into COV address list where address is stored
 * @param  dest - address to be filled when found
 *
 * @return true if valid address, false if not valid or not found
 */
static BACNET_ROUTE *cov_address_get(
    int index)
{
    if (index < MAX_COV_ADDRESSES) {
        if (COV_Addresses[index].valid) {
            return &COV_Addresses[index].destRoute;
        }
    }
    return NULL;
}


/**
 * Removes the address from the list of COV addresses, if it is not
 * used by other COV subscriptions
 */
static void cov_address_remove_unused(
    DEVICE_OBJECT_DATA *pDev)
{
    uint16_t index ;
    int8_t cov_index;
    bool found = false;

    for (cov_index = 0; cov_index < MAX_COV_ADDRESSES; cov_index++) {
        if (COV_Addresses[cov_index].valid) {
            found = false;
            for (index = 0; index < MAX_COV_SUBCRIPTIONS; index++) {
                if (( pDev->COV_Subscriptions[index].flag.valid) &&
                    ( pDev->COV_Subscriptions[index].dest_index == cov_index)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                COV_Addresses[cov_index].valid = false;
            }
        }
    }
}

/**
 * Adds the address to the list of COV addresses
 *
 * @param  dest - address to be added if there is room in the list
 *
 * @return index number 0..N, or -1 if unable to add
 */
static int cov_address_add(
    BACNET_ROUTE *rxDetails )
{
    int index = -1;
    unsigned i ;
    bool found = false;
    bool valid ;
    BACNET_ROUTE *cov_dest = NULL;

    for (i = 0; i < MAX_COV_ADDRESSES; i++) {
        valid = COV_Addresses[i].valid;
        if (valid) {
            cov_dest = &COV_Addresses[i].destRoute;
            found = bacnet_path_same( &rxDetails->bacnetPath, &cov_dest->bacnetPath);
            if (found) {
                index = i;
                break;
            }
        }
    }

    if (!found) {
        /* find a free place to add a new address */
        for (i = 0; i < MAX_COV_ADDRESSES; i++) {
            valid = COV_Addresses[i].valid;
            if (!valid) {
                index = i;
                cov_dest = &COV_Addresses[i].destRoute;
                cov_dest->portParams = rxDetails->portParams;
                cov_dest->bacnetPath = rxDetails->bacnetPath;
                COV_Addresses[i].valid = true;
                break;
            }
        }
    }

    return index;
}

/*
BACnetCOVSubscription ::= SEQUENCE {
Recipient [0] BACnetRecipientProcess,
    BACnetRecipient ::= CHOICE {
    device [0] BACnetObjectIdentifier,
    address [1] BACnetAddress
        BACnetAddress ::= SEQUENCE {
        network-number Unsigned16, -- A value of 0 indicates the local network
        mac-address OCTET STRING -- A string of length 0 indicates a broadcast
        }
    }
    BACnetRecipientProcess ::= SEQUENCE {
    recipient [0] BACnetRecipient,
    processIdentifier [1] Unsigned32
    }
MonitoredPropertyReference [1] BACnetObjectPropertyReference,
    BACnetObjectPropertyReference ::= SEQUENCE {
    objectIdentifier [0] BACnetObjectIdentifier,
    propertyIdentifier [1] BACnetPropertyIdentifier,
    propertyArrayIndex [2] Unsigned OPTIONAL -- used only with array datatype
    -- if omitted with an array the entire array is referenced
    }
IssueConfirmedNotifications [2] BOOLEAN,
TimeRemaining [3] Unsigned,
COVIncrement [4] REAL OPTIONAL
 */

static int cov_encode_subscription(
    uint8_t * apdu,
    int max_apdu,
    BACNET_COV_SUBSCRIPTION * cov_subscription)
{
    int len = 0;
    int apdu_len = 0;
    BACNET_OCTET_STRING octet_string;
    BACNET_ROUTE *dest = NULL;


    /* FIXME: unused parameter */
    (void) max_apdu;
    if (!cov_subscription) {
        return 0;
    }
    dest = cov_address_get(cov_subscription->dest_index);
    if (!dest) {
        return 0;
    }

    /* Recipient [0] BACnetRecipientProcess - opening */
    len = encode_opening_tag(&apdu[apdu_len], 0);
    apdu_len += len;
    /*  recipient [0] BACnetRecipient - opening */
    len = encode_opening_tag(&apdu[apdu_len], 0);
    apdu_len += len;
    /* CHOICE - address [1] BACnetAddress - opening */
    len = encode_opening_tag(&apdu[apdu_len], 1);
    apdu_len += len;
    /* network-number Unsigned16, */
    /* -- A value of 0 indicates the local network */
    len = encode_application_unsigned(&apdu[apdu_len], dest->bacnetPath.glAdr.net);
    apdu_len += len;
    /* mac-address OCTET STRING */
    /* -- A string of length 0 indicates a broadcast */
    if (dest->bacnetPath.glAdr.net) {
        octetstring_init(&octet_string, &dest->bacnetPath.glAdr.mac.bytes[0], dest->bacnetPath.glAdr.mac.len);
    } else {
        octetstring_init(&octet_string, &dest->bacnetPath.localMac.bytes[0], dest->bacnetPath.localMac.len);
    }
    len = encode_application_octet_string(&apdu[apdu_len], &octet_string);
    apdu_len += len;
    /* CHOICE - address [1] BACnetAddress - closing */
    len = encode_closing_tag(&apdu[apdu_len], 1);
    apdu_len += len;
    /*  recipient [0] BACnetRecipient - closing */
    len = encode_closing_tag(&apdu[apdu_len], 0);
    apdu_len += len;
    /* processIdentifier [1] Unsigned32 */
    len =
        encode_context_unsigned(&apdu[apdu_len], 1,
        cov_subscription->subscriberProcessIdentifier);
    apdu_len += len;
    /* Recipient [0] BACnetRecipientProcess - closing */
    len = encode_closing_tag(&apdu[apdu_len], 0);
    apdu_len += len;
    /*  MonitoredPropertyReference [1] BACnetObjectPropertyReference, */
    len = encode_opening_tag(&apdu[apdu_len], 1);
    apdu_len += len;
    /* objectIdentifier [0] */
    len =
        encode_context_object_id(&apdu[apdu_len], 0,
                                 cov_subscription->monitoredObjectIdentifier.type,
                                 cov_subscription->monitoredObjectIdentifier.instance);
    apdu_len += len;
    /* propertyIdentifier [1] */
    /* FIXME: we are monitoring 2 properties! How to encode? */
    len = encode_context_enumerated(&apdu[apdu_len], 1, PROP_PRESENT_VALUE);
    apdu_len += len;
    /* MonitoredPropertyReference [1] - closing */
    len = encode_closing_tag(&apdu[apdu_len], 1);
    apdu_len += len;
    /* IssueConfirmedNotifications [2] BOOLEAN, */
    len =
        encode_context_boolean(&apdu[apdu_len], 2,
                               cov_subscription->flag.issueConfirmedNotifications);
    apdu_len += len;
    /* TimeRemaining [3] Unsigned, */
    len =
        encode_context_unsigned(&apdu[apdu_len], 3,
                                cov_subscription->lifetime);
    apdu_len += len;

    return apdu_len;
}

/** Handle a request to list all the COV subscriptions.
 * @ingroup DSCOV
 *  Invoked by a request to read the Device object's PROP_ACTIVE_COV_SUBSCRIPTIONS.
 *  Loops through the list of COV Subscriptions, and, for each valid one,
 *  adds its description to the APDU.
 *  @note This function needs some work to better handle buffer overruns.
 *  @param apdu [out] Buffer in which the APDU contents are built.
 *  @param max_apdu [in] Max length of the APDU buffer.
 *  @return How many bytes were encoded in the buffer, or -2 if the response
 *          would not fit within the buffer.
 */
int handler_cov_encode_subscriptions(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * apdu,
    int max_apdu)
{
    int len = 0;
    int apdu_len = 0;
    unsigned index = 0;

    if (pDev->COV_Subscriptions == NULL )
    {
        panic();
        return 0;
    }

    for (index = 0; index < MAX_COV_SUBCRIPTIONS; index++) {
        if (pDev->COV_Subscriptions[index].flag.valid) {
            len =
                cov_encode_subscription(
                    &apdu[apdu_len],
                    max_apdu - apdu_len,
                    &pDev->COV_Subscriptions[index]);
            apdu_len += len;
            /* TODO 1: too late here to notice that we overran the buffer */
            if (apdu_len > max_apdu) {
                return -2;
            }
        }
    }

    return apdu_len;
}


/** Handler to initialize the COV list, clearing and disabling each entry.
 * @ingroup DSCOV
 */
void handler_cov_init(
    DEVICE_OBJECT_DATA* pDev) {
    unsigned index = 0;
    BACNET_COV_SUBSCRIPTION *pCsub;

    // todo 2 Note, this is 20000 bytes. DO SOMETHING (Make it a list? It will be very 'empty' but needs to accommodate a minimum per object)

    pDev->COV_Subscriptions = (BACNET_COV_SUBSCRIPTION*)emm_calloc(sizeof(BACNET_COV_SUBSCRIPTION) * MAX_COV_SUBCRIPTIONS);
    if (pDev->COV_Subscriptions == NULL) {
        panic();
        return;
    }

    pDev->cov_task_state = COV_STATE_IDLE;

    for (index = 0; index < MAX_COV_SUBCRIPTIONS; index++) {
        pCsub = &pDev->COV_Subscriptions[index];
        pCsub->flag.valid = false;
        pCsub->dest_index = -1;
        pCsub->subscriberProcessIdentifier = 0;
        pCsub->monitoredObjectIdentifier.type =
            OBJECT_ANALOG_INPUT;
        pCsub->monitoredObjectIdentifier.instance = 0;
        pCsub->flag.issueConfirmedNotifications = false;
        pCsub->invokeID = 0;
        pCsub->lifetime = 0;
        pCsub->lifetimeSet = 0;
        pCsub->flag.send_requested = false;
    }

    // todo 3 - this gets executed redundantly, consider options later..
    for (index = 0; index < MAX_COV_ADDRESSES; index++) {
        COV_Addresses[index].valid = false;
    }

	tsm_init(pDev);
}


void handler_cov_deinit(
    DEVICE_OBJECT_DATA* pDev)
{
    tsm_deinit(pDev);
    emm_free(pDev->COV_Subscriptions);
}


static bool cov_list_subscribe(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE * srcRoute,
    BACNET_SUBSCRIBE_COV_DATA * cov_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    bool existing_entry = false;
    int index;
    int first_invalid_index = -1;
    bool found = true;
    bool address_match ;
    BACNET_ROUTE *dest = NULL;
    BACNET_COV_SUBSCRIPTION *pCsub;
    time_t timenow = time(NULL);

    /* unable to subscribe - resources? */
    /* unable to cancel subscription - other? */

    /* existing? - match Object ID and Process ID and address */
    for (index = 0; index < MAX_COV_SUBCRIPTIONS; index++) {
        pCsub = &pDev->COV_Subscriptions[index];
        if (pCsub->flag.valid) {
            dest = cov_address_get(pCsub->dest_index);
            if (dest) {
                address_match = bacnet_route_same( srcRoute, dest);
            } else {
                /* skip address matching - we don't have an address */
                address_match = true;
            }
            if ((pCsub->monitoredObjectIdentifier.type ==
                    cov_data->monitoredObjectIdentifier.type) &&
                 (pCsub->monitoredObjectIdentifier.instance ==
                    cov_data->monitoredObjectIdentifier.instance) &&
                 (pCsub->subscriberProcessIdentifier ==
                    cov_data->subscriberProcessIdentifier) && address_match) {
                existing_entry = true;
                pDev->covPersist = true;
                if (cov_data->cancellationRequest) {
                    pCsub->flag.valid = false;
                    pCsub->dest_index = -1;
                    cov_address_remove_unused(pDev);
                } else {
                    pCsub->dest_index = cov_address_add(srcRoute);
                    pCsub->flag.issueConfirmedNotifications =
                        cov_data->issueConfirmedNotifications;
                    pCsub->lifetime = cov_data->lifetime;
                    // persistence:
                    pCsub->lifetimeSet = timenow;
                    pCsub->srcRoute = srcRoute->bacnetPath.glAdr;
                    pCsub->localMac = srcRoute->bacnetPath.localMac;
                    pCsub->portType = srcRoute->portParams->portType;
                    pCsub->flag.send_requested = true;
                }
                if (pCsub->invokeID) {
                    tsm_free_invoke_id(pDev, pCsub->invokeID);
                    pCsub->invokeID = 0;
                }
                break;
            }
        }
        else {
            if (first_invalid_index < 0) {
                first_invalid_index = index;
            }
        }
    }

    if (!existing_entry && (first_invalid_index >= 0) &&
        (!cov_data->cancellationRequest)) {
        index = first_invalid_index;
        found = true;
        pCsub = &pDev->COV_Subscriptions[index];
        pCsub->flag.valid = true;
        pCsub->dest_index = cov_address_add(srcRoute);
        pCsub->monitoredObjectIdentifier.type = cov_data->monitoredObjectIdentifier.type;
        pCsub->monitoredObjectIdentifier.instance = cov_data->monitoredObjectIdentifier.instance;
        pCsub->subscriberProcessIdentifier = cov_data->subscriberProcessIdentifier;
        pCsub->flag.issueConfirmedNotifications = cov_data->issueConfirmedNotifications;
        pCsub->invokeID = 0;
        pCsub->lifetime = cov_data->lifetime;
        // persistence:
        pCsub->lifetimeSet = timenow;
        pCsub->srcRoute = srcRoute->bacnetPath.glAdr;
        pCsub->localMac = srcRoute->bacnetPath.localMac;
        pCsub->portType = srcRoute->portParams->portType;   // probably not required - yet.
        pCsub->flag.send_requested = true;
        pDev->covPersist = true;
    }
    else if (!existing_entry) {
        if (first_invalid_index < 0) {
            /* Out of resources */
            *error_class = ERROR_CLASS_RESOURCES;
            *error_code = ERROR_CODE_NO_SPACE_TO_ADD_LIST_ELEMENT;
            found = false;
        }
        else {
            /* cancellationRequest - valid object not subscribed */
            /* From BACnet Standard 135-2010-13.14.2
               ...Cancellations that are issued for which no matching COV
               context can be found shall succeed as if a context had
               existed, returning 'Result(+)'. */
            found = true;
        }
    }

    return found;
}


static bool cov_send_request(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_COV_SUBSCRIPTION *cov_subscription,
    BACNET_PROPERTY_VALUE * value_list)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_NPCI_DATA npci_data;

    int bytes_sent = 0;
    uint8_t invoke_id = 0;
    bool status = false;        /* return value */
    BACNET_COV_DATA cov_data;
    BACNET_ROUTE *dest ;
    VirtualDeviceInfo *vDev = NULL;

    if (!dcc_communication_enabled(pDev)) {
        return status;
    }

    dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, "COVnotification: Sending 2");

#if BAC_DEBUG
    if (!cov_subscription) {
        panic();
        return status;
    }
#endif

    dest = cov_address_get(cov_subscription->dest_index);
    if (!dest) {
        dbMessage(DBD_COVoperations, DB_UNEXPECTED_ERROR, "COVnotification: Dest not found!");
        return status;
    }

    char tbuf[100];
    uint16_t cursor = 0;
    dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, "COVnotification: Destination Adr is %s", 
        BACnetGlobalAddressToString( tbuf, sizeof(tbuf), &cursor, &dest->bacnetPath.glAdr ) );

    DLCB *dlcb = alloc_dlcb_response('C', &dest->bacnetPath, pDev->datalink->max_lpdu);
    if (dlcb == NULL)
    {
        return false;
    }

    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(
        &npci_data,
        cov_subscription->flag.issueConfirmedNotifications, 
        MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &dest->bacnetPath.glAdr, NULL, // &my_address,
        &npci_data);
    /* load the COV data structure for outgoing message */
    cov_data.subscriberProcessIdentifier =
        cov_subscription->subscriberProcessIdentifier;
    cov_data.initiatingDeviceIdentifier = Device_Object_Instance_Number(pDev);
    cov_data.monitoredObjectIdentifier.type =
        cov_subscription->monitoredObjectIdentifier.type;
    cov_data.monitoredObjectIdentifier.instance =
        cov_subscription->monitoredObjectIdentifier.instance;
    cov_data.timeRemaining = cov_subscription->lifetime;
    cov_data.listOfValues = value_list;
    if (cov_subscription->flag.issueConfirmedNotifications) {
        // done above npci_data.data_expecting_reply = true;
        invoke_id = tsm_next_free_invokeID(routerApplicationEntity);
        if (invoke_id) {
            cov_subscription->invokeID = invoke_id;
            len =
                ccov_notify_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                dlcb->lpduMax - pdu_len, invoke_id, &cov_data);
        }
        else {
            dbMessage(DBD_COVoperations, DB_UNEXPECTED_ERROR, "COVnotification: Unable to obtain an InvokeID");
            dlcb_free(dlcb);
            goto COV_FAILED;
        }
    }
    else {
        len =
            ucov_notify_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            dlcb->lpduMax - pdu_len, &cov_data);
    }

    status = true;
    pdu_len += len;
    dlcb->optr = pdu_len;

    vDev = (VirtualDeviceInfo *) pDev->userData;
    if (vDev != NULL) {
        pDev->datalink->localMAC = &vDev->virtualMACaddr ;
    }

    if (cov_subscription->flag.issueConfirmedNotifications) {
        tsm_set_confirmed_unsegmented_transaction(
			pDev, 
			invoke_id,
            pDev->datalink->localMAC,
            dlcb );
    }

    dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, "COVnotification: Sending PDU");
    pDev->datalink->SendPdu(pDev->datalink, dlcb);

COV_FAILED:
    return status ;
}


// this gets called every second, while the COV subscription has a live timer
static void cov_lifetime_expiration_handler(
    DEVICE_OBJECT_DATA *pDev,
    unsigned index,
    uint32_t elapsed_seconds,
    uint32_t lifetime_seconds)
{
    if (index < MAX_COV_SUBCRIPTIONS) {
        /* handle lifetime expiration */
        if (lifetime_seconds >= elapsed_seconds) {
            pDev->COV_Subscriptions[index].lifetime -= elapsed_seconds;
#if 0
            dbMessage(DB_UNEXPECTED_ERROR, "COVtimer: subscription[%d].lifetime=%lu\n", index,
                (unsigned long) COV_Subscriptions[index].lifetime);
#endif
        } else {
            pDev->COV_Subscriptions[index].lifetime = 0;
        }
        if (pDev->COV_Subscriptions[index].lifetime == 0) {
            /* expire the subscription */
#if PRINT_ENABLED
            dbMessage(DB_UNEXPECTED_ERROR, "COVtimer: PID=%u ",
                COV_Subscriptions[index].subscriberProcessIdentifier);
            dbMessage(DB_UNEXPECTED_ERROR, "%s %u ",
                bactext_object_type_name(COV_Subscriptions[index].
                    monitoredObjectIdentifier.type),
                COV_Subscriptions[index].monitoredObjectIdentifier.instance);
            dbMessage(DB_UNEXPECTED_ERROR, "time remaining=%u seconds ",
                COV_Subscriptions[index].lifetime);
            dbMessage(DB_UNEXPECTED_ERROR, "\n");
#endif
            pDev->COV_Subscriptions[index].flag.valid = false;
            pDev->COV_Subscriptions[index].dest_index = -1;
            cov_address_remove_unused(pDev);
            if (pDev->COV_Subscriptions[index].flag.issueConfirmedNotifications) {
                if (pDev->COV_Subscriptions[index].invokeID) {
                    tsm_free_invoke_id(pDev, pDev->COV_Subscriptions[index].invokeID);
                    pDev->COV_Subscriptions[index].invokeID = 0;
                }
            }
        }
    }
}

/** Handler to check the list of subscribed objects for any that have changed
 *  and so need to have notifications sent.
 * @ingroup DSCOV
 * This handler will be invoked by the main program every second or so.
 * This example only handles Binary Inputs, but can be easily extended to
 * support other types.
 * For each subscribed object,
 *  - See if the subscription has timed out
 *    - Remove it if it has timed out.
 *  - See if the subscribed object instance has changed
 *    (eg, check with Binary_Input_Change_Of_Value() )
 *  - If changed,
 *    - Clear the COV (eg, Binary_Input_Change_Of_Value_Clear() )
 *    - Send the notice with cov_send_request()
 *      - Will be confirmed or unconfirmed, as per the subscription.
 *
 * @note worst case tasking: MS/TP with the ability to send only
 *        one notification per task cycle.
 *
 * @param elapsed_seconds [in] How many seconds have elapsed since last called.
 */
void handler_cov_timer_seconds(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t elapsed_seconds)
{
    unsigned index ;
    uint32_t lifetime_seconds ;

    // client side devices don't do COV
    if (!pDev->COV_Subscriptions) return;

    if (elapsed_seconds) {
        /* handle the subscription timeouts */
        for (index = 0; index < MAX_COV_SUBCRIPTIONS; index++) {
            if (pDev->COV_Subscriptions[index].flag.valid) {
                lifetime_seconds = pDev->COV_Subscriptions[index].lifetime;
                if (lifetime_seconds) {
                    /* only expire COV with definite lifetimes */
                    cov_lifetime_expiration_handler(pDev, index, elapsed_seconds,
                        lifetime_seconds);
                }
            }
        }
    }
}


bool handler_cov_fsm(
    DEVICE_OBJECT_DATA *pDev
)
{
    // client side devices don't do COV
    if (!pDev->COV_Subscriptions) return false ;

    // todo 3, this process goes through the whole list, empty or not. There has to be a better way.

    BACNET_OBJECT_TYPE object_type = MAX_BACNET_OBJECT_TYPE;
    uint32_t object_instance = 0;
    bool status = false;
    bool send = false;
    BACNET_PROPERTY_VALUE value_list[2];

#if 0
    /* states for transmitting */
    moved to device structure
    static enum {
        COV_STATE_IDLE = 0,
        COV_STATE_MARK,
        COV_STATE_CLEAR,
        COV_STATE_FREE,
        COV_STATE_SEND
    } cov_task_state = COV_STATE_IDLE;
#endif

    if (pDev->covPersist) {
        pDev->covPersist = false;
        handler_cov_persist(pDev);
    }

    switch ( pDev->cov_task_state) {
    case COV_STATE_IDLE:
        pDev->covIndex = 0;
        pDev->cov_task_state = COV_STATE_MARK;
        break;

    case COV_STATE_MARK:
        /* mark any subscriptions where the value has changed */
        if (pDev->COV_Subscriptions[pDev->covIndex].flag.valid) {
            object_type = (BACNET_OBJECT_TYPE)
                pDev->COV_Subscriptions[pDev->covIndex].monitoredObjectIdentifier.type;
            object_instance =
                pDev->COV_Subscriptions[pDev->covIndex].
                monitoredObjectIdentifier.instance;
            status = Device_COV(pDev, object_type, object_instance);
            if (status) {
                pDev->COV_Subscriptions[pDev->covIndex].flag.send_requested = true;

                dbMessage(DBD_COVoperations, DB_UNUSUAL_TRAFFIC, 
                    "COVtask: Marking [%s]", 
                    BACnet_ObjectID_ToString(object_type, object_instance));
            }
        }
        pDev->covIndex++;
        if (pDev->covIndex >= MAX_COV_SUBCRIPTIONS) {
            pDev->covIndex = 0;
            pDev->cov_task_state = COV_STATE_CLEAR;
        }
        break;
    case COV_STATE_CLEAR:
        /* clear the COV flag after checking all subscriptions */
        if ((pDev->COV_Subscriptions[pDev->covIndex].flag.valid) &&
            (pDev->COV_Subscriptions[pDev->covIndex].flag.send_requested)) {
            object_type = (BACNET_OBJECT_TYPE)
                pDev->COV_Subscriptions[pDev->covIndex].monitoredObjectIdentifier.type;
            object_instance =
                pDev->COV_Subscriptions[pDev->covIndex].
                monitoredObjectIdentifier.instance;
            Device_COV_Clear(pDev, object_type, object_instance);
        }
        pDev->covIndex++;
        if (pDev->covIndex >= MAX_COV_SUBCRIPTIONS) {
            pDev->covIndex = 0;
            pDev->cov_task_state = COV_STATE_FREE;
        }
        break;
    case COV_STATE_FREE:
        /* confirmed notification house keeping */
        if ((pDev->COV_Subscriptions[pDev->covIndex].flag.valid) &&
            (pDev->COV_Subscriptions[pDev->covIndex].flag.issueConfirmedNotifications) &&
            (pDev->COV_Subscriptions[pDev->covIndex].invokeID)) {
            if (is_tsm_invoke_id_free(pDev, pDev->COV_Subscriptions[pDev->covIndex].invokeID)) {
                pDev->COV_Subscriptions[pDev->covIndex].invokeID = 0;
            } else
                if (tsm_invoke_id_failed(pDev, pDev->COV_Subscriptions
                    [pDev->covIndex].invokeID)) {
                tsm_free_invoke_id(pDev, pDev->COV_Subscriptions[pDev->covIndex].invokeID);
                pDev->COV_Subscriptions[pDev->covIndex].invokeID = 0;
            }
        }
        pDev->covIndex++;
        if (pDev->covIndex >= MAX_COV_SUBCRIPTIONS) {
            pDev->covIndex = 0;
            pDev->cov_task_state = COV_STATE_SEND;
        }
        break;
    case COV_STATE_SEND:
        /* send any COVs that are requested */
        if ((pDev->COV_Subscriptions[pDev->covIndex].flag.valid) &&
            (pDev->COV_Subscriptions[pDev->covIndex].flag.send_requested)) {
            send = true;
            if (pDev->COV_Subscriptions[pDev->covIndex].flag.issueConfirmedNotifications) {
                if (pDev->COV_Subscriptions[pDev->covIndex].invokeID != 0) {
                    /* already sending */
                    send = false;
                }
                if (!tsm_transaction_available(pDev)) {
                    /* no transactions available - can't send now */
                    send = false;
                }
            }
            if (send) {
                object_type = (BACNET_OBJECT_TYPE)
                    pDev->COV_Subscriptions[pDev->covIndex].
                              monitoredObjectIdentifier.type;
                object_instance =
                    pDev->COV_Subscriptions[pDev->covIndex].
                    monitoredObjectIdentifier.instance;

                dbMessage(DBD_COVoperations, DB_NOTE,
                    "COVtask: Sending [%s] from [%s]",
                    BACnet_ObjectID_ToString(object_type, object_instance),
                    BACnetMacAddrToString( pDev->datalink->localMAC ) );

                /* configure the linked list for the two properties */
                value_list[0].next = &value_list[1];
                value_list[1].next = NULL;
                (void) Device_Encode_Value_List(pDev, object_type,
                                                object_instance, &value_list[0]);
                status =
                    cov_send_request(
						pDev, &pDev->COV_Subscriptions[pDev->covIndex],
                        &value_list[0]);
                if (status) {
                    pDev->COV_Subscriptions[pDev->covIndex].flag.send_requested = false;
                }
            }
        }
        pDev->covIndex++;
        if (pDev->covIndex >= MAX_COV_SUBCRIPTIONS) {
            pDev->covIndex = 0;
            pDev->cov_task_state = COV_STATE_IDLE;
        }
        break;

    default:
        panic();
        pDev->covIndex = 0;
        pDev->cov_task_state = COV_STATE_IDLE;
        break;
    }
    return (pDev->cov_task_state == COV_STATE_IDLE );
}

void handler_cov_task(
    DEVICE_OBJECT_DATA *pDev)
{
    handler_cov_fsm( 
		pDev );
}

#if ( BACNET_SVC_COV_B == 1)
bool cov_subscribe(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_ROUTE *srcRoute,
    BACNET_SUBSCRIBE_COV_DATA * cov_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    bool status ;        /* return value */
    BACNET_OBJECT_TYPE object_type ;
    uint32_t object_instance ;

    object_type =
        (BACNET_OBJECT_TYPE) cov_data->monitoredObjectIdentifier.type;
    object_instance = cov_data->monitoredObjectIdentifier.instance;
    status = Device_Valid_Object_Id(pDev, object_type, object_instance);
    if (status) {
        status = Device_Value_List_Supported(object_type);
        if (status) {
            status =
                cov_list_subscribe(pDev, srcRoute, cov_data, error_class, error_code);
        } else {
            *error_class = ERROR_CLASS_OBJECT;
            *error_code = ERROR_CODE_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED;
        }
    } else {
        *error_class = ERROR_CLASS_OBJECT;
        *error_code = ERROR_CODE_UNKNOWN_OBJECT;
    }

    return status;
}
#endif

/** Handler for a COV Subscribe Service request.
 * @ingroup DSCOV
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - an ACK, if cov_subscribe() succeeds
 * - an Error if cov_subscribe() fails
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_GLOBAL_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_cov_subscribe(
    DEVICE_OBJECT_DATA *pDev,
    uint8_t * service_request,
    uint16_t service_len,
	BACNET_ROUTE* rxDetails,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_SUBSCRIBE_COV_DATA cov_data;
    int len ;
    int pdu_len ;
    int npdu_len ; 
    int apdu_len ;
    BACNET_NPCI_DATA npci_data;
    bool success = false;
    // int bytes_sent = 0;
    // BACNET_PATH my_address;
    bool error = false;

    memset(&cov_data, 0, sizeof(cov_data));

  	DLCB *dlcb = alloc_dlcb_response('D', &rxDetails->bacnetPath, pDev->datalink->max_lpdu);
  	if (dlcb == NULL) return;

    /* initialize a common abort code */
    cov_data.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
    /* encode the NPDU portion of the packet */
    //datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len =
        npdu_encode_pdu(
            &dlcb->Handler_Transmit_Buffer[0],
            &rxDetails->bacnetPath.glAdr,
            NULL, // &my_address,
            &npci_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len = BACNET_STATUS_ABORT;
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Segmented message.  Sending Abort!");
        error = true;
        goto COV_ABORT;
    }

    len =
        cov_subscribe_decode_service_request(service_request, service_len,
                &cov_data);
    if (len <= 0)
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Unable to decode Request!");
    if (len < 0) {
        error = true;
        goto COV_ABORT;
    }

    cov_data.error_class = ERROR_CLASS_OBJECT;
    cov_data.error_code = ERROR_CODE_UNKNOWN_OBJECT;

    success =
        cov_subscribe(pDev, rxDetails, &cov_data, &cov_data.error_class,
            &cov_data.error_code);

    if (success) {
        apdu_len =
            encode_simple_ack(&dlcb->Handler_Transmit_Buffer[npdu_len],
                              service_data->invoke_id, SERVICE_CONFIRMED_SUBSCRIBE_COV);
        dbMessage(DBD_COVoperations, DB_NOTE, "SubscribeCOV: Sending Simple Ack!");
    } else {
        len = BACNET_STATUS_ERROR;
        error = true;
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Sending Error!");
    }
COV_ABORT:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            apdu_len =
                abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                  service_data->invoke_id,
                                  abort_convert_error_code(cov_data.error_code), true);
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Sending Abort!");
        } else if (len == BACNET_STATUS_ERROR) {
            apdu_len =
                bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                     service_data->invoke_id, SERVICE_CONFIRMED_SUBSCRIBE_COV,
                                     cov_data.error_class, cov_data.error_code);
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Sending Error!");
        } else if (len == BACNET_STATUS_REJECT) {
            apdu_len =
                reject_encode_apdu(&dlcb->Handler_Transmit_Buffer[npdu_len],
                                   service_data->invoke_id,
                                   reject_convert_error_code(cov_data.error_code));
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "SubscribeCOV: Sending Reject!");
        }
    }

    pdu_len = npdu_len + apdu_len;
    dlcb->optr = pdu_len;
    pDev->datalink->SendPdu(pDev->datalink, dlcb);
}

#endif // ( BACNET_SVC_COV_B == 1 )
