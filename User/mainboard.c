/*
 * =====================================================================================
 *
 *       Filename:  mainboard.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/10/2 13:17:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  madcc09 (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "mainboard.h"
#include "button.h"
#include "menu.h"
#include "timeout.h"
#include "tempature.h"
#include <stdlib.h>
#include <stdio.h>

void runBoardSlice(void)/*{{{*/
{
	if(timeoutHasElapsed(&ex1HeatPwmTimeout))
	{
		tempatureControl();
		timeoutStart(&ex1HeatPwmTimeout,ex1HeatTimeoutNum);
	}
	if(timeoutHasElapsed(&pfHeatPwmTimeout))
	{
		platformTempatureControl();
		//timeoutStart(&pfHeatPwmTimeout,5*1000000);
	}
	if(BC_NULL!=btnCode)/*{{{*/
	{
		switch(btnCode)
		{
			case BC_Down:
				printf("down\r\n");
				break;
			case BC_Up:
				printf("up\r\n");
				break;
			case BC_Center:
				printf("center\r\n");
				break;
			case BC_Left:
				printf("left\r\n");
				break;
			case BC_Right:
				printf("right\r\n");
				break;
		}
		if(NULL!=(pCurItem->itemFunc))  //must ! 否则程序就跑飞了 
			(*(pCurItem->itemFunc))();
	}/*}}}*/
	else if(timeoutHasElapsed(&menuUpdateTimeout))
	{
		menuUpdate();
		timeoutStart(&menuUpdateTimeout,200*1000);
	}
}/*}}}*/
