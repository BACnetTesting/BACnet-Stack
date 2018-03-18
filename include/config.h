/**************************************************************************
*
* Copyright (C) 2004 Steve Karg <skarg@users.sourceforge.net>
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

    Modifications Copyright (C) 2017 BACnet Interoperability Testing Services, Inc.

    July 1, 2017    BITS    Modifications to this file have been made in compliance
                            to original licensing.

    This file contains changes made by BACnet Interoperability Testing
    Services, Inc. These changes are subject to the permissions,
    warranty terms and limitations above.
    For more information: info@bac-test.com
    For access to source code:  info@bac-test.com
            or      www.github.com/bacnettesting/bacnet-stack

####COPYRIGHTEND####
*
*********************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#ifdef _MSC_VER
	// The Visual Studio automatically defines _DEBUG symbol for Debug builds(and NDEBUG for non - debug builds).
#ifndef BAC_DEBUG
#if _DEBUG
#define BAC_DEBUG	1
#else
#define BAC_DEBUG	0
#endif
#endif
#endif

/* Although this stack can implement a later revision,
* sometimes another revision is desired */
#ifndef BACNET_PROTOCOL_REVISION
#define BACNET_PROTOCOL_REVISION 14
#endif

/* Note: these defines can be defined in your makefile or project or Microsoft VS2015 Property Sheets
   or here or not defined. If not defined, the following defaults will be used: */

/* declare a single physical layer using your compiler define.
   see datalink.h for possible defines. */
#if !(defined(BACDL_ETHERNET) || defined(BACDL_ARCNET) || \
    defined(BACDL_MSTP) || defined(BACDL_BIP) || defined(BACDL_BIP6) || \
    defined(BACDL_TEST) || defined(BACDL_ALL))
#define BACDL_BIP
#endif

// NB: You need to set this to 1 if you e.g. are running on Windows and you have multiple interfaces/adapters/VPNs and 
// you want to avoid specifying the _specific_ interface you want BACnet to bind to.
// Defining this to 1 will bind to "any" and todo1
#define USE_INADDR  1

/* optional configuration for BACnet/IP datalink layers */
#if (defined(BACDL_BIP) || defined(BACDL_ALL))
/* other BIP defines (define as 1 to enable):
    USE_INADDR - uses INADDR_BROADCAST for broadcast and binds using INADDR_ANY
    USE_CLASSADDR = uses IN_CLASSx_HOST where x=A,B,C or D for broadcast
*/
#if !defined(BBMD_ENABLED)
#define BBMD_ENABLED 1
#endif
#endif

/* optional configuration for BACnet/IPv6 datalink layer */
#if defined(BACDL_BIP6)
#if !defined(BBMD6_ENABLED)
#define BBMD6_ENABLED 0
#endif
#endif

/* Enable the Gateway (Routing) functionality here, if desired. */
#if !defined(MAX_NUM_DEVICES)
#if ( BAC_ROUTING == 1 )
#define MAX_NUM_DEVICES 3       /* Eg, Gateway + two remote devices */
#else
#define MAX_NUM_DEVICES 1       /* Just the one normal BACnet Device Object */
#endif
#endif


/* Define your processor architecture as
 Big Endian       (PowerPC,68K,Sparc) or
 Little Endian    (Intel,AVR)
 ARM and MIPS can be either - what is the most common? */

// renamed from BIG_ENDIAN to BACNET_STACK_BIG_ENDIAN due to ambiguous collisions with some compilers
#if !defined(BACNET_STACK_BIG_ENDIAN)
#define BACNET_STACK_BIG_ENDIAN 0
#endif

/* Define your Vendor Identifier assigned by ASHRAE */
#define BACNET_VENDOR_ID 343
#define BACNET_VENDOR_NAME "BACnet Interoperability Testing Services, Inc."

/* Max number of bytes in an APDU. */
/* Typical sizes are 50, 128, 206, 480, 1024, and 1476 octets */
/* This is used in constructing messages and to tell others our limits */
/* 50 is the minimum; adjust to your memory and physical layer constraints */
/* Lon=206, MS/TP=480, ARCNET=480, Ethernet=1476, BACnet/IP=1476 */
#if !defined(MAX_APDU)
/* #define MAX_APDU 50 */
/* #define MAX_APDU 1476 */
#if defined(BACDL_BIP)
#define MAX_APDU 1024
/* #define MAX_APDU 128 enable this IP for testing
 readrange so you get the More Follows flag set */
