#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skiplist.h"

/********* internal default functions **********/
static int keyCmpDefault(const void *p1,const void *p2)
{
	return (int)((long)p1 - (long)p2);
	//return strcmp((const char *)p1,(const char *)p2);
}

static void *keyDupDefault(const void *key)
{
	return (void *)key;
}

static void *valDupDefault(const void *val)
{
	unsigned int len = strlen((const char *)val);
	char *p = (char *)malloc(len+1);
	if(p)
	{
		memset(p,0,len+1);
		memcpy(p,val,len);
	}
	return (void *)p;
}

static void keyDestructorDefault(void *key)
{
	key = key;	//just tell compiler not to report warning
}

static void valDestructorDefault(void *val)
{
	if(val)
		free(val);
}

/********* skipLink related utils functions **********/
static inline void skipLinkInit(skipLink *pLink)
{
	pLink->next = pLink;
	pLink->prev = pLink;
	pLink->span = 0;
}

static inline void skipLinkAdd(skipLink *pInsertPos1,skipLink *pInsertPos2,skipLink *pTarget)
{
	pTarget->next = pInsertPos2;
	pTarget->prev = pInsertPos1;
	pInsertPos1->next = pTarget;
	pInsertPos2->prev = pTarget;
}

static inline void skipLinkDel(skipLink *pTarget)
{
    pTarget->next->prev = pTarget->prev;
    pTarget->prev->next = pTarget->next;
	pTarget->prev = pTarget;
	pTarget->next = pTarget;
}

#define skipLink2Node(ptr, type, member) \
	((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))
#define skipLinkEmpty(link)	((link) == (link)->next)
#define skipLinkHead(link)	((link)->next)
#define skipLinkTail(link)	((link)->prev)

/********* internal static functions **********/
static skipListNode *createSkipListNode(skipListManager *psl,void *k,void *v,unsigned int level)
{
	skipListNode *node = (skipListNode *)malloc(sizeof(skipListNode) + level * sizeof(skipLink));
	if(node != NULL)
	{
		node->level = level;
		node->key = (*(psl->keyDup))(k);
		node->val = (*(psl->valDup))(v);
	}
	return node;
}

static void destroySkipListNode(skipListManager *psl,skipListNode *pNode)
{
	(*(psl->keyDestructor))(pNode->key);
	(*(psl->valDestructor))(pNode->val);
	free(pNode);
}

//产生的 level 符合比例系数为 SKIPLIST_P 的等比分布.且最小为 1,最大为 SKIPLIST_MAX_LEVEL
static unsigned int randomLevel()
{
	unsigned int level = 1;
	while (((random() & 0xffff) < 0xffff * SKIPLIST_P) && (level < SKIPLIST_MAX_LEVEL))
		level++;
	return level;
}


/********* public skiplist interface functions **********/
skipListManager *skipListCreate(int (*keyCmp)(const void *p1,const void *p2),
								void *(*keyDup)(const void *key),
								void *(*valDup)(const void *val),
								void (*keyDestructor)(void *key),
								void (*valDestructor)(void *val))
{
	unsigned int i = 0;
	skipListManager *pSL = (skipListManager *)malloc(sizeof(skipListManager));
	if(pSL != NULL)
	{
		memset(pSL,0,sizeof(skipListManager));
		pSL->level = 1;	//
		pSL->count = 0;	//
		pSL->keyCmp = keyCmp ? keyCmp : keyCmpDefault;
		pSL->keyDup = keyDup ? keyDup : keyDupDefault;
		pSL->valDup = valDup ? valDup : valDupDefault;
		pSL->keyDestructor = keyDestructor ? keyDestructor : keyDestructorDefault;
		pSL->valDestructor = valDestructor ? valDestructor : valDestructorDefault;

		for(i = 0; i < SKIPLIST_MAX_LEVEL; i++)
			skipLinkInit(&pSL->head[i]);
	}
	return pSL;
}

void skipListDestroy(skipListManager *pSL)
{
	skipLink *plink = NULL;
	skipListNode *skipNode = NULL;

	if(!pSL)
		return;
	while(!skipLinkEmpty(&pSL->head[0]))
	{
		plink = skipLinkHead(&pSL->head[0]);
		skipNode = skipLink2Node(plink, skipListNode, link[0]);	//get entry
		skipLinkDel(plink);	//remove from link list
		destroySkipListNode(pSL,skipNode);
		pSL->count--;
	}
	free(pSL);
}

