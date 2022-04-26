/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
 *
 *   This program is free software : you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

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

#include "osLayer.h"

#include <stdlib.h>
#include <time.h>

#ifdef _MSC_VER
#include <process.h>
#endif

#include "bacnet/basic/binding/address.h"
// #include "datalink.h"
#include "bacnet/basic/services.h"
#include "bitsUtil.h"
#include "bacnet/dcc.h"
#include "bacnet/datalink/dlenv.h"
#include "eLib/util/logging.h"
#include "bacnet/bits/bitsRouter/bitsRouter.h"
#include "osNet.h"
#include "bacnet/basic/tsm/tsm.h"
#include "eLib/util/eLibDebug.h"
#include "appApi.h"
#include "eLib/util/eLibUtil.h"

#if (INTRINSIC_REPORTING_B==1)
#include "nc.h"
#endif

extern ROUTER_PORT *headRouterPort;
extern PORT_SUPPORT *datalinkSupportHead;

bool bacnetShuttingDown;
static time_t last_seconds ;

bits_mutex_define(bacnetStackMutex);

void bits_Init_Service_Handlers(
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

#if ( BACNET_CLIENT == 1 )
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM,
        handler_i_am_add);
#endif

    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler(
        handler_unrecognized_service);

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

#if 0
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);
#endif

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


extern ROUTER_PORT* applicationRouterPort;

void TickAllDatalinks(void)
{

#if ( BACNET_CLIENT == 1 )
    if (applicationRouterPort && applicationRouterPort->port_support->Address_Cache) {
        address_cache_timer(applicationRouterPort->port_support, 1);
    }
#endif

    PORT_SUPPORT *datalink = datalinkSupportHead;
    while (datalink) {

#if ( BACNET_CLIENT == 1 )
        if (datalink->Address_Cache) {
            address_cache_timer(datalink, 1);
        }
#endif

        if (datalink->Datalink_Maintenance) {
            datalink->Datalink_Maintenance(datalink);
        }
        datalink = (PORT_SUPPORT *)datalink->llist.next;
    }
}


void BACnetIdle_Thread(void *pArgs)
{
    static int countdown = 100;

    while (!bacnetShuttingDown) {

        bits_mutex_lock(bacnetStackMutex);

        TickAllBACnetDevices();

        // every second... tick the datalinks
        countdown--;
        if (countdown <= 0)
        {
            TickAllDatalinks();
            countdown = 100;
        }

        bits_mutex_unlock(bacnetStackMutex);

        msSleep(10);
    }
#ifndef _MSC_VER
    return NULL;
#endif
}

static void Init_BACnetIdle_Thread(void)
{
    bitsCreateThread(BACnetIdle_Thread, NULL);
}


void ExitWarning(void)
{
    printf("BACnet app terminated 6");
}

void ShutdownBACnet(void)
{
    bacnetShuttingDown = true;
}


void InitBACnet(void)
{
    atexit(ExitWarning);

    /* load any static address bindings to show up
        in our device bindings list */
    // needs to be done per datalink now address_init();

    atexit(datalink_cleanup);

    bits_mutex_init(bacnetStackMutex);

#if ( BITS_ROUTER_LAYER == 1)
    RoutingUtilInit();
#endif

    // in routing, tsm_init() happens when we initialize each virtual device
    // tsm_init();
    last_seconds = time(NULL);

    Init_ServerSide();

#if ( BACNET_CLIENT == 1 )
    Init_ClientSide();
#endif

    Init_BACnetIdle_Thread();

}


// this gets called continuously, but every now and then there will be a second passed
static void TickBACnetDevice(DEVICE_OBJECT_DATA * pDev, uint elapsed_seconds)
{
    // If the device is virtual, we need to prepare the datalink with the device's MAC address for internal routing
    VirtualDeviceInfo *vDev = (VirtualDeviceInfo *)pDev->userData;
    if (vDev)
    {
        pDev->datalink->localMAC = &vDev->virtualMACaddr;
    }

    /* at least one second has passed */
    if (elapsed_seconds)
    {

        dcc_timer_seconds(pDev, elapsed_seconds);

#if defined (LOAD_CONTROL)
        Load_Control_State_Machine_Handler();
#endif

#if ( BACNET_SVC_COV_B == 1 )
        handler_cov_timer_seconds(pDev, elapsed_seconds);
#endif

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
        tsm_timer_milliseconds(pDev, elapsed_seconds * 1000);
#endif

#if ( BACNET_USE_OBJECT_TRENDLOG == 1 )
        trend_log_timer(pDev, elapsed_seconds);
#endif

#if (INTRINSIC_REPORTING_B == 1)
        Device_local_reporting(pDev);
#endif

#if (BACNET_TIME_MASTER == 1)
        Device_getCurrentDateTime(&bdatetime);
        handler_timesync_task(&bdatetime);
#endif
    }


    // the rest of the code runs a quickly as possible (not just once per second)

#if ( BACNET_SVC_COV_B == 1 )
    handler_cov_task(pDev);
#endif

#if (INTRINSIC_REPORTING_B==1)
    static uint32_t recipient_scan_tmr;
    recipient_scan_tmr += elapsed_seconds;
    if (recipient_scan_tmr >= NC_RESCAN_RECIPIENTS_SECS) {
        Notification_Class_find_recipient_Task(pDev);
        recipient_scan_tmr = 0;
    }
#endif

}


extern LLIST_HDR serverDeviceCB;

// If this returns false, time to shutdown
bool TickAllBACnetDevices(void)
{
    time_t current_seconds = time(NULL);

    DEVICE_OBJECT_DATA *pDev;

    uint elapsed_seconds = (uint)(current_seconds - last_seconds);
    last_seconds = current_seconds;

    // todo 2 - check for recursive locks - linux and win differ here, although linux has a special recursive locking option
    // we should not have to depend on that.
    bits_mutex_lock(bacnetStackMutex);

    pDev = (DEVICE_OBJECT_DATA *)ll_First(&serverDeviceCB);
    while (pDev != NULL)
    {
        TickBACnetDevice(pDev, elapsed_seconds);
        pDev = (DEVICE_OBJECT_DATA *)ll_Next(&serverDeviceCB, pDev);
    }

    bits_mutex_unlock(bacnetStackMutex);

    // we will continue to use a return code - one day a BACnet client will request a restart, and we want to be able
    // to handle that.
    return true;
}

