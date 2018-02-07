#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "hashTable.h"

//size is at least 2^HT_BASE_SIZE
#define HT_BASE_SIZE 5


static unsigned int murMurHash(const void *key,unsigned int len)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	const int seed = 97;
	unsigned int h = seed ^ len;
	// Mix 4 bytes at a time into the hash
	const unsigned char *data = (const unsigned char *)key;
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		k *= m;
		k ^= k >> r;
		k *= m;
		h *= m;
		h ^= k;
		data += 4;
		len -= 4;
	}
	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
		h *= m;
	}
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}

static unsigned int hashFuncDefault(const void *key)
{
	unsigned int len = strlen((const char *)key);
	return murMurHash(key,len);
}

static int keyCmpDefault(const void *p1,const void *p2)
{
	//return ((unsigned int)p1 - (unsigned int)p2);	//默认是按照 key 指针值排序(即key的地址值)
	return strcmp((const char *)p1,(const char *)p2);
}

static void *keyDupDefault(const void *key)
{
	unsigned int len = strlen((const char *)key);
	char *p = (char *)malloc(len+1);
	if(p)
	{
		memset(p,0,len+1);
		memcpy(p,key,len);
	}
	return (void *)p;
}

static void *valDupDefault(const void *val)
{
	return (void *)val;
}

static void keyDestructorDefault(void *key)
{
	if(key)
		free(key);
}

static void valDestructorDefault(void *val)
{
	val = val;	//just tell compiler not to report warning
}

static htItem *pickHeadFromCollisionList(htItem **bucketTbl,unsigned int index)
{
	htItem **ppHead = &bucketTbl[index];
	htItem *pCurItem = *ppHead;

	if(pCurItem)
		*ppHead = pCurItem->next;
	return pCurItem;
}

static void linkToHeadOfCollisionList(htItem **bucketTbl,unsigned int index,htItem *pitem)
{
	htItem **ppHead = &bucketTbl[index];
	pitem->next = *ppHead;
	*ppHead = pitem;
}

static void rehashOnTheGo(htManager *pht,unsigned int tryCnt)
{
	//increase rehash index on the go. 
	unsigned int rehashElemCnt = 0;
	htItem *pCurItem = NULL;
	if(pht->rehash_index < 0 || pht->rehash_index >= pht->size0)
		return;

	while((pht->rehash_index < pht->size0) && rehashElemCnt<tryCnt)
	{
		if(pht->tb0[pht->rehash_index])
		{
			//rehash this collision list
			while((pCurItem = pickHeadFromCollisionList(pht->tb0,pht->rehash_index)) != NULL)
			{
				linkToHeadOfCollisionList(pht->tb1,(pCurItem->hashVal) & (pht->size1 - 1),pCurItem);
				rehashElemCnt++;
			}
			pht->rehash_index++;
		}
		else
		{
			pht->rehash_index++;
			rehashElemCnt++;
		}
	}
	if(pht->rehash_index >= pht->size0)	//means rehash has completed!
	{
		//destroy tb0 and tb1->tb0
		free(pht->tb0);
		pht->tb0 = pht->tb1;
		pht->size0 = pht->size1;
		pht->tb1 = NULL;
		pht->size1 = 0;
		pht->rehash_index = -1;	//
	}
	return;
}

