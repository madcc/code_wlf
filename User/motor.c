/*
 * =====================================================================================
 *
 *       Filename:  motor.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/29 22:16:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "motor.h"
#include "testDefine.h"
#include <stdio.h>

bool motorIsRunning;
bool motorIsHoming;

volatile bool axisDirection[STEPPER_COUNT];
volatile int axisCounter[STEPPER_COUNT];
volatile int axisDelta[STEPPER_COUNT];
//bool axisInvertEndstop[STEPPER_COUNT];
volatile int axisPosition[STEPPER_COUNT];
volatile int32_t axisTarget[STEPPER_COUNT];
volatile int32_t toolOffsets[STEPPER_COUNT];
bool axisInvertDirection[STEPPER_COUNT];

volatile int motorIntervalsRemaining;
volatile int motorIntervals;

volatile int dda_position[STEPPER_COUNT];
volatile bool    axis_homing[STEPPER_COUNT];
//volatile s16 e_steps[EXTRUDERS];
volatile u8 axisEnabled;			//Planner axis enabled
volatile u8 axisHardwareEnabled;		//Hardware axis enabled

#define 	INT32_MAX   0x7fffffffL
#define IntervalFactor 2

bool endStopFlagTmpDebug;

bool holdZ=false;
//function
void axisSetTarget(u8 idx,int32_t target_in,bool relative); 


void motorGPIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	
	GPIO_InitStructure.GPIO_Pin = X_Axis_Start_Pin;
	GPIO_Init(X_Axis_Start_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = X_Axis_Step_Pin;
	GPIO_Init(X_Axis_Step_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = X_Axis_Dir_Pin;
	GPIO_Init(X_Axis_Dir_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = Y_Axis_Start_Pin;
	GPIO_Init(Y_Axis_Start_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Y_Axis_Step_Pin;
	GPIO_Init(Y_Axis_Step_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Y_Axis_Dir_Pin;
	GPIO_Init(Y_Axis_Dir_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = Z_Axis_Start_Pin;
	GPIO_Init(Z_Axis_Start_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Z_Axis_Step_Pin;
	GPIO_Init(Z_Axis_Step_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Z_Axis_Dir_Pin;
	GPIO_Init(Z_Axis_Dir_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = A_Axis_Start_Pin;
	GPIO_Init(A_Axis_Start_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = A_Axis_Step_Pin;
	GPIO_Init(A_Axis_Step_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = A_Axis_Dir_Pin;
	GPIO_Init(A_Axis_Dir_Port, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz

	GPIO_InitStructure.GPIO_Pin = X_Axis_Min_Pin;
	GPIO_Init(X_Axis_Min_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Y_Axis_Min_Pin;
	GPIO_Init(Y_Axis_Min_Port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = Z_Axis_Max_Pin;
	GPIO_Init(Z_Axis_Max_Port, &GPIO_InitStructure);
	
	axisDirection[X_Axis]=false;
	axisDirection[Y_Axis]=false;
	axisDirection[Z_Axis]=false;
	axisDirection[A_Axis]=false;

	axisInvertDirection[0]=true;
	axisInvertDirection[1]=false;
	axisInvertDirection[2]=false;
	axisInvertDirection[3]=false;

	//	Skyworks printer 
//	axisInvertDirection[0]=false;
//	axisInvertDirection[1]=true;
//	axisInvertDirection[2]=false;
//	axisInvertDirection[3]=false;

}/*}}}*/
void motorDisable(void) //Disable步进电机的使能引脚/*{{{*/
{
	int i;
	for(i=0;i<STEPPER_COUNT;i++)
		axisSetEnablePin(i,false);
	motorAbort();
}/*}}}*/
void motorAbort(void) //更改motor is running/homing的状态/*{{{*/
{
	motorIsRunning=false;
	motorIsHoming=false;
	//toFix
//	loadToleranceOffsets();
}/*}}}*/