#elif defined(BACDL_BIP6)
#define MAX_APDU 1476
#elif defined (BACDL_ETHERNET)
#if defined(BACNET_SECURITY)
#define MAX_APDU 1420
#else
#define MAX_APDU 1476
#endif
#else
#if defined(BACNET_SECURITY)
#define MAX_APDU 412
#else
#define MAX_APDU 480
#endif
#endif
#endif

/* for confirmed messages, this is the number of transactions */
/* that we hold in a queue waiting for timeout. */
/* Configure to zero if you don't want any confirmed messages */
/* Configure from 1..255 for number of outstanding confirmed */
/* requests available. */
#if !defined(MAX_TSM_TRANSACTIONS)
#define MAX_TSM_TRANSACTIONS 255
#endif
/* The address cache is used for binding to BACnet devices */
/* The number of entries corresponds to the number of */
/* devices that might respond to an I-Am on the network. */
/* If your device is a simple server and does not need to bind, */
/* then you don't need to use this. */
#if !defined(MAX_ADDRESS_CACHE)
#define MAX_ADDRESS_CACHE 255
#endif

/* some modules have debugging enabled using PRINT_ENABLED */
// todo 4 - remove all references to this once new dbXxxx() fully implemented.
#if !defined(PRINT_ENABLED)
#define PRINT_ENABLED 0
#endif

/* BACAPP decodes WriteProperty service requests
   Choose the datatypes that your application supports */
#if !(defined(BACAPP_ALL) || \
    defined(BACAPP_MINIMAL) || \
    defined(BACAPP_NULL) || \
    defined(BACAPP_BOOLEAN) || \
    defined(BACAPP_UNSIGNED) || \
    defined(BACAPP_SIGNED) || \
    defined(BACAPP_REAL) || \
    defined(BACAPP_DOUBLE) || \
    defined(BACAPP_OCTET_STRING) || \
    defined(BACAPP_CHARACTER_STRING) || \
    defined(BACAPP_BIT_STRING) || \
    defined(BACAPP_ENUMERATED) || \
    defined(BACAPP_DATE) || \
    defined(BACAPP_TIME) || \
    defined(BACAPP_LIGHTING_COMMAND) || \
    defined(BACAPP_DEVICE_OBJECT_PROP_REF) || \
    defined(BACAPP_OBJECT_ID))
#define BACAPP_ALL
#endif

#if defined (BACAPP_ALL)
#define BACAPP_NULL
#define BACAPP_BOOLEAN
#define BACAPP_UNSIGNED
#define BACAPP_SIGNED
#define BACAPP_REAL
#define BACAPP_DOUBLE
#define BACAPP_OCTET_STRING
#define BACAPP_CHARACTER_STRING
#define BACAPP_BIT_STRING
#define BACAPP_ENUMERATED
#define BACAPP_DATE
#define BACAPP_TIME
#define BACAPP_OBJECT_ID
#define BACAPP_DEVICE_OBJECT_PROP_REF
#define BACAPP_LIGHTING_COMMAND
#elif defined (BACAPP_MINIMAL)
#define BACAPP_NULL
#define BACAPP_BOOLEAN
#define BACAPP_UNSIGNED
#define BACAPP_SIGNED
#define BACAPP_REAL
#define BACAPP_CHARACTER_STRING
#define BACAPP_ENUMERATED
#define BACAPP_DATE
#define BACAPP_TIME
#define BACAPP_OBJECT_ID
#endif

/*
 ** Set the maximum vector type sizes
 */
#ifndef MAX_BITSTRING_BYTES
#define MAX_BITSTRING_BYTES (15)
#endif

#ifndef MAX_CHARACTER_STRING_BYTES
#define MAX_CHARACTER_STRING_BYTES 64
#endif