htManager *htCreate(unsigned int baseSize,
					unsigned int (*hashFunc)(const void *key),
					int (*keyCmp)(const void *p1,const void *p2),
					void *(*keyDup)(const void *key),
					void *(*valDup)(const void *val),
					void (*keyDestructor)(void *key),
					void (*valDestructor)(void *val))
{
	unsigned int i = 0;
	htManager *pht = (htManager *)malloc(sizeof(htManager));
	if(!pht)
		goto FAIL_1;

	pht->hashFunc = (hashFunc)?hashFunc:hashFuncDefault;
	pht->keyCmp = (keyCmp)?keyCmp:keyCmpDefault;
	pht->keyDup = (keyDup)?keyDup:keyDupDefault;
	pht->valDup = (valDup)?valDup:valDupDefault;
	pht->keyDestructor = (keyDestructor)?keyDestructor:keyDestructorDefault;
	pht->valDestructor = (valDestructor)?valDestructor:valDestructorDefault;
	if(keyDup && !hashFunc)
	{
		//如果使用 hashFuncDefault 那就最好也使用默认的 strdup
		//WARNING
	}
	pht->count = 0;
	pht->rehash_index = -1;
	pht->size1 = 0;
	pht->tb1 = NULL;

	for(i=HT_BASE_SIZE; (unsigned int)(1<<i) < baseSize; i++)
		;
	pht->size0 = (unsigned int)(1<<i);
	pht->tb0 = (htItem **)malloc((pht->size0)*sizeof(htItem *));
	if(!(pht->tb0))
		goto FAIL_2;
	memset(pht->tb0,0,(pht->size0)*sizeof(htItem *));
	return pht;

FAIL_2:
	free(pht);
FAIL_1:
	return NULL;
}

int htInsert(htManager *pht,const void *key,const void *val,void **oldVal)
{
	htItem *pItemNew = NULL;
	htItem **ppHead = NULL;
	htItem *pCurItem = NULL;
	unsigned int hashVal = 0;
	unsigned int indexInTb0 = 0;
	unsigned int indexInTb1 = 0;
	unsigned char updateFlag = 0;
	if(!pht)
		goto FAIL;
	if(pht->rehash_index < 0)	//we do expand/shrink only when we are not doing rehash
	{
		unsigned int load = (pht->count*100)/(pht->size0);
		if(load >= 100)
		{
			unsigned int newi = 0;
			for(newi = HT_BASE_SIZE; (unsigned int)(1<<newi) < (2 * pht->count); newi++)
				;
			pht->size1 = (unsigned int)(1<<newi);
			pht->tb1 = (htItem **)malloc((pht->size1)*sizeof(htItem *));
			if(!(pht->tb1))
				goto FAIL;
			memset(pht->tb1,0,(pht->size1)*sizeof(htItem *));
			pht->rehash_index = 0;	//a new rehash process has begun~
		}
	}

	hashVal = (*(pht->hashFunc))(key);

	if(pht->rehash_index < 0)
	{
		//go to tb0 to insert directly
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		ppHead = &pht->tb0[indexInTb0];
		pCurItem = *ppHead;
		updateFlag = 0;
		while(pCurItem)
		{
			if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
			{
				if(oldVal)	//here I dump for you, but you MUST free them yourself
					*oldVal = pCurItem->val;	//no need to *oldVal = (*(pht->valDup))(pCurItem->val);
				else		//you don't care old value, I free them for you.
					(*(pht->valDestructor))(pCurItem->val);
				pCurItem->val = (*(pht->valDup))(val);	//update
				updateFlag = 1;
				break;
			}
			pCurItem = pCurItem->next;
		}
		if(!updateFlag)
		{
			pItemNew = (htItem *)malloc(sizeof(htItem));
			if(!pItemNew)
				goto FAIL;
			pItemNew->key = (*(pht->keyDup))(key);
			pItemNew->val = (*(pht->valDup))(val);
			pItemNew->hashVal = hashVal;
			pItemNew->next = *ppHead;
			*ppHead = pItemNew;	//insert to be the new head
			pht->count++;
		}
		return (updateFlag? 0:1);
	}
	else
	{
		//1.go to tb0 to find collision list
		//2.if not null, search and rehash list to tb1(then it turn to null)
		//3.if null, turn to tb1 to search
		//4.select one list from rehash_index and rehash (if rehash_index >= tb0.size, free tb0 and tb1->tb0,
		//  and change rehash_index to -1)
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		pCurItem = pht->tb0[indexInTb0];
		updateFlag = 0;
		if(pCurItem)	//means this collision list has not rehashed
		{
			while((pCurItem = pickHeadFromCollisionList(pht->tb0,indexInTb0)) != NULL)
			{
				if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
				{
					if(oldVal)	//here I dump for you, but you MUST free them yourself
						*oldVal = pCurItem->val;	//no need to *oldVal = (*(pht->valDup))(pCurItem->val);
					else		//you don't care old value, I free them for you.
						(*(pht->valDestructor))(pCurItem->val);
					pCurItem->val = (*(pht->valDup))(val);	//update
					updateFlag = 1;
				}
				indexInTb1 = (pCurItem->hashVal) & (pht->size1 - 1);	//size mask
				linkToHeadOfCollisionList(pht->tb1,indexInTb1,pCurItem);
			}
			if(!updateFlag)
			{
				pItemNew = (htItem *)malloc(sizeof(htItem));
				if(!pItemNew)
					goto FAIL;
				pItemNew->key = (*(pht->keyDup))(key);
				pItemNew->val = (*(pht->valDup))(val);
				pItemNew->hashVal = hashVal;
				indexInTb1 = (pItemNew->hashVal) & (pht->size1 - 1);	//size mask
				linkToHeadOfCollisionList(pht->tb1,indexInTb1,pItemNew);
				pht->count++;
			}
		}
		else	//means this collision list has rehashed to tb1 already (or just a normal null collision list)
		{
			//go to tb1 to insert directly
			indexInTb1 = hashVal & (pht->size1 - 1);
			ppHead = &pht->tb1[indexInTb1];
			pCurItem = *ppHead;
			while(pCurItem)
			{
				if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
				{
					if(oldVal)	//here I dump for you, but you MUST free them yourself
						*oldVal = pCurItem->val;	//no need to *oldVal = (*(pht->valDup))(pCurItem->val);
					else		//you don't care old value, I free them for you.
						(*(pht->valDestructor))(pCurItem->val);
					pCurItem->val = (*(pht->valDup))(val);	//update
					updateFlag = 1;
					break;
				}
				pCurItem = pCurItem->next;
			}
			if(!updateFlag)
			{
				pItemNew = (htItem *)malloc(sizeof(htItem));
				if(!pItemNew)
					goto FAIL;
				pItemNew->key = (*(pht->keyDup))(key);
				pItemNew->val = (*(pht->valDup))(val);
				pItemNew->hashVal = hashVal;
				pItemNew->next = *ppHead;
				*ppHead = pItemNew;	//insert to be the new head
				pht->count++;
			}
		}
		//increase rehash index on the go. try at least 100 operations
		rehashOnTheGo(pht,100);
		return (updateFlag ? 0:1);
	}

FAIL:
	return -1;
}

