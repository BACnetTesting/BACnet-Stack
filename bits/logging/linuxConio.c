/*
 * linuxConio.c
 *
 *  Created on: Sep 7, 2017
 *      Author: ed
 */

//#include <sys/time.h> /* struct timeval, select() */
///* ICANON, ECHO, TCSANOW, struct termios */
//#include <termios.h> /* tcgetattr(), tcsetattr() */
//#include <unistd.h> /* read() */
//#include <stdlib.h>
//#include <memory.h>
//#include "logging.h"
//
//static struct termios g_old_kbd_mode;
//
///*****************************************************************************
// *****************************************************************************/
//static void cooked(void)
//{
//    tcsetattr(0, TCSANOW, &g_old_kbd_mode);
//}
///*****************************************************************************
// *****************************************************************************/
//static void raw(void)
//{
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
//}
///*****************************************************************************
// *****************************************************************************/
//int bitsKBhit(void)
//{
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
//}
///*****************************************************************************
// *****************************************************************************/
//int bitsGetch(void)
//{
//    unsigned char temp;
//
//    raw();
//    /* stdin = fd 0 */
//    if (read(0, &temp, 1) != 1)
//        return 0;
//    return temp;
//}




