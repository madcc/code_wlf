//ʹ��printf����ʱע��Ҫ��option for target�й�ѡUSE MicroLibѡ������stdio.h
#include "stm32f10x.h"
#include <stdio.h>
#include "USART.h"
extern uint8_t setTargetTemp;
extern uint16_t setTempIntegrade;
extern bool setIntegradeChange;
void USART_Config(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef  USART_ClockInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				   //PA10Ϊ��������RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				   //PA9Ϊ�������TX 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		   //���߷�ת�ٶ�Ϊ2MHz	 �������ֻ��115.2k����ô��2M��GPIO�������ٶȾ͹��ˣ���ʡ��Ҳ����С
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;	 //��ʼ����������
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1,&USART_ClockInitStructure);

	USART_InitStructure.USART_BaudRate = 115200;				   //���ô��ڲ���
	USART_InitStructure.USART_WordLength =USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStructure);

	USART_Cmd(USART1,ENABLE);
}/*}}}*/

void USART_IT_Config(void)/*{{{*/
{
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); /*�����ж�ʹ��*/
	//USART_ITConfig(USART1, USART_IT_RXNE|USART_IT_TXE, ENABLE); /*�����ж�ʹ��*/
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;   /*3.4�Ŀⲻ��ʹ��USART1_IRQChannel����stm32f10x.h��*/	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}/*}}}*/

void USART1_IRQHandler(void) 	 //�����жϽ���/*{{{*/
{ 
    if(USART_GetFlagStatus(USART1,USART_IT_RXNE)==SET) 
    {               
       		USART_ReceiveData(USART1); 
      	//while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ; 
	     if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	    { 
	        /* ��������жϱ�־*/ 
	        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	    }        
    }
	if(USART_GetFlagStatus(USART1,USART_IT_TC)==SET)
	{
		USART_ClearFlag(USART1, USART_FLAG_TC);
		if(USART_GetITStatus(USART1, USART_IT_TC) != RESET) 
	    { 
	        /* ��������жϱ�־*/ 
	        USART_ClearITPendingBit(USART1, USART_IT_TC);
	    }
	} 
	//���ʹ����USART_IT_TXE���ǵü��������⼸�䣬��������жϻ�һֱ������
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) 
	  {
		  USART_ClearITPendingBit(USART1, USART_IT_TXE);
          USART_ITConfig(USART1, USART_IT_TXE, DISABLE);    //ʧ�ܷ����ж�	
	  }

}/*}}}*/

/*******************************************************************************
* Function Name  : int fputc(int ch, FILE *f)
* Description    : Retargets the C library printf function to the USART.printf�ض���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int fputc(int ch, FILE *f)
{
  /* Write a character to the USART */
  USART_SendData(USART1, (u8) ch);

  /* Loop until the end of transmission */
  while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET))
  {
  }

  return ch;
}

/*******************************************************************************
* Function Name  : int fgetc(FILE *f)
* Description    : Retargets the C library printf function to the USART.fgetc�ض���
* Input          : None
* Output         : None
* Return         : ��ȡ�����ַ�
*******************************************************************************/
int fgetc(FILE *f)
{
  /* Loop until received a char */
  while(!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET))
  {
  }
  
    /* Read a character from the USART and RETURN */
  return (USART_ReceiveData(USART1));
}
