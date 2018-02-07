#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "commonTree.h" 


static int cmp_my(const void *p1,const void *p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}
static void apply_display(void *k,void **pv,void *cl)
{
	int i;
	if(k)
		printf("%10d\n",*(int *)k);	
}

/*static void apply_display2(void **x,void *cl,int lev)
{
	int i;
	for(i=0;i<lev;i++)
		printf("    ");	//3 blanks
	if(x)
		printf("%10d\n",(int)*x);
	else
		printf("%10d\n",(int)0);		
}*/

#define NUMBER 100

int commonTree_test1()
{
	int i=0;
	cTree myTree = cTreeCreate(cmp_my);
	cTreeNodeId element_id;
	
	int *p = (int *)malloc(NUMBER*sizeof(int));
	srand(0); //生成随机种子

	for(i=0;i<NUMBER;i++)
	{
		p[i] = rand()%(NUMBER*10);
		cTreeInsert(myTree,(void *)&p[i],sizeof(p[i]),NULL);
		printf("[%d]insert %d\n",i,(int)p[i]);
	}
	printf("\n");
	printf("total %d elements\n",myTree->total);
	cTreeMap(myTree,apply_display,NULL);
	//cTreeMap2(myTree,apply_display2, NULL);

	for(i=0;i<NUMBER/10;i++)
	{
		cTreeErase(myTree,(void *)&p[i]);
		printf("erase %d\n",(int)p[i]);
	}
	printf("\n");
	printf("total %d elements\n",myTree->total);
	cTreeMap(myTree,apply_display,NULL);

    getchar();
	cTreeDestroy(&myTree);
	free(p);
	return 0;
}

int commonTree_test2()
{
	int i=0;
	cTree myTree = cTreeCreate(NULL);
	cTreeNodeId element_id;
	
	int *p = (int *)malloc(NUMBER*sizeof(int));
	srand(0); //生成随机种子

	for(i=0;i<NUMBER;i++)
	{
		p[i] = rand()%(NUMBER*10);
		cTreeInsert(myTree,(void *)&p[i],sizeof(p[i]),NULL);
		printf("[%d]insert %d\n",i,(int)p[i]);
	}
	printf("\n");
	printf("total %d elements\n",myTree->total);
	cTreeMap(myTree,apply_display,NULL);

	
	for(i=0;i<NUMBER;i++)
	{
		element_id = cTreeSearch(myTree,(void *)p[i]);
		if(element_id)
		{
			printf("found element %d\n",(int)getKdataFromNodeId(element_id));
			cTreeEraseFromNodeId(myTree,&element_id);
		}
		else
			printf("ERRRRRRRRRROR,element %d cannot be found .........\n",p[i]);			
	}
	printf("total %d elements\n",myTree->total);
	printf("\n");
	

    getchar();
	cTreeDestroy(&myTree);
	free(p);
	return 0;
}


/////////////////////////////////
typedef struct
{
	char key[20];
	int value;
} binding;

static binding *createBinds(char *k,int value)
{
	binding *p = (binding *)malloc(sizeof(binding));
	if(p)
	{
		memset(p,0,sizeof(binding));
		memcpy(p->key,k,strlen(k));
		p->value = value;
	}
	return p;
}

static void destroyBinds(binding *p)
{
	free(p);
}

static void apply_kvdisplay(void **x,void *cl)
{
	binding *pB = *x;
	printf("key:%s, value %d\n",pB->key,pB->value);	
}

static int mycmp(const void *p1,const void *p2)
{
	const binding *pB1 = p1;
	const binding *pB2 = p2;
	
	return strcmp(pB1->key,pB2->key);
}
