/**************************************************************************
*
* Copyright (C) 2016 ConnectEx, Inc.
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
*********************************************************************/
#ifndef ALERT_ENROLLMENT_H
#define ALERT_ENROLLMENT_H

#include "event.h"
#include "nc.h"

#define NC_RESCAN_RECIPIENTS_SECS   60

/* max "length" of recipient_list */
#define NC_MAX_RECIPIENTS 10

/* Recipient types */
//typedef enum {
//    RECIPIENT_TYPE_NOTINITIALIZED = 0,
//    RECIPIENT_TYPE_DEVICE = 1,
//    RECIPIENT_TYPE_ADDRESS = 2
//} NC_RECIPIENT_TYPE;


//#if defined(INTRINSIC_REPORTING)
/* BACnetRecipient structure */
/*
BACnetRecipient ::= CHOICE {
    device [0] BACnetObjectIdentifier,
    address [1] BACnetAddress
}
*/
//typedef struct BACnet_Recipient {
//	NC_RECIPIENT_TYPE RecipientType;  /* Type of Recipient */
//    union {
//        uint32_t DeviceIdentifier;
//        BACNET_GLOBAL_ADDRESS Address;
//    } _;
//} BACNET_RECIPIENT;


///* BACnetDestination structure */
//typedef struct BACnet_Destination {
//    uint8_t ValidDays;
//    BACNET_TIME FromTime;
//    BACNET_TIME ToTime;
//    BACNET_RECIPIENT Recipient;
//    uint32_t ProcessIdentifier;
//    uint8_t Transitions;
//    bool ConfirmedNotify;
//} BACNET_DESTINATION;


/* Structure containing configuration for a Alert_Enrollment Class */
typedef struct Alert_Enrollment_Class_info {
    uint8_t Priority[MAX_BACNET_EVENT_TRANSITION];  /* BACnetARRAY[3] of Unsigned */
    uint8_t Ack_Required;   /* BACnetEventTransitionBits */
    BACNET_DESTINATION Recipient_List[NC_MAX_RECIPIENTS];   /* List of BACnetDestination */
} ALERT_ENROLLMENT_CLASS_INFO;


///* Indicates whether the transaction has been confirmed */
//typedef struct Acked_info {
//    bool bIsAcked;  /* true when transitions is acked */
//    BACNET_DATE_TIME Time_Stamp;    /* time stamp of when a alarm was generated */
//} ACKED_INFO;


/* Information needed to send AckAlert_Enrollment */
typedef struct Ack_Alert_Enrollment {
    bool bSendAckNotify;    /* true if need to send AckAlert_Enrollment */
    uint8_t EventState;
} ACK_ALERT_ENROLLMENT;



void Alert_Enrollment_Class_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary);

void Alert_Enrollment_Class_Init(
    void);

bool Alert_Enrollment_Class_Valid_Instance(
    uint32_t object_instance);
unsigned Alert_Enrollment_Class_Count(
    void);
uint32_t Alert_Enrollment_Class_Index_To_Instance(
    unsigned index);
unsigned Alert_Enrollment_Class_Instance_To_Index(
    uint32_t object_instance);
bool Alert_Enrollment_Class_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name);

int Alert_Enrollment_Class_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata);

bool Alert_Enrollment_Class_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

void Alert_Enrollment_Class_Get_Priorities(
    uint32_t Object_Instance,
    uint32_t * pPriorityArray);

// void Alert_Enrollment_Class_common_reporting_function(
//     BACNET_EVENT_ALERT_ENROLLMENT_DATA * event_data);

void Alert_Enrollment_Class_find_recipient(
    void);
//#endif /* defined(INTRINSIC_REPORTING) */


#endif
