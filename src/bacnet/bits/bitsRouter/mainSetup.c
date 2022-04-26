/****************************************************************************************
 *
 *   Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *   As a special exception, if other files instantiate templates or
 *   use macros or inline functions from this file, or you compile
 *   this file and link it with other works to produce a work based
 *   on this file, this file does not by itself cause the resulting
 *   work to be covered by the GNU General Public License. However
 *   the source code for this file must still be made available in
 *   accordance with section (3) of the GNU General Public License.

 *
 *   For more information : info@bac-test.com
 *
 *   For access to source code :
 *
 *       info@bac-test.com
 *           or
 *       www.github.com/bacnettesting/bacnet-stack
 *
 ****************************************************************************************/

#ifdef _MSC_VER
//#include <conio.h>
#include <direct.h>
#endif

#if defined(__GNUC__)
#include "osLayer.h"
#endif

#include "configProj.h"
#include "eLib/util/btaDebug.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/bits/util/bitsUtil.h"
#include "eLib/util/logging.h"
#include "osLayer.h"
#include "bacnet/basic/services.h"


/*  Punchlist for Open Source BACnet Reference Stack at www.github.com/bacnettesting/bacnet-stack
    =============================================================================================

    incoming update parameter is being parsed, next:
        add create virtual router, device, object
        test parsed data store to bacnet object
    excise gw_device.c and all those other gateway parasites

    --------------------------------------------------------------------------

    2016.10.27 need to put semaphores around stack - two threads now use it

    Network number is, what is network number on startup?
    check local router table

 */

/*      Project Readme

    File structure
    ==============
        dev                 All source code
        dev/bacnet          All original Karg FOSS (Free Open Source Software) code
        dev/bacnet/bits     All BACnet Interoperability Testing Services, Inc. FOSS code

    Test framework
    ==============
        MSVC project    Source files        Description
        ------------    ------------        -----------
        testJSON                            Standalone sanity test for cJSON library.
        testMQTTsub                         Standalone sanity test for mosquitto_sub utility
        server                              Simply to test FOSS BACnet stack

 */

// extern DEVICE_OBJECT_DATA  applicationDevice;   // todo3 rename to 'routerApplicationEntity' something.

DEVICE_OBJECT_DATA virtualDevice1;
DEVICE_OBJECT_DATA virtualDevice2;
DEVICE_OBJECT_DATA virtualDevice3;

extern PORT_SUPPORT *applicationDatalink ;      // Where we put the router Application Entity device
extern PORT_SUPPORT *virtualDatalink;           // Where we put the virtual devices


#ifdef _MSC_VER
HANDLE mutexRingbuf;
// HANDLE mutexStuffingRingbuf;
HANDLE mutexLon;
extern HANDLE mutexDnet;
#else
// pthread_mutex_t mutexRingbuf = PTHREAD_MUTEX_INITIALIZER;
LockExtern(mutexRingbuf);
LockExtern(mutexDnet);
// pthread_mutex_t mutexStuffingRingbuf = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t mutexLon = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t mutexDnet = PTHREAD_MUTEX_INITIALIZER;
#endif


// ConfigType config;

static void Init_Service_Handlers(
    void)
{
    // now in Device_Init Device_Tables_Init(NULL);

    /* we need to handle who-is to support dynamic device binding
     * For the gateway, we will use the unicast variety so we can
     * get back through switches to different subnets.
     */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS,
        handler_who_is_unicast);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS,
        handler_who_has);

#if 0
    /*  BACnet Testing Observed Incident oi00107
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
#if 0
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
#endif

    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
#if defined (BAC_WPM)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
        handler_write_property_multiple);
#endif

#if ( BACNET_SVC_RR_B)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range);
#endif

#if (BACFILE)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_READ_FILE,
        handler_atomic_read_file);

    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ATOMIC_WRITE_FILE,
        handler_atomic_write_file);
#endif
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);

#if ( BACTIME )
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
        handler_timesync_utc);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
        handler_timesync);
#endif

#if ( BACNET_SVC_COV_B == 1 ) // Feedback karg
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
        handler_cov_subscribe);
#endif

#if ( BACNET_SVC_COV_A == 1 ) // Feedback karg
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_COV_NOTIFICATION,
        handler_ucov_notification);
#endif

    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* handle the data coming back from private requests */
