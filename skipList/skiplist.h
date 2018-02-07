
#ifndef _SKIPLIST_H
#define _SKIPLIST_H

//SKIPLIST_MAX_LEVEL 为 1 则退化为普通的链表
#define SKIPLIST_MAX_LEVEL	32
//理想的二分法的话,p应该是0.5,但是好像0.25-0.5之间性能都差不多
#define SKIPLIST_P	0.25

//gstreamer 里面貌似就是这么用的,basesink.h
typedef struct skipListNode		skipListNode;
typedef struct skipLink			skipLink;
typedef struct skipListManager	skipListManager;

//public 的 struct 放在了.h 文件里,如果是 private,可以将 struct 的定义放在.c
struct skipLink
{
	struct skipLink *next;	//直接指向下一个节点
	struct skipLink *prev;	//直接指向上一个节点
	unsigned int span;	//跨度(next)
};

struct skipListNode
{
	void *key;
	void *val;
	unsigned int level;	//本节点的 level
	skipLink link[0];	//
};

struct skipListManager
{
	unsigned int level;	//max level of skipListNode
	unsigned int count;	//list len
	int (*keyCmp)(const void *p1,const void *p2);	//key 比较函数
	void *(*keyDup)(const void *key);				//key 复制函数
	void *(*valDup)(const void *val);				//val 复制函数
	void (*keyDestructor)(void *key);				//key 销毁
	void (*valDestructor)(void *val);				//val 销毁
	skipLink head[SKIPLIST_MAX_LEVEL];
};

//注意这些接口如果 k/v 当作 int 来用应该使用 8bytes 长度的 int,
//因为我们在内部操作的 k/v 是当作指针来操作的,在64位机器上指针长度为 8bytes!
//这样如果 在栈上定义 4bytes 长度的 int, 有可能会发生踩内存!
//当然这些都是对使用者提出的要求

skipListManager *skipListCreate(int (*keyCmp)(const void *p1,const void *p2),
								void *(*keyDup)(const void *key),
								void *(*valDup)(const void *val),
								void (*keyDestructor)(void *key),
								void (*valDestructor)(void *val));
void skipListDestroy(skipListManager *pSL);
int skipListInsert(skipListManager *pSL, void *k, void *v,void **vOld,unsigned int *pRank);
int skipListSearch(skipListManager *pSL, void *k, void **v,unsigned int *pRank);
int skipListDelete(skipListManager *pSL, void *k, void **v);
int skipListSearchKth(skipListManager *pSL, unsigned int rank, void **k, void **v);
int skipListDeleteKth(skipListManager *pSL, unsigned int rank, void **k, void **v);


int skipListSearchRangeByKey(skipListManager *pSL, void *keyStart, void *keyEnd,
						void apply(void *k, void *v,void *cl), void *cl);
int skipListSearchRangeByRank(skipListManager *pSL, unsigned int rankStart, unsigned int rankEnd,
						void apply(void *k, void *v,void *cl), void *cl);
int skipListDeleteRangeByKey(skipListManager *pSL, void *keyStart, void *keyEnd);
int skipListDeleteRangeByRank(skipListManager *pSL, unsigned int rankStart, unsigned int rankEnd);

void debugPrint(skipListManager *pSL);
void debugPrint2(skipListManager *pSL);

#endif

