/*
 * =====================================================================================
 *
 *       Filename:  extruderOutput.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/23 15:47:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "extruderOutput.h"
u16 CCR1_Val = 2000;
u16 CCR2_Val = 2000;
void ExtruderIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	   //GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = Ex1Fan_Pin | Ex1Heat_Pin;	   //LED1/2       //将PB4/PB5 配置为通用推挽输出  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	GPIO_Init(Ex1_Port, &GPIO_InitStructure);
	Tim1_config();
  Tim2_config();
}/*}}}*/

void Tim1_config(void)/*{{{*/
{
	TIM_TimeBaseInitTypeDef  TIM1_TimeBaseStructure; //定时器初始化结构
	TIM_OCInitTypeDef  TIM1_OCInitStructure;		   //通道输出初始化结构

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);/* TIM3 clock enable */
	TIM_DeInit(TIM1);

	TIM1_TimeBaseStructure.TIM_Prescaler = 35;//(36-1)预分频 72M/36=2M
	TIM1_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM1_TimeBaseStructure.TIM_Period = 40000;//周期
	TIM1_TimeBaseStructure.TIM_ClockDivision = 0x0;// 时钟分割

	TIM_TimeBaseInit(TIM1,&TIM1_TimeBaseStructure);

	TIM_OCStructInit(&TIM1_OCInitStructure);          //默认参数
	/* PWM1 Mode configuration: Channel1 */
	TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //脉宽调制方式2
	TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//使能输出比较                 
	TIM1_OCInitStructure.TIM_Pulse = CCR1_Val;  //输出比较值 调制占空比
	TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;	//输出比较极性低

	TIM_OC1Init(TIM1,&TIM1_OCInitStructure);	  //初始化TIM4 通道1
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);        
	TIM_Cmd(TIM1, ENABLE);
	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}/*}}}*/

void Tim2_config(void)/*{{{*/
{
	TIM_TimeBaseInitTypeDef  TIM2_TimeBaseStructure; //定时器初始化结构
	TIM_OCInitTypeDef  TIM2_OCInitStructure;		   //通道输出初始化结构

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);/* TIM3 clock enable */

	TIM_DeInit(TIM2);

	TIM2_TimeBaseStructure.TIM_Prescaler = 35;//(36-1)预分频 72M/36=2M
	TIM2_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM2_TimeBaseStructure.TIM_Period = 40000;//周期
	TIM2_TimeBaseStructure.TIM_ClockDivision = 0x0;// 时钟分割

	TIM_TimeBaseInit(TIM2,&TIM2_TimeBaseStructure);

	TIM_OCStructInit(&TIM2_OCInitStructure);          //默认参数
	/* PWM1 Mode configuration: Channel1 */
	TIM2_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //脉宽调制方式2
	TIM2_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//使能输出比较                 
	TIM2_OCInitStructure.TIM_Pulse = CCR1_Val;  //输出比较值 调制占空比
	TIM2_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;	//输出比较极性低
	TIM_OC1Init(TIM2,&TIM2_OCInitStructure);	  //初始化TIM2 通道1
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	/* TIM1 Main Output Enable */
	//	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}/*}}}*/

