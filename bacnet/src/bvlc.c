/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2006 Steve Karg

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

////#include <stdbool.h>    /* for the standard bool type. */
////#include <time.h>
//#include <stdbool.h>    /* for the standard bool type. */
#include <stdbool.h>    /* for the standard bool type. */
#include <time.h>
//#include "bacenum.h"
#include "bip.h"
////#include "bacdcode.h"
//#include "bacint.h"
//#include "net.h"
//#include "bvlc.h"
//#include "bip.h"

#if defined(BBMD_ENABLED) && BBMD_ENABLED
#include "bbmd.h"
#endif

//#ifndef DEBUG_ENABLED
//#define DEBUG_ENABLED 0
//#endif

#include "BACnetToString.h"

#include "debug.h"
#include "btaDebug.h"
#include "datalink.h"
#include "device.h"
#include "bitsDebug.h"

/** @file bvlc.c  Handle the BACnet Virtual Link Control (BVLC),
 * which includes: BACnet Broadcast Management Device,
 * Broadcast Distribution Table, and
 * Foreign Device Registration.
 */

/** if we are a foreign device, store the
   remote BBMD address/port here in network byte order */
// static struct sockaddr_in Remote_BBMD;

/** Global IP address for NAT handling */
// in_addr does not contain port information
// sockaddr_in does..
/* IP Address - stored in network byte order */
// todo1 - remove this, it is not required. BC with bip.c
// static struct sockaddr_in BVLC_Global_Address;

/** Flag to indicate if NAT handling is enabled/disabled */
// static bool BVLC_NAT_Handling = false;

/** result from a client request */
BACNET_BVLC_RESULT BVLC_Result_Code = BVLC_RESULT_SUCCESSFUL_COMPLETION;

/** The current BVLC Function Code being handled. */
BACNET_BVLC_FUNCTION BVLC_Function_Code = BVLC_RESULT;  /* A safe default */

/* Define BBMD_ENABLED to get the functions that a
 * BBMD needs to handle its services.
 * Separately, define BBMD_CLIENT_ENABLED to get the
 * functions that allow a client to manage a BBMD.
 */
#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)

// moved to PORT_SUPPORT
//#ifndef MAX_BBMD_ENTRIES
//#define MAX_BBMD_ENTRIES 16
//#endif

//static BBMD_TABLE_ENTRY BBMD_Table[MAX_BBMD_ENTRIES];

///*Each device that registers as a foreign device shall be placed
//in an entry in the BBMD's Foreign Device Table (FDT). Each
//entry shall consist of the 6-octet B/IP address of the registrant;
//the 2-octet Time-to-Live value supplied at the time of
//registration; and a 2-octet value representing the number of
//seconds remaining before the BBMD will purge the registrant's FDT
//entry if no re-registration occurs. This value will be initialized
//to the 2-octet Time-to-Live value supplied at the time of
//registration.*/
//typedef struct {
//    bool valid;
//    /* BACnet/IP address */
//    struct in_addr dest_address;
//    /* BACnet/IP port number - not always 47808=BAC0h */
//    uint16_t dest_port;
//    /* seconds for valid entry lifetime */
//    uint16_t time_to_live;
//    /* our counter */
//    time_t seconds_remaining;   /* includes 30 second grace period */
//} FD_TABLE_ENTRY;

//#ifndef MAX_FD_ENTRIES
//#define MAX_FD_ENTRIES 128
//#endif
//static FD_TABLE_ENTRY FD_Table[MAX_FD_ENTRIES];

static int SendMPDU(const PORT_SUPPORT *portParams, const BACNET_BVLC_FUNCTION function, const uint32_t nHostAddr, const uint16_t nPort, const DLCB *dlcb );


/* Define BBMD_BACKUP_FILE if the contents of the BDT
 * (broadcast distribution table) are to be stored in 
 * a backup file, so the contents are not lost across
 * power failures, shutdowns, etc...
 * (this is required behaviour as defined in BACnet standard).
 * 
 * BBMD_BACKUP_FILE should be set to the file name
 * in which to store the BDT.
 */
#define BBMD_BACKUP_FILE BACnet_BDT_table
#if defined(BBMD_BACKUP_FILE)

#define tostr(a) str(a)
#define str(a) #a

void bvlc_bdt_backup_local(
    void) 
{
    static FILE *bdt_file_ptr = NULL;
    
    /* only try opening the file if not already opened previously */
    if (!bdt_file_ptr)
        bdt_file_ptr = fopen(tostr(BBMD_BACKUP_FILE),"wb");
    
    /* if error opening file for writing -> silently abort */
    if (!bdt_file_ptr)
        return;
        
    fseek(bdt_file_ptr, 0, SEEK_SET);
    panic();
//     fwrite(BBMD_Table, sizeof(BBMD_TABLE_ENTRY), MAX_BBMD_ENTRIES, bdt_file_ptr); 
    fflush(bdt_file_ptr);
}

void bvlc_bdt_restore_local(
    void) 
{
    static FILE *bdt_file_ptr = NULL;
    
    /* only try opening the file if not already opened previously */
    if (!bdt_file_ptr)
        bdt_file_ptr = fopen(tostr(BBMD_BACKUP_FILE),"rb");
    
    /* if error opening file for reading -> silently abort */
    if (!bdt_file_ptr)
        return;
        
    fseek(bdt_file_ptr, 0, SEEK_SET);
    {
        BBMD_TABLE_ENTRY BBMD_Table_tmp[MAX_BBMD_ENTRIES];
        size_t entries = 0;
        
        entries = fread(BBMD_Table_tmp, sizeof(BBMD_TABLE_ENTRY), MAX_BBMD_ENTRIES, bdt_file_ptr); 
        if (entries == MAX_BBMD_ENTRIES)
            /* success reading the BDT table. */
            panic();
//            memcpy(BBMD_Table, BBMD_Table_tmp, sizeof(BBMD_TABLE_ENTRY) * MAX_BBMD_ENTRIES);
    }
}
#else
void bvlc_bdt_backup_local (void) {}
void bvlc_bdt_restore_local(void) {}
#endif

char *PortSupportToString(PORT_SUPPORT *ps)
{
    static char stringbuf[1000];
    char tbuf[200];
    const char *type = "Error";

    switch (ps->portType)
    {
    case BPT_BBMD:
        type = "BBMD";
        break;
    case BPT_BIP:
        type = "BIP";
        break;
    case BPT_VIRT:
        type = "Virtual";
        break;
    case BPT_APP:
        type = "App";
        break;
    case BPT_MSTP:
        type = "MSTP";
        break;
    case BPT_FD:
        type = "FD";
        break;
    default:
        panic();
        break;
    }

    sprintf(stringbuf, "[%d] %s: %s:%d", ps->datalinkId, type, NwoIPAddrToString(tbuf, ps->datalink.bipParams.nwoLocal_addr), ntohs(ps->datalink.bipParams.nwoPort));
    return stringbuf;
}


void ShowFDtable(
    PORT_SUPPORT *portParams)
{
    unsigned int i;
    printf("\n   Foreign Device Registrations for port ID: %2d, Type: %s:\n", portParams->datalinkId, PortSupportToString(portParams));
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            printf("      ");
            DumpIpPort("Dest Addr", portParams->FD_Table[i].dest_address.s_addr, ntohs(portParams->FD_Table[i].dest_port));
            dbTraffic(DBD_BBMD, DB_NOTE, " TTL:%d Rem:%5d Valid:%d\n",
                portParams->FD_Table[i].time_to_live,
                portParams->FD_Table[i].seconds_remaining,
                portParams->FD_Table[i].valid);
        }
    }
}

extern PORT_SUPPORT *datalinkSupportHead;

void ShowFDtables(void)
{
    printf("\nForeign Device Tables:");
    PORT_SUPPORT *ps = datalinkSupportHead;
    while (ps != NULL)
    {
        if (ps->portType == BPT_BBMD)
        {
            ShowFDtable(ps);
        }
        ps = (PORT_SUPPORT *)ps->llist.next;
    }
}
#else
void bvlc_bdt_backup_local (void) {}
void bvlc_bdt_restore_local(void) {}
#endif

/** A timer function that is called about once a second.
 *
 * @param seconds - number of elapsed seconds since the last call
 */
void bvlc_maintenance_timer(
    PORT_SUPPORT *portParams,
    time_t seconds)
{
    unsigned i;

    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            if (portParams->FD_Table[i].seconds_remaining) {
                if (portParams->FD_Table[i].seconds_remaining < seconds) {
                    portParams->FD_Table[i].seconds_remaining = 0;
                }
                else {
                    portParams->FD_Table[i].seconds_remaining -= seconds;
                }
                if (portParams->FD_Table[i].seconds_remaining == 0) {
                    portParams->FD_Table[i].valid = false;
                }
            }
        }
    }
}

/** Copy the source internet address to the BACnet address
 *
 * FIXME: IPv6?
 *
 * @param src - returns the BACnet source address
 * @param sin - source address in network order
 *
 * @return number of bytes decoded
 */
static void bvlc_internet_to_bacnet_mac_address(
    BACNET_MAC_ADDRESS *src,
    struct sockaddr_in *sin)
{
    if (src && sin) {
        memcpy(&src->bytes[0], &sin->sin_addr.s_addr, 4);
        memcpy(&src->bytes[4], &sin->sin_port, 2);
        src->len = (uint8_t)6;
        // src->net = 0;
        // src->len = 0;
    }

}

/** Encode the address entry.  Used for both read and write entries.
 *
 * Addressing within B/IP Networks
 * In the case of B/IP networks, six octets consisting of the four-octet
 * IP address followed by a two-octet UDP port number (both of
 * which shall be transmitted most significant octet first).
 * Note: for local storage, the storage order is NETWORK byte order.
 * Note: BACnet unsigned is encoded as most significant octet.
 *
 * @param pdu - buffer to extract encoded address
 * @param address - address in network order
 * @param port - UDP port number in network order
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_bip_address(
    uint8_t * pdu,
    const struct in_addr *address,
    const uint16_t port)
{
    int len = 0;

    if (pdu) {
        memcpy(&pdu[0], &address->s_addr, 4);
        memcpy(&pdu[4], &port, 2);
        len = 6;
    }

    return len;
}

/** Decode the address entry.  Used for both read and write entries.
 *
 * @param pdu - buffer to extract encoded address
 * @param address - address in network order
 * @param port - UDP port number in network order
 *
 * @return number of bytes decoded
 */