//ret: (0) -> update, (1)->insert, (-1) -> insert fail
int skipListInsert(skipListManager *pSL, void *k, void *v,void **vOld,unsigned int *pRank)
{
	unsigned int newLevel;
	int curLevel;
	unsigned int rank[SKIPLIST_MAX_LEVEL];	//存储节点的排名信息,除了 header 为0外,其他节点从1起始
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	skipLink *update[SKIPLIST_MAX_LEVEL];	//存储插入节点的前驱结点

	curLevel = SKIPLIST_MAX_LEVEL - 1;	//从可能的最高 level 开始搜
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	while(curLevel >= 0)
	{
		//从顶开始搜,顶层初始化为 0(说明没降过级),降级不改变排名,因此使用上一级的 rank
		rank[curLevel] = (curLevel == SKIPLIST_MAX_LEVEL-1)?0:rank[curLevel+1];

		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	(*(pSL->keyCmp))(pSkipNodeNext->key,k) < 0	)	//pSkipNodeNext->key < k
		{
			//一旦enter此while循环,排名必然增长啊
			rank[curLevel] += pLink->span;

			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		//1.pLink = &pSL->head[curLevel] 导致退出,这一层为空,或者新插入的节点k是最大的
		//2.pSkipNodeNext->key >= key 导致退出
		update[curLevel] = pLink;	//表示如果 new 出来的 level 达到这个高度的话,这一层就插在这个位置后
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索
			pLinkNext = pLink->next;
		}
	}
	//走到这里,pLink 和 pLinkNext 之间理论上就是 level 0 层 新node 应该插入的位置
	//而 rank 应该是其前驱结点的排名
	pLink = update[0];			//所以这一句应该不写也可
	pLinkNext = pLink->next;	//所以这一句应该不写也可
	if(pLinkNext != &pSL->head[0])
	{
		pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0]);
		if(!(*(pSL->keyCmp))(pSkipNodeNext->key,k))	//found this (k,v)already exists,update
		{
			if(vOld)	//you MUST free val yourself
				*vOld = pSkipNodeNext->val;
			else		//here I free them for you
				(*(pSL->valDestructor))(pSkipNodeNext->val);
			pSkipNodeNext->val = (*(pSL->valDup))(v);
			if(pRank)
				*pRank = rank[0]+1;
			return 0;	//更新.不改变排位信息
		}
	}

	newLevel = randomLevel();
	skipListNode *pskipNodeNew = createSkipListNode(pSL,k,v,newLevel);
	if(!pskipNodeNew)
		return -1;
	if(newLevel > pSL->level)	//
		pSL->level = newLevel;

	for(curLevel=0;curLevel<SKIPLIST_MAX_LEVEL;curLevel++)
	{
		if(curLevel<newLevel)
		{
			skipLinkAdd(update[curLevel],update[curLevel]->next,&(pskipNodeNew->link[curLevel]));
			//rank[curLevel] + update[curLevel]->span 是后继结点的原排名,新排名是 原排名+1, rank[0]+1 是新插入节点的排名
			pskipNodeNew->link[curLevel].span = rank[curLevel] + update[curLevel]->span - rank[0];	//后继结点的新排名-新插入节点的排名
			update[curLevel]->span = rank[0] - rank[curLevel] + 1;	//更新前驱结点跨度=新插入节点的排名-前驱结点的排名,后继结点的跨度不变
		}
		else
		{
			//所有前驱结点跨度++
			update[curLevel]->span++;
		}
	}
	if(pRank)
		*pRank = rank[0]+1;
	pSL->count++;
	return 1;
}

//ret: (0) -> no such node, (1)->found it, (-1)->search error
int skipListSearch(skipListManager *pSL, void *k, void **v,unsigned int *pRank)
{
	int curLevel;
	int rank;
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	rank = 0;
	while(curLevel >= 0)
	{
		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	(*(pSL->keyCmp))(pSkipNodeNext->key,k) < 0	)	//pSkipNodeNext->key < k
		{
			rank += pLink->span;

			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索,rank 不变
			pLinkNext = pLink->next;
		}
	}
	if(pLinkNext != &pSL->head[0])
	{
		pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0]);
		if(!(*(pSL->keyCmp))(pSkipNodeNext->key,k))	//find it !
		{
			if(v)	//you MUST free val yourself
				*v = (*(pSL->valDup))(pSkipNodeNext->val);
			if(pRank)
				*pRank = rank+1;	//此时的 rank 拿着的还是前驱结点的排名
			return 1;
		}
	}
	if(pRank)
		*pRank = rank;	//直接返回前驱结点的排名
	return 0;
}

