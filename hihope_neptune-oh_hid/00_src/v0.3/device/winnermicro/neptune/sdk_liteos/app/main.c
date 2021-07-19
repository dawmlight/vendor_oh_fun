/***************************************************************************** 
* 
* File Name : main.c
* 
* Description: main 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-14
*****************************************************************************/ 
#include "wm_include.h"

void UserMain(void)
{
	printf("\n user task:%s:%s \n",__TIME__,__DATE__);

	extern int tls_at_bt_enable(int uart_no, int log_level);
        tls_at_bt_enable(0xff,0);		
#if DEMO_CONSOLE
	CreateDemoTask();
#endif
}

