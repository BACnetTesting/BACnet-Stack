/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 Peter Mc Shane

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

#include <stdint.h>
#include "bacnet/bacenum.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacdef.h"
#include "bacnet/readrange.h"
#include "bacnet/bits.h"
#include "bacnet/datalink/bip.h"

/** @file readrange.c  Encode/Decode ReadRange requests */

/*
 * ReadRange-Request ::= SEQUENCE {
 *     objectIdentifier   [0] BACnetObjectIdentifier,
 *     propertyIdentifier [1] BACnetPropertyIdentifier,
 *     propertyArrayIndex [2] Unsigned OPTIONAL, -- used only with array datatype
 *     range CHOICE {
 *         byPosition [3] SEQUENCE {
 *             referenceIndex Unsigned,
 *             count          INTEGER
 *         },
 *     -- context tag 4 is deprecated
 *     -- context tag 5 is deprecated
 *         bySequenceNumber [6] SEQUENCE {
 *             referenceIndex Unsigned,
 *             count          INTEGER
 *         },
 *         byTime [7] SEQUENCE {
 *             referenceTime BACnetDateTime,
 *             count         INTEGER
 *             }
 *     } OPTIONAL
 * }
 */

/**
 *  Build a ReadRange request packet.
 *
 *  @param apdu  Pointer to the APDU buffer.
 *  @param invoke_id  Invoke ID
 *  @param rrdata  Pointer to the data used for encoding.
 *
 *  @return Bytes encoded.
 */
int rr_encode_apdu(
    uint8_t *apdu, 
    uint8_t invoke_id, 
    BACNET_READ_RANGE_DATA *rrdata)
{
    int apdu_len = 0; /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_LPDU_IP);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_READ_RANGE; /* service choice */
        apdu_len = 4;

        apdu_len +=
            encode_context_object_id(&apdu[apdu_len], 0, rrdata->object_type,
                                     rrdata->object_instance);
        apdu_len +=
            encode_context_enumerated(&apdu[apdu_len], 1,
                                      rrdata->object_property);

        /* optional array index */

        if (rrdata->array_index != BACNET_ARRAY_ALL) {
            apdu_len +=
                encode_context_unsigned(&apdu[apdu_len], 2,
                                        rrdata->array_index);
        }

        /* Build the appropriate (optional) range parameter based on the request type */

        switch (rrdata->RequestType) {
            case RR_BY_POSITION:
                apdu_len += encode_opening_tag(&apdu[apdu_len], 3);
                apdu_len += encode_application_unsigned(
                    &apdu[apdu_len], rrdata->Range.RefIndex);
                apdu_len +=
                    encode_application_signed(&apdu[apdu_len], rrdata->Count);
                apdu_len += encode_closing_tag(&apdu[apdu_len], 3);
                break;

            case RR_BY_SEQUENCE:
                apdu_len += encode_opening_tag(&apdu[apdu_len], 6);
                apdu_len += encode_application_unsigned(
                    &apdu[apdu_len], rrdata->Range.RefSeqNum);
                apdu_len +=
                    encode_application_signed(&apdu[apdu_len], rrdata->Count);
                apdu_len += encode_closing_tag(&apdu[apdu_len], 6);
                break;

            case RR_BY_TIME:
                apdu_len += encode_opening_tag(&apdu[apdu_len], 7);
                apdu_len += encode_application_date(
                    &apdu[apdu_len], &rrdata->Range.RefTime.date);
                apdu_len += encode_application_time(
                    &apdu[apdu_len], &rrdata->Range.RefTime.time);
                apdu_len +=
                    encode_application_signed(&apdu[apdu_len], rrdata->Count);
                apdu_len += encode_closing_tag(&apdu[apdu_len], 7);
                break;

        case RR_READ_ALL:  /* to attempt a read of the whole array or list, omit the range parameter */
            break;

            default:
                break;
        }
    }

    return apdu_len;
}

