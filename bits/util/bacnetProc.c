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
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

#include <stdlib.h>
#include <time.h>

#ifdef _MSC_VER
#include <process.h>
#endif

#include "address.h"
#include "datalink.h"
#include "handlers.h"
#include "bitsUtil.h"
#include "dcc.h"
#include "dlenv.h"
#include "logging/logging.h"
#include "net.h"
#include "bitsDebug.h"
#include "bacTarget.h"
#include "nc.h"
#include "tsm.h"

LockDefine(stackLock);

extern PORT_SUPPORT *datalinkSupportHead;

#ifdef _MSC_VER
void BACnetIdle_Thread(void *pArgs)
#else
void* BACnetIdle_Thread(void *pArgs)
#endif
{
    while (true) {
        TickBACnet();
        msSleep(10);
    }
#ifndef _MSC_VER
    return NULL;
#endif
}

static void Init_BACnetIdle_Thread(void)
{

#ifdef _MSC_VER
    uintptr_t rcode;
    rcode = _beginthread(BACnetIdle_Thread, 0, NULL);
    if (rcode == -1L) {
        syslog(LOG_ERR, "Failed to create thread");
    }
#else
    int rcode;
    pthread_t threadvar;
    rcode = pthread_create(&threadvar, NULL,
        (void *(*)(void *)) BACnetIdle_Thread, NULL);
    if (rcode != 0) {
        log_printf("Failed to create thread");
    }
    // so we don't have to wait for the thread to complete before exiting main()
    pthread_detach(threadvar);
#endif

}


void InitBACnet(void)
{
    // no longer Device_Tables_Init();

    /* load any static address bindings to show up
        in our device bindings list */
    address_init();

    // todo - replace atexit(datalink_cleanup);

    LockTransactionInit(stackLock);

// - no longer - this is now called when Port Objects are created
// Init_Datalink_Thread();

    Init_BACnetIdle_Thread();
}


static void TickBACnetDevice(void)
{
    LockTransaction(stackLock);

    /* input */
    time_t current_seconds = time(NULL);
    static time_t last_seconds = current_seconds;
    static uint32_t address_binding_tmr = 0;

#if (INTRINSIC_REPORTING==1)
    static uint32_t recipient_scan_tmr;
#endif

    ///* returns 0 bytes on timeout */
    // uint16_t pdu_len = ourDatalink.ReceiveMPDU(&ourDatalink, &src.srcPath.localMac, &Rx_Buf[0], MAX_LPDU_IP);
    ///* process */
    //if (pdu_len) {
    //    // todo1 src.npdu = Rx_Buf;
    //    // todo1 src.npdu_len = pdu_len;
    //    src.portParams = &ourDatalink;
    //    // todo1 npdu_handler(&src, &ourDevice);
    //}
    /* at least one second has passed */
    uint16_t elapsed_seconds = (uint16_t)(current_seconds - last_seconds);
    if (elapsed_seconds) {

        last_seconds = current_seconds;

        dcc_timer_seconds(elapsed_seconds);

#ifdef todo1 

#if defined(BACDL_BIP) && BBMD_ENABLED
        bvlc_maintenance_timer(elapsed_seconds);
#endif
#endif

        for (PORT_SUPPORT *dl = datalinkSupportHead; dl != NULL; dl = (PORT_SUPPORT *)dl->llist.next) {
            dlenv_maintenance_timer( dl, elapsed_seconds);
        }

#if defined (LOAD_CONTROL)
		Load_Control_State_Machine_Handler();
#endif

#if ( BACNET_SVC_COV_B == 1 )
		handler_cov_timer_seconds(elapsed_seconds);
#endif

#if ( BACNET_CLIENT == 1 ) || ( BACNET_SVC_COV_B == 1 )
		tsm_timer_milliseconds(elapsed_seconds * 1000);
#endif

#ifdef todo2
		trend_log_timer(elapsed_seconds);
#endif

#if (INTRINSIC_REPORTING_B == 1)
	//    // todo1  Device_local_reporting(&ourDevice);
#endif

#if (BACNET_TIME_MASTER)
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
#if (INTRINSIC_REPORTING_B==1)
    /* try to find addresses of recipients */
    recipient_scan_tmr += elapsed_seconds;
    if (recipient_scan_tmr >= NC_RESCAN_RECIPIENTS_SECS) {
        Notification_Class_find_recipient();
        recipient_scan_tmr = 0;
    }
#endif

    UnlockTransaction(stackLock);
}


// If this returns false, time to shutdown
bool TickBACnet(void)
{


    TickBACnetDevice();


    return true;
}

