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

#include <unistd.h>
#include "bitsUtil.h"

void bitsCreateThread( void (*threadFunc) (void *arg), void *argListPtr)
{
    int rcode;
    pthread_t threadvar;
    rcode = pthread_create(&threadvar,
        NULL,
        (void *(*)(void *)) threadFunc,
        argListPtr);
    if (rcode != 0) {
        // log_printf("Failed to create thread");
    }
    // so we don't have to wait for the thread to complete before exiting main()
    pthread_detach(threadvar);
}


void msSleep(uint16_t ms)
{
    sleep(ms);
}


