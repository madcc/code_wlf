#include "stm32f10x.h"
#include "ff.h"
#include "diskio.h"
#include "SPI_SD_driver.h"

#include <stdlib.h>
#include "USART.h"

#include "lcdSerial.h"
#include "menu.h"
#include "host.h"
#include "button.h"
#include "thermcouple.h"
#include "timeout.h"
#include "motor.h"
#include "globalDef.h"
#include "sdCardOper.h"
#include "timerset.h"
#include "extruderOutput.h"
#include "heaterBoard.h"
#include "command.h"
#include "tempature.h"
#include "testDefine.h"
#include "mainboard.h"

//浮空输入_IN_FLOATING 
//带上拉输入_IPU   
//带下拉输入_IPD            
//模拟输入_AIN
//开漏输出_OUT_OD      
//推挽输出_OUT_PP 
//复用功能的推挽输出_AF_PP 
//复用功能的开漏输出_AF_OD
#define LED1_OFF GPIO_SetBits(GPIOC, GPIO_Pin_3)  	  //LED低电平亮，高电平灭
#define LED1_ON GPIO_ResetBits(GPIOC, GPIO_Pin_3)


#define JUST_TEST_PID
//#define SD_Card_Logic

FATFS fs;
FRESULT res;
UINT br;
FIL file;

int32_t ex1FanCircleNum;
int32_t ex1FanDutyNum;
int32_t ex1HeatCircleNum;
int32_t ex1HeatDutyNum;
int32_t hbpHeatCircleNum;
int32_t hbpHeatDutyNum;
uint16_t setTempIntegrade;
bool setIntegradeChange;
uint32_t ledToggleTime;

extern uint8_t btnCode;
void SysTick_Set(void);
void valueInit(void);
void periphInit(void);
void Led_Config(void);


u8 j=0;
char haha[]="good Lucky,RY(But RY, it should be 'Luck' instead of 'Lucky'!";
int main()
{

	int i;
	uint16_t pfTemp;
	ledToggleTime=500000;
	setIntegradeChange=false;

	SysTick_Set();
	periphInit();
	printf("good\r\n");
	GPIO_SetBits(Ex1_Port,Ex1Fan_Pin);//开启风扇
	LED1_ON;

	valueInit();
	timeoutStart(&ledToggleTimeout,ledToggleTime);
#ifdef TEST_STEP_DIR/*{{{*/
	axisDirection[0]=0;
	axisSetStart(0,true);
	while(1)
	{
		if(axisDirection[0]==0)
			axisDirection[0]=1;
		else axisDirection[0]=0;
		axisSetDirection(0);
		for(i=0;i<2000;i++)
		{
			axisSetStepPin(0,true);
			delay_us(20);
			axisSetStepPin(0,false);
			delay_us(500);
		}
		delay_ms(500);
	}
#endif/*}}}*/

	while (1)
	{
		if(timeoutHasElapsed(&ledToggleTimeout))
		{
			GPIOC->ODR ^=GPIO_Pin_3;  //toggle LED1 
			timeoutStart(&ledToggleTimeout,ledToggleTime);
			pfTemp=getCurPlatformTempature();
			printf("pftp: %d\r\n",pfTemp);
		}
		runCmdSlice();
		runHostSlice();
		runBoardSlice();
	}
}

void SysTick_Set(void)/*{{{*/
{
	//SYSTICK分频--1us的系统时钟中断,
	if (SysTick_Config(SystemCoreClock /1000000))	 /* Reload value impossible */
  	{ 
  	  	/* Capture error */ 
    	while (1);
  	}
}/*}}}*/
void Led_Config(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	   //AFIO时钟,管脚重映射时需开启  	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	   //GPIOC时钟
		
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);  // 改变指定管脚的映射 GPIO_Remap_SWJ_JTAGDisable ，JTAG-DP 禁用 + SW-DP 使能
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	   //LED1/2       //将PB4/PB5 配置为通用推挽输出  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}/*}}}*/
void valueInit(void)/*{{{*/
{
	int i;
	hostCurrentState=HOST_STATE_READY;
	sdCardIsPlaying=false;
	cmdPaused=true;
	cmdMode=READY;
	commandReset();
	for(i=0;i<STEPPER_COUNT;i++)
		axisSetEnablePin(i,false);
	extruderStopHeat();
	timeoutInit(&ex1HeatPwmTimeout);
	timeoutInit(&axisAstepTimeout);
	timeoutInit(&ledToggleTimeout);
	timeoutInit(&heaterBoardTimeout);

	timeoutStart(&menuUpdateTimeout,200*1000);
//#ifdef JustTestLogic
//	GPIO_ResetBits(GPIOC,GPIO_Pin_2);
//#endif
//#define justTestSD
//#ifdef justTestSD
//	cmdMode=READY;
//	strcpy(sdPrintFileName,"BOX.S3G");
//	hostStartBuildFromSD();
//#endif
}/*}}}*/
void periphInit(void)/*{{{*/
{
	Led_Config();
	SPI_Configuration();
	USART_Config();
	ExtruderIOinit();  //
	thermcoupleIOinit(); //**
	thermistorIOinit(); 	//**
	heaterBoardIOinit();
	USART_IT_Config(); //串口中断设置，可选
	motorGPIOinit();
	btnGPIOinit();
	sdCardInit();
	lcdSerialInit();
	menuInit();
	showCurItem();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //占先优先级、副优先级的资源分配
	Tim4IntrInit();
}/*}}}*/