int htDelete(htManager *pht,const void *key,void **pVal)
{
	htItem *pCurItem = NULL;
	htItem **ppCurItem = NULL;
	unsigned int hashVal = 0;
	unsigned int indexInTb0 = 0;
	unsigned int indexInTb1 = 0;
	unsigned char gotDelItem = 0;
	if(!pht)
		goto FAIL;
	//we do expand/shrink only when we are not doing rehash
	//if we reached the HT_BASE_SIZE, we do not do shrink.
	if(pht->rehash_index < 0 && (pht->size0 > (unsigned int)(1<<HT_BASE_SIZE)))
	{
		unsigned int load = (pht->count*100)/(pht->size0);
		if(load <= 10)
		{
			unsigned int newi = 0;
			for(newi = HT_BASE_SIZE; (unsigned int)(1<<newi) < pht->count; newi++)
				;
			pht->size1 = (unsigned int)(1<<newi);
			pht->tb1 = (htItem **)malloc((pht->size1)*sizeof(htItem *));
			if(!(pht->tb1))
				goto FAIL;
			memset(pht->tb1,0,(pht->size1)*sizeof(htItem *));
			pht->rehash_index = 0;	//a new rehash process has begun~
		}
	}

	hashVal = (*(pht->hashFunc))(key);

	if(pht->rehash_index < 0)
	{
		//go to tb0 to search and delete directly
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		ppCurItem = &pht->tb0[indexInTb0];
		gotDelItem = 0;
		while(*ppCurItem)
		{
			if(((*ppCurItem)->hashVal == hashVal) && ((*(pht->keyCmp))((*ppCurItem)->key,key) == 0))
			{
				pCurItem = *ppCurItem;
				*ppCurItem = (*ppCurItem)->next;	//del from collision list

				if(pVal)	//here I dump for you, but you MUST free them yourself
					*pVal = pCurItem->val;	//no need to *pVal = (*(pht->valDup))(pCurItem->val);
				else		//you don't care del value, I free them for you.
					(*(pht->valDestructor))(pCurItem->val);
				(*(pht->keyDestructor))(pCurItem->key);
				free(pCurItem);

				pht->count--;
				gotDelItem = 1;
				break;
			}
			ppCurItem = &((*ppCurItem)->next);
		}
		return (gotDelItem ? 1:0);
	}
	else
	{
		//1.go to tb0 to find collision list
		//2.if not null, search and rehash list to tb1(then it turn to null)
		//3.if null, turn to tb1 to search
		//4.select one list from rehash_index and rehash (if rehash_index >= tb0.size, free tb0 and tb1->tb0,
		//  and change rehash_index to -1)
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		pCurItem = pht->tb0[indexInTb0];
		if(pCurItem)	//means this collision list has not rehashed
		{
			gotDelItem = 0;
			while((pCurItem = pickHeadFromCollisionList(pht->tb0,indexInTb0)) != NULL)
			{
				if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
				{
					if(pVal)	//here I dump for you, but you MUST free them yourself
						*pVal = pCurItem->val;	//no need to *pVal = (*(pht->valDup))(pCurItem->val);
					else		//you don't care del value, I free them for you.
						(*(pht->valDestructor))(pCurItem->val);
					(*(pht->keyDestructor))(pCurItem->key);
					free(pCurItem);

					pht->count--;
					gotDelItem = 1;
				}
				else
				{
					indexInTb1 = (pCurItem->hashVal) & (pht->size1 - 1);	//size mask
					linkToHeadOfCollisionList(pht->tb1,indexInTb1,pCurItem);
				}
			}
		}
		else	//means this collision list has rehashed already
		{
			//go to tb1 to search and delete directly
			indexInTb1 = hashVal & (pht->size1 - 1);
			ppCurItem = &pht->tb1[indexInTb1];
			gotDelItem = 0;
			while(*ppCurItem)
			{
				if(((*ppCurItem)->hashVal == hashVal) && ((*(pht->keyCmp))((*ppCurItem)->key,key) == 0))
				{
					pCurItem = *ppCurItem;
					*ppCurItem = (*ppCurItem)->next;	//del from collision list

					if(pVal)	//here I dump for you, but you MUST free them yourself
						*pVal = pCurItem->val;	//no need to *pVal = (*(pht->valDup))(pCurItem->val);
					else		//you don't care del value, I free them for you.
						(*(pht->valDestructor))(pCurItem->val);
					(*(pht->keyDestructor))(pCurItem->key);
					free(pCurItem);

					pht->count--;
					gotDelItem = 1;
					break;
				}
				ppCurItem = &((*ppCurItem)->next);
			}
		}
		//increase rehash index on the go. try 100 times if encounter continues null collision list
		rehashOnTheGo(pht,100);
		return (gotDelItem ? 1:0);
	}
FAIL:
	return -1;
}

