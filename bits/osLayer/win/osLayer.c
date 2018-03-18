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

#include "osLayer.h"

void *bitsCreateThread(bitsThreadVar threadVar, void *(*threadFunc) (void *arg), void *argListPtr)
//void *bitsCreateThread( bitsThreadVar threadVar,  _beginthread_proc_type threadFunc, void *argListPtr)
{
    // second parameter is stack size
    _beginthread((_beginthread_proc_type)threadFunc, 0, argListPtr );
    return NULL;
}

int sys_bits_mutex_lock(bits_mutex_t *mutexName, int ms )
{
    if (*mutexName == 0)
    {
        *mutexName = CreateMutex(NULL, FALSE, NULL);
    }

    return WaitForSingleObject(*mutexName, ms);
}

