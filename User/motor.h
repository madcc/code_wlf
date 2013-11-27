/*
 * =====================================================================================
 *
 *       Filename:  motor.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/29 22:16:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f10x.h"
#include "globalDef.h"
#include "hardwareDef.h"

//注意，不是所有变量都要用extern 的，toFix
//extern u8 motorPlanMasterStepIdx;
//extern u32 motorPlanMasterStep;
//
//extern int motorPlanPos[STEPPER_COUNT];
//extern int motorPlanTarget[STEPPER_COUNT];
//extern int toolOffset[STEPPER_COUNT];
//extern int motorPlanStep[STEPPER_COUNT];
//extern int motorCurrentPos[STEPPER_COUNT];

extern bool motorIsRunning;
extern bool motorIsHoming;
 
#define EnableMotorInterrupt TIM_Cmd(TIM2, ENABLE);
#define DisableMotorInterrupt TIM_Cmd(TIM2, DISABLE);


extern volatile bool axisDirection[STEPPER_COUNT];
extern volatile int axisCount[STEPPER_COUNT];
extern volatile int axisDelta[STEPPER_COUNT];
extern bool axisInvertEndstop[STEPPER_COUNT];
extern volatile int axisPosition[STEPPER_COUNT];
extern volatile int32_t dda_position[STEPPER_COUNT];
extern volatile bool    axis_homing[STEPPER_COUNT];
//extern volatile int16_t e_steps[EXTRUDERS];
extern volatile uint8_t axisEnabled;			//Planner axis enabled
extern volatile uint8_t axisHardwareEnabled;		//Hardware axis enabled

void motorSetTarget(int * pTarget,s32 dda_interval);
void motorSetTargetNew(int* pTarget, int32_t us, uint8_t relative);
void motorDefinePosition(int* pPos);
void motorStartHoming(bool maximums,uint8_t axes_enabled, uint32_t us_per_step);
void motorGPIOinit(void);
void motorDisable(void);
void motorAbort(void);
bool motorDoInterrupt(void);

void axisSetStart(uint8_t idx,bool isEnable);
void axisSetDirection(u8 idx);
void axisSetStepPin(u8 idx,bool out);
void axisSetEnablePin(u8 idx,bool isEnable);
bool axisDoHoming(u8 idx,int intervals);
bool axisDoInterrupt(u8 idx,int intervals);
void axisReset(u8 idx);
bool axisCheckEndstop ( u8 idx,bool isHoming);
extern int32_t LastCntIntr2OnceStepTmp[STEPPER_COUNT];


#endif

