/*
 * =====================================================================================
 *
 *       Filename:  timeout.hh
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/1/20 15:14:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef TIMEOUT_H
#define TIMEOUT_H

#include "globalDef.h"

typedef struct TimeoutStruct
{
	bool active;
	bool elapsed;
	u32 startStamp;
	u32 duration;
}Timeout;

void timeoutInit(Timeout* pTimeout);
void timeoutStart(Timeout* pTimeout,u32 durationIn);
bool timeoutHasElapsed(Timeout * pTimeout);
void timeoutAbort(Timeout * pTimeout);
void timeoutClear(Timeout * pTimeout);

extern Timeout ex1HeatPwmTimeout;
extern Timeout pfHeatPwmTimeout;
extern Timeout axisAstepTimeout;
extern Timeout ledToggleTimeout;
extern Timeout sdOverTimeout;
extern Timeout heaterBoardTimeout;
extern Timeout usrInputTimeout;
extern Timeout menuUpdateTimeout;
#endif

