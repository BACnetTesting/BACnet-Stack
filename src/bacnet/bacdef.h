/**************************************************************************
 *
 * Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef BACDEF_H
#define BACDEF_H

#include "configProj.h"

#include <stddef.h>
#include <stdint.h>
#include "bacnet/bacenum.h"
#include "bacnet/bacint.h"

#if defined(_MSC_VER)
/* Silence the warnings about unsafe versions of library functions */
/* as we need to keep the code portable */
#pragma warning( disable : 4996)        // 'use strncpy_s' type warnings
#endif

/* This stack implements this version of BACnet */
#define BACNET_PROTOCOL_VERSION 1

/* Although this stack can implement a later revision,
 * sometimes another revision is desired */
// moved to configProj.h to avoid circular .h file inclusion with bacdef.h bacenum.h
//#ifndef BACNET_PROTOCOL_REVISION
//#define BACNET_PROTOCOL_REVISION 14
//#endif

/* there are a few dependencies on the BACnet Protocol-Revision */
#if (BACNET_PROTOCOL_REVISION == 0)
#define MAX_ASHRAE_OBJECT_TYPE 18
#define MAX_BACNET_SERVICES_SUPPORTED 35
#elif (BACNET_PROTOCOL_REVISION == 1)
#define MAX_ASHRAE_OBJECT_TYPE 21
#define MAX_BACNET_SERVICES_SUPPORTED 37
#elif (BACNET_PROTOCOL_REVISION == 2)
    /* from 135-2001 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 23
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 3)
#define MAX_ASHRAE_OBJECT_TYPE 23
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 4)
    /* from 135-2004 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 25
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 5)
#define MAX_ASHRAE_OBJECT_TYPE 30
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 6)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 7)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 8)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 9)
    /* from 135-2008 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 38
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 10)
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 11)
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 12)
    /* from 135-2010 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 13)
#define MAX_ASHRAE_OBJECT_TYPE 53
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 14) || (BACNET_PROTOCOL_REVISION == 15)
    /* from 135-2012 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 55
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 16)
    /* Addendum 135-2012an, 135-2012at, 135-2012au,
       135-2012av, 135-2012aw, 135-2012ax, 135-2012az */
#define MAX_ASHRAE_OBJECT_TYPE 56
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 17)
    /* Addendum 135-2012ai */
#define MAX_ASHRAE_OBJECT_TYPE 57
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 18) || (BACNET_PROTOCOL_REVISION == 19)
    /* from 135-2016 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 60
#define MAX_BACNET_SERVICES_SUPPORTED 44
#elif (BACNET_PROTOCOL_REVISION == 20)
    /* Addendum 135-2016bd  */
#define MAX_ASHRAE_OBJECT_TYPE 61
#define MAX_BACNET_SERVICES_SUPPORTED 44
#else
#error MAX_ASHRAE_OBJECT_TYPE and MAX_BACNET_SERVICES_SUPPORTED not defined!
#endif

/* largest BACnet Instance Number */
/* Also used as a device instance number wildcard address */
#define BACNET_MAX_INSTANCE (0x3FFFFF)
#define BACNET_INSTANCE_BITS 22
/* large BACnet Object Type */
#define BACNET_MAX_OBJECT (0x3FF)
/* Array index 0=size of array, n=array element n,  MAX=all array elements */
#define BACNET_ARRAY_ALL UINT32_MAX
typedef uint32_t BACNET_ARRAY_INDEX;
/* For device object property references with no device id defined */
#define BACNET_NO_DEV_ID   0xFFFFFFFFu
// made part of BACNET_OBJECT_TYPE enum #define BACNET_NO_DEV_TYPE 0xFFFFu
/* Priority Array for commandable objects */
#define BACNET_NO_PRIORITY 0
#define BACNET_MIN_PRIORITY 1
#define BACNET_MIN_ON_OFF_PRIORITY 6
#define BACNET_MAX_PRIORITY 16

