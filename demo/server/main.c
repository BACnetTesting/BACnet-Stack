/**************************************************************************
*
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
*   This file contains changes made by ConnectEx, Inc. If published,
*   these changes are subject to the permissions, warranty 
*   terms and limitations above. If not published, then these terms
*   apply to ConnectEx, Inc's customers to whom the code has 
*   been supplied. For more details info@connect-ex.com
*   Where appropriate, the changes are Copyright (C) 2014-2017 ConnectEx, Inc.
*
*********************************************************************/

#include <conio.h>
//#include <stddef.h>
#include <stdint.h>
//#include <stdio.h>
#include <stdlib.h>
//#include <signal.h>
//#include <time.h>
//
//#include "config.h"
//#include "server.h"
#include "address.h"
//#include "bacdef.h"
#include "handlers.h"
//#include "client.h"
#include "dlenv.h"
//#include "bacdcode.h"
//#include "npdu.h"
//#include "apdu.h"
//#include "iam.h"
//#if ( BACNET_CLIENT == 1 )
//#include "tsm.h"
//#endif
#include "device.h"
//#include "bacfile.h"
#include "datalink.h"
#include "dcc.h"
#include "filename.h"
//#include "getevent.h"
//#include "net.h"
//#include "txbuf.h"
//#include "lc.h"
#include "version.h"
///* include the device object */
//#include "device.h"
//#include "trendlog.h"
#if defined(INTRINSIC_REPORTING)
#include "nc.h"
#endif /* defined(INTRINSIC_REPORTING) */
//#if defined(BACFILE)
//#include "bacfile.h"
//#endif /* defined(BACFILE) */
//#if defined(BAC_UCI)
//#include "ucix.h"
//#endif /* defined(BAC_UCI) */
#include "bip.h"
#include "client.h"
#include "bvlc.h"
#include "btaDebug.h"

/** @file server/main.c  Example server application using the BACnet Stack. */

/* (Doxygen note: The next two lines pull all the following Javadoc
 *  into the ServerDemo module.) */
/** @addtogroup ServerDemo */
/*@{*/

/** Buffer used for receiving */
static uint8_t Rx_Buf[MAX_MPDU_IP] = { 0 };

extern DLINK_SUPPORT *datalinkSupportHead;

/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);

#if ( INTRINSIC_REPORTING == 1 )
	/* 	BACnet Testing Observed Incident oi00107
		Server only devices should not indicate that they EXECUTE I-Am
		Revealed by BACnet Test Client v1.8.16 ( www.bac-test.com/bacnet-test-client-download )
			BITS: BIT00040
		Any discussions can be directed to edward@bac-test.com
		Please feel free to remove this comment when my changes accepted after suitable time for
		review by all interested parties. Say 6 months -> September 2016 */

	/* In this demo, we are usually acting as a server only ( BACnet "B" device ) so we do not indicate
	   that we can execute the I-Am message.
	   However there are exceptions, one exception is if we are supporting Intrinsic Alarming. 
	   There may be others */

    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
#endif

    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
#if defined (BAC_WPM)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
        handler_write_property_multiple);
#endif

#if (BACNET_SVC_READ_RANGE_B == 1)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range);
#endif

#if defined(BACFILE)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_READ_FILE,
        handler_atomic_read_file);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_WRITE_FILE,
        handler_atomic_write_file);
#endif
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
        handler_timesync_utc);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
        handler_timesync);

#if ( BACNET_SVC_COV_B == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
        handler_cov_subscribe);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_COV_NOTIFICATION,
        handler_ucov_notification);
#endif

    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);

#if ( BACNET_SVC_PRIVATE_TRANSFER )
    /* handle the data coming back from private requests */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
#endif

#if (INTRINSIC_REPORTING == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM,
        handler_alarm_ack);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_EVENT_INFORMATION,
        handler_get_event_information);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_ALARM_SUMMARY,
        handler_get_alarm_summary);
