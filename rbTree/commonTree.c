//�о�����һ��ʹ������ʵ�ֵļ�ֵ�Լ�����
//������ϸ����,֮ǰ������Щ����ȷʵ����ʹ�ü�ֵ����ʵ��
//���Ȳ��� num �Զ�ʵ��������ʵ����(k=num,v=null)
//����ĳЩ num/string �õ����ʵĽṹ�� ����(k=num/string,v=void *)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "commonTree.h"

//Ĭ���ǰ��� kdata ָ��ֵ����(��key�ĵ�ֵַ)
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
			return pTreeNode;	//cTreeNodeId ����һ�� cTreeNode*
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
			//�������ͬ�� key Ҫ����,����֮ǰ��old vdata,�û� MUST ���б�Ҫ�� �ͷ� ���� !!!
			//����ʹ�õ� kdata ��û�п��ܴ���й¶? 
			//case.1: kdata ͨ��malloc�����,��ô���º�� pTreeNode->kdata ��Ȼָ����,û�ж�ʧ
			//case.2: kdata ��vdataָ��ṹ���ĳ����Ա,��ô���º�� pTreeNode->kdata ��ַʧЧ
			//����̫������,�ؼ��ǲ��ɿ���,���������������Լ�����һ��kdata!,�����Ļ�,�����kdata�����޺ζ�����Ч��(ֻ�����ǲ�����memcpyһ����)
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
	memcpy(pTargetTreeNode->kdata,kdata,kdataSize);	//����Ĭ�� kdata ָ��ĵ�ַ��һ����Ч��ֵ!! ������ "(void *)i" �������淨!!
	pTargetTreeNode->vdata = vdata;

	rb_link_node(&pTargetTreeNode->node, pParent,ppNode);	//first, link this node to rbTree
	rb_insert_color(&pTargetTreeNode->node, pRoot);			//rebalance

	ctree->total++;
	return NULL;
}

//���ֻ��ɾ��(�ͷ�) cTreeNode,�� vdata ָ����ڴ���Ҫ�Լ����ͷ�!
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
	return oldVdata;	//�����û��ͷŵ� vdata (���б�Ҫ�ͷ�)
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
			return oldVdata;	//�����û��ͷŵ� vdata (���б�Ҫ�ͷ�)
		}
	}
	return NULL;
}

int cTreeMap(cTree ctree,void apply(void *k,void **pv,void *cl), void *cl)
{
	struct rb_root *pRoot = &(ctree->root);	//the root, including a pointer to rb_node
	struct rb_node *pNode = NULL;
	cTreeNode *pTreeNode = NULL;
	//(��ʵ����while����ʵ�ֵ��������),rb_first()�ҵ���nodeҲ��������������ϵĵ�һ��
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

//���ñ� cTreeDestroy ǰ,Ӧ���ȵ��� cTreeMap �ͷŵ����е� vdata ָ����ڴ�
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


