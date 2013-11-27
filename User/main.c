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

//��������_IN_FLOATING 
//����������_IPU   
//����������_IPD            
//ģ������_AIN
//��©���_OUT_OD      
//�������_OUT_PP 
//���ù��ܵ��������_AF_PP 
//���ù��ܵĿ�©���_AF_OD
#define LED1_OFF GPIO_SetBits(GPIOC, GPIO_Pin_3)  	  //LED�͵�ƽ�����ߵ�ƽ��
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
	GPIO_SetBits(Ex1_Port,Ex1Fan_Pin);//��������
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
	//SYSTICK��Ƶ--1us��ϵͳʱ���ж�,
	if (SysTick_Config(SystemCoreClock /1000000))	 /* Reload value impossible */
  	{ 
  	  	/* Capture error */ 
    	while (1);
  	}
}/*}}}*/
void Led_Config(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);	   //AFIOʱ��,�ܽ���ӳ��ʱ�迪��  	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	   //GPIOCʱ��
		
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);  // �ı�ָ���ܽŵ�ӳ�� GPIO_Remap_SWJ_JTAGDisable ��JTAG-DP ���� + SW-DP ʹ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	   //LED1/2       //��PB4/PB5 ����Ϊͨ���������  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //���߷�ת�ٶ�Ϊ50MHz
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
	USART_IT_Config(); //�����ж����ã���ѡ
	motorGPIOinit();
	btnGPIOinit();
	sdCardInit();
	lcdSerialInit();
	menuInit();
	showCurItem();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //ռ�����ȼ��������ȼ�����Դ����
	Tim4IntrInit();
}/*}}}*/