static int bvlc_decode_bip_address(
    const uint8_t * pdu,
    struct in_addr *address,
    uint16_t * port)
{
    int len = 0;

    if (pdu) {
        memcpy(&address->s_addr, &pdu[0], 4);
        memcpy(port, &pdu[4], 2);
        len = 6;
    }

    return len;
}

/** Encode the address entry.  Used for both read and write entries.
 *
 * @param pdu - buffer to store the encoding
 * @param address - address in network order
 * @param port - UDP port number in network order
 * @param mask - address mask in network order
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_address_entry(
    uint8_t * pdu,
    struct in_addr *address,
    uint16_t port,      /* in network byte order */
    struct in_addr *mask)
{
    int len = 0;

    if (pdu) {
        len = bvlc_encode_bip_address(pdu, address, port);
        memcpy(&pdu[len], &mask->s_addr, 4);
        len += 4;
    }

    return len;
}


/** Encode the BVLC Result message
 *
 * @param pdu - buffer to store the encoding
 * @param result_code - BVLC result code
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_bvlc_result(
    uint8_t * pdu,
    BACNET_BVLC_RESULT result_code)
{
    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_RESULT;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 6);
        encode_unsigned16(&pdu[4], (uint16_t) result_code);
    }

    return 6;
}

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode the initial part of the Read-Broadcast-Distribution-Table message
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of BDT entries
 *
 * @return number of bytes encoded
 */
int bvlc_encode_write_bdt_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) (entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}
#endif

/** Encode a Read-Broadcast-Distribution-Table message
 *
 * @param pdu - buffer to store the encoding
 *
 * @return number of bytes encoded
 */
int bvlc_encode_read_bdt(
    uint8_t * pdu)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_BROADCAST_DIST_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 4);
        len = 4;
    }

    return len;
}

/**
 * Read the Read-Broadcast-Distribution-Table of a BBMD
 *
 * @param bbmd_address - IPv4 address (long) of BBMD to read,
 *  in network byte order.
 * @param bbmd_port - Network port of BBMD to read, in network byte order
 * @return Upon successful completion, returns the number of bytes sent.
 *  Otherwise, -1 shall be returned and errno set to indicate the error.
 */
#if 0
int bvlc_bbmd_read_bdt(
    PORT_SUPPORT *portParams,
    uint32_t bbmd_address,
    uint16_t bbmd_port)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };   // todo1 malloc?
    uint16_t mtu_len;
    int rv = 0;
    struct sockaddr_in bbmd = { 0 };

    mtu_len = bvlc_encode_read_bdt(mtu);
    if (mtu_len > 0) {
        bbmd.sin_addr.s_addr = bbmd_address;
        bbmd.sin_port = bbmd_port;
        rv = bvlc_send_mpdu(&bbmd, &mtu[0], mtu_len);
    }

    return rv;
}
#endif

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Encode the initial part of the Read BDT Ack message
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of BDT entries
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_bdt_ack_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_BROADCAST_DIST_TABLE_ACK;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t)(entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}

/** Encode a Read BDT Ack message
 *
 * @param pdu - buffer to store the encoding
 * @param max_pdu - size of the buffer to store the encoding
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_bdt_ack(
    PORT_SUPPORT *portParams,
    uint8_t * pdu,
    uint16_t max_pdu)
{
    int pdu_len = 0;    /* return value */
    int len = 0;
    unsigned count = 0;
    unsigned i;

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (portParams->BBMD_Table[i].valid) {
            count++;
        }
    }
    len = bvlc_encode_read_bdt_ack_init(&pdu[0], count);
    pdu_len += len;
    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (portParams->BBMD_Table[i].valid) {
            /* too much to send */
            if ((pdu_len + 10) > max_pdu) {
                pdu_len = 0;
                break;
            }
            len =
                bvlc_encode_address_entry(&pdu[pdu_len],
                &portParams->BBMD_Table[i].dest_address,
                portParams->BBMD_Table[i].dest_port,
                &portParams->BBMD_Table[i].broadcast_mask);
            pdu_len += len;
        }
    }

    return pdu_len;
}

/** Encode a Forwarded NPDU message
 *
 * @param pdu - buffer to store the encoding
 * @param sin - source address in network order
 * @param npdu - NPDU to forward
 * @param max_npdu - amount of space available in the NPDU
 * @param npdu_length - size of the NPDU to forward
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_forwarded_npdu(
    uint8_t * pdu,
    const struct sockaddr_in *sin,    /* source address in network order */
    const uint8_t * npdu,
    const uint16_t max_npdu,
    const uint16_t npdu_length)
{
    int len = 0;

    unsigned i; /* for loop counter */

    if (pdu && sin && npdu) {
        if ((npdu_length + 4 + 6) <= max_npdu) {
            pdu[0] = BVLL_TYPE_BACNET_IP;
            pdu[1] = BVLC_FORWARDED_NPDU;
            /* The 2-octet BVLC Length field is the length, in octets,
               of the entire BVLL message, including the two octets of the
               length field itself, most significant octet first. */
            encode_unsigned16(&pdu[2], (uint16_t) (4 + 6 + npdu_length));
            len = 4;
            /* 6-octet address encoding */
            len +=
                bvlc_encode_bip_address(&pdu[len], &sin->sin_addr,
                    sin->sin_port);
            for (i = 0; i < npdu_length; i++) {
                pdu[len] = npdu[i];
                len++;
            }
        }
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode a Read Foreign Device Table message
 *
 * @param pdu - buffer to store the encoding
 *
 * @return number of bytes encoded
 */
int bvlc_encode_read_fdt(
    uint8_t * pdu)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_FOREIGN_DEVICE_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 4);
        len = 4;
    }

    return len;
}
#endif


#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Encode the initial part of a Read Foreign Device Table Ack
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of foreign device entries in this Ack.
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_fdt_ack_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_FOREIGN_DEVICE_TABLE_ACK;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t)(entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}

/** Encode a Read Foreign Device Table Ack
 *
 * @param pdu - buffer to store the encoding
 * @param max_pdu - number of bytes available to encode
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_fdt_ack(
    PORT_SUPPORT *portParams,
    uint8_t * pdu,
    uint16_t max_pdu)
{
    int pdu_len = 0;    /* return value */
    int len = 0;
    unsigned count = 0;
    unsigned i;
    uint16_t seconds_remaining = 0;

    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            count++;
        }
    }
    len = bvlc_encode_read_fdt_ack_init(&pdu[0], count);
    pdu_len += len;
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            /* too much to send */
            if ((pdu_len + 10) > max_pdu) {
                pdu_len = 0;
                break;
            }
            len =
                bvlc_encode_bip_address(&pdu[pdu_len],
                &portParams->FD_Table[i].dest_address, portParams->FD_Table[i].dest_port);
            pdu_len += len;
            len = encode_unsigned16(&pdu[pdu_len], portParams->FD_Table[i].time_to_live);
            pdu_len += len;
            seconds_remaining = (uint16_t)portParams->FD_Table[i].seconds_remaining;
            len = encode_unsigned16(&pdu[pdu_len], seconds_remaining);
            pdu_len += len;
        }
    }

    return pdu_len;
}
#endif


#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Foreign Device Table entry
 *
 * @param pdu - buffer to store the encoding
 * @param address - in network byte order
 * @param port - in network byte order
 *
 * @return number of bytes encoded
 */
int bvlc_encode_delete_fdt_entry(
    uint8_t * pdu,
    uint32_t address,   /* in network byte order */
    uint16_t port)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 10);
        /* FDT Entry */
        encode_unsigned32(&pdu[4], address);
        encode_unsigned16(&pdu[8], port);
        len = 10;
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Original Unicast NPDU
 *
 * @param pdu - buffer to store the encoding
 * @param npdu - NPDU portion of message
 * @param npdu_length - number of bytes to encode
 *
 * @return number of bytes encoded
 */
int bvlc_encode_original_unicast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length)
{
    int len = 0;        /* return value */
    unsigned i = 0;     /* loop counter */
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) npdu_length;
        len = encode_unsigned16(&pdu[2], BVLC_length) + 2;
        for (i = 0; i < npdu_length; i++) {
            pdu[len] = npdu[i];
            len++;
        }
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Original Broadcast NPDU
 *
 * @param pdu - buffer to store the encoding
 * @param npdu - NPDU portion of message
 * @param npdu_length - number of bytes to encode
 *
 * @return number of bytes encoded
 */
int bvlc_encode_original_broadcast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length)
{
    int len = 0;        /* return value */
    unsigned i = 0;     /* loop counter */
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) npdu_length;
        len = encode_unsigned16(&pdu[2], BVLC_length) + 2;
        for (i = 0; i < npdu_length; i++) {
            pdu[len] = npdu[i];
            len++;
        }
    }

    return len;
}
#endif


#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Create a Broadcast Distribution Table from message
 *
 * @param npdu - message from which the devices are decoded
 * @param npdu_length - number of bytes to decode
 *
 * @return true if all the entries fit in the table
 */
static bool bvlc_create_bdt(
    PORT_SUPPORT *portParams,
    uint8_t * npdu,
    uint16_t npdu_length)
{
    bool status = false;
    unsigned i;
    uint16_t pdu_offset = 0;

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (npdu_length >= 10) {
            portParams->BBMD_Table[i].valid = true;
            memcpy(&portParams->BBMD_Table[i].dest_address.s_addr, &npdu[pdu_offset], 4);
            pdu_offset += 4;
            memcpy(&portParams->BBMD_Table[i].dest_port, &npdu[pdu_offset], 2);
            pdu_offset += 2;
            memcpy(&portParams->BBMD_Table[i].broadcast_mask.s_addr, &npdu[pdu_offset], 4);
            pdu_offset += 4;
            npdu_length -= (4 + 2 + 4);
        }
        else {
            portParams->BBMD_Table[i].valid = false;
            portParams->BBMD_Table[i].dest_address.s_addr = 0;
            portParams->BBMD_Table[i].dest_port = 0;
            portParams->BBMD_Table[i].broadcast_mask.s_addr = 0;
        }
    }
    /* BDT changed! Save backup to file */
    bvlc_bdt_backup_local();

    /* did they all fit? */
    if (npdu_length < 10) {
        status = true;
    }

    return status;
}