//ret: (0) -> no such node, (1)->delete ok, (-1)->delete error
int skipListDelete(skipListManager *pSL, void *k, void **v)
{
	unsigned int remainLevel;
	int curLevel;
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	skipLink *update[SKIPLIST_MAX_LEVEL];

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	while(curLevel >= 0)
	{
		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	(*(pSL->keyCmp))(pSkipNodeNext->key,k) < 0	)	//pSkipNodeNext->key < k
		{
			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		update[curLevel] = pLink;	//记录本节点前驱结点
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索
			pLinkNext = pLink->next;
		}
	}
	pLink = update[0];			//这一句不写也可
	pLinkNext = pLink->next;	//这一句不写也可
	if(pLinkNext != &pSL->head[0])
	{
		pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0]);
		if(!(*(pSL->keyCmp))(pSkipNodeNext->key,k))	//find it !
		{
			remainLevel = pSL->level;
			//走到这里说明 pLinkNext 正是待删节点 第0层的 link
			for(curLevel = 0; curLevel < SKIPLIST_MAX_LEVEL; curLevel++)
			{
				if(curLevel < pSkipNodeNext->level)	//将此节点从 link 中删除
				{
					if(pLinkNext->next == &pSL->head[curLevel])	//说明这个高度就没有其他节点了
						remainLevel = curLevel;	//curLevel 对应的 level 是 curLevel+1
					update[curLevel]->span += pLinkNext->span - 1;	//前驱结点继承本节点的跨度
					skipLinkDel(pLinkNext);
					pLinkNext++;	//去更高一层删除
				}
				else
				{
					//前驱结点跨度--
					update[curLevel]->span--;
				}
			}
			if(v)	//you MUST free val yourself
				*v = pSkipNodeNext->val;
			else	//here I free them for you
				(*(pSL->valDestructor))(pSkipNodeNext->val);
			(*(pSL->keyDestructor))(pSkipNodeNext->key);
			free(pSkipNodeNext);
			pSL->level = remainLevel;
			pSL->count--;
			return 1;
		}
	}
	return 0;
}

//查找第k个元素 (0) -> no such node, (1)-> search ok, (-1) -> error
int skipListSearchKth(skipListManager *pSL, unsigned int rank, void **k, void **v)
{
	int curLevel;
	int r;
	skipLink *pLink = NULL;
	skipListNode *pSkipNode = NULL;

	if(rank > pSL->count || rank <= 0)
		return 0;	//no such node

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	r = 0;
	while(curLevel >= 0)
	{
		//r + pLink->span 是 pLink 后一个节点的 rank
		while((pLink->next != &pSL->head[curLevel]) && (r + pLink->span < rank))
		{
			r += pLink->span;
			pLink = pLink->next;
		}
		if(curLevel-- > 0)
			pLink--;	//去更低的 level 搜索,rank 不变,curLevel 原已为0时不执行这句
	}
	//走到这里 pLink 应该身处 level 0 且排名为 rank-1
	pLink = pLink->next;
	if(pLink != &pSL->head[0])
	{
		pSkipNode = skipLink2Node(pLink, skipListNode, link[0]);
		if(k)	//you MUST free key yourself
			*k = (*(pSL->keyDup))(pSkipNode->key);
		if(v)	//you MUST free val yourself
			*v = (*(pSL->valDup))(pSkipNode->val);
		return 1;
	}
	return 0;
}

