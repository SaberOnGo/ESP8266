/*
 * Copyright (C) 2014-2016 Wilddog Technologies. All Rights Reserved. 
 *
 * FileName: test_mutiple_observer.c
 *
 * ����Ŀ�� :  ���ڹҲ� ͬһ�� hosts  ����һ���ڵ�ʱ �����Ƿ�ɹ���Ҳ��ʱ��һ������Ƿ���Ȼ��Ч��
 * ���Բ��� : 	1. cmd:./test_mutiple_observer host/path1 host/path2
 *			 	2. ��Ұ����ҳ���޸� host/path1/value1 �� host/path2/value2 	 �����Ƿ����ͳɹ���
 *            	3. ���1����ظ�2�Ĳ���.
 *
 *************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wilddog.h"


STATIC void addObserver_callback_a
    (
    const Wilddog_Node_T* p_snapshot, 
    void* arg,
    Wilddog_Return_T err
    )
{
	
    *(BOOL*)arg = TRUE;
     if((err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED ) &&
        err != WILDDOG_ERR_RECONNECT)
    {
        wilddog_debug(" A addObserver failed! error code = %d",err);
        return;
    }
	wilddog_debug(" get A Observe !");
    wilddog_debug_printnode(p_snapshot);
    printf("\n");

    return;
}

STATIC void addObserver_callback_b
    (
    const Wilddog_Node_T* p_snapshot, 
    void* arg,
    Wilddog_Return_T err
    )
{
    *(BOOL*)arg = TRUE;
     if((err < WILDDOG_HTTP_OK || err >= WILDDOG_HTTP_NOT_MODIFIED ) &&
        err != WILDDOG_ERR_RECONNECT)
    {
        wilddog_debug(" B addObserver_callback2 failed! error code = %d",err);
        return;
    }
	 wilddog_debug(" get B Observe !");
    wilddog_debug_printnode(p_snapshot);
    printf("\n");

    return;
}
int main(int argc, char **argv) 
{
    BOOL aisFinish = FALSE,bisFinish = FALSE;
    Wilddog_T wilddog_a = 0;
	Wilddog_T wilddog_b = 0;

	if( argc != 3 )
	{
		printf("input : \t ./test_mutiple_observer url1/path1 url2/path2 \n");
		return -1;
	}
	wilddog_a = wilddog_initWithUrl((Wilddog_Str_T*)argv[1]);
	wilddog_b = wilddog_initWithUrl((Wilddog_Str_T*)argv[2]);
	wilddog_addObserver(wilddog_a, WD_ET_VALUECHANGE,\
                                      addObserver_callback_a, \
                                      (void*)&aisFinish);	
	
    wilddog_addObserver(wilddog_b, WD_ET_VALUECHANGE,\
                                      addObserver_callback_b, \
                                      (void*)&bisFinish);
    while(1)
    {

        wilddog_trySync();
		
    }

    wilddog_destroy(&wilddog_a);
    wilddog_destroy(&wilddog_b);
	

    return 0;
}