#endif /* defined(INTRINSIC_REPORTING) */
#if defined(BACNET_TIME_MASTER)
    handler_timesync_init();
#endif
}

static void print_usage(const char *filename)
{
    printf("Usage: %s [device-instance [device-name]]\n", filename);
    printf("       [--version][--help]\n");
}

static void print_help(const char *filename)
{
    printf("Simulate a BACnet server device\n"
        "device-instance:\n"
        "BACnet Device Object Instance number that you are\n"
        "trying simulate.\n"
        "device-name:\n"
        "The Device object-name is the text name for the device.\n"
        "\nExample:\n");
    printf("To simulate Device 123, use the following command:\n"
        "%s 123\n", filename);
    printf("To simulate Device 123 named Fred, use following command:\n"
        "%s 123 Fred\n", filename);
}


static void print_menu_help(void)
{
    printf("\r\nKeys: Q)uit");
    printf("\r\n      P) Port Info       E) EMM trace log");
    printf("\r\n      A) Address cache   T)SM cache\r\n");
}


static bool UserMenu(int ch)
{
    switch (toupper(ch))
    {
    case 'A':
        print_address_cache();
        break;

    case 'T':
        print_tsm_cache();
        break;

    case 'E':
//        DumpEmmTraceLog();
        break;

    case 'P':
//        print_port_info();
        break;

    case 'Q':
        return false;

    default:
        print_menu_help();
        break;
    }

    return true;
}


/** Main function of server demo.
 *
 * @see Device_Set_Object_Instance_Number, dlenv_init, Send_I_Am,
 *      datalink_receive, npdu_handler,
 *      dcc_timer_seconds, bvlc_maintenance_timer,
 *      Load_Control_State_Machine_Handler, handler_cov_task,
 *      tsm_timer_milliseconds
 *
 * @param argc [in] Arg count.
 * @param argv [in] Takes one argument: the Device Instance #.
 * @return 0 on success.
 */
