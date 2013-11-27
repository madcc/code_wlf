/*
 * =====================================================================================
 *
 *       Filename:  timeout.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/1/20 15:15:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "timeout.h"

Timeout menuUpdateTimeout;
Timeout ex1HeatPwmTimeout;
Timeout axisAstepTimeout;
Timeout ledToggleTimeout;
Timeout sdOverTimeout;
Timeout heaterBoardTimeout;
Timeout usrInputTimeout;
Timeout pfHeatPwmTimeout;

void timeoutInit(Timeout* pTimeout)/*{{{*/
{
	pTimeout->active=false;
	pTimeout->elapsed=false;
}/*}}}*/
void timeoutStart(Timeout* pTimeout,u32 durationIn)/*{{{*/
{
	pTimeout->active=true;
	pTimeout->elapsed= false;
	pTimeout->startStamp=sysMicros;
	pTimeout->duration=durationIn;
}/*}}}*/
bool timeoutHasElapsed(Timeout * pTimeout)/*{{{*/
{
	u32 delta;
	if(pTimeout->active && !pTimeout->elapsed)
	{
		delta=sysMicros - pTimeout->startStamp;//toFix,如果sysMicros超过u32的范围
		if(delta >= pTimeout->duration)
		{
			pTimeout->active=false;
			pTimeout->elapsed=true;
		}
	}
	return pTimeout->elapsed;
}/*}}}*/

void timeoutAbort(Timeout * pTimeout) //停止当前计时器/*{{{*/
{
	pTimeout->active=false;
	//pTimeout->elapsed=false;
}/*}}}*/
void timeoutClear(Timeout * pTimeout) //清除当前定时器的超时标志/*{{{*/
{
	pTimeout->elapsed=false;//这其实应该没什么用？下次调用 timeoutHasElapsed 时又会设置该标志了
}/*}}}*/