//删除第k个元素
int skipListDeleteKth(skipListManager *pSL, unsigned int rank, void **k, void **v)
{
	int r;
	int curLevel;
	unsigned int remainLevel;
	skipLink *pLink = NULL;
	skipListNode *pSkipNode = NULL;

	skipLink *update[SKIPLIST_MAX_LEVEL];

	if(rank > pSL->count || rank <= 0)
		return 0;	//no such node

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	r = 0;
	while(curLevel >= 0)
	{
		while((pLink->next != &pSL->head[curLevel]) && (r + pLink->span < rank))
		{
			r += pLink->span;
			pLink = pLink->next;
		}
		update[curLevel] = pLink;	//记录前驱结点
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索
		}
	}
	pLink = pLink->next;	//待删节点
	if(pLink != &pSL->head[0])
	{
		remainLevel = pSL->level;
		pSkipNode = skipLink2Node(pLink, skipListNode, link[0]);
		for(curLevel = 0; curLevel < SKIPLIST_MAX_LEVEL; curLevel++)
		{
			if(curLevel < pSkipNode->level)	//将此节点从 link 中删除
			{
				if(pLink->next == &pSL->head[curLevel])	//说明这个高度就没有其他节点了
					remainLevel = curLevel;	//curLevel 对应的 level 是 curLevel+1
				update[curLevel]->span += pLink->span - 1;	//前驱结点继承本节点的跨度
				skipLinkDel(pLink);
				pLink++;	//去更高一层删除
			}
			else
			{
				//前驱结点跨度--
				update[curLevel]->span--;
			}
		}
		if(k)	//you MUST free key yourself
			*k = pSkipNode->key;
		else	//here I free them for you
			(*(pSL->keyDestructor))(pSkipNode->key);
		if(v)	//you MUST free val yourself
			*v = pSkipNode->val;
		else	//here I free them for you
			(*(pSL->valDestructor))(pSkipNode->val);
		free(pSkipNode);
		pSL->level = remainLevel;
		pSL->count--;
		return 1;
	}
	return 0;
}

//在(keyStart,keyEnd) 与 (rankStart,rankEnd)的交集上 search, 同时,不考虑范围非法的情况
static int skipListIterRange(skipListManager *pSL, void *keyStart, void *keyEnd,
								unsigned int rankStart, unsigned int rankEnd,
								void iterApply(skipListNode *pSkipNode,void *cl), void *cl)
{
	int curLevel;
	int rank;
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	rank = 0;
	while(curLevel >= 0)
	{
		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	((*(pSL->keyCmp))(pSkipNodeNext->key,keyStart) < 0 || rank + pLink->span < rankStart))
		{
			rank += pLink->span;

			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索,rank 不变
			pLinkNext = pLink->next;
		}
	}
	//走到这里 pLinkNext 就是第一个在范围内的节点,其排名为 rank + pLink->span
	//且 curLevel= -1,我们身处最底层次的那个链表上
	rank += pLink->span;
	curLevel = 0;	//到这里 curLevel 看起来没啥用了,我们就让他来计数吧

	while(	(pLinkNext != &pSL->head[0])	//首先保证 curLevel 这条链表非空
		&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0])) != NULL
		&&	(*(pSL->keyCmp))(pSkipNodeNext->key,keyEnd) <= 0 
		&&	rank++ <= rankEnd	)
	{
		pLink = pLinkNext->next;	//反正走到这里 pLink 就没多大作用了,用它备份 pLinkNext 的下一个节点
		iterApply(pSkipNodeNext, cl);	//外面传进来的那个apply函数指针可以放在cl里面啊
		//用 pLink 确保链表不会断掉,因为我们允许 apply 吧 pSkipNodeNext 删掉(虽然不如直接调用 skipListDeleteRange 来的高效)!
		pLinkNext = pLink;
		curLevel++;
	}
	return curLevel;
}

typedef struct
{
	void *p1;
	void *p2;
} privateData;

static void iterApplySearch(skipListNode *pSkipNode,void *cl)
{
	typedef void (*usrApply)(void *k, void *v,void *cl);
	privateData *pcl = (privateData *)cl;

	usrApply apply = (usrApply)(pcl->p1);
	void *userCl = pcl->p2;
	apply(pSkipNode->key,pSkipNode->val,userCl);
}

//查找某个范围的元素,对所有符合条件的元素只读式调用 apply 函数
int skipListSearchRangeByKey(skipListManager *pSL, void *keyStart, void *keyEnd,
						void apply(void *k, void *v,void *cl), void *cl)
{
	privateData localCl;
	localCl.p1 = (void *)apply;
	localCl.p2 = cl;
	return skipListIterRange(pSL, keyStart, keyEnd, 1, pSL->count, iterApplySearch, &localCl);
}

//查找某个范围的元素,对所有符合条件的元素只读式调用 apply 函数
int skipListSearchRangeByRank(skipListManager *pSL, unsigned int rankStart, unsigned int rankEnd,
						void apply(void *k, void *v,void *cl), void *cl)
{
	privateData localCl;
	localCl.p1 = (void *)apply;
	localCl.p2 = cl;
	skipLink *pLink = NULL;
	skipListNode *pSkipNodeFirst = NULL;
	skipListNode *pSkipNodeLast = NULL;

	if(skipLinkEmpty(&pSL->head[0]))
		return 0;
	pLink = skipLinkHead(&pSL->head[0]);
	pSkipNodeFirst = skipLink2Node(pLink, skipListNode, link[0]);
	pLink = skipLinkTail(&pSL->head[0]);
	pSkipNodeLast = skipLink2Node(pLink, skipListNode, link[0]);
	return skipListIterRange(pSL, pSkipNodeFirst->key, pSkipNodeLast->key, rankStart, rankEnd, iterApplySearch, &localCl);
}


