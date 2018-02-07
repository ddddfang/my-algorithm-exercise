#ifndef COMMON_TREE_H
#define COMMON_TREE_H

#include "rbtree.h"

typedef struct 
{
	struct rb_node node;
	void *kdata;	//�� kdata Ϊ�ؼ�����ɶ���������
	void *vdata;	//custom app data, maybe a structure
}cTreeNode,*cTreeNodeId;

typedef struct 
{
	int total;
	int (*cmp)(const void *p1,const void *p2);	//p1<p2 return <0
	struct rb_root root;
}cTreeManager,*cTree;


cTree cTreeCreate(int (*cmp)(const void *p1,const void *p2));

//�����Ҫ������� cTreeEraseFromNodeId ����ʹ�õ�
cTreeNodeId cTreeSearch(cTree ctree,void *kdata);

void *getVdataFromNodeId(cTreeNodeId id);
void *getKdataFromNodeId(cTreeNodeId id);

//���vdataָ���Զ�������ݽṹ,�򷵻�update֮ǰ��vdata,�����ͷ�,��������ڴ�й©
//vdata����Ҫ�ͷŵ�����Բ��ù���free���������ص�void*
void *cTreeInsert(cTree ctree,void *kdata,unsigned int kdataSize,void *vdata);

void *cTreeEraseFromNodeId(cTree ctree,cTreeNodeId *pid);
void *cTreeErase(cTree ctree,void *kdata);
int cTreeMap(cTree ctree,void apply(void *k,void **pv,void *cl), void *cl);
int cTreeMap2(cTree ctree,void apply(void *k,void **x,void *cl,int lev), void *cl);
int cTreeDestroy(cTree *pctree);


#endif