void motorSetTarget(int * pTarget,s32 dda_interval)/*{{{*/
{
	int32_t max_delta = 0;
	int32_t delta ;
	int32_t negative_half_interval ;
	u8 i;
	for ( i = 0; i < STEPPER_COUNT; i++) 
	{
		/// if x, y, or z axis, add the toolhead offset to all movement
		//axes[i].setTarget(target[i], false);
		axisSetTarget(i,pTarget[i],false);
		delta = axisDelta[i];
		// Only shut z axis on inactivity
		if (i == 2 && !holdZ) axisSetEnablePin(i,delta!=0);//axes[i].enableStepper(delta != 0);
		else if (delta != 0) axisSetEnablePin(i,true);//axes[i].enableStepper(true);
		if (delta > max_delta) 
			max_delta = delta;

	}
	// compute number of intervals for this move
	motorIntervals = ((max_delta * dda_interval) / INTERVAL_IN_MICROSECONDS*IntervalFactor);
	motorIntervalsRemaining= motorIntervals;
	negative_half_interval = -motorIntervals / 2;
	for (i = 0; i < STEPPER_COUNT; i++) 
		axisCounter[i] = negative_half_interval;

	motorIsRunning= true;
}/*}}}*/

void motorSetTargetNew(int* pTarget, int32_t us, uint8_t relative) /*{{{*/
{
	u8 i;
	int32_t move ;
	bool relative_move ;
	int32_t delta ;
	int32_t negative_half_interval ;

	for (i = 0; i < STEPPER_COUNT; i++) 
	{
		move = pTarget[i];

		relative_move = (relative & (1 << i)) != 0;
		/// if x, y, or z axis, add the toolhead offset to all movement
		if(!relative_move)
			axisSetTarget(i,pTarget[i] + toolOffsets[i], false);
		else
			axisSetTarget(i,pTarget[i], relative_move);
		// Only shut z axis on inactivity
		delta = axisDelta[i];
		if (i == 2 && !holdZ) 
			axisSetEnablePin(i,(delta != 0));
		else if (delta != 0) 
			axisSetEnablePin(i,true);
	}
	// compute number of intervals for this move
	motorIntervals = us / INTERVAL_IN_MICROSECONDS *IntervalFactor;
	motorIntervalsRemaining= motorIntervals;
	negative_half_interval = -motorIntervals/ 2;
	for (i = 0; i < STEPPER_COUNT; i++) 
		axisCounter[i]= negative_half_interval;

	motorIsRunning= true;
}/*}}}*/

void motorDefinePosition(int* pPos)/*{{{*/
{
	uint8_t i;
	for(i=0;i<STEPPER_COUNT;i++)
		axisPosition[i]=pPos[i];
}/*}}}*/
/// Start homing
void motorStartHoming(bool maximums,uint8_t axes_enabled, uint32_t us_per_step) /*{{{*/
{
	int32_t negative_half_interval ;
	uint8_t i;
	motorIntervalsRemaining= INT32_MAX;
	motorIntervals = us_per_step / INTERVAL_IN_MICROSECONDS*IntervalFactor;
	negative_half_interval = -motorIntervals / 2;
	for (i = 0; i < STEPPER_COUNT; i++) 
	{
		axisCounter[i] = negative_half_interval;
		if ((axes_enabled & (1<<i)) != 0) 
		{
			axisDirection[i]=maximums;
			axisSetEnablePin(i,true);
			axisDelta[i]=1;
		}
		else 
			axisDelta[i] = 0;

	}
	motorIsHoming= true;
}/*}}}*/
bool motorDoInterrupt(void)/*{{{*/
{
	volatile int i;
	bool stillHoming;
	if(motorIsRunning)
	{
		if(motorIntervalsRemaining-- == 0)
			motorIsRunning=false;
		else
		{
			for(i=0;i<STEPPER_COUNT;i++)
				axisDoInterrupt(i,motorIntervals);
		}
		return motorIsRunning;
	}
	else if(motorIsHoming)
	{
		motorIsHoming=false;
		for(i=0;i<STEPPER_COUNT;i++)
		{
			stillHoming=axisDoHoming(i,motorIntervals);
			motorIsHoming= stillHoming || motorIsHoming;
		}
		return motorIsHoming;
	}
	return false;
}/*}}}*/