#ifndef MAX_OCTET_STRING_BYTES
#define MAX_OCTET_STRING_BYTES 64
#endif

/*
 ** Control the selection of services etc to enable code size reduction for those
 ** compiler suites which do not handle removing of unused functions in modules
 ** so well.
 **
 ** We will start with the A type services code first as these are least likely
 ** to be required in embedded systems using the stack.
 */

/*
 ** First we see if this is a test build and enable all the services as they
 ** may be required.
 **
 ** Note: I've left everything enabled here in the main config.h. You should
 ** use a local copy of config.h with settings configured for your needs to
 ** make use of the code space reductions.
 **/

#ifdef TEST
#define BACNET_SVC_I_HAVE_A    1
#define BACNET_SVC_WP_A        1
#define BACNET_SVC_RP_A        1
#define BACNET_SVC_RPM_A       1
#define BACNET_SVC_DCC_A       1
#define BACNET_SVC_RD_A        1
#define BACNET_SVC_TS_A        1
#define BACNET_SVC_SERVER      0
#define BACNET_USE_OCTETSTRING 1
#define BACNET_USE_DOUBLE      1
#define BACNET_USE_SIGNED      1
#else /* Otherwise define our working set - all enabled here to avoid breaking things */
#define BACNET_SVC_I_HAVE_A    1
#define BACNET_SVC_WP_A        1
#define BACNET_SVC_RP_A        1
#define BACNET_SVC_RPM_A       1
#define BACNET_SVC_DCC_A       1
#define BACNET_SVC_RD_A        1
#define BACNET_SVC_TS_A        1
#define BACNET_SVC_SERVER      0
#define BACNET_USE_OCTETSTRING 1
#define BACNET_USE_DOUBLE      1
#define BACNET_USE_SIGNED      1
#endif

#ifndef BACNET_CLIENT
#define BACNET_CLIENT           0               // todo 3 move client functions out to their own library
#endif

// In linux, these can be (actually, are) defined in the makefile.
#ifndef INTRINSIC_REPORTING
#define INTRINSIC_REPORTING     0
#endif

/* Do them one by one */
//#ifndef BACNET_SVC_COV_B		/* EKH, I am only defining B for now, make A vs B more fine grained when next dealing with COVs */
//#define BACNET_SVC_COV_B		1
//#endif

#ifndef BACNET_SVC_I_HAVE_A         /* Do we send I_Have requests? */
#define BACNET_SVC_I_HAVE_A         0
#endif

#ifndef BACNET_SVC_WP_A         /* Do we send WriteProperty requests? */
#define BACNET_SVC_WP_A         0
#endif

#ifndef BACNET_SVC_RP_A         /* Do we send ReadProperty requests? */
#define BACNET_SVC_RP_A         0
#endif

#ifndef BACNET_SVC_RR_B                 /* Do we respond to Read Range requests? */
#define BACNET_SVC_RR_B         1
#endif

#ifndef BACNET_SVC_RPM_A                /* Do we send ReadPropertyMultiple requests? */
#define BACNET_SVC_RPM_A        0
#endif

#ifndef BACNET_SVC_DCC_A        /* Do we send DeviceCommunicationControl requests? */
#define BACNET_SVC_DCC_A 0
#endif

#ifndef BACNET_SVC_RD_A         /* Do we send ReinitialiseDevice requests? */
#define BACNET_SVC_RD_A 0
#endif



/* B-side or Server services */

// Per PICS email trail, support Intrinsic Alarming on AI, COV on BI.

#ifndef INTRINSIC_REPORTING_AI_B
#define INTRINSIC_REPORTING_AI_B 0
#endif

#ifndef INTRINSIC_REPORTING_AV_B
#define INTRINSIC_REPORTING_AV_B 0
#endif

#ifndef BACNET_SVC_COV_BI_B        /* EKH, I am only defining B for now, make A vs B more fine grained when next dealing with COVs */
#define BACNET_SVC_COV_BI_B 0
#endif

#ifndef BACNET_SVC_COV_AI_B        /* EKH, I am only defining B for now, make A vs B more fine grained when next dealing with COVs */
#define BACNET_SVC_COV_AI_B 0
#endif

