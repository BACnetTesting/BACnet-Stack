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

