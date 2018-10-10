/**************************************************************************
*
* Copyright (C) 2016 BACnet Interoperability Testing Services, Inc.
*
*       <info@bac-test.com>
*
* Permission is hereby granted, to whom a copy of this software and
* associated documentation files (the "Software") is provided by BACnet
* Interoperability Testing Services, Inc., to deal in the Software
* without restriction, including without limitation the rights to use,
* copy, modify, merge, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* The software is provided on a non-exclusive basis.
*
* The permission is provided in perpetuity.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#include "logging.h"
#include "osLayer.h"
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <unistd.h> /* read() */
#include <stdlib.h>
#include <memory.h>
#include "logging.h"

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

    raw();
    /* stdin = fd 0 */
    if (read(0, &temp, 1) != 1)
        return 0;
    return temp;
}