#ifndef BACNET_SVC_COV_AV_B        /* EKH, I am only defining B for now, make A vs B more fine grained when next dealing with COVs */
#define BACNET_SVC_COV_AV_B 0
#endif

#ifndef BACNET_SVC_RR_B         /* Do we respond to Read Range requests? */
#define BACNET_SVC_RR_B     0
#endif

#ifndef BACNET_SVC_PRIVATE_TRANSFER
#define BACNET_SVC_PRIVATE_TRANSFER 0
#endif

#ifndef BACNET_SVC_RD_A                 /* Do we send ReinitialiseDevice requests? */
#define BACNET_SVC_RD_A         0
#endif

#ifndef BACNET_SVC_SERVER               /* Are we a pure server type device? */
#define BACNET_SVC_SERVER       1
#endif


/* Other */

#ifndef BACNET_USE_OCTETSTRING  /* Do we need any octet strings? */
#define BACNET_USE_OCTETSTRING 0
#endif

#ifndef BACNET_USE_DOUBLE       /* Do we need any doubles? */
#define BACNET_USE_DOUBLE 0
#endif

#ifndef BACNET_USE_SIGNED       /* Do we need any signed integers */
#define BACNET_USE_SIGNED 0
#endif


/* And a similar method for optional BACnet Objects */

#ifndef BACNET_USE_OBJECT_ANALOG_INPUT
#define BACNET_USE_OBJECT_ANALOG_INPUT      1
#endif


#ifndef BACNET_USE_OBJECT_ANALOG_VALUE
#define BACNET_USE_OBJECT_ANALOG_VALUE      1
#endif

#ifndef BACNET_USE_OBJECT_ANALOG_OUTPUT
#define BACNET_USE_OBJECT_ANALOG_OUTPUT     1
#endif

#ifndef BACNET_USE_OBJECT_BINARY_INPUT
#define BACNET_USE_OBJECT_BINARY_INPUT      1
#endif

#ifndef BACNET_USE_OBJECT_BINARY_VALUE
#define BACNET_USE_OBJECT_BINARY_VALUE      1
#endif

#ifndef BACNET_USE_OBJECT_BINARY_OUTPUT
#define BACNET_USE_OBJECT_BINARY_OUTPUT     1
#endif

#ifndef BACNET_USE_OBJECT_ALERT_ENROLLMENT
#define BACNET_USE_OBJECT_ALERT_ENROLLMENT  1
#endif

#ifndef BACNET_USE_OBJECT_LOAD_CONTROL
#define BACNET_USE_OBJECT_LOAD_CONTROL      0
#endif

#ifndef BACNET_USE_OBJECT_TRENDLOG
#define BACNET_USE_OBJECT_TRENDLOG          0
#endif

#ifndef BACNET_USE_OBJECT_LIFE_SAFETY
#define BACNET_USE_OBJECT_LIFE_SAFETY       0
#endif

#ifndef BACNET_USE_OBJECT_CHANNEL
#define BACNET_USE_OBJECT_CHANNEL			0
#endif

#ifndef BACNET_USE_OBJECT_SCHEDULE
#define BACNET_USE_OBJECT_SCHEDULE			0
#endif


// There are some cascading dependencies, check and resolve them here

#ifndef INTRINSIC_REPORTING
#if INTRINSIC_REPORTING_AI_B == 1 || INTRINSIC_REPORTING_AV_B == 1
#define INTRINSIC_REPORTING 0
#else
#define INTRINSIC_REPORTING 0
#endif
#endif

#define BAC_ROUTING                         0   // Switch to 1 only at "Virtual Devices" reference project stage/branch

#define LIST_MANIPULATION                   0

#if (BACNET_USE_OBJECT_ALERT_ENROLLMENT) || (INTRINSIC_REPORTING == 1)
#define BACNET_USE_OBJECT_NOTIFICATION_CLASS    1
#define BACNET_USE_EVENT_HANDLING               1
#endif

#if (BACNET_SVC_COV_BI_B == 1) || (BACNET_SVC_COV_AI_B == 1 )
#define BACNET_SVC_COV_B 0
#endif
   
#endif // CONFIG_H

