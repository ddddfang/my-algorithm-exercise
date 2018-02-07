#include <stdio.h>
#include "cList.h"

static void show(void **x,int index, void *cl)
{
	//char *pStr = *x;
	//printf("emms,%s\n",pStr);
	int i = (int)*x;
	printf("elem[%d],%d\n",index,i);	
}

static void showStr(void **x,int index, void *cl)
{
	char *pStr = *x;
	if(cl)
		printf("%p,",cl);
	printf("elem[%d],%s\n",index,pStr);
}

static void showList(void **x,int index, void *cl)
{
	clist li = *x;
	List_map(li, showStr, index);	//index shows this is no.x list
}

static void freeList(void **x,int index, void *cl)
{
	clist li = *x;
	List_free(&li);
}


///////////////////////////////////////
int commonList_test1(void)
{
	int i=0;
	clist li = List_push(NULL,(void *)0);
	for(i=1;i<1000;i++)
	{
		li = List_push(li, (void *)i);
	}

	printf("this list is like this (have %d elements):\n",List_length(li));
	List_map(li, show, NULL);
	List_reverse(li);
	printf("after reverse ,this list is like this :\n");
	List_map(li, show, NULL);

	
	void *tmp=NULL;
	for(i=0;i<600;i++)
	{
		li = List_pop(li, &tmp);
		printf("pop %d\n",(int)tmp);
	}
	printf("after pop ,this list is like this (remainning %d elements):\n",List_length(li));
	List_map(li, show, NULL);
	
	
	
	
	clist li2 = List_list((void *)123,(void *)456,(void *)789,NULL);
	printf("No.2 list is like this :\n");
	List_map(li2, show, NULL);
	
	
	li = List_append(li, li2);
	printf("after append, li look like this,(%d  elements) :\n",List_length(li));
	List_map(li, show, NULL);
	
	
	
	
	void **a = NULL;
	a = List_toArray(li, NULL);
	printf("after convert to an array\n");
	for(i=0;i<List_length(li);i++)
	{
		printf("%d\n",(int)a[i]);
	}
	free(a);
	
	
	List_free(&li);
	List_free(&li2);
	return 0;
}

int commonList_test2()
{
	printf("list of list test\n");
	//list of list
	clist li3 = List_list(	(void *)List_list("hello",NULL),
							(void *)List_list("tom",NULL),
							(void *)List_list("list","of","list",NULL),
							(void *)List_list("haha",NULL),
							NULL);
	List_map(li3, showList, NULL);
	List_map(li3, freeList, NULL);	//you MUST free sub list first! otherwise memleak will occurs
	
	List_free(&li3);
}

int commonList_test3()
{
	int i=0;
	clist li = List_push(NULL,(void *)0);
	for(i=1;i<50;i++)
	{
		li = List_push(li, (void *)i);
	}
	printf("this list is like this (have %d elements):\n",List_length(li));
	List_map(li, show, NULL);
	
	clist li2 = List_reverse(List_copy(li));
	
	printf("after reverse ,this origin list like :\n");
	List_map(li, show, NULL);
	printf("after reverse ,No.2 list like :\n");
	List_map(li2, show, NULL);
	
	List_free(&li);
	List_free(&li2);
}
