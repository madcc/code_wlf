/*
 * =====================================================================================
 *
 *       Filename:  thermcouple.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/23 20:10:37
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "thermcouple.h"

uint16_t oldTemparture;
void thermcoupleIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	   //GPIOB时钟
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	
	GPIO_InitStructure.GPIO_Pin = ThermcoupleSck_Pin;
	GPIO_Init(ThermcoupleSck_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = ThermcoupleCs_Pin;
	GPIO_Init(ThermcoupleCs_Port, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = ThermcoupleDo_Pin;
	GPIO_Init(ThermcoupleDo_Port, &GPIO_InitStructure);
	oldTemparture=25;
}/*}}}*/
uint16_t thermcoupleReadTemp(void)/*{{{*/
{
	uint16_t data=0;
	uint8_t i,result;
	bool badTemperature=false;
	bool haveData=false;
	GPIO_ResetBits(ThermcoupleSck_Port,ThermcoupleSck_Pin);
	GPIO_ResetBits(ThermcoupleCs_Port,ThermcoupleCs_Pin);
	delay_us(20);
	for(i=0;i<16;i++)
	{
		GPIO_SetBits(ThermcoupleSck_Port,ThermcoupleSck_Pin);
		delay_us(1);
		if(i>=1 && i<11)
		{
			data=data<<1;
			if(GPIO_ReadInputDataBit(ThermcoupleDo_Port,ThermcoupleDo_Pin))
			{
				data=data|0x01;
				haveData=true;
			}
		}
		if(i==13)
		{
			if(GPIO_ReadInputDataBit(ThermcoupleDo_Port,ThermcoupleDo_Pin))
				badTemperature=true;
		}
		delay_us(1);
		GPIO_ResetBits(ThermcoupleSck_Port,ThermcoupleSck_Pin);
		delay_us(2);
	}
	GPIO_SetBits(ThermcoupleCs_Port,ThermcoupleCs_Pin);
	delay_us(20);
	GPIO_ResetBits(ThermcoupleSck_Port,ThermcoupleSck_Pin);
	if(badTemperature)
	{
		data=oldTemparture;
	}
	else
	{
	}
	return data;
}/*}}}*/
void thermistorIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	   //GPIOC时钟

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	
	GPIO_InitStructure.GPIO_Pin = HeaterBoardTherm_Pin;
	GPIO_Init(HeaterBoardTherm_Port,&GPIO_InitStructure);	


	RCC_ADCCLKConfig(RCC_CFGR_ADCPRE_DIV6);//ADC时钟PLCK2的6分频 12M。ADC最大时钟不能超过14M!
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; //使能扫描
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//ADC转换工作在连续模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//有软件控制转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//转换数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;//转换通道1个
	ADC_Init(ADC1, &ADC_InitStructure); //初始化ADC
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11,1, ADC_SampleTime_239Cycles5);//选择ADC1 通道11(因为是ADC_11),次序1,采样239.5周期 
		
	ADC_Cmd(ADC1, ENABLE);//使能ADC1
	
	ADC_ResetCalibration(ADC1); //重置ADC1校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));//等待ADC1校准重置完成
	ADC_StartCalibration(ADC1);//开始ADC1校准
	while(ADC_GetCalibrationStatus(ADC1));//等待ADC1校准完成
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能ADC1软件开始转换

}/*}}}*/
uint16_t thermistorReadTemp(void)/*{{{*/
{
	uint16_t adc;
	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==SET)
	{
		adc=ADC_GetConversionValue(ADC1);
	}
	return adc;
}/*}}}*/