/** Register a Foreign Device in the Foreign Device Table
 *
 * @param sin - source address in network order
 * @param time_to_live - time in seconds
 *
 * @return true if the Foreign Device was added
 */
static bool bvlc_register_foreign_device(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *sin,
    uint16_t time_to_live_seconds )
{
    unsigned i;
    bool status = false;

    /* am I here already?  If so, update my time to live... */
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            if ((portParams->FD_Table[i].dest_address.s_addr == sin->sin_addr.s_addr) &&
                (portParams->FD_Table[i].dest_port == sin->sin_port)) {
                status = true;
                portParams->FD_Table[i].time_to_live = time_to_live_seconds;
                /*  Upon receipt of a BVLL Register-Foreign-Device message,
                   a BBMD shall start a timer with a value equal to the
                   Time-to-Live parameter supplied plus a fixed grace
                   period of 30 seconds. */
                portParams->FD_Table[i].seconds_remaining = time_to_live_seconds + 30;
                break;
            }
        }
    }
    if (!status) {
        for (i = 0; i < MAX_FD_ENTRIES; i++) {
            if (!portParams->FD_Table[i].valid) {
                portParams->FD_Table[i].dest_address.s_addr = sin->sin_addr.s_addr;
                portParams->FD_Table[i].dest_port = sin->sin_port;
                portParams->FD_Table[i].time_to_live = time_to_live_seconds;
                portParams->FD_Table[i].seconds_remaining = time_to_live_seconds + 30;
                portParams->FD_Table[i].valid = true;
                status = true;
                break;
            }
        }
    }


    return status;
}

/** Delete a Foreign Device from the Foreign Device Table
 *
 * @param pdu - BACnet/IP address in PDU form
 *
 * @return true if the Foreign Device was found and removed.
 */
static bool bvlc_delete_foreign_device(
    PORT_SUPPORT *portParams,
    uint8_t * pdu)
{
    struct sockaddr_in sin = { 0 };     /* the ip address */
    bool status = false;        /* return value */
    unsigned i;

    bvlc_decode_bip_address(pdu, &sin.sin_addr, &sin.sin_port);
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid) {
            if ((portParams->FD_Table[i].dest_address.s_addr == sin.sin_addr.s_addr) &&
                (portParams->FD_Table[i].dest_port == sin.sin_port)) {
                portParams->FD_Table[i].valid = false;
                portParams->FD_Table[i].seconds_remaining = 0;
                status = true;
                break;
            }
        }
    }
    return status;
}
#endif

/**
 * The common send function for bvlc functions, using b/ip.
 *
 * @param dest - Points to a sockaddr_in structure containing the
 *  destination address. The length and format of the address depend
 *  on the address family of the socket (AF_INET).
 *  The address is in network byte order.
 * @param mtu - the bytes of data to send
 * @param mtu_len - the number of bytes of data to send
 * @return Upon successful completion, returns the number of bytes sent.
 *  Otherwise, -1 shall be returned and errno set to indicate the error.
 */
static int bvlc_send_mpdu(
    const PORT_SUPPORT *portParams,
    const struct sockaddr_in *dest,
    const uint8_t * mtu,
    const uint16_t mtu_len)
{
    struct sockaddr_in bvlc_dest = { 0 };

    /* assumes that the driver has already been initialized */
    if (portParams->datalink.bipParams.socket < 0) {
        return 0;
    }

    // todo4 - grok why we create a temp dest...
    bvlc_dest.sin_family = AF_INET;
    bvlc_dest.sin_addr.s_addr = dest->sin_addr.s_addr;
    bvlc_dest.sin_port = dest->sin_port;
    memset(&(bvlc_dest.sin_zero), '\0', 8);

    //     bip_ipAddr_port_from_bacnet_address( &bvlc_dest.sin_addr, &bvlc_dest.sin_port, phyDest);

    BACNET_MAC_ADDRESS smac, dmac;

    bip_ipAddr_port_to_bacnet_mac(&smac, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);
    bip_ipAddr_port_to_bacnet_mac(&dmac, bvlc_dest.sin_addr.s_addr, bvlc_dest.sin_port);

    SendBTApacketTx(portParams->datalinkId, &smac, &dmac, mtu, mtu_len);

    /* Send the packet */
    return sendto(portParams->datalink.bipParams.socket, (char *)mtu, mtu_len, 0,
        (struct sockaddr *) &bvlc_dest, sizeof(struct sockaddr));
}


#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Sends all Broadcast Devices a Forwarded NPDU
 *
 * @param sin - source address in network order
 * @param npdu - the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param npdu_length - length of the NPDU
 * @param original - was the message an original (not forwarded)
 */
static void bvlc_bdt_forward_npdu(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *sin,
    uint8_t * npdu,
    uint16_t max_npdu,
    uint16_t npdu_length,
    bool original)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;
    unsigned i = 0;     /* loop counter */
    struct sockaddr_in bip_dest = { 0 };

    /* If we are forwarding an original broadcast message and the NAT
     * handling is enabled, change the source address to NAT routers
     * global IP address so the recipient can reply (local IP address
     * is not accesible from internet side).
     *
     * If we are forwarding a message from peer BBMD or foreign device
     * or the NAT handling is disabled, leave the source address as is.
     */
    if (portParams->datalink.bipParams.BVLC_NAT_Handling && original) {
        // struct sockaddr_in nat_addr = *sin;
        // nat_addr.sin_addr = BVLC_Global_Address;
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
            &portParams->datalink.bipParams.BVLC_Global_Address, npdu, max_npdu, npdu_length);
    }
    else {
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
            sin, npdu, max_npdu, npdu_length);
    }

    /* loop through the BDT and send one to each entry, except us */
    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (portParams->BBMD_Table[i].valid) {
            /* The B/IP address to which the Forwarded-NPDU message is
               sent is formed by inverting the broadcast distribution
               mask in the BDT entry and logically ORing it with the
               BBMD address of the same entry. */
            bip_dest.sin_addr.s_addr =
                ((~portParams->BBMD_Table[i].broadcast_mask.
                s_addr) | portParams->BBMD_Table[i].dest_address.s_addr);
            bip_dest.sin_port = portParams->BBMD_Table[i].dest_port;

            /* don't send to my broadcast address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_broadcast_ipAddr(portParams))
                && (bip_dest.sin_port == bip_get_local_port(portParams))) {
                continue;
            }
            /* don't send to my IP address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_addr(portParams)) &&
                (bip_dest.sin_port == bip_get_local_port(portParams))) {
                continue;
            }
            /* NAT router port forwards BACnet packets from global IP to us.
             * Packets sent to that global IP by us would end up back, creating
             * a loop.
             */
            if (portParams->datalink.bipParams.BVLC_NAT_Handling &&
                (bip_dest.sin_addr.s_addr == portParams->datalink.bipParams.BVLC_Global_Address.sin_addr.s_addr) &&
                (bip_dest.sin_port == portParams->datalink.bipParams.BVLC_Global_Address.sin_port)) {
                continue;
            }
            bvlc_send_mpdu(portParams, &bip_dest, mtu, mtu_len);

            // char tstring[30];
            // dbTraffic(DBD_ALL, DB_INFO, "BVLC: BDT Sent Forwarded-NPDU to %s:%04d",
        }
    }
}


/** Send a BVLL Forwarded-NPDU message on its local IP subnet using
 * the local B/IP broadcast address as the destination address.
 *
 * @param sin - source address in network order
 * @param npdu - the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param npdu_length - reported length of the NPDU
 */
static void bvlc_broadcast_npdu(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *sin,
    uint8_t * npdu,
    uint16_t max_npdu,
    uint16_t npdu_length)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;
    struct sockaddr_in bip_dest = { 0 };

    mtu_len =
        (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0], sin, npdu,
        max_npdu, npdu_length);
    bip_dest.sin_addr.s_addr = bip_get_broadcast_ipAddr(portParams);
    bip_dest.sin_port = bip_get_local_port(portParams);

    // Use forwarded-NPDU per the spec J.4.5.
    dbTraffic(DBD_ALL, DB_INFO, "BVLC: Sending Forwarded-NPDU as local broadcast on port %d.", ntohs(bip_dest.sin_port));

    bvlc_send_mpdu(portParams, &bip_dest, mtu, mtu_len);
}

/** Sends all Foreign Devices a Forwarded NPDU
 *
 * @param sin - source address in network order
 * @param npdu - returns the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param npdu_length - reported length of the NPDU
 * @param original - was the message an original (not forwarded)
 */
static void bvlc_fdt_forward_npdu(
    const PORT_SUPPORT *portParams,
    const struct sockaddr_in *sin,
    const uint8_t * npdu,
    const uint16_t max_npdu,
    const uint16_t npdu_length,
    const bool original)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;
    unsigned i = 0;     /* loop counter */
    struct sockaddr_in bip_dest = { 0 };

    /* If we are forwarding an original broadcast message and the NAT
     * handling is enabled, change the source address to NAT routers
     * global IP address so the recipient can reply (local IP address
     * is not accesible from internet side.
     *
     * If we are forwarding a message from peer BBMD or foreign device
     * or the NAT handling is disabled, leave the source address as is.
     */
    if (portParams->datalink.bipParams.BVLC_NAT_Handling && original) {
        // struct sockaddr_in nat_addr = *sin;
        // nat_addr.sin_addr = BVLC_Global_Address;
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
            &portParams->datalink.bipParams.BVLC_Global_Address, npdu, max_npdu, npdu_length);
    }
    else {
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
            sin, npdu, max_npdu, npdu_length);
    }

    /* loop through the FDT and send one to each entry */
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (portParams->FD_Table[i].valid && portParams->FD_Table[i].seconds_remaining) {
            bip_dest.sin_addr.s_addr = portParams->FD_Table[i].dest_address.s_addr;
            bip_dest.sin_port = portParams->FD_Table[i].dest_port;
            /* don't send to my ip address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_addr(portParams)) &&
                (bip_dest.sin_port == bip_get_local_port(portParams))) {
                continue;
            }
            /* don't send to src ip address and same port */
            if ((bip_dest.sin_addr.s_addr == sin->sin_addr.s_addr) &&
                (bip_dest.sin_port == sin->sin_port)) {
                continue;
            }
            /* NAT router port forwards BACnet packets from global IP to us.
             * Packets sent to that global IP by us would end up back, creating
             * a loop.
             */
            if (portParams->datalink.bipParams.BVLC_NAT_Handling &&
                (bip_dest.sin_addr.s_addr == portParams->datalink.bipParams.BVLC_Global_Address.sin_addr.s_addr) &&
                (bip_dest.sin_port == portParams->datalink.bipParams.BVLC_Global_Address.sin_port)) {
                continue;
            }
            dbTraffic(DBD_ALL, DB_INFO, "m0033 - BVLC: FDT Sending Forwarded-NPDU to %s:%d",
                inet_ntoa(bip_dest.sin_addr), ntohs(bip_dest.sin_port));
            bvlc_send_mpdu(portParams, &bip_dest, mtu, mtu_len);
        }
    }
}
#endif


