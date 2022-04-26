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

#if defined ( _MSC_VER  )
// #include <windows.h>
#include <memory.h>
#include <assert.h>
#endif

#include "llist.h"
#include "osLayer.h"
#include "bitsDebug.h"
#include "bitsUtil.h"

static bits_mutex_define(llistMutex);

#if ( LL_DEBUG == 1 )
void ll_Init(LLIST_HDR *llhdr, const uint max, const char *name )
#else
void ll_Init(LLIST_HDR *llhdr, const uint max )
#endif
{
    bits_mutex_init(llistMutex);
    bits_memset(llhdr, 0, sizeof(LLIST_HDR));
    llhdr->max = max;

#if ( LL_DEBUG == 1 )
    bits_strlcpy(llhdr->name, name, MX_QUEUE_NAME );
#endif

#if ( BAC_DEBUG == 1 )
    llhdr->signature = 'i';
#endif

}


uint ll_GetCount(LLIST_HDR *llhdr)
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return 0;
    }
#endif

    bits_mutex_lock(llistMutex);
    uint r = llhdr->count;
    bits_mutex_unlock(llistMutex);
    return r;
}


bool ll_Enqueue(LLIST_HDR *llhdr, void *newitem)
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return false;
    }
#endif

    bits_mutex_lock(llistMutex);

    if (llhdr->count >= llhdr->max) {
        bits_mutex_unlock(llistMutex);
        // todo3 throw a panic here?
        return false;
    }

    LLIST_LB *newllb = (LLIST_LB *)newitem;
    newllb->next = NULL;

    if (llhdr->count == 0) {
        llhdr->first = newllb;
    }
    else {
        // enqueue at the end
        llhdr->last->next = newllb;
    }

    llhdr->last = newllb;
    llhdr->count++;
    if (llhdr->count > llhdr->max_depth) llhdr->max_depth = llhdr->count;

    bits_mutex_unlock(llistMutex);
    return true;
}


void* ll_Dequeue(LLIST_HDR *llhdr)
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return NULL;
    }
#endif

    bits_mutex_lock(llistMutex);
    LLIST_LB *firstblk = llhdr->first;

    switch (llhdr->count) {
    case 0:
        // throw panic
        bits_mutex_unlock(llistMutex);
        return NULL;

    case 1:
        // only one block
        llhdr->first = NULL;
        break;

    default:
        // dequeue from the start
        llhdr->first = firstblk->next;
        break;
    }

    llhdr->count--;
    bits_mutex_unlock(llistMutex);
    return firstblk;
}


// note, this is not a generic remove, it depends on the header prior pointer. Intended for use by ll_pluck.
// also, there are not mutexes aound the operations.. because ll_pluck already sets those.

static void ll_Remove(LLIST_HDR *llhdr, LLIST_LB *toRemove)
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return ;
    }
#endif

    bits_mutex_lock(llistMutex);
    if (llhdr->prior == NULL) {
        // we know we are removing the first block.
        llhdr->first = toRemove->next;
    }
    else {
        // we are removing 2... could also be 2nd and last...
        llhdr->prior->next = toRemove->next;
    }
    llhdr->count--;
    bits_mutex_unlock(llistMutex);
}


// If these are ever re-instated they need critical sections...
//void *ll_First(LLIST_HDR *llhdr)
//{
//    return llhdr->first;
//}
//
//
//void *ll_Next(LLIST_LB *lb)
//{
//    return lb->next ;
//}

void ll_TransferAtLeast(LLIST_HDR *destQueue, LLIST_HDR *srcQueue, uint count) 
{
    bits_mutex_lock(llistMutex);

    for (uint i = 0; i < count; i++) {

        if (destQueue->count == destQueue->max) {
#if ( LL_DEBUG == 1 )
            dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Destination queue [%s] overrun.", destQueue->name);
#else
            dbTraffic(DBD_ALL, DB_UNEXPECTED_ERROR, "Destination queue overrun." );
#endif
            bits_mutex_unlock(llistMutex);
            return;
        }

        if (srcQueue->count == 0) {
            // we are done 
            bits_mutex_unlock(llistMutex);
            return;
        }

        void *orig = ll_Dequeue(srcQueue);
        ll_Enqueue(destQueue, orig );
    }
    bits_mutex_unlock(llistMutex);
}


void *ll_Find(LLIST_HDR *llhdr, void *matchitem, bool(match)(void *listitem, void *matchitem))
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return NULL;
    }
#endif

    bits_mutex_lock(llistMutex);
    llhdr->prior = NULL;

    if (llhdr->count == 0) {
        // Nothing to find
        bits_mutex_unlock(llistMutex);
        return NULL;
    }

    LLIST_LB *examineblk = llhdr->first;
    do {
        // does this item match?
        if (match(examineblk, matchitem)) {
            bits_mutex_unlock(llistMutex);
            return examineblk;
        }
        llhdr->prior = examineblk;
        examineblk = examineblk->next;
    } while (examineblk != NULL);

    bits_mutex_unlock(llistMutex);
    return NULL ;
}


void *ll_Pluck(LLIST_HDR *llhdr, void *matchitem, bool(match)(void *listitem, void *matchitem))
{
#if ( BAC_DEBUG == 1 )
    if (llhdr->signature != 'i') {
        panic();
        return NULL;
    }
#endif

    bits_mutex_lock(llistMutex);

    LLIST_LB *examineblk = (LLIST_LB *) ll_Find(llhdr, matchitem, match);
    if ( examineblk != NULL ) {
            ll_Remove(llhdr, examineblk);
        }

    bits_mutex_unlock(llistMutex);
    return examineblk;
}


void *ll_GetPtr(LLIST_HDR *llhdr, const uint index)
{
    bits_mutex_lock(llistMutex);

    if (llhdr->count == 0 || index >= llhdr->count ) {
        bits_mutex_unlock(llistMutex);
        panic();
        return NULL;
    }

    uint count = 0;
    LLIST_LB *examineblk = llhdr->first;
    do {
        if ( count == index ) {
            bits_mutex_unlock(llistMutex);
            return examineblk;
        }
        examineblk = examineblk->next;
        count++;
    } while ( count < llhdr->count );
    
    panic();
    bits_mutex_unlock(llistMutex);
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
