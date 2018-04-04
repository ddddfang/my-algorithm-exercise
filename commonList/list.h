#ifndef _LIST_H_
#define _LIST_H_
#ifdef __cplusplus
extern "C"
{
#endif



struct list_node
{
	struct list_node *next;
	struct list_node *prev;
};

/**
 * ListNodeToItem - get the struct for this entry
 * @node:	the &struct list_node pointer.
 * @container:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define ListNodeToItem(node, container, member) \
	(container *) (((char*) (node)) - ((unsigned long) &((container *)0)->member))

#define ListDeclare(name) \
	struct list_node name = { \
		.next = &name, \
		.prev = &name, \
	}

/**
 * ListForEach	-	iterate over a list
 * @node:	the &struct list_node to use as a loop cursor.
 * @list:	the head for your list.
 */
#define ListForEach(node, list) \
	for (node = (list)->next; node != (list); node = node->next)

/**
 * ListForEachReverse	-	iterate over a list backwards
 * @node:	the &struct list_node to use as a loop cursor.
 * @list:	the head for your list.
 */
#define ListForEachReverse(node, list) \
	for (node = (list)->prev; node != (list); node = node->prev)

/**
 * ListForEachSafe - iterate over a list safe against removal of list entry
 * @node:	the &struct list_node to use as a loop cursor.
 * @n:		another &struct list_node to use as temporary storage
 * @list:	the head for your list.
 */
#define ListForEachSafe(node, n, list) \
	for (node = (list)->next, n = node->next; \
		 node != (list); \
		 node = n, n = node->next)

static inline void ListInit(struct list_node *node)
{
	node->next = node;
	node->prev = node;
}
    
/**
 * ListAddTail - add a new entry
 * @head: list head to add it before
 * @item: new entry to be added
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void ListAddTail(struct list_node *head, struct list_node *item)
{
	item->next = head;
	item->prev = head->prev;
	head->prev->next = item;
	head->prev = item;
}

 /**
 * ListAddHead - add a new entry
 * @head: list head to add it after
 * @item: new entry to be added
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void ListAddHead(struct list_node *head, struct list_node *item)
{
	item->next = head->next;
	item->prev = head;
	head->next->prev = item;
	head->next = item;
}

 /**
 * ListAppend - let list2(head2) append to the tail of list1(head1)
 */
static inline void ListAppend(struct list_node *head1, struct list_node *head2)
{
	head2->prev->next = head1;
	head2->next->prev = head1->prev;
	head1->prev->next = head2->next;
	head1->prev = head2->prev;

	head2->prev = head2;
	head2->next = head2;
}

static inline void ListReverse(struct list_node *head)
{
	struct list_node *pNode = head;
	struct list_node *pTmp = NULL;
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
static inline void ListRemove(struct list_node *item)
{
	item->next->prev = item->prev;
	item->prev->next = item->next;
}

#define ListIsEmpty(list) ((list) == (list)->next)
#define ListHead(list) ((list)->next)
#define ListTail(list) ((list)->prev)


#ifdef __cplusplus
}
#endif
#endif //LIST_H_

