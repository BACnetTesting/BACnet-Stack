/****************************************************************************************
 *
 *   Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.
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

#include "osLayer.h"
#include "eLib/util/logging.h"
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <unistd.h> /* read() */
#include <stdlib.h>
#include <memory.h>
// duplicate #include "eLib/util/logging.h"
// #include "timerCommon.h"
// #include "bacnet/bits/util/bitsDebug.h"
#include "bacnet/bits/util/bitsIpUtil.h"
#include "bacnet/bits/util/multipleDatalink.h"
// #include "bacnet/datalink/bip.h"

#include <sys/time.h>

void bitsCreateThread( void (*threadFunc) (void *arg), void *argListPtr)
{
    int rcode;
    pthread_t threadvar;
    rcode = pthread_create(&threadvar,
        NULL,
        (void *(*)(void *)) threadFunc,
        argListPtr);
    if (rcode != 0) {
        log_printf("Failed to create thread");
    }
    // so we don't have to wait for the thread to complete before exiting main()
    pthread_detach(threadvar);
}


static time_t oldMultiTime ;

//TimerHandle bits_multiTimer_init(
//                            void)
//{
//    oldMultiTime = bits_sysTimer_get_milliseconds();
//    return 0;
//}


uint32_t bits_timer_delta_time(
    uint32_t prevTime,
    uint32_t newTime )
{
  if ( prevTime <= newTime ) return newTime - prevTime ;
  // we have wrapped
  return (UINT32_MAX - prevTime) + newTime + 1;
}


uint32_t bits_sysTimer_get_milliseconds(
    void )
{
    panic();
    // return (uint32_t) timeGetTime();
}


uint32_t bits_sysTimer_elapsed_milliseconds(
                                         uint32_t prevTime )
{
    return bits_timer_delta_time ( prevTime, bits_sysTimer_get_milliseconds() ) ;
}


//uint32_t bits_multiTimer_elapsed_milliseconds(
//                                         TimerHandle handle )
//{
//    uint32_t newTime = bits_sysTimer_get_milliseconds() ;
//
//    return ( bits_timer_delta_time( oldMultiTime, newTime ) ) ;
//}
//
//
//void bits_multiTimer_reset(
//                      TimerHandle handle )
//{
//  oldMultiTime = bits_sysTimer_get_milliseconds();
//}


void msSleep(uint16_t ms)
{
    // 2019.11.14 - I found usleep deprecated,
    // switching to nanosleep as a result
    // usleep(1000 * ms);
    struct timespec req;
    struct timespec rem;

    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000;

    int rc = nanosleep(&req, &rem);
    if (rc) {
        panic();
    }
}


static struct termios g_old_kbd_mode;


static void cooked(void)
{
    tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}


static void raw(void)
{
    static char init = 0;
    /**/
    struct termios new_kbd_mode;

    if (init)
        return;
    /* get current state of console */
    tcgetattr(0, &g_old_kbd_mode);
    /* put keyboard (stdin, actually) in raw, unbuffered mode */
    memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_kbd_mode);
    /* when we exit, go back to normal, "cooked" mode */
    atexit(cooked);

    init = 1;
}


int osKBhit(void)
{
    struct timeval timeout;
    fd_set read_handles;
    int status;

    raw();
    /* check stdin (fd 0) for activity */
    FD_ZERO(&read_handles);
    FD_SET(0, &read_handles);
    timeout.tv_sec = timeout.tv_usec = 0;
    status = select(1, &read_handles, NULL, NULL, &timeout);
    if (status < 0)
    {
        log_printf("select() failed in kbhit()");
        exit(1);
    }
    return status;
}


int osGetch(void)
{
    unsigned char temp;

    // RPi has issues here...
    // return 0;

    raw();
    /* stdin = fd 0 */
    if (read(0, &temp, 1) != 1)
        return 0;
    return temp;
}


bool bits_get_port_params(
    PORT_SUPPORT* datalink,
    IP_ADDR_PARAMS *params)
{
    IP_ADDR_PARAMS localParams = { 0 };

    int rv = get_local_address_ioctl(datalink->ifName, &localParams.local_address, SIOCGIFADDR);
    if (rv < 0) {
        return false;
    }

    rv = get_local_address_ioctl(datalink->ifName, &localParams.netmask, SIOCGIFNETMASK);
    if (rv < 0) {
        localParams.broadcast_address.s_addr = ~0;
    }
    else {
        localParams.broadcast_address = localParams.local_address;
        localParams.broadcast_address.s_addr |= (~localParams.netmask.s_addr);
    }

    memcpy(params, &localParams, sizeof(IP_ADDR_PARAMS));
    return true ;
}


void bits_set_port_params(
    PORT_SUPPORT *portParams,
    IP_ADDR_PARAMS *portIpParams)
{
    bip_set_addr(portParams, portIpParams->local_address.s_addr);
    bip_set_netmask_addr(portParams, portIpParams->netmask.s_addr);
    bip_set_broadcast_addr(portParams, portIpParams->broadcast_address.s_addr);
    bip_set_subnet_addr(portParams, portIpParams->local_address.s_addr & portIpParams->netmask.s_addr);
}


//bool bits_FileExists( const char *name)
//{
//    struct stat buffer;
//    return (stat(name, &buffer) == 0);
//}