void axisSetTarget(u8 idx,int32_t target_in,bool relative) /*{{{*/
{
	axisTarget[idx]=target_in;
	if (relative) 
		axisDelta[idx] = axisTarget[idx];
	else 
		axisDelta[idx] = axisTarget[idx] - axisPosition[idx];
	axisDirection[idx] = true;
	if (axisDelta[idx] != 0) 
		axisSetEnablePin(idx,true);//do nothing ,toFix

	if (axisDelta[idx] < 0) 
	{
		axisDelta[idx] = -axisDelta[idx];
		axisDirection[idx] = false;
	} 
}/*}}}*/
void axisReset(u8 idx)/*{{{*/
{
	axisPosition[idx] = 0;
	axisCounter[idx] = 0;
	axisDelta[idx] = 0;
}/*}}}*/
bool axisDoInterrupt(u8 idx,int intervals)/*{{{*/
{
	bool hitEndstop=false;
	axisCounter[idx] += axisDelta[idx];
	if(axisCounter[idx] >= 0)
	{
		axisSetDirection(idx);
		axisCounter[idx] -= intervals;
		hitEndstop= axisCheckEndstop(idx,false);//toFix 
		
		if(axisDirection[idx])
		{
			if(! hitEndstop)
			{
				axisSetStepPin(idx,true);
			}
			axisPosition[idx]++;
		}
		else
		{
			if(!hitEndstop)
			{
				axisSetStepPin(idx,true);
			}
			axisPosition[idx]--;
		}
		//delay_us(2);//may be to fix,delay_us可能是不可重入的，所以这里不能用.
		__NOP();
		__NOP();
		axisSetStepPin(idx,false);
	}
	return !hitEndstop;
}/*}}}*/

bool axisDoHoming(u8 idx,int intervals)/*{{{*/
{
	bool hitEndstop;
	if(axisDelta[idx]==0)
		return false;
	axisCounter[idx] +=axisDelta[idx];
	if(axisCounter[idx] >= 0)
	{
		axisSetDirection(idx);
		axisCounter[idx] -= intervals;
		hitEndstop=axisCheckEndstop(idx,true);
		if(axisDirection[idx])
		{
			if(!hitEndstop)
				axisSetStepPin(idx,true);
			else
				return false;
			axisPosition[idx]++;
		}
		else
		{
			if(!hitEndstop)
				axisSetStepPin(idx,true);
			else
				return false;
			axisPosition[idx]--;
		}
		__NOP();
		__NOP();
		axisSetStepPin(idx,false);
	}
	return true;
}/*}}}*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  axisCheckEndstop
 *  Description:  
 * =====================================================================================
 */
bool axisCheckEndstop ( u8 idx,bool isHoming )/*{{{*/
{
//toFix, what is #if defined(SINGLE_SWITCH_ENDSTOPS) && (SINGLE_SWITCH_ENDSTOPS == 1) 
	bool isLimit;
	switch(idx)
	{
		case 0:isLimit=GPIO_ReadInputDataBit(X_Axis_Min_Port,X_Axis_Min_Pin);
			   if(axisDirection[0]==false)
				   return false;
			   break;
		case 1:isLimit=GPIO_ReadInputDataBit(Y_Axis_Min_Port,Y_Axis_Min_Pin);
			   if(axisDirection[1]==false)
				   return false;
			   break;
		case 2:isLimit=GPIO_ReadInputDataBit(Z_Axis_Max_Port,Z_Axis_Max_Pin);
			   if(axisDirection[2]==true)
				   return false;
			   break;
	}
	//isLimit=~isLimit; 错误，不能这么写。。。
	if(isLimit==0)
		return true;
	else 
		return false;
}		/* -----  end of function axisCheckEndstop  ----- *//*}}}*/