//删除某个范围的元素
int skipListDeleteRangeByKey(skipListManager *pSL, void *keyStart, void *keyEnd)
{
	int curLevel;
	int cnt;
	unsigned int remainLevel;
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	skipLink *update[SKIPLIST_MAX_LEVEL];

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	while(curLevel >= 0)
	{
		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	(*(pSL->keyCmp))(pSkipNodeNext->key,keyStart) < 0	)	//pSkipNodeNext->key < k
		{
			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		update[curLevel] = pLink;	//记录本节点前驱结点
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索
			pLinkNext = pLink->next;
		}
	}
	pLinkNext = update[0]->next;	//这一句不写也可,pLinkNext 现在就是第一个在范围内的节点
	cnt = 0;
	while(	(pLinkNext != &pSL->head[0])	//首先保证 curLevel 这条链表非空
		&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0])) != NULL
		&&	(*(pSL->keyCmp))(pSkipNodeNext->key,keyEnd) <= 0 )
	{
		pLink = pLinkNext->next;	//保存下一个节点
		//删节点
		remainLevel = pSL->level;
		for(curLevel = 0; curLevel < SKIPLIST_MAX_LEVEL; curLevel++)
		{
			if(curLevel < pSkipNodeNext->level)	//将此节点从 link 中删除
			{
				if(pLinkNext->next == &pSL->head[curLevel])	//说明这个高度就没有其他节点了
					remainLevel = curLevel;	//curLevel 对应的 level 是 curLevel+1
				update[curLevel]->span += pLinkNext->span - 1;	//前驱结点继承本节点的跨度
				skipLinkDel(pLinkNext);
				pLinkNext++;	//去更高一层删除
			}
			else
			{
				//前驱结点跨度--
				update[curLevel]->span--;
			}
		}
		(*(pSL->valDestructor))(pSkipNodeNext->val);
		(*(pSL->keyDestructor))(pSkipNodeNext->key);
		free(pSkipNodeNext);
		pSL->level = remainLevel;
		pSL->count--;
		cnt++;
		pLinkNext = pLink;	//
	}
	return cnt;
}

//删除某个范围的元素
int skipListDeleteRangeByRank(skipListManager *pSL, unsigned int rankStart, unsigned int rankEnd)
{
	int rank;
	int curLevel;
	unsigned int remainLevel;
	int cnt;
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;

	skipLink *update[SKIPLIST_MAX_LEVEL];

	curLevel = SKIPLIST_MAX_LEVEL - 1;
	pLink = &pSL->head[curLevel];
	pLinkNext = pLink->next;
	rank = 0;
	while(curLevel >= 0)
	{
		while(	(pLinkNext != &pSL->head[curLevel])	//首先保证 curLevel 这条链表非空
			&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[curLevel])) != NULL
			&&	(rank + pLink->span < rankStart)	)	//
		{
			rank += pLink->span;
			pLink = pLinkNext;
			pLinkNext = pLink->next;
		}
		update[curLevel] = pLink;	//记录本节点前驱结点
		if(curLevel-- > 0)
		{	//curLevel 原来已经为0的话,-- 肯定变成-1了,安全起见就不要执行下面这些了
			pLink--;	//去更低的 level 搜索
			pLinkNext = pLink->next;
		}
	}
	pLinkNext = update[0]->next;	//这一句不写也可,pLinkNext 现在就是第一个在范围内的节点
	rank += update[0]->span;		//rank 现在是 pLinkNext 的 rank
	cnt = 0;
	while(	(pLinkNext != &pSL->head[0])	//首先保证 curLevel 这条链表非空
		&&	(pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0])) != NULL
		&&	(rank++ <= rankEnd) )
	{
		pLink = pLinkNext->next;	//保存下一个节点
		//删节点
		remainLevel = pSL->level;
		for(curLevel = 0; curLevel < SKIPLIST_MAX_LEVEL; curLevel++)
		{
			if(curLevel < pSkipNodeNext->level)	//将此节点从 link 中删除
			{
				if(pLinkNext->next == &pSL->head[curLevel])	//说明这个高度就没有其他节点了
					remainLevel = curLevel;	//curLevel 对应的 level 是 curLevel+1
				update[curLevel]->span += pLinkNext->span - 1;	//前驱结点继承本节点的跨度
				skipLinkDel(pLinkNext);
				pLinkNext++;	//去更高一层删除
			}
			else
			{
				//前驱结点跨度--
				update[curLevel]->span--;
			}
		}
		(*(pSL->valDestructor))(pSkipNodeNext->val);
		(*(pSL->keyDestructor))(pSkipNodeNext->key);
		free(pSkipNodeNext);
		pSL->level = remainLevel;
		pSL->count--;
		cnt++;
		pLinkNext = pLink;	//
	}
	return cnt;
}


