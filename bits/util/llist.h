/**************************************************************************
*
* Copyright (C) 2016 Bacnet Interoperability Testing Services, Inc.
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

