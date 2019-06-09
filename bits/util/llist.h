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
*   For more information : info@bac-test.com
*
*   For access to source code :
*
*       info@bac-test.com
*           or
*       www.github.com/bacnettesting/bacnet-stack
*
****************************************************************************************/

// todo, this should be called 'queue' and differentiated from generic 'link list'
// it has some overheads pertaining to message queues to increase performance
// if you want a linked list, use linklist.c (which is a bit of a hack) (requires the first item to be llcb)

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "osLayer.h"

typedef struct _LLIST_LB LLIST_LB;  // Old name
typedef struct _LLIST_LB QUEUE_LB;  // New name, use it.

struct _LLIST_LB
{
	LLIST_LB *next;
} ;

typedef struct 
{
	uint		count;          // turns out ARM uint8_t operations are not atomic. (confirmation required)
	uint		max;            // regardless, these are now fully protected, so can be any size.
	LLIST_LB	*first;
	LLIST_LB	*last;
	LLIST_LB	*prior;
} LLIST_HDR, QUEUE_HDR;

void    ll_Init(LLIST_HDR *cb, const uint max);
uint    ll_GetCount(LLIST_HDR *cb ) ;
bool    ll_Enqueue(LLIST_HDR *cb, void *newitem);
void   *ll_Dequeue(LLIST_HDR *cb);
void   *ll_Pluck(LLIST_HDR *llhdr, void *matchitem, bool (match)(void *listitem, void *matchitem));

// gets a pointer to the nth item in the list - This pointer will no longer be protected by a critical section !!!
void   *ll_GetPtr(LLIST_HDR *llhdr, const uint index);

void Queue_Init(QUEUE_HDR *cb, uint8_t max);
int  Queue_GetCount(QUEUE_HDR *cb);
bool Queue_Enqueue(QUEUE_HDR *cb, void *newitem);
void *Queue_Dequeue(QUEUE_HDR *cb);
void *Queue_Pluck(QUEUE_HDR *llhdr, void *matchitem, bool (match)(void *listitem, void *matchitem));