/********* some skiplist debug functions **********/
void debugPrint(skipListManager *pSL)
{
	unsigned int rank;
	int i;
	int maxLevel = 0;
	int levelStat[SKIPLIST_MAX_LEVEL] = {0};
	skipLink *pLink = NULL;
	skipLink *pLinkNext = NULL;
	skipListNode *pSkipNodeNext = NULL;
	skipLink *pLinkNextTmp = NULL;

	pLink = &pSL->head[0];
	pLinkNext = pLink->next;
	pLinkNextTmp = pLinkNext;

	rank = 1;
	while(pLinkNext != &pSL->head[0])
	{
		pSkipNodeNext = skipLink2Node(pLinkNext, skipListNode, link[0]);
		
		//更新统计信息
		if(pSkipNodeNext->level > maxLevel)
			maxLevel = pSkipNodeNext->level;
		levelStat[pSkipNodeNext->level - 1]++;

		//打印这个节点的信息
		printf("rank %u, (%ld,%s)",rank,(long)(pSkipNodeNext->key),(char *)(pSkipNodeNext->val));
		for(i=0;i<pSkipNodeNext->level;i++)
		{
			printf(" <span %u>",pLinkNextTmp->span);
			pLinkNextTmp++;
		}
		printf("\n");
		
		//next
		pLink = pLinkNext;
		pLinkNext = pLink->next;
		pLinkNextTmp = pLinkNext;
		rank++;
	}
	printf("maxlevel is %d\n",maxLevel);
	for(i=0;i<maxLevel;i++)
	{
		printf("level %d has %d elements\n",i+1,levelStat[i]);
	}
}

static void iterApplyDebug(skipListNode *pSkipNode,void *cl)
{
	int i;
	privateData *pcl = (privateData *)cl;
	int *maxLevel = (int *)(pcl->p1);
	int *levelStat = (int *)(pcl->p2);
	skipLink *pSkipNodeTmp = &pSkipNode->link[0];

	//统计信息
	if(pSkipNode->level > *maxLevel)
		*maxLevel = pSkipNode->level;
	levelStat[pSkipNode->level - 1]++;

	//打印这个节点的信息
	printf("(%ld,%s)",(long)(pSkipNode->key),(char *)(pSkipNode->val));
	for(i=0;i<pSkipNode->level;i++)
	{
		printf(" <span %u>",pSkipNodeTmp->span);
		pSkipNodeTmp++;
	}
	printf("\n");
}

void debugPrint2(skipListManager *pSL)
{
	int i;
	static int maxLevel = 0;
	static int levelStat[SKIPLIST_MAX_LEVEL] = {0};
	privateData localCl;
	localCl.p1 = (void *)&maxLevel;
	localCl.p2 = (void *)levelStat;

	skipLink *pLink = NULL;
	skipListNode *pSkipNodeFirst = NULL;
	skipListNode *pSkipNodeLast = NULL;

	if(skipLinkEmpty(&pSL->head[0]))
		return;
	pLink = skipLinkHead(&pSL->head[0]);
	pSkipNodeFirst = skipLink2Node(pLink, skipListNode, link[0]);
	pLink = skipLinkTail(&pSL->head[0]);
	pSkipNodeLast = skipLink2Node(pLink, skipListNode, link[0]);
	skipListIterRange(pSL, pSkipNodeFirst->key, pSkipNodeLast->key, 1, pSL->count, iterApplyDebug, &localCl);

	printf("maxlevel is %d\n",maxLevel);
	for(i=0;i<maxLevel;i++)
	{
		printf("level %d has %d elements\n",i+1,levelStat[i]);
	}
	return;
}