#if ( BACNET_SVC_PRIVATE_TRANSFER )
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
#endif

#if (INTRINSIC_REPORTING==1)
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM,
        handler_alarm_ack);

    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_EVENT_INFORMATION,
        handler_get_event_information);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_ALARM_SUMMARY,
        handler_get_alarm_summary);
#endif // defined(INTRINSIC_REPORTING)

#if defined(BACNET_TIME_MASTER)
    handler_timesync_init();
#endif
    // apdu_set_confirmed_handler(SERVICE_CONFIRMED_ADD_LIST_ELEMENT,
    //    handler_add_list_element);
    // apdu_set_confirmed_handler(SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT,
    //    handler_remove_list_element);
}


// void showrouterports(void);
// void showDatalinks(void);

#if 0
void LoadRouterPortsManually(void)
{
    bool ok = true;
    if (config.localBACnetPort) {
        log_printf("BACnet Port          %u, Network Number %u", config.localBACnetPort, config.localNetworkNumber);
        ok = InitRouterport(BPT_BIP, config.localNetworkNumber, config.localBACnetPort);
        if (!ok) return -1;
    }

    // showrouterports();
    // showDatalinks();
    // printf("\n");

    if(config.localBBMDbacnetPort)
    {
        log_printf("BACnet BBMD Port     %u, Network Number %u", config.localBBMDbacnetPort, config.localBBMDnetworkNumber);
        // ok = InitRouterport(BPT_BBMD, config.localBBMDnetworkNumber, config.localBBMDbacnetPort);
        if(!ok) return - 1;
    }

    // showrouterports();
    // showDatalinks();
    // printf("\n");

    if(config.globalBACnetPort)
    {
        config.natRouting = true;
        log_printf("BACnet NAT BBMD Port %u, Network Number %u", config.globalBACnetPort, config.globalNetworkNumber);
        // ok &= InitRouterportWithNAT(BPT_BBMD, config.globalNetworkNumber, config.globalBACnetPort, &config.globalIPEP);
        if(!ok) return - 1;
    }

#if 0
    showrouterports();
    showDatalinks();
    printf("\n");
#endif

    if (config.virtualNetworkNumber) {
        log_printf("Virtual Network             Network Number %u", config.virtualNetworkNumber);
        ok &= InitRouterportVirtual(config.virtualNetworkNumber);
        if (!ok) return -1;

    }

    // initialize the application datalink, and associate it with one of the above via Network Number
    ok = InitRouterportApp(config.localNetworkNumber);      // Network Number to 'associate' with.
                                                            /// ok = InitRouterportApp(config.localBBMDnetworkNumber);      // Network Number to 'associate' with.
    if(!ok) return - 1;
}
#endif

void BACnetSetup(int argc, char **argv)
{
    const char *ourName = "BBRS - Full Routing C++ - v" BACNET_VERSION_TEXT;

#ifdef _MSC_VER
    char cwd[MAX_PATH];
    _getcwd(cwd, MAX_PATH);
#else
    char *cwd = getcwd(NULL, 0);
#endif
     
    log_printf("Running [%s]", argv[0]);
    log_printf("In dir  [%s]", cwd);
    log_printf("App     [%s%s]", ourName, " - v" BACNET_VERSION_TEXT);

#ifndef _MSC_VER
    free(cwd);
#endif

    SendBTAstartMessage(ourName);

#if ( BITS_USE_IPC )
    ipc_init_BACnet();
#endif

    // SetConfigDefaults(&config);

    Init_Service_Handlers();

    InitBACnet();

#if (BAC_DEBUG == 1)
    CreateTestConfiguration();
#endif

    AnnounceDevices();

    ShowRouterports();
}



