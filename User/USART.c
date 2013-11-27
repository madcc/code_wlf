//使用printf函数时注意要在option for target中勾选USE MicroLib选项，并添加stdio.h
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

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				   //PA10为串口输入RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				   //PA9为串口输出TX 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		   //口线翻转速度为2MHz	 最大波特率只需115.2k，那么用2M的GPIO的引脚速度就够了，既省电也噪声小
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;	 //初始化串口设置
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1,&USART_ClockInitStructure);

	USART_InitStructure.USART_BaudRate = 115200;				   //设置串口参数
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

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); /*接收中断使能*/
	//USART_ITConfig(USART1, USART_IT_RXNE|USART_IT_TXE, ENABLE); /*接收中断使能*/
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;   /*3.4的库不是使用USART1_IRQChannel，看stm32f10x.h吧*/	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 
}/*}}}*/

void USART1_IRQHandler(void) 	 //串口中断接收/*{{{*/
{ 
    if(USART_GetFlagStatus(USART1,USART_IT_RXNE)==SET) 
    {               
       		USART_ReceiveData(USART1); 
      	//while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ; 
	     if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	    { 
	        /* 清除接收中断标志*/ 
	        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	    }        
    }
	if(USART_GetFlagStatus(USART1,USART_IT_TC)==SET)
	{
		USART_ClearFlag(USART1, USART_FLAG_TC);
		if(USART_GetITStatus(USART1, USART_IT_TC) != RESET) 
	    { 
	        /* 清除发送中断标志*/ 
	        USART_ClearITPendingBit(USART1, USART_IT_TC);
	    }
	} 
	//如果使能了USART_IT_TXE，记得加上下面这几句，否则这个中断会一直被触发
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) 
	  {
		  USART_ClearITPendingBit(USART1, USART_IT_TXE);
          USART_ITConfig(USART1, USART_IT_TXE, DISABLE);    //失能发送中断	
	  }

}/*}}}*/

/*******************************************************************************
* Function Name  : int fputc(int ch, FILE *f)
* Description    : Retargets the C library printf function to the USART.printf重定向
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
* Description    : Retargets the C library printf function to the USART.fgetc重定向
* Input          : None
* Output         : None
* Return         : 读取到的字符
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
