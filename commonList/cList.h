#ifndef _COMMON_LIST_H
#define _COMMON_LIST_H

#include "list.h"

typedef struct clistManager{
	struct list_node listNode;
	unsigned int u32ListSize;
} *clist;

typedef struct {
	struct list_node listNode;
	void *data;
} clistNode;

clist List_push(clist list, void *x);
clist List_list(void *x, ...);
clist List_append(clist list, clist tail);
clist List_copy(clist list);
clist List_pop(clist list, void **x);
clist List_reverse(clist list);
int List_length(clist list);
void List_free(clist *plist);
void List_map(clist list, void apply(void **x,int index, void *cl), void *cl);
void **List_toArray(clist list, void *end);


#endif
