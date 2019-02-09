/**************************************************************************

Copyright (C) 2018 BACnet Interoperability Testing Services, Inc.

This program is free software : you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.

    For more information : info@bac-test.com
        For access to source code :
                info@bac-test.com
                    or
                www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#include <stdlib.h>     // for memset

#if defined ( _MSC_VER  )
// #include <windows.h>
#include <memory.h>
#include <assert.h>
#endif

#include "llist.h"
#include "osLayer.h"
#include "bitsDebug.h"
#include "bitsUtil.h"

static SemaDefine(llistMutex);

void ll_Init(LLIST_HDR *llhdr, const uint max)
{
    // SemaInit(llistMutex);
    bits_memset(llhdr, 0, sizeof(LLIST_HDR));
    llhdr->max = max;
}


uint ll_GetCount(LLIST_HDR *llhdr)
{
    SemaWait(llistMutex);
    uint r = llhdr->count;
    SemaFree(llistMutex);
    return r;
}


bool ll_Enqueue(LLIST_HDR *llhdr, void *newitem)
{
        //  SemaWait(llistMutex);

    if (llhdr->count >= llhdr->max) {
        // SemaFree(llistMutex);
        // todo3 throw a panic here?
        return false;
    }

    LLIST_LB *newllb = (LLIST_LB *)newitem;
    newllb->next = NULL;

    if (llhdr->count == 0) {
        llhdr->first = newllb;
        llhdr->last = newllb;
    }
    else {
        // enqueue at the end
        llhdr->last->next = newllb;
        llhdr->last = newllb;
    }
    llhdr->count++;
        // SemaFree(llistMutex);
    return true;
}


void* ll_Dequeue(LLIST_HDR *llhdr)
{
        // SemaWait(llistMutex);
    LLIST_LB *firstblk = llhdr->first;

    switch (llhdr->count) {
    case 0:
        // throw panic
         // SemaFree(llistMutex);
        return NULL;

    case 1:
        // only one block, not much to do
        break;

    default:
        // dequeue from the start
        llhdr->first = firstblk->next;
        break;
    }

    llhdr->count--;
        // SemaFree(llistMutex);
    return firstblk;
}


// note, this is not a generic remove, it depends on the header prior pointer. Intended for use by ll_pluck.
// also, there are not mutexes aound the operations.. because ll_pluck already sets those.

static void ll_Remove(LLIST_HDR *llhdr, LLIST_LB *toRemove)
{
    SemaWait(llistMutex);
    if (llhdr->prior == NULL) {
        // we know we are removing the first block.
        llhdr->first = toRemove->next;
    }
    else {
        // we are removing 2... could also be 2nd and last...
        llhdr->prior->next = toRemove->next;
    }
    llhdr->count--;
    SemaFree(llistMutex);
}

void *ll_First(LLIST_HDR *llhdr)
{
    return llhdr->first;
}

void *ll_Next(LLIST_LB *lb)
{
    return lb->next ;
}

void* ll_Pluck(LLIST_HDR *llhdr, void *matchitem, bool(match)(void *listitem, void *matchitem))
{
    // SemaWait(llistMutex);

    llhdr->prior = NULL;

    if (llhdr->count == 0) {
        // dont throw a panic, this will happen often while watching a queue for an item to arrive
         // SemaFree(llistMutex);
        return NULL;
    }

    LLIST_LB *examineblk = llhdr->first;
    do {
        // does this item match?
        if (match(examineblk, matchitem)) {
            // we have one, remove from list and return
            ll_Remove(llhdr, examineblk);
            // SemaFree(llistMutex);
            return examineblk;
        }
        llhdr->prior = examineblk;
        examineblk = examineblk->next;
    } while (examineblk != NULL);


    // SemaFree(llistMutex);
    return examineblk;
}


void* ll_GetPtr(LLIST_HDR *llhdr, const uint index)
{
    SemaWait(llistMutex);

    if (llhdr->count == 0 || index >= llhdr->count ) {
        SemaFree(llistMutex);
        panic();
        return NULL;
    }

    uint count = 0;
    LLIST_LB *examineblk = llhdr->first;
    do {
        if ( count == index ) {
            SemaFree(llistMutex);
            return examineblk;
        }
        examineblk = examineblk->next;
        count++;
    } while ( count < llhdr->count );
    
    panic();
    SemaFree(llistMutex);
    return NULL ;
}

#if 0
typedef struct
{
    LLIST_LB ll_lb;     // must be first
    int data;
}
teststruct;

void *makets(int count)
{
    teststruct *ts = (teststruct *)malloc(sizeof(teststruct));
    ts->data = count;
    return ts;
}

bool compareFunc(void *llitem, void *compareitem)
{
    teststruct *tsa = (teststruct *)llitem;
    teststruct *tsb = (teststruct *)compareitem;

    return (tsa->data == tsb->data);
}

void TestLL(void)
{
    bool rc;
    teststruct *ts;


    LLIST_HDR *hdr = (LLIST_HDR *)malloc(sizeof(LLIST_HDR));

    ll_Init(hdr, 3);

    // test what happens when we want to make 4
    rc = ll_Enqueue(hdr, makets(1));
    assert(rc == true);
    rc = ll_Enqueue(hdr, makets(2));
    assert(rc == true);
    rc = ll_Enqueue(hdr, makets(3));
    assert(rc == true);
    rc = ll_Enqueue(hdr, makets(4));
    assert(rc == false);

    assert(hdr->count == 3);

    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 1);
    assert(hdr->count == 2);
    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 2);
    assert(hdr->count == 1);
    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 3);
    assert(hdr->count == 0);
    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts == NULL);
    assert(hdr->count == 0);


    // find and remove that middle one

    // test what happens when we want to make 4
    rc = ll_Enqueue(hdr, makets(1));
    assert(rc == true);
    rc = ll_Enqueue(hdr, makets(2));
    assert(rc == true);
    rc = ll_Enqueue(hdr, makets(3));
    assert(rc == true);

    assert(hdr->count == 3);

    teststruct *test = (teststruct *)makets(2);
    ts = (teststruct *)ll_Pluck(hdr, test, compareFunc);

    assert(ts->data == 2);
    assert(hdr->count == 2);

    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 1);
    assert(hdr->count == 1);
    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 3);
    assert(hdr->count == 0);


    // find and remove the only one

    // test what happens when we want to make 4
    rc = ll_Enqueue(hdr, makets(2));
    assert(rc == true);
    assert(hdr->count == 1);

    ts = (teststruct *)ll_Pluck(hdr, test, compareFunc);

    assert(ts->data == 2);
    assert(hdr->count == 0);



    // find and remove the second (and last) one

    // test what happens when we want to make 4
    rc = ll_Enqueue(hdr, makets(1));
    assert(rc == true);
    assert(hdr->count == 1);

    rc = ll_Enqueue(hdr, makets(2));
    assert(rc == true);
    assert(hdr->count == 2);

    ts = (teststruct *)ll_Pluck(hdr, test, compareFunc);
    assert(ts->data == 2);
    assert(hdr->count == 1);

    ts = (teststruct *)ll_Dequeue(hdr);
    assert(ts->data == 1);
    assert(hdr->count == 0);

}
#endif
