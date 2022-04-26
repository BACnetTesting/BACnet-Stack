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

#ifdef _MSC_VER

#include <direct.h>         // for cwd
#endif

#include "config.h"

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
#include "bbmd.h"
// todo2 #include "readConfigCSVbbmd.h"
#endif

//#include "server.h"
#include "address.h"
#include "handlers.h"
#include "client.h"             // todo4 - hate this.. remove one day
#include "dlenv.h"
#include "iam.h"
#include "tsm.h"
#include "bitsUtil.h"

#include "device.h"
#include "datalink.h"
#include "filename.h"
#include "txbuf.h"
#include "version.h"

#include "netport.h"
#include "ai.h"
#include "ao.h"
#include "av.h"
#include "bi.h"
#include "bo.h"
#include "bv.h"

#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1)
#include "ms-input.h"
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1)
#include "mso.h"
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1)
#include "ms-value.h"
#endif

#include "calendar.h"
#include "schedule.h"

#include "dcc.h"
#include "configParams.h"
#include "btaDebug.h"
#include "bitsUtil.h"

extern const char *BACnet_Version;

/** Buffer used for receiving */
// static uint8_t Rx_Buf[MAX_MPDU] = { 0 };
static PORT_SUPPORT ourDatalink;
ConfigType config;

static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS,
        handler_who_is_unicast);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, 
        handler_who_has);

#if 0
    /* 	BACnet Testing Observed Incident oi00107
        Server only devices should not indicate that they EXECUTE I-Am
        Revealed by BACnet Test Client v1.8.16 ( www.bac-test.com/bacnet-test-client-download )
        BITS: BIT00040
        Any discussions can be directed to edward@bac-test.com
        Please feel free to remove this comment when my changes accepted after suitable time for
        review by all interested parties. Say 6 months -> September 2016 */
        /* In this demo, we are the server only ( BACnet "B" device ) so we do not indicate
           that we can execute the I-Am message */
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
#endif

    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler(
        handler_unrecognized_service);

    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);

#if ( BACNET_SVC_RPM_B == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
#endif

    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);

#if (BACNET_SVC_WPM_B == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
        handler_write_property_multiple);
#endif

#if ( BACNET_SVC_RR_B == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range);
#endif

#if (BACFILE == 1)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_READ_FILE,
        handler_atomic_read_file);

    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_WRITE_FILE,
        handler_atomic_write_file);
#endif
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);

#if ( BACNET_SVC_TIME == 1 )
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
        handler_timesync_utc);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
        handler_timesync);
#endif

#if ( BACNET_SVC_COV_B == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
        handler_cov_subscribe);
#endif

#if ( BACNET_SVC_COV_A == 1 )
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

#if (INTRINSIC_REPORTING_B==1)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM,
        handler_alarm_ack);
#endif // (INTRINSIC_REPORTING_B == 1)

#if ( BACNET_PROTOCOL_REVISION < 13 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_ALARM_SUMMARY,
        handler_get_alarm_summary);
#endif

#if ( BACNET_USE_EVENT_HANDLING == 1 )
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_EVENT_INFORMATION,
        handler_get_event_information);
#endif 

#if (BACNET_TIME_MASTER == 1)
    handler_timesync_init();
#endif

    // apdu_set_confirmed_handler(SERVICE_CONFIRMED_ADD_LIST_ELEMENT, handler_add_list_element);
    // apdu_set_confirmed_handler(SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT, handler_remove_list_element);
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
    // BACNET_ADDRESS src;         /* address where message came from */
    // uint16_t pdu_len;
    // unsigned timeout = 100;    /* milliseconds */
    time_t last_seconds = time (NULL) ;
    time_t current_seconds;
    uint32_t elapsed_seconds;
    uint32_t elapsed_milliseconds;
    //uint32_t address_binding_tmr = 0;
    //uint32_t recipient_scan_tmr;
//#if (BACNET_TIME_MASTER == 1)
    //BACNET_DATE_TIME bdatetime;
//#endif
#ifdef _MSC_VER
    char cwd[MAX_PATH];
    _getcwd(cwd, MAX_PATH);
#else 
    char *cwd = getcwd(NULL, 0);
#endif

    printf("Running [%s] 1\n", argv[0]);
    printf("In dir  [%s]\n", cwd);

#ifndef _MSC_VER
    free(cwd);
#endif

    int argi;
    const char *filename;


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

#if defined(BBMD_ENABLED) && (BBMD_ENABLED == 1)
    // todo2 ParseBBMDfile("..\\BBMDsettings.txt");
#endif

    /* allow the device ID to be set */
    if (argc > 1) {
        Device_Set_Object_Instance_Number(strtol(argv[1], NULL, 0));
    }
    if (argc > 2) {
        Device_Object_Name_ANSI_Init(argv[2]);
    }

    printf("BACnet Server Demo\n" "BACnet Stack Version %s\n"
        "BACnet Device ID: %u\n" "Max APDU: %d\n", BACnet_Version,
        Device_Object_Instance_Number(), MAX_APDU);

    /* load any static address bindings to show up
       in our device bindings list */
    address_init();

    // SetConfigDefaults(&config);

    InitBACnet();

    Init_Service_Handlers();

    // Set up user datalink settings
    // InitDatalink("eth0", BPT_BBMD, config.localBBMDbacnetPort);
    // InitDatalink("eth0", BPT_BIP, config.localBACnetPort);
    // Create some Objects, dynamically