/** Sends a BVLC Result
 *
 * @param dest - destination address
 * @param result_code - result code to send
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_result(
    const PORT_SUPPORT *portParams,
    struct sockaddr_in *dest,   /* the destination address */
    BACNET_BVLC_RESULT result_code)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t)bvlc_encode_bvlc_result(&mtu[0], result_code);
    if (mtu_len) {
        bvlc_send_mpdu(portParams, dest, mtu, mtu_len);
    }

    return mtu_len;
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Sends a Read Broadcast Device Table ACK
 *
 * @param dest - destination address
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_bdt(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *dest)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t)bvlc_encode_read_bdt_ack(portParams, &mtu[0], sizeof(mtu));
    if (mtu_len) {
        bvlc_send_mpdu(portParams, dest, &mtu[0], mtu_len);
    }

    return mtu_len;
}


/** Sends a Read Foreign Device Table ACK
 *
 * @param dest - destination address
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_fdt(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *dest)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t)bvlc_encode_read_fdt_ack(portParams, &mtu[0], sizeof(mtu));
    if (mtu_len) {
        bvlc_send_mpdu(portParams, dest, &mtu[0], mtu_len);
    }

    return mtu_len;
}

/** Determines if a BDT member has a unicast mask
 *
 * @param sin - BDT member that is sought, network byte order address
 *
 * @return True if BDT member is found and has a unicast mask
 */
static bool bvlc_bdt_member_mask_is_unicast(
    PORT_SUPPORT *portParams,
    struct sockaddr_in *sin)
{
    bool unicast = false;
    unsigned i;     /* loop counter */

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (portParams->BBMD_Table[i].valid) {

            // TODO - ADD BDC ekh todo3
            /* Skip ourself*/
            if ((portParams->BBMD_Table[i].dest_address.s_addr == portParams->datalink.bipParams.nwoLocal_addr) &&
                (portParams->BBMD_Table[i].dest_port == bip_get_local_port(portParams))) {
                continue;
            }

            /* find the source address in the table */
            if ((portParams->BBMD_Table[i].dest_address.s_addr == sin->sin_addr.s_addr) &&
                (portParams->BBMD_Table[i].dest_port == sin->sin_port)) {
                /* unicast mask? */
                if (portParams->BBMD_Table[i].broadcast_mask.s_addr == 0xFFFFFFFFL) {
                    unicast = true;
                    break;
                }
            }
        }
    }

    return unicast;
}

// todo3 - see http://stackoverflow.com/questions/2432493/how-to-detemine-which-network-interface-ip-address-will-be-used-to-send-a-pack 
// for linux https://stackoverflow.com/questions/14398099/how-to-programatically-determine-which-source-ip-address-will-be-used-to-reach-a
// todo 2 - this trick only works if we know the destination of the peer.... i.e. we should wait for a ping from BTA before trying to establish local address/interface..

#if defined ( _MSC_VER  )
void TryGetLocalAddress(struct sockaddr_in *sin)
{
    struct sockaddr_in ourAddr;
    int ourLen = sizeof(ourAddr);

    // not known, do a quick connect, check, disconnect
    int newsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (newsock < 0) {
        panic();
        return;
    }

    // quick connect to them
    int err = connect(newsock, (struct sockaddr *)sin, sizeof(struct sockaddr));
    if (err)
    {
        int err2 = WSAGetLastError();
        printf("Err = %d\n", err2);
        panic();
        closesocket(newsock);
        return;
    }

    // peek at us
    err = getsockname(newsock, (struct sockaddr *)&ourAddr, &ourLen);
    if (err)
    {
        int err2 = WSAGetLastError();
        printf("Err = %d\n", err2);
        panic();
        closesocket(newsock);
        return;
    }

    closesocket(newsock);
    *sin = ourAddr ;
}
#endif

// todo2: why does CheckLocalAddressKnown() leave socket bound?
void CheckLocalAddressKnown(PORT_SUPPORT *portParams, struct sockaddr_in *sin)
{
    if (portParams->datalink.bipParams.nwoLocal_addr == 0)
    {
        struct sockaddr_in ourAddr;
        int ourLen = sizeof(ourAddr);
        int err;

#if 0
        // peek at us
        err = getsockname(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            return;
        }

        // simply connect to the startion sending to us. The OS will bind our socket to that interface.
        // If we send to another IP address on the same subnet later, we will be OK
        err = connect(portParams->datalink.bipParams.socket, (struct sockaddr *)sin, sizeof(struct sockaddr));
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            return;
        }

        // peek at us
        err = getsockname(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2 );
            panic();
            return;
        }

        // and unconnect, ignoring possible errors. See Stevens, pg 254
        sin->sin_family = AF_UNSPEC;
        connect(portParams->datalink.bipParams.socket, (struct sockaddr *)sin, sizeof(struct sockaddr));

        portParams->datalink.bipParams.nwoLocal_addr = ourAddr.sin_addr.s_addr;
        set_broadcast_address(portParams, portParams->datalink.bipParams.nwoLocal_addr);
#endif 

#if defined ( _MSC_VER )
        // todo1 - I need to worry a bit about this...

        // not known, do a quick connect, check, disconnect
        int newsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (newsock < 0) {
            panic();
            return;
        }

        // quick connect to them
        err = connect(newsock, (struct sockaddr *)sin, sizeof(struct sockaddr));
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2);
            panic();
            closesocket(newsock);
            return;
        }

        // peek at us
        err = getsockname(newsock, (struct sockaddr *)&ourAddr, &ourLen);
        if (err)
        {
            int err2 = WSAGetLastError();
            printf("Err = %d\n", err2 );
            panic();
            closesocket(newsock);
            return;
        }

        closesocket(newsock);
        portParams->datalink.bipParams.nwoLocal_addr = ourAddr.sin_addr.s_addr;

        set_broadcast_address(portParams, portParams->datalink.bipParams.nwoLocal_addr);

        // AND... since we are doing this mainly because we are on a multi-interfaced machine, we need to "lock down" our IP endpoint so we 
        // are forever deaf to traffic on the other adapters, else we will end up representing ourselves as multiple devices in some situations. 
        // e.g. someone does a 255.255.255.255 broadcast and gets responses from our one node on different adapters... which would be a bad thing, right?
        closesocket(portParams->datalink.bipParams.socket);
        
        // reopen socket, this time binding to the IP address for the interface
        portParams->datalink.bipParams.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (portParams->datalink.bipParams.socket < 0) {
            fprintf(stderr, "bip: failed to allocate a socket.\n");
            return ;
        }
        /* Allow us to use the same socket for sending and receiving */
        /* This makes sure that the src port is correct when sending */
        int value = 1;
        err = setsockopt(portParams->datalink.bipParams.socket, SOL_SOCKET, SO_REUSEADDR, (char *)&value,
            sizeof(value));
        if (err < 0) {
            fprintf(stderr, "bip: failed to set REUSEADDR socket option.\n");
            closesocket(portParams->datalink.bipParams.socket);
            portParams->datalink.bipParams.socket = -1;
            return ;
        }
        /* Enables transmission and receipt of broadcast messages on the socket. */
        err = setsockopt(portParams->datalink.bipParams.socket, SOL_SOCKET, SO_BROADCAST, (char *)&value,
            sizeof(value));
        if (err < 0) {
            fprintf(stderr, "bip: failed to set BROADCAST socket option.\n");
            closesocket(portParams->datalink.bipParams.socket);
            portParams->datalink.bipParams.socket = -1;
            return ;
        }

        ourAddr.sin_port = portParams->datalink.bipParams.nwoPort;

        err = bind(portParams->datalink.bipParams.socket, (struct sockaddr *)&ourAddr, sizeof(struct sockaddr) );
        if (err)
        {
            int err2 = WSAGetLastError();
            const char *errmsg = winsock_error_code_text(err2);
            printf("Err = %s\n", errmsg);
            panic();
            closesocket(newsock);
            return;
        }
#endif

    }
}
#endif // _MSC_VER


/** Receive a packet from the BACnet/IP socket (Annex J)
 *
 * @param src - returns the source address
 * @param npdu - returns the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param timeout - number of milliseconds to wait for a packet
 *
 * @return Number of bytes received, or 0 if none or timeout.
 */
uint16_t bbmd_receive(
    PORT_SUPPORT *portParams,
    BACNET_MAC_ADDRESS *src,
    uint8_t * npdu,
    uint16_t max_npdu)
{
    uint16_t npdu_len ;      /* return value */
    fd_set read_fds;
    int max;
    struct timeval select_timeout;
	struct sockaddr_in sin; // = { 0 };
	struct sockaddr_in original_sin; // = { 0 };
	struct sockaddr_in dest; //  = { 0 };
#if _MSC_VER
    socklen_t sin_len = sizeof(sin);
#elif __GNUC__
    socklen_t sin_len = sizeof(sin);
#endif
    int received_bytes ;
    uint16_t result_code ;
    uint16_t i ;
    bool status = false;
    uint16_t time_to_live ;

    /* Make sure the socket is open */
    if (portParams->datalink.bipParams.socket < 0) {
        return 0;
    }

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 10000;

    FD_ZERO(&read_fds);
    FD_SET(portParams->datalink.bipParams.socket, &read_fds);
    max = portParams->datalink.bipParams.socket;
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {
        received_bytes =
            recvfrom(portParams->datalink.bipParams.socket, (char *)&npdu[0], max_npdu, 0,
            (struct sockaddr *) &sin, &sin_len);
    }
    else {
        return 0;
    }
    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }
    /* no problem, just no bytes */
    if (received_bytes == 0) {
        return 0;
    }

    // Now, do we know our local IP address? If not, now is our chance to find it...
