/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

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

#include "bacdcode.h"
#include "bacnet/npdu.h"
#include "eLib/util/eLibDebug.h"
#include "bacaddr.h"
#include "bacnet/bits.h"

/** @file npdu.c  Encode/Decode NPDUs - Network Protocol Data Units */

/** Copy the npci_data structure information from src to dest.
 * @param dest [out] The 'to' structure
 * @param src   [in] The 'from' structure
 */
void npdu_copy_data(
    BACNET_NPCI_DATA * dest,
    BACNET_NPCI_DATA * src)
{
    if (dest && src) {
        dest->protocol_version = src->protocol_version;
        dest->data_expecting_reply = src->data_expecting_reply;
        dest->network_layer_message = src->network_layer_message;
        dest->priority = src->priority;
//        dest->network_message_type = src->network_message_type;
        dest->vendor_id = src->vendor_id;
        dest->hop_count = src->hop_count;
    }
}

/*

The following ICI parameters are exchanged with the
various service primitives across an API:

'destination_address' (DA): the address of the device(s)
intended to receive the service primitive. Its format (device name,
network address, etc.) is a local matter. This address
may also be a multicast, local broadcast or global broadcast type.

'source_address' (SA): the address of the device from which
the service primitive was received. Its format (device name,
network address, etc.) is a local matter.

'network_priority' (NP): a four-level network priority parameter
described in 6.2.2.

'data_expecting_reply' (DER): a Boolean parameter that indicates
whether (TRUE) or not (FALSE) a reply service primitive
is expected for the service being issued.


Table 5-1. Applicability of ICI parameters for abstract service primitives
     Service Primitive         DA           SA         NP        DER
CONF_SERV.request              Yes          No         Yes       Yes
CONF_SERV.indication           Yes         Yes         Yes       Yes
CONF_SERV.response             Yes          No         Yes       Yes
CONF_SERV.confirm              Yes         Yes         Yes        No
UNCONF_SERV.request            Yes          No         Yes        No
UNCONF_SERV.indication         Yes         Yes         Yes        No
REJECT.request                 Yes          No         Yes        No
REJECT.indication              Yes         Yes         Yes        No
SEGMENT_ACK.request            Yes          No         Yes        No
SEGMENT_ACK.indication         Yes         Yes         Yes        No
ABORT.request                  Yes          No         Yes        No
ABORT.indication               Yes         Yes         Yes        No
 */


/** Encode the NPDU portion of a message to be sent, based on the npci_data
 *  and associated data.
 *  If this is to be a Network Layer Control Message, there are probably
 *  more bytes which will need to be encoded following the ones encoded here.
 *  The Network Layer Protocol Control Information byte is described
 *  in section 6.2.2 of the BACnet standard.
 * @param npdu [out] Buffer which will hold the encoded NPDU header bytes.
 * 					 The size isn't given, but it must be at least 2 bytes
 *                   for the simplest case, and should always be at least 24
 *                   bytes to accommodate the maximal case (all fields loaded).
 * @param dest [in] The routing destination information if the message must
 *                   be routed to reach its destination.
 *                   If dest->net and dest->len are 0, there is no
 *                   routing destination information.
 * @param src  [in] The routing source information if the message was routed
 *                   from another BACnet network.
 *                   If src->net and src->len are 0, there is no
 *                   routing source information.
 *                   This src describes the original source of the message when
 *                   it had to be routed to reach this BACnet Device.
 * @param npci_data [in] The structure which describes how the NCPI and other
 *                   NPDU bytes should be encoded.
 * @return On success, returns the number of bytes which were encoded into the
 * 		   NPDU section.
 *         If 0 or negative, there were problems with the data or encoding.
 */
