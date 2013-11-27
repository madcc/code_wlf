/*
 * =====================================================================================
 *
 *       Filename:  tempature.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/8/11 20:00:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
//ex,extruder: 喷头 pf,platform: 底板平台
#include "tempature.h"
#include "thermcouple.h"
#include "timeout.h"
#include "testDefine.h"
#include <stdio.h>

#define Delta_Sum_MAX 1500
//#define Delta_Sum_MIN -Delta_Sum_MAX
#define Delta_Sum_MIN 0
#define DiffSamples 4
#define BypassPID_Threshold 15
#define Temperature_Target_Hysteresis 4

#define OutputEx1_Pwm(x)\
	TIM_SetCompare1(TIM2,x)

bool ex_newTargetReached=false;
bool pf_newTargetReached=false;
bool ex_isPaused=false;
bool ex_isHeating=false;

int curPlatformTemp;
int setPlatformTargetTemp=100;
int setTargetTemp=240;
int currentTemperature;
int deltaTemp,deltaTempOld,deltaSum;
int kp=800,kd=700,ki=25;
//if you change kp,ki,kd, pay attention to  Delta_Sum_MAX and deltaSum =800;
int pControl,dControl,iControl,totalControl;
int ex1HeaterOut;
int isBypassPID=1;
const int minTempControl =0;
const int maxTempControl =40000;
int diffIdx,diffSum;

int diffHistory[DiffSamples];
int getCurExtruderTempature(void)	/*{{{*/
{
	currentTemperature=thermcoupleReadTemp();
	return currentTemperature;
}/*}}}*/
int getCurPlatformTempature(void)/*{{{*/
{
	curPlatformTemp=thermistorReadTemp();//_fixme FIXME 
	return curPlatformTemp;
}/*}}}*/
void platformTempatureControl(void)/*{{{*/
{
	static bool isOn=true;
	if(isOn)
	{
		GPIO_SetBits(HBP_Port,HBP_Pin);
		timeoutStart(&pfHeatPwmTimeout,1*1000000);
		isOn=true;
	}
	else
	{
		GPIO_ResetBits(HBP_Port,HBP_Pin);
		timeoutStart(&pfHeatPwmTimeout,1000000);
		isOn=true;
	}
}/*}}}*/
void tempatureControl(void)/*{{{*/
{
	currentTemperature=thermcoupleReadTemp();
	deltaTemp=setTargetTemp - currentTemperature;
	if(isBypassPID && (deltaTemp < BypassPID_Threshold))
	{
		isBypassPID=0;
		pidReset();
	}
	else if( !isBypassPID && (deltaTemp > BypassPID_Threshold + 10 ))
		isBypassPID=1;
	if(isBypassPID)
		ex1HeaterOut=maxTempControl;
	else
		ex1HeaterOut=calculatePID(deltaTemp);
	OutputEx1_Pwm(ex1HeaterOut);
	//TIM_SetCompare1(TIM1,ex1HeaterOut);
	printf("%d %d %d %d %d %d %d\r\n",currentTemperature,setTargetTemp,
			totalControl,pControl,iControl,dControl,isBypassPID);
}/*}}}*/
void pidReset(void)/*{{{*/
{
	deltaSum=800;
	//deltaSum=0;
	deltaTempOld=0;
	for(diffIdx =0;diffIdx < DiffSamples;diffIdx++)
		diffHistory[diffIdx]=0;
	diffIdx =0;
	diffSum =0;
	totalControl =0;
}/*}}}*/

int calculatePID(int e)/*{{{*/
{
	int diff;
	deltaSum += e;
	if(deltaSum > Delta_Sum_MAX)
		deltaSum = Delta_Sum_MAX;
	if(deltaSum < Delta_Sum_MIN)
		deltaSum = Delta_Sum_MIN;
	pControl=kp*e;
	iControl=ki*deltaSum;
	diff=e-deltaTempOld;
	diffSum -=diffHistory[diffIdx];
	diffHistory[diffIdx] = diff;
	diffSum += diff;
	diffIdx = (diffIdx + 1 )% DiffSamples;
	dControl = kd* diffSum;
	deltaTempOld = e;
	totalControl = pControl + iControl + dControl;
	if(totalControl < minTempControl)
		totalControl = minTempControl;
	if(totalControl > maxTempControl)
		totalControl = maxTempControl;
	return totalControl;	
}/*}}}*/
bool extruderHasReachedTargetTemp(void)/*{{{*/
{
#ifdef JustTestLogic
	return true;//FIXME
#endif
	//if(!ex_newTargetReached)
	{
		if((currentTemperature >=(setTargetTemp - Temperature_Target_Hysteresis)) &&(currentTemperature <= (setTargetTemp + Temperature_Target_Hysteresis)))
			ex_newTargetReached=true;
		else
			ex_newTargetReached=false;
	}
	return ex_newTargetReached;
}/*}}}*/

bool platformHasReachedTargetTemp(void)/*{{{*/
{
	return true;//FIXME 
	if(!pf_newTargetReached)
	{
		if((curPlatformTemp >= (setPlatformTargetTemp - Temperature_Target_Hysteresis)) && (curPlatformTemp <= (setPlatformTargetTemp + Temperature_Target_Hysteresis)))
			pf_newTargetReached=true;
	}
	return pf_newTargetReached;
}/*}}}*/
void extruderSetHeatPause(bool enable)/*{{{*/
{
	if(ex_isPaused == enable)
		return;
	if(enable && !ex_isHeating)
		return;
	OutputEx1_Pwm(0);
	if(enable)
	{
		timeoutInit(&ex1HeatPwmTimeout);
		ex_isHeating=false;
	}
	else
	{
		extruderStartHeat();
		ex_isHeating=true;
	}
}/*}}}*/
void extruderStartHeat(void)/*{{{*/
{
#ifdef JustTestLogic
	return;
#endif
	timeoutStart(&ex1HeatPwmTimeout,ex1HeatTimeoutNum);
}/*}}}*/
void platformStartHeat(void)/*{{{*/
{
#ifdef JustTestLogic
	return;
#endif
	timeoutStart(&pfHeatPwmTimeout,2*1000000);
}/*}}}*/
void extruderStopHeat(void)/*{{{*/
{
	OutputEx1_Pwm(0);
	timeoutInit(&ex1HeatPwmTimeout);
}/*}}}*/
void platformStopHeat(void)/*{{{*/
{
	//OutputEx1_Pwm(0); FIXME 
	timeoutInit(&pfHeatPwmTimeout);
}/*}}}*/