#if defined ( _MSC_VER  )
    CheckLocalAddressKnown( portParams, &sin);
#endif

    /* the signature of a BACnet/IP packet */
    if (npdu[0] != BVLL_TYPE_BACNET_IP) {
        return 0;
    }

    // we do NOT want to process packets from ourselves
    if (sin.sin_addr.s_addr == portParams->datalink.bipParams.nwoLocal_addr && sin.sin_port == portParams->datalink.bipParams.nwoPort)
    {
        return 0;
    }

    BACNET_MAC_ADDRESS dmac;

    bip_ipAddr_port_to_bacnet_mac(src, sin.sin_addr.s_addr, sin.sin_port);
    bip_ipAddr_port_to_bacnet_mac(&dmac, portParams->datalink.bipParams.nwoLocal_addr, portParams->datalink.bipParams.nwoPort);

    SendBTApacketRx(portParams->datalinkId, src, &dmac, npdu, received_bytes);


    BVLC_Function_Code = (BACNET_BVLC_FUNCTION)npdu[1];
    /* decode the length of the PDU - length is inclusive of BVLC */
    (void)decode_unsigned16(&npdu[2], &npdu_len);

    // BTC fuzz test opportunity: todo 2 - In another project I had <= 4...
    if (npdu_len < 4) {
        return 0;
    }

    // done above as byproduct of bta bip_ipAddr_port_to_bacnet_mac(src, sin.sin_addr.s_addr, sin.sin_port);

    /* subtract off the BVLC header */
    npdu_len -= 4;

    switch (BVLC_Function_Code) {
    case BVLC_RESULT:
        /* Upon receipt of a BVLC-Result message containing a result code
           of X'0000' indicating the successful completion of the
           registration, a foreign device shall start a timer with a value
           equal to the Time-to-Live parameter of the preceding Register-
           Foreign-Device message. At the expiration of the timer, the
           foreign device shall re-register with the BBMD by sending a BVLL
           Register-Foreign-Device message */
        /* Clients can now get this result */
        (void)decode_unsigned16(&npdu[4], &result_code);
        BVLC_Result_Code = (BACNET_BVLC_RESULT)result_code;
        dbTraffic(DBD_ALL, DB_DEBUG, "BVLC: Result Code=%d", BVLC_Result_Code);
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE:
        dbTraffic(DBD_ALL, DB_ERROR, "BVLC: Received Write-BDT.");
        /* Upon receipt of a BVLL Write-Broadcast-Distribution-Table
           message, a BBMD shall attempt to create or replace its BDT,
           depending on whether or not a BDT has previously existed.
           If the creation or replacement of the BDT is successful, the BBMD
           shall return a BVLC-Result message to the originating device with
           a result code of X'0000'. Otherwise, the BBMD shall return a
           BVLC-Result message to the originating device with a result code
           of X'0010' indicating that the write attempt has failed. */
        status = bvlc_create_bdt(portParams, &npdu[4], npdu_len);
        if (status) {
            bvlc_send_result(portParams, &sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
        }
        else {
            bvlc_send_result(portParams, &sin,
                BVLC_RESULT_WRITE_BROADCAST_DISTRIBUTION_TABLE_NAK);
        }
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_READ_BROADCAST_DIST_TABLE:
        dbTraffic(DBD_ALL, DB_ERROR, "BVLC: Received Read-BDT.");
        /* Upon receipt of a BVLL Read-Broadcast-Distribution-Table
           message, a BBMD shall load the contents of its BDT into a BVLL
           Read-Broadcast-Distribution-Table-Ack message and send it to the
           originating device. If the BBMD is unable to perform the
           read of its BDT, it shall return a BVLC-Result message to the
           originating device with a result code of X'0020' indicating that
           the read attempt has failed. */
        if (bvlc_send_bdt(portParams, &sin) <= 0) {
            bvlc_send_result(portParams, &sin,
                BVLC_RESULT_READ_BROADCAST_DISTRIBUTION_TABLE_NAK);
        }
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_READ_BROADCAST_DIST_TABLE_ACK:
        dbTraffic(DBD_ALL, DB_ERROR, "BVLC: Received Read-BDT-Ack.");
        /* FIXME: complete the code for client side read */
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_FORWARDED_NPDU:
        /* Upon receipt of a BVLL Forwarded-NPDU message, a BBMD shall
           process it according to whether it was received from a peer
           BBMD as the result of a directed broadcast or a unicast
           transmission. A BBMD may ascertain the method by which Forwarded-
           NPDU messages will arrive by inspecting the broadcast distribution
           mask field in its own BDT entry since all BDTs are required
           to be identical.

           <********

           If the message arrived via directed broadcast,
           it was also received by the other devices on the BBMD's subnet. In
           this case the BBMD merely retransmits the message directly to each
           foreign device currently in the BBMD's FDT.

           If the message arrived via a unicast transmission it has not yet been
           received by the other devices on the BBMD's subnet. In this case,
           the message is sent to the devices on the BBMD's subnet using the
           B/IP broadcast address as well as to each foreign device
           currently in the BBMD's FDT.

           *********

           [BTC todo3: New wording, conflicts with Steve's code, the text above]

           If the message arrived via directed broadcast,
           or if the source is a device located on the same IP subnet,
           a situation which can occur if two or more BBMDs are installed on the same IP subnet,
           it was also received by the other devices on the BBMD's subnet.
           In this case the BBMD merely retransmits the message directly to each foreign device currently in the BBMD's FDT.

           Otherwise the message arrived via a unicast transmission and it has not yet been
           received by the other devices on the BBMD's subnet. In this case,
           the message is sent to the devices on the BBMD's subnet using the
           B/IP broadcast address as well as to each foreign device
           currently in the BBMD's FDT
           [BTC todo - I dont see Karg doing this: bvlc_bdt_forward_npdu()

           ********>

           A BBMD on a subnet with no other
           BACnet devices may omit the broadcast using the B/IP
           broadcast address. The method by which a BBMD determines whether
           or not other BACnet devices are present is a local matter. */

        /* decode the 4 byte original address and 2 byte port */
        bvlc_decode_bip_address(&npdu[4], &original_sin.sin_addr,
            &original_sin.sin_port);

        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Forwarded-NPDU from %s:%d.",
            inet_ntoa(original_sin.sin_addr), ntohs(original_sin.sin_port));

        npdu_len -= 6;

        /*  Broadcast locally if received via unicast from a BDT member */
        if (bvlc_bdt_member_mask_is_unicast(portParams, &sin)) {
            dest.sin_addr.s_addr = bip_get_broadcast_ipAddr(portParams);
            dest.sin_port = portParams->datalink.bipParams.nwoPort;
            dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received unicast from BDT member, re-broadcasting locally to %s:%d.",
                inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
            bvlc_send_mpdu(portParams, &dest, &npdu[0], npdu_len + 4 + 6);
        }

        /* Distribute to Foreign Devices, use the original addr from the BVLC for src */
        dest.sin_addr.s_addr = original_sin.sin_addr.s_addr;
        dest.sin_port = original_sin.sin_port;
        bvlc_fdt_forward_npdu(portParams, &dest, &npdu[4 + 6], max_npdu - (4 + 6),
            npdu_len, false);

        // todo1 - conflicts with above
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Forwarded-NPDU from %s:%d.",
            inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
        bvlc_internet_to_bacnet_mac_address(src, &dest);

        if (npdu_len < max_npdu) {
            /* shift the buffer to return a valid PDU */
            //for (i = 0; i < npdu_len; i++) {
            //    npdu[i] = npdu[4 + 6 + i];
            //}
        	memmove(npdu, &npdu[4 + 6], npdu_len);
        }
        else {
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            npdu_len = 0;
        }
        break;

    case BVLC_REGISTER_FOREIGN_DEVICE:
        /* Upon receipt of a BVLL Register-Foreign-Device message, a BBMD
           shall start a timer with a value equal to the Time-to-Live
           parameter supplied plus a fixed grace period of 30 seconds. If,
           within the period during which the timer is active, another BVLL
           Register-Foreign-Device message from the same device is received,
           the timer shall be reset and restarted. If the time expires
           without the receipt of another BVLL Register-Foreign-Device
           message from the same foreign device, the FDT entry for this
           device shall be cleared. */
        (void)decode_unsigned16(&npdu[4], &time_to_live);
        if (bvlc_register_foreign_device(portParams, &sin, time_to_live)) {
            bvlc_send_result(portParams, &sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
            // todo3 restore dbTraffic(DBD_All, DB_INFO, "BBMD: Registered a Foreign Device.");
        }
        else {
            bvlc_send_result(portParams, &sin,
                BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK);
            dbTraffic(DBD_ALL, DB_ERROR, "BVLC: Failed to Register a Foreign Device.");
        }
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_READ_FOREIGN_DEVICE_TABLE:
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Read-FDT.");
        /* Upon receipt of a BVLL Read-Foreign-Device-Table message, a
           BBMD shall load the contents of its FDT into a BVLL Read-
           Foreign-Device-Table-Ack message and send it to the originating
           device. If the BBMD is unable to perform the read of its FDT,
           it shall return a BVLC-Result message to the originating device
           with a result code of X'0040' indicating that the read attempt has
           failed. */
        if (bvlc_send_fdt(portParams, &sin) <= 0) {
            bvlc_send_result(portParams, &sin,
                BVLC_RESULT_READ_FOREIGN_DEVICE_TABLE_NAK);
        }
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_READ_FOREIGN_DEVICE_TABLE_ACK:
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Read-FDT-Ack.\n");
        /* FIXME: complete the code for client side read */
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY:
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Delete-FDT-Entry.");
        /* Upon receipt of a BVLL Delete-Foreign-Device-Table-Entry
           message, a BBMD shall search its foreign device table for an entry
           corresponding to the B/IP address supplied in the message. If an
           entry is found, it shall be deleted and the BBMD shall return a
           BVLC-Result message to the originating device with a result code
           of X'0000'. Otherwise, the BBMD shall return a BVLCResult
           message to the originating device with a result code of X'0050'
           indicating that the deletion attempt has failed. */
        if (bvlc_delete_foreign_device(portParams, &npdu[4])) {
            bvlc_send_result(portParams, &sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
        }
        else {
            bvlc_send_result(portParams, &sin,
                BVLC_RESULT_DELETE_FOREIGN_DEVICE_TABLE_ENTRY_NAK);
        }
        /* not an NPDU */
        npdu_len = 0;
        break;

    case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Distribute-Broadcast-to-Network from %s:%d.",
            inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
        /* Upon receipt of a BVLL Distribute-Broadcast-To-Network message
           from a foreign device, the receiving BBMD shall transmit a
           BVLL Forwarded-NPDU message on its local IP subnet using the
           local B/IP broadcast address as the destination address. In
           addition, a Forwarded-NPDU message shall be sent to each entry
           in its BDT as described in the case of the receipt of a
           BVLL Original-Broadcast-NPDU as well as directly to each foreign
           device currently in the BBMD's FDT except the originating
           node. If the BBMD is unable to perform the forwarding function,
           it shall return a BVLC-Result message to the foreign device
           with a result code of X'0060' indicating that the forwarding
           attempt was unsuccessful */ // [btc todo3 - what of nodes w/o BBMDs? to they too return 0060?]

        // broadcast on local subnet. Note Karg's broadcast dependency below.
        // ...unless we are doing a NAT
        if (!portParams->datalink.bipParams.BVLC_NAT_Handling)
        {
            bvlc_broadcast_npdu(portParams, &sin, &npdu[4], max_npdu - 4, npdu_len);
        }

        // distribute to FDT and BDT. 
        bvlc_bdt_forward_npdu(portParams, &sin, &npdu[4], max_npdu - 4, npdu_len, false);
        bvlc_fdt_forward_npdu(portParams, &sin, &npdu[4], max_npdu - 4, npdu_len, false);


#if 0 // todo - From another project, investigate EKH: 2016.08.20       (enl)
        // actually, we need to process this message for all our local virtual devices too
        //            npdu_len = 0;
        if (npdu_len < max_npdu) {
            bvlc_internet_to_bacnet_address(src, &sin);
            for (i = 0; i < npdu_len; i++) {
                npdu[i] = npdu[4 + i];
            }
        }
        else {
            /* ignore packets that are too large */
            /* clients should check my max-apdu first */
            npdu_len = 0;
        }
#endif

#ifdef karg 
        // for the rest I disagree with Karg (npdu_len = 0 ). We actually do want this call to bbmd_receive to succeed
        // to that our application layer gets a chance to process the message too...
        /* not an NPDU */
        npdu_len = 0;
#else // karg        

        // we filter broadcasts from ourselves. Thus we cannot depend on the above broadcast to local subnet to arrive 'back' here. Therefore
        // we have to manipulate that distribute b'cast into an original b'cast as if just received on a regular IP port, and we need 
        bvlc_internet_to_bacnet_mac_address(src, &sin);
        if (npdu_len < max_npdu) {
            /* shift the buffer to return a valid PDU */
        	memmove(npdu, &npdu[4], npdu_len);
            //for (i = 0; i < npdu_len; i++) {
            //    npdu[i] = npdu[4 + i];
            //}
            // [btc-another fuzz test opportunity todo3]
            // npdu_len -= 4;
        }
        else {
            npdu_len = 0;
        }
#endif // karg
        break;

    case BVLC_ORIGINAL_UNICAST_NPDU:
        dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Original-Unicast-NPDU.");
        if ((sin.sin_addr.s_addr == bip_get_addr(portParams)) &&
            (sin.sin_port == portParams->datalink.bipParams.nwoPort)) {
            // btc todo btc
            /* ignore messages from me */
            npdu_len = 0;
        }
        else if (portParams->datalink.bipParams.BVLC_NAT_Handling &&
            (sin.sin_addr.s_addr == portParams->datalink.bipParams.BVLC_Global_Address.sin_addr.s_addr) &&
            (sin.sin_port == portParams->datalink.bipParams.BVLC_Global_Address.sin_port)) {
            /* If the BBMD is behind a NAT router, the router forwards packets from
               global IP and BACnet port to us. */
            npdu_len = 0;
        }
        else {
            bvlc_internet_to_bacnet_mac_address(src, &sin);
            if (npdu_len < max_npdu) {
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < npdu_len; i++) {
                    npdu[i] = npdu[4 + i];
                }
            }
            else {
                /* ignore packets that are too large */
                /* clients should check my max-apdu first */
                npdu_len = 0;
            }
        }
        break;

    case BVLC_ORIGINAL_BROADCAST_NPDU:
        if (portParams->datalink.bipParams.BVLC_NAT_Handling)
        {
            dbTraffic(DBD_ALL, DB_INFO, "BVLC: Due to NAT mode, ignoring incoming local broadcasts.");
            return 0;
        }
        else
        {
            dbTraffic(DBD_ALL, DB_INFO, "BVLC: Received Original-Broadcast-NPDU.");
            /* Upon receipt of a BVLL Original-Broadcast-NPDU message,
               a BBMD shall construct a BVLL Forwarded-NPDU message and
               send it to each IP subnet in its BDT with the exception
               of its own. The B/IP address to which the Forwarded-NPDU
               message is sent is formed by inverting the broadcast
               distribution mask in the BDT entry and logically ORing it
               with the BBMD address of the same entry. This process
               produces either the directed broadcast address of the remote
               subnet or the unicast address of the BBMD on that subnet
               depending on the contents of the broadcast distribution
               mask. See J.4.3.2.. In addition, the received BACnet NPDU
               shall be sent directly to each foreign device currently in
               the BBMD's FDT also using the BVLL Forwarded-NPDU message. */
            bvlc_internet_to_bacnet_mac_address(src, &sin);
            if (npdu_len < max_npdu) {
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < npdu_len; i++) {
                    npdu[i] = npdu[4 + i];
                }
                /* if BDT or FDT entries exist, Forward the NPDU */
                bvlc_bdt_forward_npdu(portParams, &sin, &npdu[0], max_npdu, npdu_len, true);
                bvlc_fdt_forward_npdu(portParams, &sin, &npdu[0], max_npdu, npdu_len, true);
            }
            else {
                /* ignore packets that are too large */
                npdu_len = 0;
            }
        }
        break;

    default:
        break;
    }

    return npdu_len;
}


