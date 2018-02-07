#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "cList.h"

//将一个数据push到list的head
clist List_push(clist list, void *x)
{
	unsigned int flag=0;
	clist listTmp = NULL;
	clistNode *pNodeTmp = NULL;
	if(!list)
	{	//new malloc a list manager node and init
		listTmp = (clist)malloc(sizeof(struct clistManager));
		if(!listTmp)
			goto FAIL_1;
		ZX_ListInit(&listTmp->listNode);
		listTmp->u32ListSize = 0;
		flag = 1;
	}
	else
		listTmp = list;

	//new list node and init
	pNodeTmp = (clistNode *)malloc(sizeof(clistNode));
	if(!pNodeTmp)
		goto FAIL_2;
	ZX_ListInit(&pNodeTmp->listNode);
	pNodeTmp->data = x;

	//add list node to clist manager(header)
	ZX_ListAddHead(&listTmp->listNode, &pNodeTmp->listNode);
	listTmp->u32ListSize++;
	return listTmp;

FAIL_2:
	if(flag)
		free(listTmp);
FAIL_1:
	return NULL;
}

//构造一个list,应该以NULL或者其他为false的值结尾
clist List_list(void *x, ...)
{
	va_list ap;
	va_start(ap, x);

	clist listTmp = NULL;
	clistNode *pNodeTmp = NULL;
	struct zxlist_node *plistNode = NULL;

	//new malloc a list manager node and init
	listTmp = (clist)malloc(sizeof(struct clistManager));
	if(!listTmp)
		goto FAIL_1;
	ZX_ListInit(&listTmp->listNode);
	listTmp->u32ListSize = 0;

	for ( ; x; x = va_arg(ap, void *))
	{
		//new list node and init
		pNodeTmp = (clistNode *)malloc(sizeof(clistNode));
		if(!pNodeTmp)	// TODO: this may cause memleak
			goto FAIL_2;
		ZX_ListInit(&pNodeTmp->listNode);
		pNodeTmp->data = x;

		ZX_ListAddTail(&listTmp->listNode, &pNodeTmp->listNode);
		listTmp->u32ListSize++;
	}
	va_end(ap);
	return listTmp;

FAIL_2:
	List_free(&listTmp);
FAIL_1:
	return NULL;
}

clist List_append(clist list, clist tail)
{
	if(!list)
		return tail;

	if(tail)
	{
		ZX_ListAppend(&list->listNode, &tail->listNode);
		list->u32ListSize += tail->u32ListSize;
		tail->u32ListSize = 0;
	}
	return list;
}

clist List_copy(clist list)
{
	clist listTmp = NULL;
	clistNode *pNodeTmpSrc = NULL;
	clistNode *pNodeTmpDst = NULL;

	if(list)
	{
		//new malloc a list manager node and init
		listTmp = (clist)malloc(sizeof(struct clistManager));
		if(!listTmp)
			goto FAIL_1;
		ZX_ListInit(&listTmp->listNode);
		listTmp->u32ListSize = 0;

		struct zxlist_node *plistNode = NULL;
		ZX_ListForEach(plistNode,&list->listNode)
		{
			pNodeTmpSrc = ZX_ListNodeToItem(plistNode, clistNode, listNode);
			//new list node and init
			pNodeTmpDst = (clistNode *)malloc(sizeof(clistNode));
			if(!pNodeTmpDst)
				goto FAIL_2;
			ZX_ListInit(&pNodeTmpDst->listNode);
			pNodeTmpDst->data = pNodeTmpSrc->data;
			//insert
			ZX_ListAddTail(&listTmp->listNode, &pNodeTmpDst->listNode);
			listTmp->u32ListSize++;
		}
	}
	return listTmp;

FAIL_2:
	List_free(&listTmp);
FAIL_1:
	return NULL;
}

clist List_pop(clist list, void **x)
{
	struct zxlist_node *plistNode = NULL;
	clistNode *pNodeTmp = NULL;
	if (list && !ZX_ListEmpty(&list->listNode))
	{
		plistNode = ZX_ListHead(&list->listNode);
		pNodeTmp = ZX_ListNodeToItem(plistNode, clistNode, listNode);
		if (x)
			*x = pNodeTmp->data;
		ZX_ListRemove(plistNode);
		free(pNodeTmp);
		list->u32ListSize--;
	}
	return list;
}

//NOTE :param list and returned list both reversed!
clist List_reverse(clist list)
{
	if(list)
		ZX_ListReverse(&list->listNode);
	return list;
}

int List_length(clist list)
{
	int n=0;
	if(list)
		n = list->u32ListSize;
	return n;
}

void List_free(clist *plist)
{
	struct zxlist_node *plistNode = NULL;
	clistNode *pNodeTmp = NULL;
	clist list = NULL;
	if (plist)
	{
		list = *plist;
		if(list)
		{
			while(!ZX_ListEmpty(&list->listNode))
			{
				plistNode = ZX_ListHead(&list->listNode);
				pNodeTmp = ZX_ListNodeToItem(plistNode, clistNode, listNode);
				ZX_ListRemove(plistNode);
				free(pNodeTmp);
				list->u32ListSize--;	
			}
			free(list);
		}
		*plist = NULL;
	}
}

void List_map(clist list, void apply(void **x,int index, void *cl), void *cl)
{
	struct zxlist_node *plistNode = NULL;
	clistNode *pNodeTmp = NULL;
	int i=0;
	if(apply && list)
	{
		ZX_ListForEach(plistNode,&list->listNode)
		{
			//apply 使用的是指向 first 的指针,因此可能改变 first 指针的指向
			pNodeTmp = ZX_ListNodeToItem(plistNode, clistNode, listNode);
			apply(&pNodeTmp->data,i++,cl);	//
		}
	}
}

void **List_toArray(clist list, void *end)
{
	int i=0, n;
	struct zxlist_node *plistNode = NULL;
	clistNode *pNodeTmp = NULL;
	void **array = NULL;
	if(list)
	{
		n = list->u32ListSize;
		array = (void **)malloc((n + 1)*sizeof (void *));
		if(!array)
			return NULL;
		ZX_ListForEach(plistNode,&list->listNode)
		{
			pNodeTmp = ZX_ListNodeToItem(plistNode, clistNode, listNode);
			array[i++] = pNodeTmp->data;
		}
		array[i] = end;	
	}
	return array;
}

