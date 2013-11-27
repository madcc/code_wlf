/*
 * =====================================================================================
 *
 *       Filename:  timerset.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/4/1 21:29:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "timerset.h"
#include "motor.h"


void Tim4IntrInit(void)/*{{{*/
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_DeInit(TIM4);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseStructure.TIM_Period=INTERVAL_IN_MICROSECONDS;//ARR的值
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);   // 抢占式优先级别 
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //指定中断源
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;// 指定响应优先级别1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_Cmd(TIM4, ENABLE); //开启时钟
}/*}}}*/


void TIM4_IRQHandler()/*{{{*/
{
	if(TIM_GetITStatus(TIM4 , TIM_IT_Update) == SET)
	{
		motorDoInterrupt();
		TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update);
	}
}/*}}}*/