/**
 * Decode the received ReadRange request
 *
 *  @param apdu  Pointer to the APDU buffer.
 *  @param apdu_len  Bytes valid in the APDU buffer.
 *  @param rrdata  Pointer to the data used for encoding.
 *
 *  @return Bytes encoded.
 */
int rr_decode_service_request(
    uint8_t *apdu, 
    unsigned apdu_len, 
    BACNET_READ_RANGE_DATA *rrdata)
{
    unsigned len = 0;
    unsigned TagLen ;
    uint8_t tag_number ;
    uint32_t len_value_type ;
    BACNET_OBJECT_TYPE type;  /* for decoding */
    uint32_t enum_value;
    BACNET_UNSIGNED_INTEGER unsigned_value;

    /* check for value pointers */
    if (apdu_len >= 5) {
        /* Tag 0: Object ID */
        if (!decode_is_context_tag(&apdu[len++], 0)) {
            return -1;
        }
        len += decode_object_id(&apdu[len], &type, &rrdata->object_instance);
        rrdata->object_type = type;
        /* Tag 1: Property ID */
        if (len >= apdu_len) {
            return (-1);
        }
        len += decode_tag_number_and_value(
            &apdu[len], &tag_number, &len_value_type);
        if (tag_number != 1) {
            return -1;
        }
        len += decode_enumerated(&apdu[len], len_value_type, &enum_value);
        rrdata->object_property = (BACNET_PROPERTY_ID)enum_value;
        rrdata->Overhead = RR_OVERHEAD; /* Start with the fixed overhead */

        /* Tag 2: Optional Array Index - set to ALL if not present */
        rrdata->array_index = BACNET_ARRAY_ALL; /* Assuming this is the most common outcome... */
        if (len < apdu_len) {
            TagLen = (unsigned)decode_tag_number_and_value(
                &apdu[len], &tag_number, &len_value_type);
            if (tag_number == 2) {
                len += TagLen;
                len += decode_unsigned(
                    &apdu[len], len_value_type, &unsigned_value);
                rrdata->array_index = (BACNET_ARRAY_INDEX)unsigned_value;
                rrdata->Overhead += RR_INDEX_OVERHEAD; /* Allow for this in the response */
            }
        }
        /* And/or optional range selection- Tags 3, 6 and 7 */
        rrdata->RequestType = RR_READ_ALL;      /* Assume the worst to cut out explicit checking later */
        if (len < apdu_len) {
            /*
             * Note: We pick up the opening tag and then decode the parameter
             * types we recognise. We deal with the count and the closing tag in
             * each case statement even though it might appear that we could do
             * them after the switch statement as common elements. This is so
             * that if we receive a tag we don't recognise, we don't try to
             * decode it blindly and make a mess of it.
             */
            len += decode_tag_number_and_value(
                &apdu[len], &tag_number, &len_value_type);
            switch (tag_number) {
                case 3: /* ReadRange by position */
                    rrdata->RequestType = RR_BY_POSITION;
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_unsigned(
                        &apdu[len], len_value_type, &unsigned_value);
                    rrdata->Range.RefIndex = (uint32_t)unsigned_value;
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_signed(
                        &apdu[len], len_value_type, &rrdata->Count);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    break;

                case 6: /* ReadRange by sequence number */
                    rrdata->RequestType = RR_BY_SEQUENCE;
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_unsigned(
                        &apdu[len], len_value_type, &unsigned_value);
                    rrdata->Range.RefSeqNum = (uint32_t)unsigned_value;
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_signed(
                        &apdu[len], len_value_type, &rrdata->Count);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    /* Allow for this in the response */
                    rrdata->Overhead += RR_1ST_SEQ_OVERHEAD;
                    break;

                case 7: /* ReadRange by time stamp */
                    rrdata->RequestType = RR_BY_TIME;
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_date(&apdu[len], &rrdata->Range.RefTime.date);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_bacnet_time(
                        &apdu[len], &rrdata->Range.RefTime.time);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_signed(
                        &apdu[len], len_value_type, &rrdata->Count);
                    if (len >= apdu_len) {
                        break;
                    }
                    len += decode_tag_number_and_value(
                        &apdu[len], &tag_number, &len_value_type);
                    /* Allow for this in the response */
                    rrdata->Overhead += RR_1ST_SEQ_OVERHEAD;
                    break;

                default: /* If we don't recognise the tag then we do nothing
                          * here and try to return all elements of the array */
                    break;
            }
        }
    } else {
        return (-1);
    }

    return (int)len;
}

