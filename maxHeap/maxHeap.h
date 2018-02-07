#ifndef __MAX_HEAP
#define __MAX_HEAP

void shiftDown(int *a, int pos, int last);
void shiftDown2(int *a, int pos, int last);
void ShiftUp(int *a, int pos);

//============================================================
void maxHeapBuild(int *a, int size);
void maxHeapSort(int *a, int size);


//============================================================
#define PRIOR_QUEUE_BASE_SIZE		50
#define PRIOR_QUEUE_MAX_SIZE		100
#define PRIOR_QUEUE_INCREASE_SIZE	10
typedef struct
{
	int *a;
	int totalSize;
	int validSize;	//validSize 其实也是第一个空闲的位置
	//mutex SHOULD be used
}PriorQueue;

void createPriorQueue(PriorQueue **ppPriorQueue);
void destroyPriorQueue(PriorQueue *pPriorQueue);
void enPriorQueue(PriorQueue *pPriorQueue,int prior);
int dePriorQueue(PriorQueue *pPriorQueue);
void showQueue(PriorQueue *pPriorQueue);

#endif

