/*
 * =====================================================================================
 *
 *       Filename:  globalDef.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/1/22 17:10:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "globalDef.h"

vu32 sysMicros=0;
vu32 TimingDelay;
		
#ifdef VERSION
const u16 firmwareVersion=5;
#endif

uint32_t usrInputTime=300*1000*1000;


void delay_us(u32 nTime)
{
	TimingDelay=nTime;
	while(TimingDelay !=0);
}
void delay_ms(u32 nTime)
{
	int i;
	for(i=0;i<1000;i++)
		delay_us(nTime);
}