static void bvlc_ipAddr_port_to_internet_address(
struct sockaddr_in *sin,    /* source address in network order */
    uint32_t ipAddr,
    uint16_t port)
{
    /* returns the BACnet source address */

    memcpy(&sin->sin_addr.s_addr, &ipAddr, 4);
    memcpy(&sin->sin_port, &port, 2);
}


static int SendMPDU(const PORT_SUPPORT *portParams, const BACNET_BVLC_FUNCTION function, const uint32_t nHostAddr, const uint16_t nPort, const DLCB *dlcb )
{
    uint8_t mtu[MAX_LPDU_IP];
    struct sockaddr_in bvlc_dest = { 0 };
    uint16_t BVLC_length;
    uint16_t mtu_len;

    mtu[0] = BVLL_TYPE_BACNET_IP;
    mtu[1] = (uint8_t)function;

#if 0
    // no longer a gateway, now a router...
    if (function == BVLC_FORWARDED_NPDU) {
        // insert the original source address, in this gateway demo, it is us.
        BVLC_length = (uint16_t) pdu_len + 10 /*inclusive */;
        dbTraffic(DBD_NETWORK_LAYER, DB_NOTE, "Sending forwarded NPDU to : ");
        dbTraffic(DBD_All, DB_ERROR, "todo, with the change to bvlc_fdt_forward_npdu(), we don't use this anymore. Postponing removal decision todo 3");
    } else {
        BVLC_length = (uint16_t) pdu_len + 4 /*inclusive */;
        // dbTraffic(DBD_NETWORK_LAYER, DB_NOTE, "Sending other     NPDU to : ");
    }
#else
    BVLC_length = (uint16_t)dlcb->optr + 4; /*inclusive */
#endif

    bvlc_dest.sin_addr.s_addr = nHostAddr;
    bvlc_dest.sin_port = nPort;

    mtu_len = 2;
    mtu_len += (uint16_t)encode_unsigned16(&mtu[mtu_len], BVLC_length);

#if 0
    // no longer a gateway, now a router...
    if (function == BVLC_FORWARDED_NPDU) {
        // insert the original source address, in this gateway implementation, it is us.
        uint16_t myPort = portParams->datalink.bipParams.nwoPort ;
        struct in_addr myAddress;
        myAddress.s_addr = bip_get_addr(portParams);
        mtu_len += bvlc_encode_bip_address(&mtu[mtu_len], &myAddress, myPort );
    }
#endif


    memcpy(&mtu[mtu_len], dlcb->Handler_Transmit_Buffer, dlcb->optr );
    mtu_len += (uint16_t)dlcb->optr;

    return bvlc_send_mpdu(portParams, &bvlc_dest, mtu, mtu_len);
}


