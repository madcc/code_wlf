/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    10/15/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "globalDef.h"
#include "button.h"
extern uint32_t ledToggleTime;
extern int toggleLed;
extern uint8_t btnCode;
uint8_t btnShakeCode=0;
uint8_t tim3IntrCnt=0;
extern uint16_t currentTemperature;
extern uint8_t setTargetTemp;
bool isNoise;
uint8_t stableTempCnt=0;
extern bool isTempStable;
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	sysMicros++;
	if(TimingDelay!=0x00)
		TimingDelay--;
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)		   /*{{{*/
{
	if (EXTI_GetITStatus(EXTI_Line10) != RESET)
	{
		btnShakeCode =BC_Down;
		TIM_Cmd(TIM3,ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line10);	 //清中断标志位		
	}
	if (EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		btnShakeCode =BC_Center;
		TIM_Cmd(TIM3,ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line11);	 //清中断标志位		
	}
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		btnShakeCode =BC_Up;
		TIM_Cmd(TIM3,ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line12);	 //清中断标志位		
	}
	if (EXTI_GetITStatus(EXTI_Line15) != RESET)
	{
		btnShakeCode =BC_Left;
		TIM_Cmd(TIM3,ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line15);	 //清中断标志位		
	}
}/*}}}*/
void EXTI2_IRQHandler(void)		   /*{{{*/
{
	if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		btnShakeCode =BC_Right;
		TIM_Cmd(TIM3,ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line2);	 //清中断标志位		
	}
}/*}}}*/

void TIM3_IRQHandler()/*{{{*/
{
 if(TIM_GetITStatus(TIM3 , TIM_IT_Update) == SET)
 {
	 switch(btnShakeCode)
	 {
		case BC_Down:
			if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_10))
				tim3IntrCnt++;
			else isNoise=true;
			break;
		case BC_Left:
			if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15))
				tim3IntrCnt++;
			else isNoise=true;
			break;
		case BC_Center:
			if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11))
				tim3IntrCnt++;
			else isNoise=true;
			break;
		case BC_Right:
			if(!GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2))
				tim3IntrCnt++;
			else isNoise=true;
			break;
		case BC_Up:
			if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12))
				tim3IntrCnt++;
			else isNoise=true;
			break;
		default:tim3IntrCnt=0;break;
	 }
//	 TIM_Cmd(TIM3,DISABLE);
//这个判断tim3IntrCnt要在判断 isNoise之前，否则btnShakeCode=0；会导致即使进这个if也会使得btnCode=0;
	 // 或许 tim3IntrCnt > 1这些理论上会更好，但是实践中发现会按键不灵敏，也就是有时按了，但不能识别。
	if(tim3IntrCnt>0)
	{
		isNoise=false;
		tim3IntrCnt=0;
		btnCode=btnShakeCode;
		TIM_Cmd(TIM3,DISABLE);
	}
	if(isNoise)
	{
		TIM_Cmd(TIM3,DISABLE);
		btnShakeCode=0;
	}
	TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
 }
}/*}}}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
