/*************************************************************************
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

/* command line tool that sends a BACnet service, and displays the reply */

// aka Read Property Friendly 
// based on Steve Karg's original demo/readprop

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>       /* for time */

#define PRINT_ENABLED 1

#include "bacdef.h"
#include "config.h"
#include "bactext.h"
#include "bacerror.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "whois.h"
#include "version.h"
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* converted command line arguments */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
static int32_t Target_Object_Index = BACNET_ARRAY_ALL;
/* the invoke id is needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;

static void MyErrorHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
		BACNET_ERROR_CLASS error_class, BACNET_ERROR_CODE error_code) {
	if (address_match(&Target_Address, src)
			&& (invoke_id == Request_Invoke_ID)) {
		printf("BACnet Error: %s: %s\n",
				bactext_error_class_name((int) error_class),
				bactext_error_code_name((int) error_code));
		Error_Detected = true;
	}
}


void MyAbortHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
    BACNET_ABORT_REASON abort_reason,
		bool server) {
	(void) server;
	if (address_match(&Target_Address, src)
			&& (invoke_id == Request_Invoke_ID)) {
		printf("BACnet Abort: %s\n",
				bactext_abort_reason_name((int) abort_reason));
		Error_Detected = true;
	}
}


void MyRejectHandler(BACNET_ADDRESS * src, uint8_t invoke_id,
    BACNET_REJECT_REASON reject_reason) {
	if (address_match(&Target_Address, src)
			&& (invoke_id == Request_Invoke_ID)) {
		printf("BACnet Reject: %s\n",
				bactext_reject_reason_name( reject_reason));
		Error_Detected = true;
	}
}

/** Handler for a ReadProperty ACK.
 * @ingroup DSRP
 * Doesn't actually do anything, except, for debugging, to
 * print out the ACK data of a matching request.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void My_Read_Property_Ack_Handler(uint8_t * service_request,
		uint16_t service_len, BACNET_ADDRESS * src,
		BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data) {
	int len = 0;
	BACNET_READ_PROPERTY_DATA data;

	if (address_match(&Target_Address, src)
			&& (service_data->invoke_id == Request_Invoke_ID)) {
		len = rp_ack_decode_service_request(service_request, service_len,
				&data);
		if (len < 0) {
			printf("<decode failed!>\n");
		} else {
			rp_ack_print_data(&data);
		}
	}
}


static void Init_Service_Handlers(void) {
	Device_Init(NULL);
	/* we need to handle who-is
	 to support dynamic device binding to us */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is_unicast);
	/* handle i-am to support binding to other devices */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
	/* set the handler for all the services we don't implement
	 It is required to send the proper reject message... */
	apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
	/* we must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
			handler_read_property);
	/* handle the data coming back from confirmed requests */
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
			My_Read_Property_Ack_Handler);
	/* handle any errors coming back */
	apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
	apdu_set_abort_handler(MyAbortHandler);
	apdu_set_reject_handler(MyRejectHandler);
}


static void print_usage(const char *filename) {
	printf("Usage: %s device-instance object-type object-instance "
			"property [index]\n", filename);
	printf("       [--version][--help]\n");
}


static void print_help(const char *filename) {
	printf("Read a property from an object in a BACnet device\n"
			"and print the value.\n"
			"device-instance:\n"
			"BACnet Device Object Instance number that you are\n"
			"trying to communicate to.  This number will be used\n"
			"to try and bind with the device using Who-Is and\n"
			"I-Am services.  For example, if you were reading\n"
			"Device Object 123, the device-instance would be 123.\n"
			"\nobject-type:\n"
			"Text for object type, e.g. AI, AO, Dev\n"
			"\nobject-instance:\n"
			"This is the object instance number of the object that\n"
			"you are reading.  For example, if you were reading\n"
			"Analog Output 2, the object-instance would be 2.\n"
			"\nproperty:\n"
			"Text for property, e.g. PV, Desc, Name\n"
			"\nindex:\n"
			"This integer parameter is the index number of an array.\n"
			"If the property is an array, individual elements can\n"
			"be read.  If this parameter is missing and the property\n"
			"is an array, the entire array will be read.\n"
			"Trailing options: -p local port\n"
			"                  -b BBMD address\n"
			"                  -r BBMD port\n"
			"\nExample:\n"
			"If you want read the Present-Value of Analog Output 101\n"
			"in Device 123, you could send the following command:\n"
			"%s 123 AI 101 PV\n", filename);
}

int ParseCmd(int argc, char *argv[], char srch) {
	int i = 1;
	for (; i < argc - 1; i++) {
		if ((argv[i][0] == '-' || argv[i][0] == '/') && argv[i][1] == srch) {
			return i + 1;
		}
	}
	return 0;
}

void SetOurEnv( char *ev, char *nev )
{
#ifdef _MSC_VER
		_putenv_s( ev, nev );
#else
		setenv( ev, nev, 1);
#endif
}