int htSearch(htManager *pht,const void *key,void **pVal)
{
	htItem *pCurItem = NULL;
	unsigned int hashVal = 0;
	unsigned int indexInTb0 = 0;
	unsigned int indexInTb1 = 0;
	unsigned char gotItem = 0;
	if(!pht)
		goto FAIL;

	hashVal = (*(pht->hashFunc))(key);

	if(pht->rehash_index < 0)
	{
		//go to tb0 to search directly
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		pCurItem = pht->tb0[indexInTb0];
		gotItem = 0;
		while(pCurItem)
		{
			if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
			{
				if(pVal)	//here I dump for you, but you MUST free them yourself
					*pVal = (*(pht->valDup))(pCurItem->val);
				gotItem = 1;
				break;
			}
			pCurItem = pCurItem->next;
		}
		return (gotItem ? 1:0);
	}
	else
	{
		//1.go to tb0 to find collision list
		//2.if not null, search and rehash list to tb1(then it turn to null)
		//3.if null, turn to tb1 to search
		//4.select one list from rehash_index and rehash (if rehash_index >= tb0.size, free tb0 and tb1->tb0,
		//  and change rehash_index to -1)
		indexInTb0 = hashVal & (pht->size0 - 1);	//size mask
		pCurItem = pht->tb0[indexInTb0];
		gotItem = 0;
		if(pCurItem)	//means this collision list has not rehashed
		{
			while((pCurItem = pickHeadFromCollisionList(pht->tb0,indexInTb0)) != NULL)
			{
				if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
				{
					if(pVal)	//here I dump for you, but you MUST free them yourself
						*pVal = (*(pht->valDup))(pCurItem->val);
					gotItem = 1;
				}
				indexInTb1 = (pCurItem->hashVal) & (pht->size1 - 1);	//size mask
				linkToHeadOfCollisionList(pht->tb1,indexInTb1,pCurItem);
			}
		}
		else
		{
			//go to tb1 to search and delete directly
			indexInTb1 = hashVal & (pht->size1 - 1);	//size mask
			pCurItem = pht->tb1[indexInTb1];
			while(pCurItem)
			{
				if((pCurItem->hashVal == hashVal) && ((*(pht->keyCmp))(pCurItem->key,key) == 0))
				{
					if(pVal)	//here I dump for you, but you MUST free them yourself
						*pVal = (*(pht->valDup))(pCurItem->val);
					gotItem = 1;
					break;
				}
				pCurItem = pCurItem->next;
			}
		}
		rehashOnTheGo(pht,100);
		return (gotItem ? 1:0);
	}
FAIL:
	return -1;
}

