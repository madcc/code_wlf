//global definition

#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H


#include "stm32f10x.h"
//toFix
//#define  INTERVAL_IN_MICROSECONDS 64
//#define  INTERVAL_IN_MICROSECONDS 200
#define  INTERVAL_IN_MICROSECONDS 100 //old ok.2013.11.23
//#define  INTERVAL_IN_MICROSECONDS 20
//#define  INTERVAL_IN_MICROSECONDS 10
//改变这个值好像没影响，为什么？

extern vu32 sysMicros;
extern vu32 TimingDelay;
void delay_us(u32 nTime);
void delay_ms(u32 nTime);

#define LED_OFF GPIO_SetBits(GPIOC, GPIO_Pin_2)  
#define LED_ON GPIO_ResetBits(GPIOC, GPIO_Pin_2)
#define LED_BLINK(x)\
	LED_ON;\
	delay_ms(x);\
	LED_OFF;\
	delay_ms(x);\
	LED_ON;

#ifndef VERSION
#define VERSION
extern const u16 firmwareVersion;
#endif

#define USER_INPUT_TIMEOUT 1200000000 //20 minutes
#define ex1HeatTimeoutNum 2000000
#define axisAstepTimeoutNum 50000

#define COMMAND_BUFFER_SIZE 512

//#define abs(X) ((X) < 0 ? -(X) : (X)) 

#define MAX_PACKET_PAYLOAD 32

#ifndef bool
#define bool uint8_t
#define true 1
#define false 0
#endif

#define STEPPER_COUNT 4
#define AXIS_COUNT 4
#define X_Axis 0
#define Y_Axis 1
#define Z_Axis 2
#define A_Axis 3

extern uint32_t usrInputTime;


#endif