/*
 * ReadRange-ACK ::= SEQUENCE {
 *     objectIdentifier    [0] BACnetObjectIdentifier,
 *     propertyIdentifier  [1] BACnetPropertyIdentifier,
 *     propertyArrayIndex  [2] Unsigned OPTIONAL	,  -- used only with array datatype
 *     resultFlags         [3] BACnetResultFlags,
 *     itemCount           [4] Unsigned,
 *     itemData            [5] SEQUENCE OF ABSTRACT-SYNTAX.&TYPE,
 *     firstSequenceNumber [6] Unsigned32 OPTIONAL -- used only if 'Item Count' > 0 and the request was either of
 *                                                  -- type 'By Sequence Number' or 'By Time'
 * }
 */

/**
 * Build a ReadRange response packet
 *
 * @param apdu  Pointer to the buffer.
 * @param invoke_id  ID invoked.
 * @param rrdata  Pointer to the read range data structure used for encoding.
 *
 * @return The count of encoded bytes.
 */
int rr_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_READ_RANGE_DATA * rrdata)
{
    int imax = 0;
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

        apdu[0] = PDU_TYPE_COMPLEX_ACK;         /* complex ACK service */
        apdu[1] = invoke_id;                    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_READ_RANGE; /* service choice */
        apdu_len = 3;
        /* service ack follows */
        apdu_len += encode_context_object_id(
            &apdu[apdu_len], 0, rrdata->object_type, rrdata->object_instance);
        apdu_len += encode_context_enumerated(
            &apdu[apdu_len], 1, rrdata->object_property);
        /* context 2 array index is optional */
        if (rrdata->array_index != BACNET_ARRAY_ALL) {
            apdu_len += encode_context_unsigned(
                &apdu[apdu_len], 2, rrdata->array_index);
        }
        /* Context 3 BACnet Result Flags */
        apdu_len +=
            encode_context_bitstring(&apdu[apdu_len], 3, &rrdata->ResultFlags);
        /* Context 4 Item Count */
        apdu_len +=
            encode_context_unsigned(&apdu[apdu_len], 4, rrdata->ItemCount);
        /* Context 5 Property list - reading the standard it looks like an empty
         * list still requires an opening and closing tag as the tagged
         * parameter is not optional
         */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 5);
        if (rrdata->ItemCount != 0) {
            imax = rrdata->application_data_len;
            if (imax > (MAX_APDU - apdu_len - 2 /*closing*/)) {
                imax = (MAX_APDU - apdu_len - 2);
            }
            for (len = 0; len < imax; len++) {
                apdu[apdu_len++] = rrdata->application_data[len];
            }
        }
        apdu_len += encode_closing_tag(&apdu[apdu_len], 5);

        if ((rrdata->ItemCount != 0) &&
            (rrdata->RequestType != RR_BY_POSITION) &&
            (rrdata->RequestType != RR_READ_ALL)) {
            /* Context 6 Sequence number of first item */
            if (apdu_len < (MAX_APDU - 4)) {
                apdu_len += encode_context_unsigned(
                    &apdu[apdu_len], 6, rrdata->FirstSequence);
            }
        }

    return apdu_len;
}

/**
 * Decode the received ReadRange response
 *
 *  @param apdu  Pointer to the APDU buffer.
 *  @param apdu_len  Bytes valid in the APDU buffer.
 *  @param rrdata  Pointer to the data filled while decoding.
 *
 *  @return Bytes decoded.
 */