//int htPairArrayGet(htManager *pht,pair **pairArray,int pairCmp(const void *p1,const void *p2))
int htToPairArray(htManager *pht,pair **pairArray,int pairCmp(const void *p1,const void *p2))
{
	unsigned int index = 0;
	unsigned int count = 0;
	pair *kv = NULL;
	htItem *pCurItem = NULL;

	if(!pht || pht->count <= 0 || !pairArray)
		return -1;

	kv = (pair *)malloc((pht->count)*sizeof(pair));
	if(!kv)
		return -1;

	if(pht->tb0)
	{
		for(index=0; index < pht->size0 && count < pht->count; index++)
		{
			//search collision list
			pCurItem = pht->tb0[index];
			while(pCurItem)
			{
				kv[count].key = (*(pht->keyDup))(pCurItem->key);
				kv[count].val = (*(pht->valDup))(pCurItem->val);
				count++;
				pCurItem = pCurItem->next;
			}
		}
	}
	//count hold ...
	if(pht->tb1)
	{
		for(index=0; index < pht->size1 && count < pht->count; index++)
		{
			//search collision list
			pCurItem = pht->tb1[index];
			while(pCurItem)
			{
				kv[count].key = (*(pht->keyDup))(pCurItem->key);
				kv[count].val = (*(pht->valDup))(pCurItem->val);
				count++;
				pCurItem = pCurItem->next;
			}
		}
	}
	if(pairCmp)	//1 待排序数组首地址 2 数组中待排序元素数量 3 各元素的占用空间大小 4 指向函数的指针
		qsort(kv,pht->count,sizeof(kv[0]),pairCmp);
	*pairArray = kv;
	return pht->count;
}

