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


#include <stdlib.h>

#include "linklist.h"
#include "debug.h"
#include "emm.h"
#include "logging.h"
#include "bitsDebug.h"

void LinkListPush(void **llistHead, void *item)
{
    ((LLIST *)item)->next = (LLIST *)*llistHead;
    *llistHead = item;
}


void LinkListAppend(void **llistHead, void *item)
{
    LLIST *tmp;
    // make sure we are terminated
    ((LLIST *)item)->next = NULL;
    if ((LLIST *)*llistHead == NULL)
    {
        *llistHead = item;
        return;
    }
    // else, find the end of the LLIST
    tmp = (LLIST *)*llistHead;
    while (tmp->next)
    {
        tmp = tmp->next;
    }
    tmp->next = (LLIST *)item;
}

void LinkListInsert(void **llistHead, void *target, void *item)
{
    LLIST *tmp;
    // make sure we are terminated
    ((LLIST *)item)->next = NULL;

    // is the Link List empty? Cannot "insert".
    if (*llistHead == NULL)
    {
        panicDesc("Attempt to insert into empty linked list");
        return;
    }

    // are we inserting before the first item?
    if (*llistHead == target)
    {
        // yes
        ((LLIST *)item)->next = (LLIST *)*llistHead;
        *llistHead = item;
        return;
    }

    // else, find the item of interest
    tmp = (LLIST *)*llistHead;
    do
    {
        if (tmp->next == target)
        {
            // want to insert before tmp->next (and it will never be the first, (taken care of above))
            ((LLIST *)item)->next = tmp->next;
            tmp->next = (LLIST *)item;
            return;
        }
        tmp = tmp->next;
    } while (tmp);
    panicDesc("Failed to insert into linked list");
}


void LinkListRemove(void **llistHead, void *item)
{
    LLIST *tmp;

    // two cases i) item is first on the list, ii) item is middle or last on the list
    // Case i)
    if (*llistHead == item)
    {
        *llistHead = ((LLIST *)item)->next;
        emm_free(item);
        return;
    }
    // else case ii)
    tmp = ((LLIST *)*llistHead);
    do
    {
        // if the next item is a match, we want to remove the next item
        if (tmp->next == item)
        {
            LLIST *anotherTemp = tmp->next;
            tmp->next = ((LLIST *)item)->next;
            emm_free(anotherTemp);
            return;
        }
        // not this one, move on
        tmp = tmp->next;
    } while (tmp);
    // if we get here, item was not found. we should warn..
    panicDesc("LList item not found");
}
