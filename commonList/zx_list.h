#ifndef _ZX_LIST_H_
#define _ZX_LIST_H_
#ifdef __cplusplus
extern "C"
{
#endif



struct zxlist_node
{
    struct zxlist_node *next;
    struct zxlist_node *prev;
};

/**
 * ZX_ListNodeToItem - get the struct for this entry
 * @node:	the &struct zxlist_node pointer.
 * @container:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define ZX_ListNodeToItem(node, container, member) \
    (container *) (((char*) (node)) - ((unsigned long) &((container *)0)->member))

#define ZX_ListDeclare(name) \
    struct zxlist_node name = { \
        .next = &name, \
        .prev = &name, \
    }

/**
 * ZX_ListForEach	-	iterate over a list
 * @node:	the &struct zxlist_node to use as a loop cursor.
 * @list:	the head for your list.
 */
#define ZX_ListForEach(node, list) \
    for (node = (list)->next; node != (list); node = node->next)

/**
 * ZX_ListForEachReverse	-	iterate over a list backwards
 * @node:	the &struct zxlist_node to use as a loop cursor.
 * @list:	the head for your list.
 */
#define ZX_ListForEachReverse(node, list) \
    for (node = (list)->prev; node != (list); node = node->prev)

/**
 * ZX_ListForEachSafe - iterate over a list safe against removal of list entry
 * @node:	the &struct zxlist_node to use as a loop cursor.
 * @n:		another &struct zxlist_node to use as temporary storage
 * @list:	the head for your list.
 */
#define ZX_ListForEachSafe(node, n, list) \
    for (node = (list)->next, n = node->next; \
         node != (list); \
         node = n, n = node->next)

static inline void ZX_ListInit(struct zxlist_node *node)
{
    node->next = node;
    node->prev = node;
}
    
/**
 * ZX_ListAddTail - add a new entry
 * @head: list head to add it before
 * @item: new entry to be added
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void ZX_ListAddTail(struct zxlist_node *head, struct zxlist_node *item)
{
    item->next = head;
    item->prev = head->prev;
    head->prev->next = item;
    head->prev = item;
}

 /**
 * ZX_ListAddHead - add a new entry
 * @head: list head to add it after
 * @item: new entry to be added
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void ZX_ListAddHead(struct zxlist_node *head, struct zxlist_node *item)
{
    item->next = head->next;
    item->prev = head;
    head->next->prev = item;
    head->next = item;
}

 /**
 * ZX_ListAppend - let list2(head2) append to the tail of list1(head1)
 */
static inline void ZX_ListAppend(struct zxlist_node *head1, struct zxlist_node *head2)
{
	head2->prev->next = head1;
	head2->next->prev = head1->prev;
	head1->prev->next = head2->next;
	head1->prev = head2->prev;

	head2->prev = head2;
	head2->next = head2;
}

static inline void ZX_ListReverse(struct zxlist_node *head)
{
	struct zxlist_node *pNode = head;
	struct zxlist_node *pTmp = NULL;
	while(pNode->next != head)
	{
		//store the next node
		pTmp = pNode->next;
		//swap
		//pTmp = pNode->next;
		pNode->next = pNode->prev;
		pNode->prev = pTmp;

		pNode = pTmp;
	}
	//last swap
	pTmp = pNode->next;
	pNode->next = pNode->prev;
	pNode->prev = pTmp;
}

/*
 * ZX_ListRemove - delete a list entry
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void ZX_ListRemove(struct zxlist_node *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
}

#define ZX_ListEmpty(list) ((list) == (list)->next)
#define ZX_ListHead(list) ((list)->next)
#define ZX_ListTail(list) ((list)->prev)


#ifdef __cplusplus
}
#endif
#endif //ZX_LIST_H_

