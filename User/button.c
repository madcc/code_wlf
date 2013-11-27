/*
 * =====================================================================================
 *
 *       Filename:  button.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/30 16:47:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "button.h"

enum BtnCode btnCode;
void btnGPIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOA, ENABLE);	   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //���߷�ת�ٶ�Ϊ50MHz
	
	GPIO_InitStructure.GPIO_Pin = Btn_Left_Pin;
	GPIO_Init(Btn_Left_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Btn_Right_Pin;
	GPIO_Init(Btn_Right_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Btn_Up_Pin;
	GPIO_Init(Btn_Up_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Btn_Down_Pin;
	GPIO_Init(Btn_Down_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Btn_Center_Pin;
	GPIO_Init(Btn_Center_Port, &GPIO_InitStructure);

	btnExtiInit();
}/*}}}*/
void btnExtiInit(void) //���������ⲿ�жϣ����ö�ʱ���ж�ȥ�ظ�����Է���/*{{{*/
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);	
	EXTI_InitStructure.EXTI_Line = EXTI_Line15;						
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource10);	
	EXTI_InitStructure.EXTI_Line = EXTI_Line10;						
	EXTI_Init(&EXTI_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource2);	
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;						
	EXTI_Init(&EXTI_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource12);	
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;						
	EXTI_Init(&EXTI_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource11);	
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;						
	EXTI_Init(&EXTI_InitStructure);	
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //ռ�����ȼ��������ȼ�����Դ����

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; //ָ���ж�Դ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //ռ�����ȼ��趨
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //�����ȼ��趨
    NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;
    NVIC_Init(&NVIC_InitStructure);			 

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //ָ���ж�Դ
    NVIC_Init(&NVIC_InitStructure);			 

	btnCode=0;
	Tim3IntrInit();
}/*}}}*/
void Tim3IntrInit(void)/*{{{*/
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_DeInit(TIM3);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseStructure.TIM_Period=20;//ARR��ֵ
	TIM_TimeBaseStructure.TIM_Prescaler=7200-1;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);   // ��ռʽ���ȼ��� 
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;//ָ���ж�Դ
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;// ָ����Ӧ���ȼ���1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM3,DISABLE);
	//	TIM_Cmd(TIM3, ENABLE); //����ʱ��
}/*}}}*/