void axisSetStart(uint8_t idx,bool isEnable)//底层驱动start引脚/*{{{*/
{
#ifdef JustTestLogic
//	isEnable=false;
#endif
	if(isEnable)
	{
		switch(idx)
		{
			case 0: GPIO_SetBits(X_Axis_Start_Port,X_Axis_Start_Pin);
					break;
			case 1: GPIO_SetBits(Y_Axis_Start_Port,Y_Axis_Start_Pin);
					break;
			case 2: GPIO_SetBits(Z_Axis_Start_Port,Z_Axis_Start_Pin);
					break;
			case 3: GPIO_SetBits(A_Axis_Start_Port,A_Axis_Start_Pin);
					break;
			default:break;
		}
	}
	else
	{
		switch(idx)
		{
			case 0: GPIO_ResetBits(X_Axis_Start_Port,X_Axis_Start_Pin);
					break;
			case 1: GPIO_ResetBits(Y_Axis_Start_Port,Y_Axis_Start_Pin);
					break;
			case 2: GPIO_ResetBits(Z_Axis_Start_Port,Z_Axis_Start_Pin);
					break;
			case 3: GPIO_ResetBits(A_Axis_Start_Port,A_Axis_Start_Pin);
					break;
			default:break;
		}
	}
}/*}}}*/

void axisSetDirection(u8 idx)/*{{{*/
{
	bool convDir=axisDirection[idx];
	if(axisInvertDirection[idx])
	{
		if(false==convDir)
			convDir=true;
		else
			convDir=false;
	}
	if(convDir == true)
	{
		switch(idx)
		{
			case 0: GPIO_SetBits(X_Axis_Dir_Port,X_Axis_Dir_Pin);
					break;
			case 1: GPIO_SetBits(Y_Axis_Dir_Port,Y_Axis_Dir_Pin);
					break;
			case 2: GPIO_SetBits(Z_Axis_Dir_Port,Z_Axis_Dir_Pin);
					break;
			case 3: GPIO_SetBits(A_Axis_Dir_Port,A_Axis_Dir_Pin);
					break;
			default:break;
		}
	}
	else
	{
		switch(idx)
		{
			case 0: GPIO_ResetBits(X_Axis_Dir_Port,X_Axis_Dir_Pin);
					break;
			case 1: GPIO_ResetBits(Y_Axis_Dir_Port,Y_Axis_Dir_Pin);
					break;
			case 2: GPIO_ResetBits(Z_Axis_Dir_Port,Z_Axis_Dir_Pin);
					break;
			case 3: GPIO_ResetBits(A_Axis_Dir_Port,A_Axis_Dir_Pin);
					break;
			default:break;
		}
	}
}/*}}}*/

void axisSetStepPin(u8 idx,bool out)/*{{{*/
{
	if(out)
	{
		switch(idx)
		{
			case 0: GPIO_SetBits(X_Axis_Step_Port,X_Axis_Step_Pin);
					break;
			case 1: GPIO_SetBits(Y_Axis_Step_Port,Y_Axis_Step_Pin);
					break;
			case 2: GPIO_SetBits(Z_Axis_Step_Port,Z_Axis_Step_Pin);
					break;
			case 3: GPIO_SetBits(A_Axis_Step_Port,A_Axis_Step_Pin);
					break;
			default:break;
		}
	}
	else
	{
		switch(idx)
		{
			case 0: GPIO_ResetBits(X_Axis_Step_Port,X_Axis_Step_Pin);
					break;
			case 1: GPIO_ResetBits(Y_Axis_Step_Port,Y_Axis_Step_Pin);
					break;
			case 2: GPIO_ResetBits(Z_Axis_Step_Port,Z_Axis_Step_Pin);
					break;
			case 3: GPIO_ResetBits(A_Axis_Step_Port,A_Axis_Step_Pin);
					break;
			default:break;
		}
	}
}/*}}}*/

// 输出 OE引脚的值
void axisSetEnablePin(u8 idx,bool isEnable)/*{{{*/
{
	axisSetStart(idx,isEnable);
}/*}}}*/

