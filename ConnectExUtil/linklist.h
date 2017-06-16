#pragma once

typedef struct _LLIST
{
    struct _LLIST *next;
} LLIST;

void LinkListAppend(void **llistHead, void *item);
void LinkListRemove(void **llistHead, void *target);
void LinkListInsert(void **llistHead, void *target, void *item);
void LinkListPush(void **llistHead, void *item);