int16_t npdu_encode_pdu(
    uint8_t * npdu,
    const BACNET_GLOBAL_ADDRESS * dest,
    const BACNET_GLOBAL_ADDRESS * src,
    const BACNET_NPCI_DATA * npci_data)
{
    int16_t len = 0;    /* return value - number of octets loaded in this function */
    uint8_t i = 0;      /* counter  */

    if (npdu && npci_data) {
        /* protocol version */
        npdu[0] = npci_data->protocol_version;
        /* initialize the control octet */
        npdu[1] = 0;
        /* Bit 7: 1 indicates that the NSDU conveys a network layer message. */
        /*          Message Type field is present. */
        /*        0 indicates that the NSDU contains a BACnet APDU. */
        /*          Message Type field is absent. */
        if (npci_data->network_layer_message)
            npdu[1] |= BIT(7);
        /*Bit 6: Reserved. Shall be zero. */
        /*Bit 5: Destination specifier where: */
        /* 0 = DNET, DLEN, DADR, and Hop Count absent */
        /* 1 = DNET, DLEN, and Hop Count present */
        /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
        /* DLEN > 0 specifies length of DADR field */
        if (dest && dest->net)
            npdu[1] |= BIT(5);
        /* Bit 4: Reserved. Shall be zero. */
        /* Bit 3: Source specifier where: */
        /* 0 =  SNET, SLEN, and SADR absent */
        /* 1 =  SNET, SLEN, and SADR present */
        /* SLEN = 0 Invalid */
        /* SLEN > 0 specifies length of SADR field */
        if (src && src->net && src->mac.len)
            npdu[1] |= BIT(3);
        /* Bit 2: The value of this bit corresponds to the */
        /* data_expecting_reply parameter in the N-UNITDATA primitives. */
        /* 1 indicates that a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        /* 0 indicates that other than a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        if (npci_data->data_expecting_reply)
            npdu[1] |= BIT(2);
        /* Bits 1,0: Network priority where: */
        /* B'11' = Life Safety message */
        /* B'10' = Critical Equipment message */
        /* B'01' = Urgent message */
        /* B'00' = Normal message */
        npdu[1] |= (npci_data->priority & 0x03);
        len = 2;
        if (dest && dest->net) {
            len += encode_unsigned16(&npdu[len], dest->net);
            npdu[len++] = dest->mac.len;
            /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
            /* DLEN > 0 specifies length of DADR field */
            if (dest->mac.len) {
                for (i = 0; i < dest->mac.len; i++) {
                    npdu[len++] = dest->mac.bytes[i];
                }
            }
        }

        if (src && src->net && src->mac.len) {      /* Only insert if valid */
            len += encode_unsigned16(&npdu[len], src->net);
            npdu[len++] = src->mac.len;
            /* SLEN = 0 denotes broadcast MAC SADR and SADR field is absent */
            /* SLEN > 0 specifies length of SADR field */
            if (src->mac.len) {
                for (i = 0; i < src->mac.len; i++) {
                    npdu[len++] = src->mac.bytes[i];
                }
            }
        }

        /* The Hop Count field shall be present only if the message is */
        /* destined for a remote network, i.e., if DNET is present. */
        /* This is a one-octet field that is initialized to a value of 0xff. */
        if (dest && dest->net) {
            npdu[len] = npci_data->hop_count;
            len++;
        }

        // 2016.12.29 moving elsewhere, the Network Message has to be preserved for e.g. router layer.
        //if (npci_data->network_layer_message) {
        //    npdu[len] = npci_data->network_message_type;
        //    len++;
        //    /* Message Type field contains a value in the range 0x80 - 0xFF, */
        //    /* then a Vendor ID field shall be present */
        //    if (npci_data->network_message_type >= 0x80)
        //        len += encode_unsigned16(&npdu[len], npci_data->vendor_id);
        //}

    }

    return len;
}

/* Configure the NPDU portion of the packet for an APDU */
/* This function does not handle the network messages, just APDUs. */
/* From BACnet 5.1:
Applicability of ICI parameters for abstract service primitives
Service Primitive      DA  SA  NP  DER
-----------------      --- --- --- ---
CONF_SERV.request      Yes No  Yes Yes
CONF_SERV.indication   Yes Yes Yes Yes
CONF_SERV.response     Yes No  Yes Yes
CONF_SERV.confirm      Yes Yes Yes No
UNCONF_SERV.request    Yes No  Yes No
UNCONF_SERV.indication Yes Yes Yes No
REJECT.request         Yes No  Yes No
REJECT.indication      Yes Yes Yes No
SEGMENT_ACK.request    Yes No  Yes No
SEGMENT_ACK.indication Yes Yes Yes No
ABORT.request          Yes No  Yes No
ABORT.indication       Yes Yes Yes No

Where:
'destination_address' (DA): the address of the device(s) intended
to receive the service primitive. Its format (device name,
network address, etc.) is a local matter. This address may
also be a multicast, local broadcast or global broadcast type.
'source_address' (SA): the address of the device from which
the service primitive was received. Its format (device name,
network address, etc.) is a local matter.
'network_priority' (NP): a four-level network priority parameter
described in 6.2.2.
'data_expecting_reply' (DER): a Boolean parameter that indicates
whether (TRUE) or not (FALSE) a reply service primitive
is expected for the service being issued.
 */