int rr_ack_decode_service_request(
    uint8_t *apdu,
    int apdu_len, /* total length of the apdu */
    BACNET_READ_RANGE_DATA *rrdata)
{
    uint8_t tag_number ;
    uint32_t len_value_type ;
    int tag_len ;                       /* length of tag decode */
    int len = 0;                        /* total length of decodes */
    int start_len;
    BACNET_OBJECT_TYPE object_type ;    /* object type */
    uint32_t property ;                 /* for decoding */
    BACNET_UNSIGNED_INTEGER unsigned_value;

    /* Check apdu_len against the len during decode. */
    if (apdu_len >= 5 /* minimum */) {
        /* Tag 0: Object ID */
        if (!decode_is_context_tag(&apdu[0], 0)) {
            return -1;
        }
        len = 1;
        len += decode_object_id(
            &apdu[len], &object_type, &rrdata->object_instance);
        rrdata->object_type = object_type;

        /* Tag 1: Property ID */
        if (len >= apdu_len) {
            return -1;
        }
        len += decode_tag_number_and_value(
            &apdu[len], &tag_number, &len_value_type);
        if (tag_number != 1) {
            return -1;
        }
        len += decode_enumerated(&apdu[len], len_value_type, &property);
        rrdata->object_property = (BACNET_PROPERTY_ID)property;

        /* Tag 2: Optional Array Index */
        if (len >= apdu_len) {
            return -1;
        }
        tag_len = decode_tag_number_and_value(
            &apdu[len], &tag_number, &len_value_type);
        if (tag_number == 2) {
            len += tag_len;
            len += decode_unsigned(&apdu[len], len_value_type, &unsigned_value);
            rrdata->array_index = (BACNET_ARRAY_INDEX)unsigned_value;
        } else {
            rrdata->array_index = BACNET_ARRAY_ALL;
        }

        /* Tag 3: Result Flags */
        if (len >= apdu_len) {
            return -1;
        }
        len += decode_tag_number_and_value(
            &apdu[len], &tag_number, &len_value_type);
        if (tag_number != 3) {
            return -1;
        }
        if (len >= apdu_len) {
            return -1;
        }
        len +=
            decode_bitstring(&apdu[len], len_value_type, &rrdata->ResultFlags);

        /* Tag 4: Item count */
        if (len >= apdu_len) {
            return -1;
        }
        len += decode_tag_number_and_value(
            &apdu[len], &tag_number, &len_value_type);
        if (tag_number != 4) {
            return -1;
        }
        if (len >= apdu_len) {
            return -1;
        }
        len += decode_unsigned(&apdu[len], len_value_type, &unsigned_value);
        rrdata->ItemCount = (uint32_t)unsigned_value;
        if (len >= apdu_len) {
            return -1;
        }
        if (decode_is_opening_tag_number(&apdu[len], 5)) {
            len++; /* A tag number of 5 is not extended so only one octet
                    * Setup the start position and length of the data returned
                    * from the request don't decode the application tag number
                    * or its data here. */
            rrdata->application_data = &apdu[len];
            start_len = len;
            while (len < apdu_len) {
                if (IS_CONTEXT_SPECIFIC(apdu[len]) &&
                    (decode_is_closing_tag_number(&apdu[len], 5))) {
                    rrdata->application_data_len = len - start_len;
                    len++; /* Step over single byte closing tag */
                    break;
                } else {
                    /* Don't care about tag number, just skipping over anyway */
                    len += decode_tag_number_and_value(
                        &apdu[len], NULL, &len_value_type);
                    len += len_value_type; /* Skip over data value as well */
                    if (len >= apdu_len) { /* APDU is exhausted so we have
                                            * failed to find closing tag */
                        return (-1);
                    }
                }
            }
        } else {
            return -1;
        }
        if (len < apdu_len) { /* Still something left to look at? */
            /* Tag 6: Item count */
            len += decode_tag_number_and_value(
                &apdu[len], &tag_number, &len_value_type);
            if (tag_number != 6) {
                return -1;
            }
            if (len < apdu_len) {
                len += decode_unsigned(
                    &apdu[len], len_value_type, &unsigned_value);
                rrdata->FirstSequence = (uint32_t)unsigned_value;
            }
        }
    }

    return len;
}

/* FIXME: Currently does not have test framework */