void fd_send_npdu(
    // const PORT_SUPPORT *portParams,
    // const DEVICE_OBJECT_DATA    *pDev,
    // const BACNET_MAC_ADDRESS *destMac,
    // const BACNET_NPCI_DATA * npci_data,
    const DLCB *dlcb)
{
    struct in_addr address;
    uint16_t port;
    // int rc = 1;

    /* bip datalink doesn't need to know the npdu data */
    // (void)npci_data;

    if (dlcb->route.bacnetPath.localMac.len == 0) {
        /* if we are a foreign device */
            // mtu[1] = BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK;
            // address.s_addr = Remote_BBMD.sin_addr.s_addr;
            // port = Remote_BBMD.sin_port;
            dbTraffic(DBD_ALL, DB_INFO, "FD: Sent Distribute-Broadcast-to-Network.");
            SendMPDU( dlcb->route.portParams, BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK,
                dlcb->route.portParams->datalink.bipParams.fd_ipep.sin_addr.s_addr,
                dlcb->route.portParams->datalink.bipParams.fd_ipep.sin_port, //  Remote_BBMD.sin_port, 
                dlcb );
    }
#if 0
    // I do not see this use-case
    else if ((destPath->glAdr.net > 0) && (destPath->glAdr.mac.len == 0)) {
        /* net > 0 and net < 65535 are network specific broadcast if len = 0 */
        if (destPath->localMac.len == 6) {
            /* remote broadcast */
            bvlc_decode_bip_address(&destPath->localMac.bytes[0], &address, &port);
            dbTraffic(DB_INFO, "m0034 - FD: Sending Original-Broadcast-NPDU.");
            rc = SendMPDU(portParams, BVLC_ORIGINAL_BROADCAST_NPDU, address.s_addr, port, pdu, pdu_len);
        }
        else {
            // address.s_addr = bip_get_broadcast_ipAddr();
            // port = bip_get_port();
            dbTraffic(DB_INFO, "m0035 - FD: Sending Original-Broadcast-NPDU.");
            rc = SendMPDU(portParams, BVLC_ORIGINAL_BROADCAST_NPDU, bip_get_broadcast_ipAddr(portParams), portParams->datalink.bipParams.nwoPort,
             pdu, pdu_len);
        }
        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
#endif
    else if (dlcb->route.bacnetPath.localMac.len == 6) {
        /* valid unicast */
        bvlc_decode_bip_address(&dlcb->route.bacnetPath.localMac.bytes[0], &address, &port);
        // mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        dbTraffic( DBD_ALL, DB_INFO, "m0036 - FD: Sending Original-Unicast-NPDU.");
        SendMPDU(dlcb->route.portParams, BVLC_ORIGINAL_UNICAST_NPDU, address.s_addr, port, dlcb );
    }
    else {
        /* invalid address */
        panic();
        // rc = -1;
    }
//    return rc;
}



/** Send a BBMD packet out the BACnet/IP socket (Annex J)
 *
 * @param dest - destination address
 * @param npci_data - network information
 * @param pdu - any data to be sent - may be null
 * @param pdu_len - number of bytes of data
 *
 * @return returns number of bytes sent on success, negative number on failure
 */
void bbmd_send_npdu(
    // const PORT_SUPPORT *portParams,
    // const DEVICE_OBJECT_DATA    *pDev,
    // const BACNET_MAC_ADDRESS *destMac,
    // const BACNET_NPCI_DATA * npci_data,

        DLCB *dlcb )
{
    /* addr and port in network format */
    struct in_addr address;
    uint16_t port;
    //int rc = 1;

    /* bip datalink doesn't need to know the npdu data */
    // todo3 - this information is only required for MSTP (for DER), which is unfortunate because this object pollutes all other calls. Think about removing this..
    // (void)npci_data;

    /* handle various broadcasts: */
    /* localMac.len = 0 is a broadcast address */
    /* net = 0 indicates local, net = 65535 indicates global */
    if (dlcb->route.bacnetPath.localMac.len == 0) {
        /* if we are a foreign device */
        //if (Remote_BBMD.sin_port) {
        //    // mtu[1] = BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK;
        //    // address.s_addr = Remote_BBMD.sin_addr.s_addr;
        //    // port = Remote_BBMD.sin_port;
        //    dbTraffic(DBD_All, DB_INFO, "BVLC: Sent Distribute-Broadcast-to-Network.");
        //    rc = SendMPDU(portParams, BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK, Remote_BBMD.sin_addr.s_addr, Remote_BBMD.sin_port, pdu, pdu_len);
        //}
        //else {
            // address.s_addr = bip_get_broadcast_ipAddr();
            // port = bip_get_port();
            // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
            if (dlcb->route.portParams->datalink.bipParams.BVLC_NAT_Handling)
            {
                dbTraffic(DBD_ALL, DB_INFO, "BVLC: Due to NAT mode, not sending local broadcasts.");
            }
            else
            {
                dbTraffic(DBD_ALL, DB_NORMAL_TRAFFIC, "BVLC: Sent Original-Broadcast-NPDU.");
                SendMPDU(dlcb->route.portParams, BVLC_ORIGINAL_BROADCAST_NPDU, bip_get_broadcast_ipAddr(dlcb->route.portParams),
                    dlcb->route.portParams->datalink.bipParams.nwoPort,
                dlcb );
            }

            struct sockaddr_in myAddr;
            bvlc_ipAddr_port_to_internet_address(&myAddr, bip_get_addr( dlcb->route.portParams), 
                dlcb->route.portParams->datalink.bipParams.nwoPort);

            // now, we may also have a populated FD table - we need to send broadcasts as forwarded messages to these too
            bvlc_fdt_forward_npdu(dlcb->route.portParams, &myAddr, dlcb->Handler_Transmit_Buffer, dlcb->route.portParams->max_lpdu, dlcb->optr, true);

            // todo2 - what about BBMD table here??
//        }
    }
#if 0
    // I do not see this use-case
    else if ((destPath->glAdr.net > 0) && (destPath->glAdr.mac.len == 0)) {
        /* net > 0 and net < 65535 are network specific broadcast if len = 0 */
        if (destPath->localMac.len == 6) {
            /* remote broadcast */
            bvlc_decode_bip_address(&destPath->localMac.bytes[0], &address, &port);
            dbTraffic(DBD_All, DB_INFO, "m0034 - BVLC: Sending Original-Broadcast-NPDU.");
            rc = SendMPDU(portParams, BVLC_ORIGINAL_BROADCAST_NPDU, address.s_addr, port, pdu, pdu_len);
        } else {
            // address.s_addr = bip_get_broadcast_ipAddr();
            // port = bip_get_port();
            dbTraffic(DBD_All, DB_INFO, "m0035 - BVLC: Sending Original-Broadcast-NPDU.");
            rc = SendMPDU(portParams, BVLC_ORIGINAL_BROADCAST_NPDU, bip_get_broadcast_ipAddr(portParams), portParams->datalink.bipParams.nwoPort, pdu, pdu_len);
        }
        // mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    }
#endif
    else if (dlcb->route.bacnetPath.localMac.len == 6) {
        /* valid unicast */
        bvlc_decode_bip_address(&dlcb->route.bacnetPath.localMac.bytes[0], &address, &port);
        // mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        dbTraffic(DBD_ALL, DB_INFO, "m0036 - BVLC: Sending Original-Unicast-NPDU.");
        SendMPDU( dlcb->route.portParams, BVLC_ORIGINAL_UNICAST_NPDU, address.s_addr, port, dlcb );
    }
    else {
        /* invalid address */
        panic();
        //rc = -1;
    }

    dlcb_free(dlcb);
}




/***********************************************
 * Functions to register us as a foreign device.
 ********************************************* */

/** Encode foreign device registration message
 *
 * @param pdu - bytes for encoding the message
 *                       in network byte order.
 * @param time_to_live_seconds - Lease time to use when registering.
 * @return Number of bytes encoded) on success,
 *         or 0 if no encoding occurred.
 */
static int bvlc_encode_register_foreign_device(
    uint8_t * pdu,
    uint16_t time_to_live_seconds)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_REGISTER_FOREIGN_DEVICE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 6);
        encode_unsigned16(&pdu[4], time_to_live_seconds);
        len = 6;
    }

    return len;
}


/** Register as a foreign device with the indicated BBMD.
 * @param bbmd_address - IPv4 address (long) of BBMD to register with,
 *                       in network byte order.
 * @param bbmd_port - Network port of BBMD, in network byte order
 * @param time_to_live_seconds - Lease time to use when registering.
 * @return Positive number (of bytes sent) on success,
 *         0 if no registration request is sent, or
 *         -1 if registration fails.
 */
int bvlc_register_with_bbmd(
    PORT_SUPPORT *portParams,
    //uint32_t bbmd_address,
    //uint16_t bbmd_port,
    uint16_t time_to_live_seconds)
{
    uint8_t mtu[MAX_LPDU_IP] = { 0 };
    uint16_t mtu_len = 0;
    int retval = 0;

    /* Store the BBMD address and port so that we
       won't broadcast locally. */
    // Remote_BBMD.sin_addr.s_addr = bbmd_address;
    // Remote_BBMD.sin_port = bbmd_port;

    /* In order for their broadcasts to get here,
       we need to register our address with the remote BBMD using
       Write Broadcast Distribution Table, or
       register with the BBMD as a Foreign Device */
    mtu_len =
        (uint16_t)bvlc_encode_register_foreign_device(&mtu[0],
        time_to_live_seconds);

    retval = bvlc_send_mpdu(portParams, &portParams->datalink.bipParams.fd_ipep, /*&Remote_BBMD,*/  &mtu[0], mtu_len);
    return retval;
}


/** Note any BVLC_RESULT code, or NAK the BVLL message in the unsupported cases.
 * Use this handler when you are not a BBMD.
 * Sets the BVLC_Function_Code in case it is needed later.
 *
 * @param sout  [in] Socket address to send any NAK back to.
 * @param npdu  [in] The received buffer.
 * @param received_bytes [in] How many bytes in npdu[].
 * @return Non-zero BVLC_RESULT_ code if we sent a response (NAK) to this
 *      BVLC message.  If zero, may need further processing.
 */

int bvlc_for_non_bbmd(
    const PORT_SUPPORT *portParams,
    struct sockaddr_in *sout,
    uint8_t * npdu,
    uint16_t received_bytes)
{
    BACNET_BVLC_RESULT result_code = BVLC_RESULT_SUCCESSFUL_COMPLETION;

    BVLC_Function_Code = (BACNET_BVLC_FUNCTION)npdu[1];       /* The BVLC function */
    switch (BVLC_Function_Code) {
    case BVLC_RESULT:
        if (received_bytes >= 6) {
            uint16_t tresult_code;
            /* This is the result of our foreign device registration */
                (void) decode_unsigned16(&npdu[4], &tresult_code);
                BVLC_Result_Code = (BACNET_BVLC_RESULT) tresult_code;
            dbTraffic(DBD_ALL, DB_DEBUG, "BVLC: Result Code=%d", BVLC_Result_Code);
            /* But don't send any response */
            result_code = BVLC_RESULT_SUCCESSFUL_COMPLETION;
        }
        break;
    case BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE:
        result_code = BVLC_RESULT_WRITE_BROADCAST_DISTRIBUTION_TABLE_NAK;
        break;
    case BVLC_READ_BROADCAST_DIST_TABLE:
        result_code = BVLC_RESULT_READ_BROADCAST_DISTRIBUTION_TABLE_NAK;
        break;
        /* case BVLC_READ_BROADCAST_DIST_TABLE_ACK: */
    case BVLC_REGISTER_FOREIGN_DEVICE:
        result_code = BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK;
        break;
    case BVLC_READ_FOREIGN_DEVICE_TABLE:
        result_code = BVLC_RESULT_READ_FOREIGN_DEVICE_TABLE_NAK;
        break;
        /* case BVLC_READ_FOREIGN_DEVICE_TABLE_ACK: */
    case BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY:
        result_code = BVLC_RESULT_DELETE_FOREIGN_DEVICE_TABLE_ENTRY_NAK;
        break;
    case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
        result_code = BVLC_RESULT_DISTRIBUTE_BROADCAST_TO_NETWORK_NAK;
        break;
        /* case BVLC_FORWARDED_NPDU: */
        /* case BVLC_ORIGINAL_UNICAST_NPDU: */
        /* case BVLC_ORIGINAL_BROADCAST_NPDU: */
    default:
        break;
    }
    if (result_code > 0) {
        bvlc_send_result(portParams, sout, result_code);
        dbTraffic(DBD_ALL, DB_ERROR, "BVLC: NAK code=%d", result_code);
    }

    return result_code;
}