/* if this put function not provide, you MUST know how to proper free kv array!
int htPairArrayPut(htManager *pht,pair *pairArray)
{
	unsigned int i=0;
	if(!pht || pht->count <= 0 || !pairArray)
		return -1;

	for(i=0;i<pht->count;i++)
	{
		(*(pht->keyDestructor))(pairArray[i].key);
		(*(pht->valDestructor))(pairArray[i].val);
	}
	free(pairArray);
	return 0;
}*/

void htDestroy(htManager *pht)
{
	unsigned int index = 0;
	htItem *pHead = NULL;
	htItem *pCurItem = NULL;
	if(!pht)
		return;

	if(pht->tb0)
	{
		for(index=0; index<pht->size0 && pht->count > 0; index++)
		{
			//search collision list
			pHead = pht->tb0[index];
			pCurItem = pHead;
			while(pCurItem)
			{
				pHead = pCurItem->next;

				(*(pht->keyDestructor))(pCurItem->key);
				(*(pht->valDestructor))(pCurItem->val);
				free(pCurItem);

				pht->count--;
				pCurItem = pHead;
			}
		}
		free(pht->tb0);
		pht->size0 = 0;
		pht->tb0 = NULL;
	}
	if(pht->tb1)
	{
		for(index=0; index<pht->size1 && pht->count > 0; index++)
		{
			//search collision list
			pHead = pht->tb1[index];
			pCurItem = pHead;
			while(pCurItem)
			{
				pHead = pCurItem->next;

				(*(pht->keyDestructor))(pCurItem->key);
				(*(pht->valDestructor))(pCurItem->val);
				free(pCurItem);

				pht->count--;
				pCurItem = pHead;
			}
		}
		free(pht->tb1);
		pht->size1 = 0;
		pht->tb1 = NULL;
	}
	free(pht);
	return;
}

//============== for DEBUG =================
static int collisionListLen(htItem *pHead)
{
	htItem *pCurItem = pHead;
	int len = 0;
	while(pCurItem)
	{
		len++;
		pCurItem = pCurItem->next;
	}
	return len;
}

void debugHt(htManager *pht)
{
	unsigned int index = 0;
	htItem *pHead = NULL;
	unsigned int maxLen = 0;
	unsigned int tmp = 0;
	int *stat = NULL;
	int sum = 0;
	
	if(pht->tb0)
	{
		maxLen = 0;
		for(index=0; index<pht->size0; index++)
		{
			pHead = pht->tb0[index];
			if((tmp = collisionListLen(pHead)) > maxLen)
				maxLen = tmp;
		}
		stat = (int *)malloc((maxLen+1)*sizeof(int));
		
		for(index=0; index<pht->size0; index++)
		{
			pHead = pht->tb0[index];
			stat[collisionListLen(pHead)]++;
		}
		printf("tb0 stat:(%u/%u),max collision len %u\n",pht->count,pht->size0,maxLen);
		for(index=0;index<=maxLen;index++)
		{
			sum += index * stat[index];
			printf("len %u (%u lists)\n",index,stat[index]);
		}
		printf("calculated sum = %u\n",sum);
		free(stat);
	}
	if(pht->tb1)
	{
		maxLen = 0;
		for(index=0; index<pht->size1; index++)
		{
			pHead = pht->tb1[index];
			if((tmp = collisionListLen(pHead)) > maxLen)
				maxLen = tmp;
		}
		stat = (int *)malloc((maxLen+1)*sizeof(int));
		
		for(index=0; index<pht->size1; index++)
		{
			pHead = pht->tb1[index];
			stat[collisionListLen(pHead)]++;
		}
		printf("tb1 stat:(%u/%u),max collision len %u\n",pht->count,pht->size1,maxLen);
		for(index=0;index<=maxLen;index++)
			printf("len %u (%u lists)\n",index,stat[index]);
		free(stat);
	}
}