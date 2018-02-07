#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <math.h>
#include "hashTable.h"


int hashTable_test1()
{
	int i=0;
	//char *v = NULL;
	char k[50] = {0};
	printf("======================================================\n");
	for(i=0;i<200;i++)
	{
		sprintf(k,"key_%d",i);
		printf("%s\n",k);
	}
	getchar();
	//i=100000;
	while(1)
	{
		htManager *pht = htCreate(0,NULL,NULL,NULL,NULL,NULL,NULL);
		//printf("htCreate ..\n");
		//usleep(1000000);

		htDestroy(pht);
		pht = NULL;
		usleep(1000);
		//printf("htDestroy \n");
	}
	return 0;
}

int hashTable_test2()
{
	int del_num[]={984,23,444,678,2550,3486,23};
	int reInsert_num[]={55,3905,344,2550,3486,23};
	void *val_temp=0;
	int i=0;
	int res = 0;
	htManager *pht = htCreate(0,NULL,NULL,NULL,NULL,NULL,NULL);
	//char *v = NULL;
	char k[50] = {0};
	
	/* insert test */
	for(i=0;i<2560;i++)
	{
		sprintf(k,"key_%d",i);
		res = htInsert(pht,k,(const void *)i,NULL);
		if(res == 1)
		{
			printf("ins(%s,%d)\n",k,i);
		}
		else if(res == 0)
		{
			printf("update a value ,wait for confirm\n");
			getchar();
		}
		else
		{
			printf("ins fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("insert ok, wait for continue\n");
	getchar();
	
	/* insert a exist element test */
	for(i=0;i<sizeof(reInsert_num)/sizeof(reInsert_num[0]);i++)
	{
		sprintf(k,"key_%d",reInsert_num[i]);
		res = htInsert(pht,k,(const void *)250,&val_temp);
		if(res == 1)
		{
			printf("ins(%s,%d)\n",k,i);
		}
		else if(res == 0)
		{
			printf("update a value k is %s,origin val is %d,wait for confirm\n",k,(int)val_temp);
			getchar();
		}
		else
		{
			printf("ins fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("insert ok, wait for continue\n");
	getchar();
	
	/* delete test */
	for(i=0;i<sizeof(del_num)/sizeof(del_num[0]);i++)
	{
		sprintf(k,"key_%d",del_num[i]);
		res = htDelete(pht,k,&val_temp);
		if(res == 1)
		{
			printf("del a element ok, k is %s,v is %d\n",k,(int)val_temp);
		}
		else if(res == 0)
		{
			printf("cannot find the emement you want to del(k=%s),need conform",k);
			getchar();
		}
		else
		{
			printf("htDelete fail ,wait for confirm\n");
			getchar();
		}
	}
	
	/* search test */
	for(i=0;i<2560;i++)
	{
		sprintf(k,"key_%d",i);
		res = htSearch(pht,k,&val_temp);
		if(res == 1)
		{
			printf("got, k is %s,v is %d\n",k,(int)val_temp);
			if((int)val_temp != i)
			{
				printf("seems this val is not correct,wait for confirm\n");
				getchar();
			}
		}
		else if(res == 0)
		{
			printf("does not find this value? wait for confirm\n");
			getchar();
		}
		else
		{
			printf("htSearch fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("search ok, wait for continue\n");
	getchar();
	//i=100000;
	//while(1)
	//	usleep(1000);
	htDestroy(pht);
	pht = NULL;
	return 0;
}

//============================================================
static void *myValDup(const void *val)
{
	unsigned int len = strlen((const char *)val);
	char *p = malloc(len+1);
	if(p)
	{
		memset(p,0,len+1);
		memcpy(p,val,len);
	}
	return (void *)p;
}

static void myValDestructor(void *val)
{
	free(val);
}

int hashTable_test3()
{
	int del_num[]={984,23,444,678,2550,3486,23};
	int reInsert_num[]={55,3905,344,2550,3486,23};
	void *val_temp=0;
	int i=0;
	int res = 0;
	htManager *pht = htCreate(0,NULL,NULL,NULL,myValDup,NULL,myValDestructor);
	//char *v = NULL;
	char k[50] = {0};
	char v[50] = {0};
	
	/* insert test */
	for(i=0;i<2560;i++)
	{
		sprintf(k,"key_%d",i);
		sprintf(v,"value_%d is me~~",i);
		res = htInsert(pht,k,v,NULL);
		if(res == 1)
		{
			printf("ins(%s,%s)\n",k,v);
		}
		else if(res == 0)
		{
			printf("update a value? ,wait for confirm\n");
			getchar();
		}
		else
		{
			printf("ins fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("insert ok, wait for continue\n");
	getchar();
	
	/* insert a exist element test */
	for(i=0;i<sizeof(reInsert_num)/sizeof(reInsert_num[0]);i++)
	{
		sprintf(k,"key_%d",reInsert_num[i]);
		sprintf(v,"I am value_%d insert in second wrap...",reInsert_num[i]);
		res = htInsert(pht,k,v,&val_temp);
		if(res == 1)
		{
			printf("ins(%s,%s)\n",k,v);
		}
		else if(res == 0)
		{
			printf("update a value k is %s,origin val is (%s),wait for confirm\n",k,(char *)val_temp);
			myValDestructor(val_temp);
			getchar();
		}
		else
		{
			printf("ins fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("insert ok, wait for continue\n");
	getchar();
	
	/* delete test */
	for(i=0;i<sizeof(del_num)/sizeof(del_num[0]);i++)
	{
		sprintf(k,"key_%d",del_num[i]);
		res = htDelete(pht,k,&val_temp);
		if(res == 1)
		{
			printf("del a element ok, k is %s,v is %s\n",k,(char *)val_temp);
			myValDestructor(val_temp);
		}
		else if(res == 0)
		{
			printf("cannot find the emement you want to del(k=%s),need conform",k);
			getchar();
		}
		else
		{
			printf("htDelete fail ,wait for confirm\n");
			getchar();
		}
	}
	
	/* search test */
	for(i=0;i<2560;i++)
	{
		sprintf(k,"key_%d",i);
		sprintf(v,"value_%d is me~~",i);
		res = htSearch(pht,k,&val_temp);
		if(res == 1)
		{
			printf("got, k is %s,v is %s\n",k,(char *)val_temp);
			if(strcmp(v,val_temp) != 0)
			{
				printf("seems this val is not correct,wait for confirm\n");
				getchar();
			}
		}
		else if(res == 0)
		{
			printf("does not find this value? wait for confirm\n");
			getchar();
		}
		else
		{
			printf("htSearch fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("search ok, wait for continue\n");
	getchar();
	debugHt(pht);
	
	//i=100000;
	//while(1)
	//	usleep(1000);
	htDestroy(pht);
	pht = NULL;
	return 0;
}

//=====================================================
static int myPairCmp(const void *a1,const void *a2)
{
	pair *p1 = (pair *)a1;
	pair *p2 = (pair *)a2;
	return ((int)(p1->val) - (int)(p2->val));
}

int hashTable_test4()
{
	int i=0;
	int res = 0;
	htManager *pht = htCreate(0,NULL,NULL,NULL,NULL,NULL,NULL);
	
	char k[50] = {0};
	int v=0;
	srand(0); //生成随机种子

	/* insert test */
	for(i=0;i<100;i++)
	{
		sprintf(k,"key_%d",i);
		v = rand()%100;
		res = htInsert(pht,k,(const void *)v,NULL);
		if(res == 1)
		{
			printf("ins(%s,%d)\n",k,v);
		}
		else if(res == 0)
		{
			printf("update a value key=%s,val=%d,wait for confirm\n",k,v);
			getchar();
		}
		else
		{
			printf("ins fail ,wait for confirm\n");
			getchar();
		}
	}
	printf("insert ok,total %u elements, wait for continue\n",pht->count);
	getchar();
	
	pair *pKV = NULL;
	unsigned int pairLen=0;
	pairLen = htToPairArray(pht,&pKV,myPairCmp);
	//pairLen = htToPairArray(pht,&pKV,NULL);

	//then we can even destroy hash table here..,because that kv array has no relationship with hashtable any more
	htDestroy(pht);
	pht = NULL;
	
	

	for(i=0;i<pairLen;i++)
	{
		printf("(%s,%d)\n",pKV[i].key,(int)pKV[i].val);
	}
	
	for(i=0;i<pairLen;i++)
	{
		free(pKV[i].key);
	}
	free(pKV);
	pKV = NULL;

	printf("ok.\n");
	getchar();
	
	return 0;
}