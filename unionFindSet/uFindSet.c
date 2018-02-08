//union-find-set
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "uFindSet.h"

static int group_count=0;

//==============================================================
void createNode(stNode **ppNode,int data)
{
	stNode *p = NULL;
	if(!ppNode)
		return;
	*ppNode = NULL;
	p = (stNode *)malloc(sizeof(stNode));
	if(!p)
		return;
	p->s32Data = data;
	p->s32GroupId = (unsigned long)p;	//每个元素的 组 都是自己,各不一样
	*ppNode = p;
	group_count++;
}

void initNode(stNode *pNode,int data)
{
	if(!pNode)
		return;
	pNode->s32Data = data;
	pNode->s32GroupId = (unsigned long)pNode;
	group_count++;
}

void destroyNode(stNode *pNode)
{
	if(!pNode)
		return;
	free(pNode);
}

unsigned long findGroupId(stNode *pNode)
{
	if(!pNode)
		return -1;
	if( pNode->s32GroupId != (unsigned long)pNode )
	{
		//递归返回的过程会将 group_id 直接指向"组 即自己"的那个元素,下次查找就是O(1)
		pNode->s32GroupId = findGroupId((stNode *)(pNode->s32GroupId));
	}
	return pNode->s32GroupId;
}

/*
//查找元素所属的 组(非递归)
//这么实现没准还真的不如递归实现的好,因为你没有使用栈,导致你压缩的只是查找起点的
//那个元素的 group_id,而上面的递归则是路径上的所有元素都同时被压缩!
unsigned long findGroupId(stNode *pNode)
{
	stNode *tmp = pNode;	//用 tmp 去找掌门人
	if(!pNode)
		return -1;
	while( tmp->s32GroupId != (unsigned long)tmp )
	{
		tmp = (stNode *)(tmp->s32GroupId);
	}
	pNode->s32GroupId = (unsigned long)tmp;
	return pNode->s32GroupId;
}*/

//将x的组id变成了y
void unionNode(stNode *x,stNode *y)
{
	unsigned long g_x = findGroupId(x);
	unsigned long g_y = findGroupId(y);
	if(g_x != g_y)
	{
		//x->s32GroupId = y->s32GroupId;
		((stNode *)g_x)->s32GroupId = g_y;
		group_count--;
	}
}

int getGroupCnt()
{
	return group_count;
}