/** Returns the last BVLL Result we received, either as the result of a BBMD
 * request we sent, or (if not a BBMD or Client), from trying to register
 * as a foreign device.
 *
 * @return BVLC_RESULT_SUCCESSFUL_COMPLETION on success,
 * BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK if registration failed,
 * or one of the other codes (if we are a BBMD).
 */
BACNET_BVLC_RESULT bvlc_get_last_result(
    void)
{
    return BVLC_Result_Code;
}

/** Returns the current BVLL Function Code we are processing.
 * We have to store this higher layer code for when the lower layers
 * need to know what it is, especially to differentiate between
 * BVLC_ORIGINAL_UNICAST_NPDU and BVLC_ORIGINAL_BROADCAST_NPDU.
 *
 * @return A BVLC_ code, such as BVLC_ORIGINAL_UNICAST_NPDU.
 */
BACNET_BVLC_FUNCTION bvlc_get_function_code(
    void)
{
    return BVLC_Function_Code;
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Get handle to broadcast distribution table (BDT).
 *
 *  Do not modify the table using the returned pointer,
 *  use the dedicated functions instead.
 *  (For optimization the table is not copied to caller)
 *
 * @param table [out] - broadcast distribution table
 *
 * @return Number of valid entries in the table or -1 on error.
 */
//int bvlc_get_bdt_local(
//    const BBMD_TABLE_ENTRY** table)
//{
//    int count = 0;
//
//    if(table == NULL)
//        return -1;
//
//    *table = BBMD_Table;
//
//    for (count = 0; count < MAX_BBMD_ENTRIES; ++count) {
//        if (!BBMD_Table[count].valid) {
//            break;
//        }
//    }
//
//    return count;
//}

/** Invalidate all entries in the broadcast distribution table (BDT).
 */
void bbmd_clear_bdt_local(
    PORT_SUPPORT *portParams)
{
    int i = 0;
    for (i = 0; i < MAX_BBMD_ENTRIES; ++i) {
        portParams->BBMD_Table[i].valid = false;
        portParams->BBMD_Table[i].dest_address.s_addr = 0;
        portParams->BBMD_Table[i].dest_port = 0;
        portParams->BBMD_Table[i].broadcast_mask.s_addr = 0;
    }

    // todo 1 - Transfer to reference

    // BDT needs at least one entry - us!
#if 0 // todo2 - i want to think this through a bit..
    uint32_t addr = bip_get_addr(portParams);
    uint16_t port = portParams->datalink.bipParams.nwoPort;
    uint32_t mask = 0xffffffff;

    if (portParams->datalink.bipParams.BVLC_NAT_Handling == true)
    {
        addr = portParams->datalink.bipParams.BVLC_Global_Address.sin_addr.s_addr;
        port = portParams->datalink.bipParams.BVLC_Global_Address.sin_port;
    }

    memcpy(&portParams->BBMD_Table[0].dest_address.s_addr, &addr, 4);
    memcpy(&portParams->BBMD_Table[0].dest_port, &port, 2);
    memcpy(&portParams->BBMD_Table[0].broadcast_mask.s_addr, &mask, 4);
    portParams->BBMD_Table[0].valid = true;
#endif
}


void bbmd_clear_fdt_local(PORT_SUPPORT *portParams)
{
    for (int i = 0; i < MAX_FD_ENTRIES; ++i) {
        portParams->FD_Table[i].valid = false;
    }
}


/** Add new entry to broadcast distribution table.
 *
 * @return True if the new entry was added successfully.
 */
bool bvlc_add_bdt_entry_local(
    PORT_SUPPORT *portParams,
    BBMD_TABLE_ENTRY* entry)
{
    bool found = false;
    int i = 0;

    if (entry == NULL)
        return false;

    /* Find first empty slot */
    for (i = 0; i < MAX_BBMD_ENTRIES; ++i) {
        if (!portParams->BBMD_Table[i].valid) {
            found = true;
            break;
        }

        /* Make sure that we are not adding a duplicate */
        if (portParams->BBMD_Table[i].dest_address.s_addr == entry->dest_address.s_addr &&
            portParams->BBMD_Table[i].broadcast_mask.s_addr == entry->broadcast_mask.s_addr &&
            portParams->BBMD_Table[i].dest_port == entry->dest_port) {
            return false;
        }
    }
    /* BDT changed! Save backup to file */
    bvlc_bdt_backup_local();
    // debug_printf("BVLC: BBMD Table entries cleared.\n");
    if (!found)
        return false;
    bvlc_bdt_backup_local();
    /* Copy new entry to the empty slot */
    portParams->BBMD_Table[i] = *entry;
    portParams->BBMD_Table[i].valid = true;

    return true;
}


/** Enable NAT handling and set the global IP address
 * @param [in] - Global IP address visible to peer BBMDs and foreign devices
 */
//void bvlc_set_global_address_for_nat(const struct sockaddr_in* addr)
//{
//    BVLC_Global_Address = *addr;
//    BVLC_NAT_Handling = true;
//}
//
///** Disable NAT handling.
// */
//void bvlc_disable_nat(void)
//{
//    BVLC_NAT_Handling = false;
//    BVLC_Global_Address.sin_addr.s_addr  = 0;
//    BVLC_Global_Address.sin_port = 0;
//}

#endif // BBMC_ENABLED 

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Add new entry to broadcast distribution table.
 *
 * @return True if the new entry was added successfully.
 */
#if 0 // never used
bool bvlc_add_bdt_entry_local(
    BBMD_TABLE_ENTRY* entry)
{
    bool found = false;
    int i = 0;

    if(entry == NULL) {
        return false;
    }

    /* Find first empty slot */
    for (i = 0; i < MAX_BBMD_ENTRIES; ++i) {
        if (!BBMD_Table[i].valid) {
            found = true;
            break;
        }

        /* Make sure that we are not adding a duplicate */
        if(BBMD_Table[i].dest_address.s_addr == entry->dest_address.s_addr &&
            BBMD_Table[i].broadcast_mask.s_addr == entry->broadcast_mask.s_addr &&
            BBMD_Table[i].dest_port == entry->dest_port) {
            return false;
        }
    }

    if(!found) {
        return false;
    }

    /* Copy new entry to the empty slot */
    BBMD_Table[i] = *entry;
    BBMD_Table[i].valid = true;
    debug_printf("BVLC: BBMD Table entry added.\n");

    /* BDT changed! Save backup to file */
    bvlc_bdt_backup_local();

    return true;
}

#endif

// todo2 - this belongs at a higher layer. extern PORT_SUPPORT routerAppPortParams;

void bvlc_cleanup(void)
{
    //if (routerAppPortParams.datalink.bipParams.socket >= 0) {
    //    closesocket(routerAppPortParams.datalink.bipParams.socket);
    debug_printf("BVLC: NAT Address disabled.\n");
    //routerAppPortParams.datalink.bipParams.socket = -1;
#if defined ( _MSC_VER  )
    WSACleanup();
#endif
}


#endif // BBMC_ENABLED 

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* copy the source internet address to the BACnet address */
/* FIXME: IPv6? */
static void bvlc_bacnet_to_internet_address(
struct sockaddr_in *sin,    /* source address in network order */
    BACNET_GLOBAL_ADDRESS * src) {
    /* returns the BACnet source address */

    if (src && sin) {
        if (src->localMac.len == 6) {
            memcpy(&sin->sin_addr.s_addr, &src->mac[0], 4);
            memcpy(&sin->sin_port, &src->mac[4], 2);
        }
    }

}

void testBIPAddress(
    Test * pTest)
{
    uint8_t apdu[50] = { 0 };
    int len = 0, test_len = 0;
    struct in_addr address;
    struct in_addr test_address;
    uint16_t port = 0, test_port = 0;

    address.s_addr = 42;
    len = bvlc_encode_bip_address(&apdu[0], &address, port);
    test_len = bvlc_decode_bip_address(&apdu[0], &test_address, &test_port);
    ct_test(pTest, len == test_len);
    ct_test(pTest, address.s_addr == test_address.s_addr);
    ct_test(pTest, port == test_port);
}

void testInternetAddress(
    Test * pTest)
{
    BACNET_GLOBAL_ADDRESS src;
    BACNET_GLOBAL_ADDRESS test_src;
    struct sockaddr_in sin = { 0 };
    struct sockaddr_in test_sin = { 0 };

    sin.sin_port = htons(0xBAC0);
    sin.sin_addr.s_addr = inet_addr("192.168.0.1");
    bvlc_internet_to_bacnet_address(&src, &sin);
    bvlc_bacnet_to_internet_address(&test_sin, &src);
    ct_test(pTest, sin.sin_port == test_sin.sin_port);
    ct_test(pTest, sin.sin_addr.s_addr == test_sin.sin_addr.s_addr);
}

#ifdef TEST_BVLC
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Virtual Link Control", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBIPAddress);
    assert(rc);
    rc = ct_addTestFunction(pTest, testInternetAddress);
    assert(rc);
    /* configure output */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_BBMD */
#endif /* TEST */
