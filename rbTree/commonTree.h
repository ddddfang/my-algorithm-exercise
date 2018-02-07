#ifndef COMMON_TREE_H
#define COMMON_TREE_H

#include "rbtree.h"

typedef struct 
{
	struct rb_node node;
	void *kdata;	//按 kdata 为关键字组成二叉排序树
	void *vdata;	//custom app data, maybe a structure
}cTreeNode,*cTreeNodeId;

typedef struct 
{
	int total;
	int (*cmp)(const void *p1,const void *p2);	//p1<p2 return <0
	struct rb_root root;
}cTreeManager,*cTree;


cTree cTreeCreate(int (*cmp)(const void *p1,const void *p2));

//这个主要是想配合 cTreeEraseFromNodeId 函数使用的
cTreeNodeId cTreeSearch(cTree ctree,void *kdata);

void *getVdataFromNodeId(cTreeNodeId id);
void *getKdataFromNodeId(cTreeNodeId id);

//如果vdata指向自定义的数据结构,则返回update之前的vdata,必须释放,否则可能内存泄漏
//vdata不需要释放的则可以不用关心free操作及返回的void*
void *cTreeInsert(cTree ctree,void *kdata,unsigned int kdataSize,void *vdata);

void *cTreeEraseFromNodeId(cTree ctree,cTreeNodeId *pid);
void *cTreeErase(cTree ctree,void *kdata);
int cTreeMap(cTree ctree,void apply(void *k,void **pv,void *cl), void *cl);
int cTreeMap2(cTree ctree,void apply(void *k,void **x,void *cl,int lev), void *cl);
int cTreeDestroy(cTree *pctree);


#endif