#define BACNET_BROADCAST_NETWORK (0xFFFFu)

/* Any size MAC address should be allowed which is less than or
   equal to 7 bytes.  The IPv6 addresses are planned to be handled
   outside this area. */
/* FIXME: mac[] only needs to be as big as our local datalink MAC */

#define MAX_MAC_LEN 7

typedef enum 
{   MAC_TYPE_ETHERNET,
    MAC_TYPE_BIP,
    MAC_TYPE_MSTP
} MAC_TYPE;

typedef struct {
    MAC_TYPE    macType;
    uint8_t     len;        
    uint8_t     bytes[MAX_MAC_LEN];

#if ( BAC_DEBUG == 1 )
    uint8_t signature;
#endif
} BACNET_MAC_ADDRESS ;

typedef struct {
    uint16_t            net;                /* BACnet network number        */
    BACNET_MAC_ADDRESS  mac;                /* Devices MAC address on NN    */
} BACNET_GLOBAL_ADDRESS ;

typedef struct {
    BACNET_MAC_ADDRESS          localMac;       // router, or device itself if no router and no network number
    BACNET_GLOBAL_ADDRESS       glAdr;          // device and network number, if behind router
} BACNET_PATH ;


/*
    Route, Path, Address, MAC addressing summary (cr348957345293855)

    MAC                                 BACnet MAC address, 1 byte for e.g. BACnet MS/TP, 6 for BACnet/IP
    NN                                  BACnet Network Number
        BACNET_GLOBAL_ADDRESS           This is the "BACnet Address" per the specification, the combo of MAC and NN
            BACNET_PATH                 This adds the datalink MAC address of either the device, or the router on the "path" to the device
                BACNET_ROUTE            Adds the datalink that the device is connected to. Mostly useful for Multidatalink, not useful for Full routing, that information is built into the router layer
 */

/*
    New adressing structure
    
    Old                     Becomes....
    BACNET_ADDRESS          BACNET_PATH
                            BACNET_GLOBAL_ADDRESS           BACNET_MAC_ADDRESS
    
    mac_len                                                 localMac.len
    mac                                                     localMac.bytes
    
    net                                                     gladr.net
    
    len                                                     gladr.mac.len
    adr                                                     gladr.mac.bytes
    
 */


/* note: with microprocessors having lots more code space than memory,
   it might be better to have a packed encoding with a library to
   easily access the data. */
typedef struct BACnet_Object_Id {
    BACNET_OBJECT_TYPE type;
    uint32_t instance;
} BACNET_OBJECT_ID;

// See: http://www.bacnetwiki.com/wiki/index.php?title=NPCI
// Note that although Lon Src can be 7, this will never be in combination of a dest of 6, always 2, so 7+2 < 6+6 for MAC length

#define MAX_NPCI (1+1+2+1+7+2+1+6+1)        // 22 (but realistically, will never exceed 21 because lon=2+7 < ip=6+6
/*
    ver                 1
    contr               1   
    snet                2
    smaclen             1
    smac                7
    dnet                2
    dmaclen             1
    dmac                6
    hopcount            1
*/

#define BACNET_ID_VALUE(bacnet_object_instance, bacnet_object_type) ((((bacnet_object_type) & BACNET_MAX_OBJECT) << BACNET_INSTANCE_BITS) | ((bacnet_object_instance) & BACNET_MAX_INSTANCE))
#define BACNET_INSTANCE(bacnet_object_id_num) ((bacnet_object_id_num)&BACNET_MAX_INSTANCE)
#define BACNET_TYPE(bacnet_object_id_num) (((bacnet_object_id_num) >> BACNET_INSTANCE_BITS ) & BACNET_MAX_OBJECT)

#define BACNET_STATUS_OK (0)
#define BACNET_STATUS_ERROR (-1)
#define BACNET_STATUS_ABORT (-2)
#define BACNET_STATUS_REJECT (-3)
#define BACNET_STATUS_UNKNOWN_PROPERTY (-4)

#endif
