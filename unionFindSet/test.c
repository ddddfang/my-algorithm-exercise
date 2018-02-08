//并查集
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "uFindSet.h"

#define U_FIND_SET_NUM 10000

//==============================================================

int uFindSet_test()
{
	int i;
	stNode *abc[U_FIND_SET_NUM]={0};
	srand(0); //生成随机种子
	
	
	printf(" init group id \n");
	for(i=0;i<U_FIND_SET_NUM;i++)
	{
		createNode(&abc[i],i);
	}

	printf(" elem and their group\n");
	for(i=0;i<U_FIND_SET_NUM;i++)
	{
		unsigned long tmpid = findGroupId(abc[i]);
		printf("<init>element %d, group_id is %lu(%d))\n",i,tmpid,((stNode *)tmpid)->s32Data );
	}

	printf(" union some data randomly\n");
	for(i=0;i<U_FIND_SET_NUM;i++)
	{
		int a = rand()%U_FIND_SET_NUM;
		int b = rand()%U_FIND_SET_NUM;
		printf("union %d and %d\n",a,b);
		unionNode(abc[a],abc[b]);
	}
	printf(" elem and their group \n");
	for(i=0;i<U_FIND_SET_NUM;i++)
	{
		unsigned long tmpid = findGroupId(abc[i]);
		printf("<after>element %d, group_id is %lu(%d))\n",i,tmpid,((stNode *)tmpid)->s32Data );
	}
	printf("group_count %d\n",getGroupCnt());

	for(i=0;i<U_FIND_SET_NUM;i++)
	{
		destroyNode(abc[i]);
		abc[i] = NULL;
	}
	return 0;
}