int main(int argc, char *argv[]) {
	BACNET_ADDRESS src = { 0 }; /* address where message came from */
	uint16_t pdu_len = 0;
	unsigned timeout = 100; /* milliseconds */
	unsigned max_apdu = 0;
	time_t elapsed_seconds = 0;
	time_t last_seconds = 0;
	time_t current_seconds = 0;
	time_t timeout_seconds = 0;
	bool found = false;
	int argi = 0;
	const char *filename ;

	filename = filename_remove_path(argv[0]);
	for (argi = 1; argi < argc; argi++) {
		if (strcmp(argv[argi], "--help") == 0) {
			print_usage(filename);
			print_help(filename);
			return 0;
		}
		if (strcmp(argv[argi], "--version") == 0) {
			printf("%s %s\n", filename, BACNET_VERSION_TEXT);
			printf(
					"Copyright (C) 2014 by Steve Karg and others.\n"
							"This is free software; see the source for copying conditions.\n"
							"There is NO warranty; not even for MERCHANTABILITY or\n"
							"FITNESS FOR A PARTICULAR PURPOSE.\n");
			return 0;
		}
	}
	if (argc < 5) {
		print_usage(filename);
		return 0;
	}

	/* decode the command line parameters */
	Target_Device_Object_Instance = strtol(argv[1], NULL, 0);

	if (strcmp(argv[2], "AI") == 0)
		Target_Object_Type = OBJECT_ANALOG_INPUT;
	else if (strcmp(argv[2], "AO") == 0)
		Target_Object_Type = OBJECT_ANALOG_OUTPUT;
	else if (strcmp(argv[2], "AV") == 0)
		Target_Object_Type = OBJECT_ANALOG_VALUE;
	else if (strcmp(argv[2], "BI") == 0)
		Target_Object_Type = OBJECT_BINARY_INPUT;
	else if (strcmp(argv[2], "BO") == 0)
		Target_Object_Type = OBJECT_BINARY_OUTPUT;
	else if (strcmp(argv[2], "BV") == 0)
		Target_Object_Type = OBJECT_BINARY_VALUE;
	else if (strcmp(argv[2], "Dev") == 0)
		Target_Object_Type = OBJECT_DEVICE;
	else {
		printf("Need to specify Object Type: Dev, AI, AO, AV, BI etc ...\n");
		exit(1);
	}

	Target_Object_Instance = strtol(argv[3], NULL, 0);

	if (strcmp(argv[4], "PV") == 0)
		Target_Object_Property = PROP_PRESENT_VALUE;
	else if (strcmp(argv[4], "Name") == 0)
		Target_Object_Property = PROP_OBJECT_NAME;
	else if (strcmp(argv[4], "Desc") == 0)
		Target_Object_Property = PROP_DESCRIPTION;
	else if (strcmp(argv[4], "ObjList") == 0)
		Target_Object_Property = PROP_OBJECT_LIST;
	else {
		printf("Need to specify Property : PV, Name, Desc ...\n");
		exit(1);
	}

	int portArg = ParseCmd(argc, argv, 'p');
	if (portArg != 0) SetOurEnv("BACNET_IP_PORT", argv[portArg] );

	portArg = ParseCmd(argc, argv, 'b');
	if (portArg != 0) SetOurEnv("BACNET_BBMD_ADDRESS", argv[portArg] );

	portArg = ParseCmd(argc, argv, 'f');
	if (portArg != 0) SetOurEnv("BACNET_BBMD_PORT", argv[portArg] );

	/* setup my info */
	Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
	address_init();
	Init_Service_Handlers();
	dlenv_init();
	atexit(datalink_cleanup);
	/* configure the timeout values */
	last_seconds = time(NULL);
	timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
	/* try to bind with the device */
	found = address_bind_request(Target_Device_Object_Instance, &max_apdu,
			&Target_Address);
	if (!found) {
		Send_WhoIs(Target_Device_Object_Instance,
				Target_Device_Object_Instance);
	}
	/* loop forever */
	for (;;) {
		/* increment timer - exit if timed out */
		current_seconds = time(NULL);

		/* at least one second has passed */
		if (current_seconds != last_seconds)
			tsm_timer_milliseconds(
					(uint16_t) ((current_seconds - last_seconds) * 1000));
		if (Error_Detected)
			break;
		/* wait until the device is bound, or timeout and quit */
		if (!found) {
			found = address_bind_request(Target_Device_Object_Instance,
					&max_apdu, &Target_Address);
		}
		if (found) {
			if (Request_Invoke_ID == 0) {
				Request_Invoke_ID = Send_Read_Property_Request(
						Target_Device_Object_Instance, Target_Object_Type,
						Target_Object_Instance, Target_Object_Property,
						Target_Object_Index);
			} else if (tsm_invoke_id_free(Request_Invoke_ID))
				break;
			else if (tsm_invoke_id_failed(Request_Invoke_ID)) {
				fprintf(stderr, "\rError: TSM Timeout!\n");
				tsm_free_invoke_id(Request_Invoke_ID);
				Error_Detected = true;
				/* try again or abort? */
				break;
			}
		} else {
			/* increment timer - exit if timed out */
			elapsed_seconds += (current_seconds - last_seconds);
			if (elapsed_seconds > timeout_seconds) {
				printf("\rError: APDU Timeout!\n");
				Error_Detected = true;
				break;
			}
		}

		/* returns 0 bytes on timeout */
		pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

		/* process */
		if (pdu_len) {
			npdu_handler(&src, &Rx_Buf[0], pdu_len);
		}

		/* keep track of time for next check */
		last_seconds = current_seconds;
	}

	if (Error_Detected)
		return 1;
	return 0;
}
