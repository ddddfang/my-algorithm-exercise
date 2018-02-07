#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "maxHeap.h"


//一般在操作堆顶后,用此做 shiftDown 调整
//eg.1 删除堆顶: 交换堆顶和最后元素(其实根本不用交换,直接用last覆盖堆顶),then size--
//eg.2 新添一个节点在pos位置(或者更改堆顶点),如果这个pos节点
//     比较小(和左右子节点比),那么此节点会一直 沉到它应该呆的位置
void shiftDown(int *a, int pos, int last)	//last 是最后一个节点的 index
{
	int left = 2*pos+1;		//pos 的左孩子的 index
	int right = 2*pos+2;	//pos 的右孩子的 index
	int max = pos;
	int tmp;

	if(pos <= (last-1)/2)	//叶子节点没必要调整
	{
		if(left <= last && a[left] > a[max])
			max = left;

		if(right <= last && a[right] > a[max])
			max = right;	//now,max已经是父子三人中最大的那个元素的 index 了

		if(max != pos)		//max 是某个子节点的 index
		{
			//swap(a[pos], a[max]);
			tmp = a[pos];
			a[pos] = a[max];
			a[max] = tmp;
			shiftDown(a, max, last);
		}
	}
}

//heapShiftDown 之非递归实现(迭代实现)
void shiftDown2(int *a, int pos, int last)
{
	int left;		//pos 的左孩子的 index
	int right;		//pos 的右孩子的 index
	int max = pos;		//先假设 pos 最大
	int tmp;
	int flag = 0;	//为1表示已经满足最大堆约束

	while(pos <= (last-1)/2 && (flag==0))	//叶子节点没必要调整
	{
		left = 2*pos+1;		//pos 的左孩子的 index
		right = 2*pos+2;	//pos 的右孩子的 index
		//max = pos;			//先假设 pos 最大

		if(left <= last && a[left] > a[max])
			max = left;

		if(right <= last && a[right] > a[max])
			max = right;	//now,max已经是父子三人中最大的那个元素的 index 了

		if(max != pos)		//max 是某个子节点的 index
		{
			//swap(a[pos], a[max]);
			tmp = a[pos];
			a[pos] = a[max];
			a[max] = tmp;
			//
			pos = max;
		}
		else
		{
			flag = 1;
		}
	}
}

//当改动了heap的最后一个元素的时候需要调用此 ShiftUp 调整一下
void ShiftUp(int *a, int pos)
{
	int flag = 0;
	int tmp;

	while((pos > 0) && (flag==0))	//如果已经是堆顶了(pos == 0),就不要再调整了
	{
		if(a[pos] > a[(pos-1)/2])
		{
			//swap(a[pos], a[(pos-1)/2]);
			tmp = a[pos];
			a[pos] = a[(pos-1)/2];
			a[(pos-1)/2] = tmp;
			//
		}
		else
		{
			flag = 1;
		}
		pos = (pos-1)/2;
	}
}

//============================= 堆排序 ================================================

//将一个数组搞成一个最大堆(并不严格排序!),a[0]不管他了???
void maxHeapBuild(int *a, int size)
{
	int i;
	//last=size-1,last的parent就是 (last-1)/2 = size/2 -1
	for(i = size/2 - 1; i>=0; i--)	//从所有有孩子的节点往上遍历(相当于挨个插入节点了)
	{
		shiftDown2(a,i,size-1);
	}
}

//将一个数组搞成最大堆,然后利用最大堆属性
//慢慢将数组搞成严格排序
void maxHeapSort(int *a, int size)
{
	int i;
	int tmp;
	maxHeapBuild(a, size);
	for(i = size-1; i>=0; i--)
	{
		//swap(a[i], a[1]);	//交换堆顶 a[0] 和最后一个元素(即 将最大值放在了最后)
		tmp = a[i];
		a[i] = a[0];
		a[0] = tmp;
		//最大值放在最后,同时最后的那个值也放在了堆顶,肯定需要重新调整为 maxHeap,
		//(最后的那个元素已经不把他当作堆中一员了)
		shiftDown(a,0,i-1);
	}
}



//============================= 优先级队列 ==============================================

void createPriorQueue(PriorQueue **ppPriorQueue)
{
	PriorQueue *p = NULL;
	if(!ppPriorQueue)
	{
		printf("createPriorQueue:invalid param..\n");
		return;
	}
	p = (PriorQueue *)malloc(sizeof(PriorQueue));
	if(!p)
	{
		printf("createPriorQueue:no avail mem for PriorQueue struct..\n");
		return;
	}
	p->a = (int *)malloc(PRIOR_QUEUE_BASE_SIZE*sizeof(int));	//sizeof(int)忘掉写了,会导致free失败?
	if(!(p->a))
	{
		printf("createPriorQueue:no avail mem for PriorQueue array..\n");
		return;
	}
	p->totalSize = PRIOR_QUEUE_BASE_SIZE;
	p->validSize = 0;
	*ppPriorQueue = p;
}

void destroyPriorQueue(PriorQueue *pPriorQueue)
{
	if(!pPriorQueue)
	{
		printf("destroyPriorQueue:invalid param..\n");
		return;
	}
	if(pPriorQueue->a)
		free(pPriorQueue->a);
	free(pPriorQueue);
}

void enPriorQueue(PriorQueue *pPriorQueue,int prior)
{
	int *pTmp = NULL;
	if(!pPriorQueue)
	{
		printf("enPriorQueue:invalid param..\n");
		return;
	}
	if(pPriorQueue->validSize >= pPriorQueue->totalSize)
	{
		if(pPriorQueue->totalSize + PRIOR_QUEUE_INCREASE_SIZE > PRIOR_QUEUE_MAX_SIZE)
		{
			printf("enPriorQueue:exceed max size, will not alloc mem for heap any more\n");
			return;
		}
		pTmp = pPriorQueue->a;
		pPriorQueue->a = (int *)malloc((pPriorQueue->totalSize+PRIOR_QUEUE_INCREASE_SIZE)*sizeof(int));
		if(!pPriorQueue->a)
		{
			printf("enPriorQueue:no avail mem for PriorQueue array..\n");
			return;
		}
		memcpy(pPriorQueue->a,pTmp,pPriorQueue->totalSize);
		pPriorQueue->totalSize += PRIOR_QUEUE_INCREASE_SIZE;
		printf("enPriorQueue:increase heap size success..\n");
		free(pTmp);
		pTmp = NULL;
	}
	pPriorQueue->a[pPriorQueue->validSize] = prior;
	ShiftUp(pPriorQueue->a, pPriorQueue->validSize);
	pPriorQueue->validSize++;
}

int dePriorQueue(PriorQueue *pPriorQueue)
{
	int prior;
	if(!pPriorQueue)
	{
		printf("dePriorQueue:invalid param..\n");
		return -1;
	}
	if(pPriorQueue->validSize == 0)
	{
		printf("dePriorQueue:prior queue is empty..\n");
		return -1;
	}
	prior = pPriorQueue->a[0];
	pPriorQueue->validSize--;
	pPriorQueue->a[0] = pPriorQueue->a[pPriorQueue->validSize];	//直接用最后一个覆盖[0]
	shiftDown2(pPriorQueue->a, 0, pPriorQueue->validSize-1);	//原来的那个最后一个已经不算再内了
	return prior;
}

//for debug
void showQueue(PriorQueue *pPriorQueue)
{
	int i;
	if(!pPriorQueue)
		return;
	for(i=0;i<pPriorQueue->validSize;i++)
	{
		printf("%d ,",pPriorQueue->a[i]);
	}
	printf("\n");
}



