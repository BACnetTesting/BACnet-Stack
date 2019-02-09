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

// #include "logging.h"
#include "osLayer.h"
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
// #include <termios.h> /* tcgetattr(), tcsetattr() */
#include <unistd.h> /* read() */
#include <stdlib.h>
#include <memory.h>
// #include "logging.h"

void bitsCreateThread( void (*threadFunc) (void *arg), void *argListPtr)
{
//    int rcode;
//    pthread_t threadvar;
//    rcode = pthread_create(&threadvar,
//        NULL,
//        (void *(*)(void *)) threadFunc,
//        argListPtr);
//    if (rcode != 0) {
//        log_printf("Failed to create thread");
//    }
//    // so we don't have to wait for the thread to complete before exiting main()
//    pthread_detach(threadvar);
}


//static struct termios g_old_kbd_mode;


//static void cooked(void)
//{
//    tcsetattr(0, TCSANOW, &g_old_kbd_mode);
//}


static void raw(void)
{
//    static char init = 0;
//    /**/
//    struct termios new_kbd_mode;
//
//    if (init)
//        return;
//    /* get current state of console */
//    tcgetattr(0, &g_old_kbd_mode);
//    /* put keyboard (stdin, actually) in raw, unbuffered mode */
//    memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
//    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
//    new_kbd_mode.c_cc[VTIME] = 0;
//    new_kbd_mode.c_cc[VMIN] = 1;
//    tcsetattr(0, TCSANOW, &new_kbd_mode);
//    /* when we exit, go back to normal, "cooked" mode */
//    atexit(cooked);
//
//    init = 1;
}


int osKBhit(void)
{
//    struct timeval timeout;
//    fd_set read_handles;
//    int status;
//
//    raw();
//    /* check stdin (fd 0) for activity */
//    FD_ZERO(&read_handles);
//    FD_SET(0, &read_handles);
//    timeout.tv_sec = timeout.tv_usec = 0;
//    status = select(1, &read_handles, NULL, NULL, &timeout);
//    if (status < 0)
//    {
//        log_printf("select() failed in kbhit()");
//        exit(1);
//    }
//    return status;
}


int osGetch(void)
{
//    unsigned char temp;
//
//    raw();
//    /* stdin = fd 0 */
//    if (read(0, &temp, 1) != 1)
//        return 0;
//    return temp;
}


