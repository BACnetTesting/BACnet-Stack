/**************************************************************************
*
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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

#ifndef NC_H
#define NC_H

#include "configProj.h"

#if ( BACNET_USE_OBJECT_NOTIFICATION_CLASS == 1 )

#include "event.h"
#include "listmanip.h"
#include "device.h"

#define NC_RESCAN_RECIPIENTS_SECS   10	// making this temporarily faster for testing - was 60

/* max "length" of recipient_list */
#define NC_MAX_RECIPIENTS 10

/* Recipient types */
    typedef enum {
        RECIPIENT_TYPE_NOTINITIALIZED = 0,          // not initialized must be 0
        RECIPIENT_TYPE_DEVICE_INSTANCE = 1,
        RECIPIENT_TYPE_ADDRESS = 2
    } NC_RECIPIENT_TYPE;


// #if (INTRINSIC_REPORTING_B == 1)
/* BACnetRecipient structure */
/*

BACnetAddress ::= SEQUENCE {
    network-number 	Unsigned16, -- A value of 0 indicates the local network
    mac-address 	OCTET STRING -- A string of length 0 indicates a broadcast
    }

BACnetRecipient ::= CHOICE {
    device [0] BACnetObjectIdentifier,
    address [1] BACnetAddress
    }

*/

typedef struct BACnet_Recipient {
    NC_RECIPIENT_TYPE RecipientType;  /* Type of Recipient */
    union {
        uint32_t DeviceIdentifier;
        BACNET_GLOBAL_ADDRESS   Address;                    // Switching from BACnet Path to BACnet Global Address
                                                            // We do not know the router to this destination, the workstation cannot be expected to write it, Karg did not deal with this issue.
                                                            // We have to discover the path to the destination by issuing a who-is-router-to-network based on the NN in this global address at a later time.
    } _;
} BACNET_RECIPIENT;


/* BACnetDestination structure */
typedef struct BACnet_Destination {		// Be careful adding fields to this structure, it is mem-compared for equality in add/remove list
    uint8_t     ValidDays;				// bitfield, not typedef
    BACNET_TIME FromTime;
    BACNET_TIME ToTime;
    BACNET_RECIPIENT Recipient;
    uint32_t ProcessIdentifier;
    uint8_t Transitions;
    bool ConfirmedNotify;
} BACNET_DESTINATION;


typedef struct {

    BACNET_OBJECT   common;				// *MUST* be first field in structure due to llist, and also because we mimic class inheritance using embedded sub-structures...

    uint8_t Priority[MAX_BACNET_EVENT_TRANSITION];          /* BACnetARRAY[3] of Unsigned */
    uint8_t Ack_Required;                                   /* BACnetEventTransitionBits */
    BACNET_DESTINATION Recipient_List[NC_MAX_RECIPIENTS];   /* List of BACnetDestination */

} NOTIFICATION_CLASS_DESCR;

// migrated to BACnetObject.h
///* Indicates whether the transaction has been confirmed */
//typedef struct Acked_info {
//    bool bIsAcked;  /* true when transitions is acked */
//    BACNET_DATE_TIME Time_Stamp;    /* time stamp of when a alarm was generated */
//} ACKED_INFO;


// migrated to BACnetObject.h
///* Information needed to send AckNotification */
//typedef struct Ack_Notification {
//    bool                bSendAckNotify;    /* true if need to send AckNotification */
//    BACNET_EVENT_STATE  EventState;
//} ACK_NOTIFICATION;



void Notification_Class_Property_Lists(
    const BACNET_PROPERTY_ID **pRequired,
    const BACNET_PROPERTY_ID **pOptional,
    const BACNET_PROPERTY_ID **pProprietary);

void Notification_Class_Init(
    void);

bool Notification_Class_Valid_Instance(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance);

unsigned Notification_Class_Count(
    DEVICE_OBJECT_DATA *pDev );

uint32_t Notification_Class_Index_To_Instance(
    DEVICE_OBJECT_DATA *pDev,
    unsigned index);

int Notification_Class_Instance_To_Index(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance);

bool Notification_Class_Object_Name(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

char *Notification_Class_Description(
    uint32_t instance);

int Notification_Class_Read_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Notification_Class_Write_Property(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_WRITE_PROPERTY_DATA * wp_data);


BACNET_OBJECT* Notification_Class_Create(
    DEVICE_OBJECT_DATA* pDev,
    const uint32_t objectInstance,
    const char* objectName);

void Notification_Class_Init(
    void);

void Notification_Class_Get_Priorities(
    DEVICE_OBJECT_DATA *pDev,
    uint32_t Object_Instance,
    uint32_t * pPriorityArray);

void Notification_Class_common_reporting_function(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_EVENT_NOTIFICATION_DATA * event_data);

void Notification_Class_find_recipient_Task(
    DEVICE_OBJECT_DATA *pDev);
        
#if ( BACNET_SVC_LIST_MANIPULATION_B == 1 )
bool Notification_Class_Add_List_Element(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_LIST_MANIPULATION_DATA * lmdata);

bool Notification_Class_Remove_List_Element(
    DEVICE_OBJECT_DATA *pDev,
    BACNET_LIST_MANIPULATION_DATA * lmdata);
#endif

bool Notification_Class_GetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,
    RR_PROP_INFO * pInfo);
#endif /* defined(BACNET_USE_OBJECT_NOTIFICATION_CLASS) */
#endif /* NC_H */
