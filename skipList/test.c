#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skiplist.h"


int skipList_test1()
{
	char v[50] = {0};
	long i=0;
	skipListManager *psl = skipListCreate(NULL,NULL,NULL,NULL,NULL);

	for(i=0;i<1000;i++)
	{
		sprintf(v,"val_%ld",i);
		printf("ins (%ld,%s)\n",i,v);
		skipListInsert(psl, (void *)i, (void *)v,NULL,NULL);
	}
	printf("ins complete,press enter to continue\n");
	getchar();

	skipListDelete(psl, (void *)560, NULL);

	for(i=555;i<575;i++)
	{
		char *deletedVal = NULL;
		int delRes = skipListDelete(psl, (void *)i, (void *)&deletedVal);
		if(delRes == 0)
		{
			printf("cannot find element,key=%ld,need confirm\n",i);
			getchar();
		}
		else if(delRes == 1)
		{
			printf("element deleted,(%ld,%s)\n",i,deletedVal);
		}
		free(deletedVal);
	}
	printf("del complete,press enter to continue\n");
	getchar();

	skipListInsert(psl, (void *)123, "hello fang~ ",NULL,NULL);

	for(i=0;i<1000;i++)
	{
		sprintf(v,"val_%ld",i);
		char *searchedVal = NULL;
		int searchRes = skipListSearch(psl, (void *)i, (void *)&searchedVal,NULL);
		if(searchRes == 0)
		{
			printf("cannot find element,key=%ld,need confirm\n",i);
			getchar();
		}
		else if(searchRes > 0)
		{
			if(strcmp(searchedVal,v)!=0)
			{
				printf("find element,key=%ld,v=%s(modified ?),need confirm\n",i,searchedVal);
				getchar();
			}
			else
			{
				printf("find element,(%ld,%s)\n",i,searchedVal);
			}
		}
		free(searchedVal);
	}
	printf("search complete,press enter to continue\n");
	getchar();
	debugPrint(psl);
	skipListDestroy(psl);
	return 0;
}

int skipList_test2()
{
	unsigned int rank;
	char v[50] = {0};
	long i=0;
	skipListManager *psl = skipListCreate(NULL,NULL,NULL,NULL,NULL);

	for(i=0;i<1000;i++)
	{
		sprintf(v,"val_%ld",i);
		//这里吧i当作指针传入其实都是有风险的,因为 skipListInsert 可能操作 以i起始的 8 bytes(但是i其实只占用4bytes)
		skipListInsert(psl, (void *)i, (void *)v,NULL,&rank);
		printf("ins (%ld,%s) in pos %u\n",i,v,rank);
	}
	printf("ins complete,press enter to continue\n");
	getchar();

	skipListDelete(psl, (void *)560, NULL);
	for(i=1;i<=1000;i++)
	{
		void *searchedKey = 0;
		void *searchedVal = NULL;
		int searchRes = skipListSearchKth(psl,i,&searchedKey,&searchedVal);
		if(searchRes == 0)
		{
			printf("cannot find element,rank=%ld,need confirm\n",i);
			getchar();
		}
		else if(searchRes > 0)
		{
			printf("find element,rank %ld is (%ld,%s)\n",i,(long)searchedKey,(char *)searchedVal);
		}
		free(searchedVal);
	}
	printf("search complete,press enter to continue\n");
	getchar();

	for(i=1;i<=15;i++)
	{
		void *deletedKey = 0;
		void *deletedVal = NULL;
		int deleteRes = skipListDeleteKth(psl, 20, &deletedKey, &deletedVal);
		if(deleteRes == 0)
		{
			printf("cannot find element,need confirm\n");
			getchar();
		}
		else
		{
			printf("del rank 20 element,(%ld,%s)\n",(long)deletedKey,(char *)deletedVal);
		}
	}
	printf("del complete,press enter to continue\n");
	getchar();
	debugPrint(psl);
	skipListDestroy(psl);
	return 0;
}

static void myapply1(void *k, void *v,void *cl)
{
	printf("hi,(%ld,%s)\n",(long)k,(char *)v);
}
static void myapply2(void *k, void *v,void *cl)
{
	if(cl == NULL)
	{
		//
	}
	skipListManager *psl = cl;
	//printf("hi,(%ld,%s)\n",(long)k,(char *)v);
	skipListDelete(psl, k, NULL);
}

int skipList_test3()
{
	unsigned int rank;
	char v[50] = {0};
	long i=0;
	skipListManager *psl = skipListCreate(NULL,NULL,NULL,NULL,NULL);

	for(i=0;i<100;i++)
	{
		sprintf(v,"val_%ld",i);
		skipListInsert(psl, (void *)i, (void *)v,NULL,&rank);
		printf("ins (%ld,%s) in pos %u\n",i,v,rank);
	}
	printf("ins complete,press enter to show\n");
	getchar();
	debugPrint(psl);
	printf("press enter to continue\n");
	getchar();

	skipListSearchRangeByKey(psl,(void *)10,(void *)15,myapply1,(void *)psl);
	printf("search range by rank,actual del ops,press enter to show\n");
	getchar();
	debugPrint(psl);
	printf("press enter to continue\n");
	getchar();

	skipListSearchRangeByRank(psl,34,55,myapply2,(void *)psl);
	printf("search range by key,press enter to show\n");
	getchar();
	debugPrint(psl);
	printf("press enter to continue\n");
	getchar();


	skipListDeleteRangeByRank(psl, 5, 100);
	printf("press enter to continue\n");
	getchar();

	debugPrint2(psl);
	skipListDestroy(psl);
	return 0;
}