#if ( BACNET_USE_OBJECT_ANALOG_INPUT == 1)
    Analog_Input_Create(1, "Ana Input 1"); // , UNITS_DEGREES_CELSIUS, 0.0 );
    Analog_Input_Create(2, "Ana Input 2"); // , UNITS_DEGREES_CELSIUS, 0.0 );
#endif

#if ( BACNET_USE_OBJECT_ANALOG_OUTPUT == 1)
    Analog_Output_Create(1, "Ana Output 1"); // , UNITS_DEGREES_CELSIUS, 0.0 );
#endif

#if ( BACNET_USE_OBJECT_ANALOG_VALUE == 1)
    Analog_Value_Create(1, "Ana Value 1", UNITS_PERCENT_RELATIVE_HUMIDITY );

#if ( BACNET_USE_OBJECT_BINARY_INPUT == 1)
    Binary_Input_Create(1, "Bin Input 1");
#endif

#if ( BACNET_USE_OBJECT_BINARY_OUTPUT == 1)
    Binary_Output_Create(1, "Bin Output 1");
#endif

#if ( BACNET_USE_OBJECT_BINARY_VALUE == 1)
    Binary_Value_Create(1, "Bin Value 1");
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_INPUT == 1)
    Multistate_Input_Create(1, "MS Input 1");
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_OUTPUT == 1)
    Multistate_Output_Create(1, "MS Output 1");
#endif

#if ( BACNET_USE_OBJECT_MULTISTATE_VALUE == 1)
    Multistate_Value_Create(1, "MS Value 1");
#endif

#if ( BACNET_USE_OBJECT_CALENDAR == 1 )
    Calendar_Create(1, "Calendar 1");
    Calendar_Create(2, "Calendar 2");
    Calendar_Create(3, "Calendar 3");
#endif 

#if ( BACNET_USE_OBJECT_SCHEDULE == 1 )
    Schedule_Create(1, "Schedule 1");
    Schedule_Create(2, "Schedule 2");
    Schedule_Create(3, "Schedule 3");
#endif

    /* broadcast an I-Am on startup */
    Send_I_Am_Broadcast();


    /* loop forever */
    bool busy = true;
    while (busy) {

        busy &= doUserMenu();

    	// do application stuff
        busy &= TickBACnet();

        /* input */
        current_seconds = time(NULL);

//        /* returns 0 bytes on timeout */
//        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
//
//        /* process */
//        if (pdu_len) {
//            npdu_handler(&src, &Rx_Buf[0], pdu_len);
//        }

        /* at least one second has passed */
        elapsed_seconds = (uint32_t)(current_seconds - last_seconds);
        if (elapsed_seconds) {
            last_seconds = current_seconds;
//            dcc_timer_seconds(elapsed_seconds);

//#if defined(BACDL_BIP) && BBMD_ENABLED
//            bvlc_maintenance_timer(elapsed_seconds);
//#endif
            dlenv_maintenance_timer(elapsed_seconds);

//#if 0 // todo3
//            Load_Control_State_Machine_Handler();
//#endif

            elapsed_milliseconds = elapsed_seconds * 1000;
#if ( BACNET_SVC_COV_B == 1 )
            handler_cov_timer_seconds(elapsed_seconds);
#endif

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
            tsm_timer_milliseconds(elapsed_milliseconds);
#endif

//#if 0 // todo3
//            trend_log_timer(elapsed_seconds);
//#endif
//
//#if (INTRINSIC_REPORTING_B == 1)
//            Device_local_reporting();
//#endif
//
//#if (BACNET_TIME_MASTER == 1)
//            Device_getCurrentDateTime(&bdatetime);
//            handler_timesync_task(&bdatetime);
//#endif
        }

#if ( BACNET_SVC_COV_B == 1 )
        handler_cov_task();
#endif

//        /* scan cache address */
//        address_binding_tmr += elapsed_seconds;
//        if (address_binding_tmr >= 60) {
//            address_cache_timer(address_binding_tmr);
//            address_binding_tmr = 0;
//        }
//#if (INTRINSIC_REPORTING_B == 1 )
//        /* try to find addresses of recipients */
//        recipient_scan_tmr += elapsed_seconds;
//        if (recipient_scan_tmr >= NC_RESCAN_RECIPIENTS_SECS) {
//            Notification_Class_find_recipient();
//            recipient_scan_tmr = 0;
//        }
//#endif
        /* output */

        /* blink LEDs, Turn on or off outputs, etc */
        
        msSleep(10);
    }

#if defined ( _MSC_VER  )
#pragma warning( disable : 4702)    // unreachable code
#endif

    return 0;
}

#endif