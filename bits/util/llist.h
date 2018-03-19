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
For access to source code : info@bac-test.com
or www.github.com/bacnettesting/bacnet-stack

*********************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _LLIST_LB LLIST_LB;

struct _LLIST_LB
{
	LLIST_LB *next;
} ;

typedef struct 
{
	uint8_t		count;          // don't make 16 bits without protecting access using semaphores!
	uint8_t		max;            // don't make 16 bits without protecting access using semaphores!
	LLIST_LB	*first;
	LLIST_LB	*last;
	LLIST_LB	*prior;
} LLIST_HDR;

void ll_Init(LLIST_HDR *cb, uint8_t max);
int  ll_GetCount ( LLIST_HDR *cb ) ;
bool ll_Enqueue(LLIST_HDR *cb, void *newitem);
void *ll_Dequeue(LLIST_HDR *cb);
void *ll_Pluck(LLIST_HDR *llhdr, void *matchitem, bool (match)(void *listitem, void *matchitem));