/** Initialize an npci_data structure to good defaults.
 * The name is a misnomer, as it doesn't do any actual encoding here.
 *
 *    EKH: The "npdu_encode_npdu_data" name was a misnomer, and
 *    confusing, so I changed the function name to
 *    npdu_setup_npci_data
 *
 * @see npdu_encode_npdu_network if you need to set a network layer msg.
 *
 * @param npci_data [out] Returns a filled-out structure with information
 * 					 provided by the other arguments and good defaults.
 * @param data_expecting_reply [in] True if message should have a reply.
 * @param priority [in] One of the 4 priorities defined in section 6.2.2,
 *                      like B'11' = Life Safety message
 */
void init_common_npci (
	BACNET_NPCI_DATA * npci_data,
	bool data_expecting_reply )
{
	npci_data->protocol_version = BACNET_PROTOCOL_VERSION;
	npci_data->vendor_id = 0;
	npci_data->priority = MESSAGE_PRIORITY_NORMAL;
	npci_data->hop_count = HOP_COUNT_DEFAULT;
	npci_data->data_expecting_reply = data_expecting_reply;
}

 // EKH: this function does not 'encode' data into a buffer at all, so I renamed npdu_encode_npdu_data() to npdu_setup_npci_data()
void npdu_setup_npci_data(
    BACNET_NPCI_DATA * npci_data,
    bool data_expecting_reply,
    BACNET_MESSAGE_PRIORITY priority)
{
    init_common_npci ( npci_data, data_expecting_reply ) ;
    npci_data->network_layer_message = false;       /* false if APDU */
//    npci_data->network_message_type = NETWORK_MESSAGE_INVALID;      /* optional */
    npci_data->priority = priority;
}

/** Decode the NPDU portion of a received message, particularly the NCPI byte.
 *  The Network Layer Protocol Control Information byte is described
 *  in section 6.2.2 of the BACnet standard.
 * @param npdu [in] Buffer holding the received NPDU header bytes (must be at least 2)
 * @param dest [out] Returned with routing destination information if the NPDU
 *                   has any and if this points to non-null storage for it.
 *                   If dest->net and dest->len are 0 on return, there is no
 *                   routing destination information.
 * @param src  [out] Returned with routing source information if the NPDU
 *                   has any and if this points to non-null storage for it.
 *                   If src->net and src->len are 0 on return, there is no
 *                   routing source information.
 *                   This src describes the original source of the message when
 *                   it had to be routed to reach this BACnet Device.
 * @param npci_data [out] Returns a filled-out structure with information
 * 					 decoded from the NCPI and other NPDU bytes.
 * @return On success, returns the number of bytes which were decoded from the
 * 		   NPDU section; if this is a  network layer message, there may be more
 *         bytes left in the NPDU; if not a network msg, the APDU follows.
 *         If 0 or negative, there were problems with the data or arguments.
 */
