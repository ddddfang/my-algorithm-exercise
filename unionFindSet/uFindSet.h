#ifndef _U_FIND_SET
#define _U_FIND_SET


typedef struct
{
	int s32Data;
	unsigned long s32GroupId;
}stNode;

void createNode(stNode **ppNode,int data);
void destroyNode(stNode *pNode);
unsigned long findGroupId(stNode *pNode);
void unionNode(stNode *x,stNode *y);
int getGroupCnt();

#endif
