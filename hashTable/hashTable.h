#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

typedef struct htItem
{
	struct htItem *next;
	void *key;
	void *val;
	unsigned int hashVal;
} htItem;

typedef struct
{
	unsigned int (*hashFunc)(const void *key);	//hash 函数
	int (*keyCmp)(const void *p1,const void *p2);	//key 比较函数
	void *(*keyDup)(const void *key);				//key 复制函数
	void *(*valDup)(const void *val);				//val 复制函数
	void (*keyDestructor)(void *key);				//key 销毁
	void (*valDestructor)(void *val);				//val 销毁
	unsigned int count;
	unsigned int size0;		//size = 2^i
	unsigned int size1;		//size = 2^i
	htItem** tb0;
	htItem** tb1;
	int rehash_index;
} htManager;

typedef struct
{
	void *key;
	void *val;
} pair;

htManager *htCreate(unsigned int baseSize,
					unsigned int (*hashFunc)(const void *key),
					int (*keyCmp)(const void *p1,const void *p2),
					void *(*keyDup)(const void *key),
					void *(*valDup)(const void *val),
					void (*keyDestructor)(void *key),
					void (*valDestructor)(void *val));
int htInsert(htManager *pht,const void *key,const void *val,void **oldVal);
int htDelete(htManager *pht,const void *key,void **pVal);
int htSearch(htManager *pht,const void *key,void **pVal);
int htToPairArray(htManager *pht,pair **pairArray,int pairCmp(const void *p1,const void *p2));
void htDestroy(htManager *pht);

void debugHt(htManager *pht);

#endif

