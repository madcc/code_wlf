/*
 * =====================================================================================
 *
 *       Filename:  tempature.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/8/11 19:59:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __Tempature__HH
#define __Tempature__HH

#include "globalDef.h"

extern int setTargetTemp;
extern int setPlatformTargetTemp;
void tempatureControl(void);
void pidReset(void);
int calculatePID(int e);
void extruderSetHeatPause(bool enable);
void extruderStartHeat(void);
void extruderStopHeat(void);
void platformStopHeat(void);
void platformStartHeat(void);
int getCurExtruderTempature(void);
int getCurPlatformTempature(void);
void platformTempatureControl(void);
bool extruderHasReachedTargetTemp(void);
bool platformHasReachedTargetTemp(void);
#endif
