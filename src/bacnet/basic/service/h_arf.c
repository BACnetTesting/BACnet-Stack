/**************************************************************************
 *
 * Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

#include "configProj.h"
    
#include "bacnet/bacerror.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/abort.h"
#include "bacnet/arf.h"
// #include "bacnet/bacfile.h"
#include "bacnet/bits/util/multipleDatalink.h"

/** @file h_arf.c  Handles Atomic Read File request. */

/*
from BACnet SSPC-135-2004

14. FILE ACCESS SERVICES

This clause defines the set of services used to access and
manipulate files contained in BACnet devices. The concept of files
is used here as a network-visible representation for a collection
of octets of arbitrary length and meaning. This is an abstract
concept only and does not imply the use of disk, tape or other
mass storage devices in the server devices. These services may
be used to access vendor-defined files as well as specific
files defined in the BACnet protocol standard.
Every file that is accessible by File Access Services shall
have a corresponding File object in the BACnet device. This File
object is used to identify the particular file by name. In addition,
the File object provides access to "header information," such
as the file's total size, creation date, and type. File Access
Services may model files in two ways: as a continuous stream of
octets or as a contiguous sequence of numbered records.
The File Access Services provide atomic read and write operations.
In this context "atomic" means that during the execution
of a read or write operation, no other AtomicReadFile or
AtomicWriteFile operations are allowed for the same file.
Synchronization of these services with internal operations
of the BACnet device is a local matter and is not defined by this
standard.

14.1 AtomicReadFile Service

14.1.5 Service Procedure

The responding BACnet-user shall first verify the validity
of the 'File Identifier' parameter and return a 'Result(-)' response
with the appropriate error class and code if the File object
is unknown, if there is currently another AtomicReadFile or
AtomicWriteFile service in progress, or if the File object is
currently inaccessible for another reason. If the 'File Start
Position' parameter or the 'File Start Record' parameter is
either less than 0 or exceeds the actual file size, then the appropriate
error is returned in a 'Result(-)' response. If not, then the
responding BACnet-user shall read the number of octets specified by
'Requested Octet Count' or the number of records specified by
'Requested Record Count'. If the number of remaining octets or
records is less than the requested amount, then the length of
the 'File Data' returned or 'Returned Record Count' shall indicate
the actual number read. If the returned response contains the
last octet or record of the file, then the 'End Of File' parameter
shall be TRUE, otherwise FALSE.
 */

#if defined(BACFILE)
void handler_atomic_read_file(
    DEVICE_OBJECT_DATA *pDev,
	BACNET_ROUTE *rxDetails, 
    DEVICE_OBJECT_DATA *sendingDev,
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_ATOMIC_READ_FILE_DATA data;
    int len = 0;
    int pdu_len = 0;
    bool error = false;
    BACNET_NPCI_DATA npci_data;
    // BACNET_PATH my_address;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_OBJECT;
    BACNET_ERROR_CODE error_code = ERROR_CODE_UNKNOWN_OBJECT;

	DLCB *dlcb = alloc_dlcb_response('B', rxDetails );
	if (dlcb == NULL) return;

    dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Received Atomic-Read-File Request!\n");

    /* encode the NPDU portion of the packet */
    // datalink_get_my_address(&my_address);
    npdu_setup_npci_data(&npci_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&dlcb->Handler_Transmit_Buffer[0], &rxDetails->bacnetPath.glAdr, NULL, // &my_address,
        &npci_data);
    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "ARF: Segmented Message. Sending Abort!\n");
        goto ARF_ABORT;
    }
    len = arf_decode_service_request(service_request, service_len, &data);
    /* bad decoding - send an abort */
    if (len < 0) {
        len =
            abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
        dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Bad Encoding. Sending Abort!\n");
        goto ARF_ABORT;
    }
    if (data.object_type == OBJECT_FILE) {
        if (!bacfile_valid_instance(data.object_instance)) {
            error = true;
        } else if (data.access == FILE_STREAM_ACCESS) {
            if (data.type.stream.requestedOctetCount <
                octetstring_capacity(&data.fileData[0])) {
                bacfile_read_stream_data(&data);
				dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "ARF: Stream offset %d, %d octets.\n",
					data.type.stream.fileStartPosition,
					data.type.stream.requestedOctetCount);
				len =
					arf_ack_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
					service_data->invoke_id, &data);
            } else {
                len =
                    abort_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
                dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Too Big To Send (%d >= %d). Sending Abort!\n",
                    data.type.stream.requestedOctetCount,
                    (int)octetstring_capacity(&data.fileData[0]));
            }
        } else if (data.access == FILE_RECORD_ACCESS) {
            if (data.type.record.fileStartRecord >=
                BACNET_READ_FILE_RECORD_COUNT) {
                error_class = ERROR_CLASS_SERVICES;
                error_code = ERROR_CODE_INVALID_FILE_START_POSITION;
                error = true;
            } else if (bacfile_read_stream_data(&data)) {
                dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "ARF: fileStartRecord %d, %u RecordCount.\n",
                    data.type.record.fileStartRecord,
                    data.type.record.RecordCount);
                len =
                    arf_ack_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, &data);
            } else {
                error = true;
                error_class = ERROR_CLASS_OBJECT;
                error_code = ERROR_CODE_FILE_ACCESS_DENIED;
            }
        } else {
            error = true;
            error_class = ERROR_CLASS_SERVICES;
            error_code = ERROR_CODE_INVALID_FILE_ACCESS_METHOD;
            dbMessage(DBD_ALL, DB_UNEXPECTED_ERROR, "Record Access Requested. Sending Error!\n");
        }
    } else {
        error = true;
        error_class = ERROR_CLASS_SERVICES;
        error_code = ERROR_CODE_INCONSISTENT_OBJECT_TYPE;
    }
    if (error) {
        len =
            bacerror_encode_apdu(&dlcb->Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, SERVICE_CONFIRMED_ATOMIC_READ_FILE,
            error_class, error_code);
    }
  ARF_ABORT:
    pdu_len += len;
    dlcb->optr = pdu_len;
    rxDetails->portParams->SendPdu(dlcb);
}
#endif
