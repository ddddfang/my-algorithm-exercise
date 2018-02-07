//感觉就是一个使用树来实现的键值对集合了
//不过仔细想想,之前树的那些操作确实可以使用键值对来实现
//首先插入 num 自动实现排序其实就是(k=num,v=null)
//根据某些 num/string 得到合适的结构体 就是(k=num/string,v=void *)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "commonTree.h"

//默认是按照 kdata 指针值排序(即key的地址值)
static int cmp_default(const void *p1,const void *p2)
{
	return ((unsigned int)p1 - (unsigned int)p2);
}

cTree cTreeCreate(int (*cmp)(const void *p1,const void *p2))
{
	cTree p = (cTreeManager *)malloc(sizeof(cTreeManager));
	if(!p)
		return NULL;
	p->total = 0;
	p->root = RB_ROOT;	//init rb tree
	p->cmp = (cmp == NULL) ? cmp_default : cmp;		//init rb tree
	return p;
}

cTreeNodeId cTreeSearch(cTree ctree,void *kdata)
{
	cTreeNode *pTreeNode = NULL;
	int cmpRes = 0;
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = pRoot->rb_node;	//the first node

	while(pNode != NULL)
	{
		pTreeNode = container_of(pNode,cTreeNode,node);
		cmpRes = (*(ctree->cmp))(kdata,pTreeNode->kdata);
		if(cmpRes < 0)
		{
			pNode = pNode->rb_left;
		}
		else if(cmpRes > 0)
		{
			pNode = pNode->rb_right;
		}
		else
		{
			return pTreeNode;	//cTreeNodeId 就是一个 cTreeNode*
		}
	}
	return NULL;
}

void *getKdataFromNodeId(cTreeNodeId id)
{
	return (id->kdata);
}
void *getVdataFromNodeId(cTreeNodeId id)
{
	return (id->vdata);
}

void *cTreeInsert(cTree ctree,void *kdata,unsigned int kdataSize,void *vdata)
{
	void *oldVdata = NULL;
	cTreeNode *pTreeNode = NULL;
	int cmpRes = 0;

	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node **ppNode = &(pRoot->rb_node);
	struct rb_node *pParent = NULL;

	while(*ppNode)
	{
		pTreeNode = container_of(*ppNode,cTreeNode,node);
		pParent = *ppNode;
		cmpRes = (*(ctree->cmp))(kdata,pTreeNode->kdata);
		if(cmpRes < 0)
		{
			ppNode = &(*ppNode)->rb_left;
		}
		else if(cmpRes > 0)
		{
			ppNode = &(*ppNode)->rb_right;
		}
		else
		{
			//如果有相同的 key 要插入,返回之前的old vdata,用户 MUST 进行必要的 释放 操作 !!!
			//我们使用的 kdata 有没有可能存在泄露? 
			//case.1: kdata 通过malloc分配的,那么更新后的 pTreeNode->kdata 仍然指向他,没有丢失
			//case.2: kdata 是vdata指向结构体的某个成员,那么更新后的 pTreeNode->kdata 地址失效
			//这样太复杂了,关键是不可控制,因此我们无论如何自己保存一份kdata!,这样的话,这里的kdata无论无何都是有效的(只是我们不必再memcpy一次了)
			oldVdata = pTreeNode->vdata;
			pTreeNode->vdata = vdata;
			return oldVdata;
		}
	}
	//malloc a tree node for (k,v)
	cTreeNode *pTargetTreeNode = (cTreeNode *)malloc(sizeof(cTreeNode)+kdataSize+1);
	if(!pTargetTreeNode)
		return NULL;
	pTargetTreeNode->kdata = (void *)(pTargetTreeNode+1);
	memset(pTargetTreeNode->kdata,0,kdataSize+1);
	memcpy(pTargetTreeNode->kdata,kdata,kdataSize);	//我们默认 kdata 指向的地址是一个有效的值!! 不允许 "(void *)i" 这样的玩法!!
	pTargetTreeNode->vdata = vdata;

	rb_link_node(&pTargetTreeNode->node, pParent,ppNode);	//first, link this node to rbTree
	rb_insert_color(&pTargetTreeNode->node, pRoot);			//rebalance

	ctree->total++;
	return NULL;
}