int main(
    int argc,
    char *argv[])
{
    DLINK_SUPPORT *ps;
    BACNET_ROUTE src; // now we need to include the datalink information

    uint16_t pdu_len = 0;
    unsigned timeout = 1;       /* milliseconds */
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    uint32_t elapsed_seconds = 0;
    uint32_t elapsed_milliseconds = 0;
    uint32_t address_binding_tmr = 0;
    uint32_t recipient_scan_tmr = 0;
#if defined(BACNET_TIME_MASTER)
    BACNET_DATE_TIME bdatetime;
#endif
#if defined(BAC_UCI)
    int uciId = 0;
    struct uci_context *ctx;
#endif
    int argi = 0;
    const char *filename = NULL;

    SendBTAstartMessage("Emulation Starting");

    filename = filename_remove_path(argv[0]);
    for (argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--help") == 0) {
            print_usage(filename);
            print_help(filename);
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0) {
            printf("%s %s\n", filename, BACNET_VERSION_TEXT);
            printf("Copyright (C) 2014 by Steve Karg and others.\n"
                "This is free software; see the source for copying conditions.\n"
                "There is NO warranty; not even for MERCHANTABILITY or\n"
                "FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        }
    }
#if defined(BAC_UCI)
    ctx = ucix_init("bacnet_dev");
    if (!ctx) {
        fprintf(stderr, "Failed to load config file bacnet_dev\n");
    }
    uciId = ucix_get_option_int(ctx, "bacnet_dev", "0", "Id", 0);
    printf("ID: %i", uciId);
    if (uciId != 0) {
        Device_Set_Object_Instance_Number(uciId);
    } else {
#endif /* defined(BAC_UCI) */
        /* allow the device ID to be set */
        if (argc > 1) {
            Device_Set_Object_Instance_Number(strtol(argv[1], NULL, 0));
        }
        if (argc > 2) {
            Device_Object_Name_ANSI_Init(argv[2]);
        }
#if defined(BAC_UCI)
    }
    ucix_cleanup(ctx);
#endif /* defined(BAC_UCI) */

#if ( BBMD_ENABLED )
    printf("BBMD is enabled\n");
#else
    printf("BBMD is vnot enabled\n");
#endif


    printf("BACnet Server Demo\n" "BACnet Stack Version %s\n"
        "BACnet Device ID: %u\n" "Max APDU: %d\n", BACnet_Version,
        Device_Object_Instance_Number(), MAX_APDU);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);

    // initialize our multiple datalinks
    InitDatalink(DL_BBMD, 47808);
    InitDatalink(DL_MSTP, 0);

    /* broadcast an I-Am on startup */
    for (ps = datalinkSupportHead; ps != NULL; ps = (DLINK_SUPPORT *)ps->llist.next)
    {
        Broadcast_I_Am(ps); //  &Handler_Transmit_Buffer[0]);
    }

    /* loop forever */
    bool busy = true;
    while ( busy ) {

#ifdef _MSC_VER
        if (kbhit())
        {
            busy = UserMenu(getch());
        }
#endif

        /* input */
        current_seconds = time(NULL);

        // any incoming on any of our datalinks?
        for (ps = datalinkSupportHead; ps != NULL; ps = (DLINK_SUPPORT *)ps->llist.next)
        {
            src.portParams = ps;

            /* returns 0 bytes on timeout */
            pdu_len = ps->ReceiveMPDU(ps, &src.bacnetPath.localMac, &Rx_Buf[0], ps->max_apdu);

            /* process */
            if (pdu_len) {
                npdu_handler(&src, &Rx_Buf[0], pdu_len);
            }
        }

        /* at least one second has passed */
        elapsed_seconds = (uint32_t) (current_seconds - last_seconds);
        if (elapsed_seconds) {
            last_seconds = current_seconds;
            dcc_timer_seconds(elapsed_seconds);

            for (ps = datalinkSupportHead; ps != NULL; ps = (DLINK_SUPPORT *)ps->llist.next)
            {
#if defined(BACDL_BIP) && BBMD_ENABLED
                bvlc_maintenance_timer(ps, elapsed_seconds);
#endif
                dlenv_maintenance_timer(ps, elapsed_seconds);
            }

#if defined (LOAD_CONTROL)
            Load_Control_State_Machine_Handler();
#endif
            elapsed_milliseconds = elapsed_seconds * 1000;
#if ( BACNET_SVC_COV_B == 1 )
            handler_cov_timer_seconds(elapsed_seconds);
#endif

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
			// todo 3 - The TSM is requested to monitor Confirmed Events. (Not only COV B). Consolidate compile options 
            tsm_timer_milliseconds(elapsed_milliseconds);
#endif

#if defined(TREND_LOG)
            trend_log_timer(elapsed_seconds);
#endif
#if defined(INTRINSIC_REPORTING)
            Device_local_reporting();
#endif
#if defined(BACNET_TIME_MASTER)
            Device_getCurrentDateTime(&bdatetime);
            handler_timesync_task(&bdatetime);
#endif
        }

#if ( BACNET_SVC_COV_B == 1 )
        handler_cov_task();
#endif

        /* scan cache address */
        address_binding_tmr += elapsed_seconds;
        if (address_binding_tmr >= 60) {
            address_cache_timer(address_binding_tmr);
            address_binding_tmr = 0;
        }
#if (INTRINSIC_REPORTING == 1)
        /* try to find addresses of recipients */
        recipient_scan_tmr += elapsed_seconds;
        if (recipient_scan_tmr >= NC_RESCAN_RECIPIENTS_SECS) {
            Notification_Class_find_recipient();
            recipient_scan_tmr = 0;
        }
#endif
        /* output */

        /* blink LEDs, Turn on or off outputs, etc */
    }

    return 0;
}

/* @} */

/* End group ServerDemo */