int npci_decode(
    const uint8_t * npdu,
    BACNET_GLOBAL_ADDRESS * dest,
    BACNET_GLOBAL_ADDRESS * src,
    BACNET_NPCI_DATA * npci_data)
{
    int len = 0;        /* return value - number of octets loaded in this function */
    uint8_t i = 0;      /* counter */
    uint16_t src_net = 0;
    uint16_t dest_net = 0;
    uint8_t address_len = 0;
    uint8_t mac_octet = 0;

    npci_data->vendor_id = 0 ;

    // Note that this is not a debug-only requirement
    // wait! Not so fast, see cr123476129461924
    if ( src ) bacnet_address_clear ( src ) ;

    if (npdu ) {
        /* Protocol Version */
        npci_data->protocol_version = npdu[0];
        /* control octet */
        /* Bit 7: 1 indicates that the NSDU conveys a network layer message. */
        /*          Message Type field is present. */
        /*        0 indicates that the NSDU contains a BACnet APDU. */
        /*          Message Type field is absent. */
        npci_data->network_layer_message = (npdu[1] & BIT(7)) ? true : false;
        /*Bit 6: Reserved. Shall be zero. */
        /* Bit 4: Reserved. Shall be zero. */
        /* Bit 2: The value of this bit corresponds to data expecting reply */
        /* parameter in the N-UNITDATA primitives. */
        /* 1 indicates that a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        /* 0 indicates that other than a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        npci_data->data_expecting_reply = (npdu[1] & BIT(2)) ? true : false;
        /* Bits 1,0: Network priority where: */
        /* B'11' = Life Safety message */
        /* B'10' = Critical Equipment message */
        /* B'01' = Urgent message */
        /* B'00' = Normal message */
        npci_data->priority = (BACNET_MESSAGE_PRIORITY) (npdu[1] & 0x03);
        /* set the offset to where the optional stuff starts */
        len = 2;
        /*Bit 5: Destination specifier where: */
        /* 0 = DNET, DLEN, DADR, and Hop Count absent */
        /* 1 = DNET, DLEN, and Hop Count present */
        /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
        /* DLEN > 0 specifies length of DADR field */
        if (dest)
        {
            /* zero out the destination address */
            bacnet_address_clear ( dest ) ;
        }
        if (npdu[1] & BIT(5)) {
            len += decode_unsigned16(&npdu[len], &dest_net);
            /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
            /* DLEN > 0 specifies length of DADR field */
            address_len = npdu[len++];
            if (dest) {
                dest->net = dest_net;
                dest->mac.len = address_len;
            }
            if (address_len) {
                if (address_len > MAX_MAC_LEN) {
                    /* address is too large could be a malformed message */
                    return -1;
                }

                for (i = 0; i < address_len; i++) {
                    mac_octet = npdu[len++];
                    if (dest) {
                        dest->mac.bytes[i] = mac_octet;
                    }
                }
            }
        }
        /* Bit 3: Source specifier where: */
        /* 0 =  SNET, SLEN, and SADR absent */
        /* 1 =  SNET, SLEN, and SADR present */
        /* SLEN > 0 specifies length of SADR field */
        if (npdu[1] & BIT(3)) {
            len += decode_unsigned16(&npdu[len], &src_net);
            /* SLEN = 0 denotes broadcast MAC SADR and SADR field is absent */
            /* SLEN > 0 specifies length of SADR field */
            address_len = npdu[len++];
            if (src) {
                src->net = src_net;
                src->mac.len = address_len;
            }
            if (address_len) {
                if (address_len > MAX_MAC_LEN) {
                    /* address is too large could be a malformed message */
                	panic();
                    return -1;
                }

                for (i = 0; i < address_len; i++) {
                    mac_octet = npdu[len++];
                    if (src)
                        src->mac.bytes[i] = mac_octet;
                }
            }
        } else if (src) {
            /* Clear the net number, with one exception: if the receive()
             * function set it to BACNET_BROADCAST_NETWORK, (eg, for
             * BVLC_ORIGINAL_BROADCAST_NPDU) then don't stomp on that.
             */
            // 2017.12.08 EKH: I have convinced myself that this is bogus. cr123476129461924
            // This seemed to cause problems with A*I customer, ... messages seemingly _coming_ from a broadcast because clearing ->net was
            // sidestepped here. Do more research.
            if (src->net != BACNET_BROADCAST_NETWORK) {
                src->net = 0;
            }
        }
        /* The Hop Count field shall be present only if the message is */
        /* destined for a remote network, i.e., if DNET is present. */
        /* This is a one-octet field that is initialized to a value of 0xff. */
        if (dest_net) {
            npci_data->hop_count = npdu[len++];
        } else {
            npci_data->hop_count = 0;
        }


        // 2016.12.29 Moving elsewhere
        ///* Indicates that the NSDU conveys a network layer message. */
        ///* Message Type field is present. */
        //if (npci_data->network_layer_message) {
        //    npci_data->network_message_type =
        //        (BACNET_NETWORK_MESSAGE_TYPE) npdu[len++];
        //    /* Message Type field contains a value in the range 0x80 - 0xFF, */
        //    /* then a Vendor ID field shall be present */
        //    if (npci_data->network_message_type >= 0x80) {
        //        len += decode_unsigned16(&npdu[len], &npci_data->vendor_id);
        //    }
        //} else {
        //    /* Since npci_data->network_layer_message is false,
        //     * it doesn't much matter what we set here; this is safe: */
        //    npci_data->network_message_type = NETWORK_MESSAGE_INVALID;
        //}

    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testNPDU2(
    Test * pTest)
{
    uint8_t pdu[480] = { 0 };
    BACNET_GLOBAL_ADDRESS dest = { 0 };
    BACNET_GLOBAL_ADDRESS src = { 0 };
    BACNET_GLOBAL_ADDRESS npdu_dest = { 0 };
    BACNET_GLOBAL_ADDRESS npdu_src = { 0 };
    int len = 0;
    bool data_expecting_reply = true;
    BACNET_MESSAGE_PRIORITY priority = MESSAGE_PRIORITY_NORMAL;
    BACNET_NPCI_DATA npci_data = { 0 };
    int i = 0;  /* counter */
    int npdu_len = 0;
    bool network_layer_message = false; /* false if APDU */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type = 0;       /* optional */
    uint16_t vendor_id = 0;     /* optional, if net message type is > 0x80 */

    dest.localMac.len = 6;
    for (i = 0; i < dest.localMac.len; i++) {
        dest.localMac.bytes[i] = i;
    }
    /* DNET,DLEN,DADR */
    dest.net = 1;
    dest.len = 6;
    for (i = 0; i < dest.len; i++) {
        dest.adr[i] = i * 10;
    }
    src.localMac.len = 1;
    for (i = 0; i < src.localMac.len; i++) {
        src.mac[i] = 0x80;
    }
    /* SNET,SLEN,SADR */
    src.net = 2;
    src.len = 1;
    for (i = 0; i < src.len; i++) {
        src.adr[i] = 0x40;
    }
    npdu_setup_npci_data(&npci_data, true, priority);
    len = npdu_encode_pdu(&pdu[0], &dest, &src, &npci_data);
    ct_test(pTest, len != 0);
    /* can we get the info back? */
    npdu_len = npci_decode(&pdu[0], &npdu_dest, &npdu_src, &npci_data);
    ct_test(pTest, npdu_len != 0);
    ct_test(pTest, npci_data.data_expecting_reply == data_expecting_reply);
    ct_test(pTest, npci_data.network_layer_message == network_layer_message);
    if (npci_data.network_layer_message) {
        ct_test(pTest, npci_data.network_message_type == network_message_type);
    }
    ct_test(pTest, npci_data.vendor_id == vendor_id);
    ct_test(pTest, npci_data.priority == priority);
    /* DNET,DLEN,DADR */
    ct_test(pTest, npdu_dest.net == dest.net);
    ct_test(pTest, npdu_dest.len == dest.len);
    for (i = 0; i < dest.len; i++) {
        ct_test(pTest, npdu_dest.adr[i] == dest.adr[i]);
    }
    /* SNET,SLEN,SADR */
    ct_test(pTest, npdu_src.net == src.net);
    ct_test(pTest, npdu_src.len == src.len);
    for (i = 0; i < src.len; i++) {
        ct_test(pTest, npdu_src.adr[i] == src.adr[i]);
    }
}

void testNPDU1(
    Test * pTest)
{
    uint8_t pdu[480] = { 0 };
    BACNET_GLOBAL_ADDRESS dest = { 0 };
    BACNET_GLOBAL_ADDRESS src = { 0 };
    BACNET_GLOBAL_ADDRESS npdu_dest = { 0 };
    BACNET_GLOBAL_ADDRESS npdu_src = { 0 };
    int len = 0;
    bool data_expecting_reply = false;
    BACNET_MESSAGE_PRIORITY priority = MESSAGE_PRIORITY_NORMAL;
    BACNET_NPCI_DATA npci_data = { 0 };
    int i = 0;  /* counter */
    int npdu_len = 0;
    bool network_layer_message = false; /* false if APDU */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type = 0;       /* optional */
    uint16_t vendor_id = 0;     /* optional, if net message type is > 0x80 */

    /* localMac.len = 0 if global address */
    dest.localMac.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest.localMac.bytes[i] = 0;
    }
    /* DNET,DLEN,DADR */
    dest.net = 0;
    dest.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest.adr[i] = 0;
    }
    src.localMac.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src.mac[i] = 0;
    }
    /* SNET,SLEN,SADR */
    src.net = 0;
    src.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src.adr[i] = 0;
    }
    npdu_setup_npci_data(&npci_data, false, priority);
    len = npdu_encode_pdu(&pdu[0], &dest, &src, &npci_data);
    ct_test(pTest, len != 0);
    /* can we get the info back? */
    npdu_len = npci_decode(&pdu[0], &npdu_dest, &npdu_src, &npci_data);
    ct_test(pTest, npdu_len != 0);
    ct_test(pTest, npci_data.data_expecting_reply == data_expecting_reply);
    ct_test(pTest, npci_data.network_layer_message == network_layer_message);
    if (npci_data.network_layer_message) {
        ct_test(pTest, npci_data.network_message_type == network_message_type);
    }
    ct_test(pTest, npci_data.vendor_id == vendor_id);
    ct_test(pTest, npci_data.priority == priority);
    ct_test(pTest, npdu_dest.localMac.len == src.localMac.len);
    ct_test(pTest, npdu_src.localMac.len == dest.localMac.len);
}

#ifdef TEST_NPDU
/* dummy stub for testing */
void tsm_free_invoke_id(
    uint8_t invokeID)
{
    (void) invokeID;
}

void iam_handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_GLOBAL_ADDRESS * src)
{
    (void) service_request;
    (void) service_len;
    (void) src;
}

int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet NPDU", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testNPDU1);
    assert(rc);
    rc = ct_addTestFunction(pTest, testNPDU2);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_NPDU */
#endif /* TEST */
