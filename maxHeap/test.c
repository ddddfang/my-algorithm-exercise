#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maxHeap.h"

#define MAX_HEAP_NUM 10000

int maxHeap_test()
{
	int prior;
	PriorQueue *pPQ = NULL;
	int i = 0;
	int *abc = malloc(MAX_HEAP_NUM*sizeof(int));
	srand(0); //生成随机种子

	printf("\n====================随机生成 %d 个数==============================\n",MAX_HEAP_NUM);
	for(i=0;i<MAX_HEAP_NUM;i++)
	{
		abc[i] = rand()%(10*MAX_HEAP_NUM);
		printf("%d ",abc[i]);
	}
	printf("\n");
	
	printf("\n==================== 优先级队列 ==============================\n");
	
	createPriorQueue(&pPQ);

	printf("PQ");
	for(i=0;i<MAX_HEAP_NUM/10;i++)
	{
		printf("<== %d ",abc[i]);
		enPriorQueue(pPQ,abc[i]);
	}
	printf("\n");
	
	
	for(i=0;i<25;i++)
	{
		prior = dePriorQueue(pPQ);
		printf("==> %d \n",prior);
		//showQueue(pPQ);
	}

	destroyPriorQueue(pPQ);
	pPQ = NULL;

	
	printf("\n==================== 堆排序 ==============================\n");
	maxHeapSort(abc, MAX_HEAP_NUM);
	for(i=0;i<MAX_HEAP_NUM;i++)
	{
		printf("%d ",abc[i]);
	}
	printf("\n");
	
	return 0;
}