//这个只是删除(释放) cTreeNode,其 vdata 指向的内存需要自己来释放!
void *cTreeEraseFromNodeId(cTree ctree,cTreeNodeId *pid)
{
	void *oldVdata = NULL;
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	if(!pid || !(*pid))
		return NULL;
	cTreeNode *pTreeNode = *pid;

	oldVdata = pTreeNode->vdata;
	rb_erase(&pTreeNode->node,pRoot);		//just remove from rbtree relationship
	free(pTreeNode);	//if pTreeNode->pdata pointer to a struct ,you MUST free yourself !!		
	ctree->total--;
	*pid = NULL;
	return oldVdata;	//留待用户释放的 vdata (如有必要释放)
}

//equals to "search and erase by id" operation
void *cTreeErase(cTree ctree,void *kdata)
{
	void *oldVdata = NULL;
	cTreeNode *pTreeNode = NULL;
	int cmpRes = 0;
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = pRoot->rb_node;	//the first node

	while(pNode != NULL)
	{
		pTreeNode = container_of(pNode,cTreeNode,node);
		cmpRes = (*(ctree->cmp))(kdata,pTreeNode->kdata);
		if(cmpRes < 0)
		{
			pNode = pNode->rb_left;
		}
		else if(cmpRes > 0)
		{
			pNode = pNode->rb_right;
		}
		else
		{
			oldVdata = pTreeNode->vdata;
			rb_erase(pNode,pRoot);
			free(pTreeNode);	//if pTreeNode->pdata pointer to a struct ,you MUST free yourself !!
			ctree->total--;
			return oldVdata;	//留待用户释放的 vdata (如有必要释放)
		}
	}
	return NULL;
}

int cTreeMap(cTree ctree,void apply(void *k,void **pv,void *cl), void *cl)
{
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = NULL;
	cTreeNode *pTreeNode = NULL;
	//(其实就是while迭代实现的中序遍历),rb_first()找到的node也是中序遍历意义上的第一个
	for(pNode = rb_first(pRoot);pNode;pNode = rb_next(pNode))
	{
		pTreeNode = container_of(pNode,cTreeNode,node);
		//printf("%d,",pMgrNode->value);
		apply(pTreeNode->kdata,&pTreeNode->vdata,cl);
	}
	return 0;
}

static int cTreeMapInternal(struct rb_node *pNode,void apply(void *k,void **x,void *cl,int lev), void *cl, int level)
{
	cTreeNode *pTreeNode = NULL;
	if(!pNode)
		return 0;
	
	if(pNode->rb_left)
		cTreeMapInternal(pNode->rb_left,apply,cl,level+1);
	//else
	//	apply(NULL,NULL,cl,level+1);

	//mid order access
	pTreeNode = container_of(pNode,cTreeNode,node);
	apply(pTreeNode->kdata,&pTreeNode->vdata,cl,level);
	
	if(pNode->rb_right)
		cTreeMapInternal(pNode->rb_right,apply,cl,level+1);
	//else
	//	apply(NULL,NULL,cl,level+1);
	return 0;
}

int cTreeMap2(cTree ctree,void apply(void *k,void **x,void *cl,int lev), void *cl)
{
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = pRoot->rb_node;	//the first node

	return cTreeMapInternal(pNode,apply,cl,0);
}

//调用本 cTreeDestroy 前,应该先调用 cTreeMap 释放掉所有的 vdata 指向的内存
int cTreeDestroy(cTree *pctree)
{
	if(!pctree || !(*pctree))
		return 0;
	cTree ctree = *pctree;
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = NULL;
	struct rb_node *pNodeTmp = NULL;
	cTreeNode *pTreeNodeTmp = NULL;

	pNode = rb_first(pRoot);
	while(pNode)
	{
		pNodeTmp = pNode;
		pNode = rb_next(pNode);

		pTreeNodeTmp = container_of(pNodeTmp,cTreeNode,node);
		rb_erase(pNodeTmp,pRoot);
		free(pTreeNodeTmp);	//if pTreeNode->pdata pointer to a struct ,you MUST free yourself !!
		ctree->total--;
	}
	free(ctree);
	*pctree = NULL;
	return 0;
}


