/*
 * =====================================================================================
 *
 *       Filename:  heaterBoard.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/4/5 23:17:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "heaterBoard.h"
//加热底板相关文件
void heaterBoardIOinit(void)/*{{{*/
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	   //GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = HBP_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //口线翻转速度为50MHz
	GPIO_Init(HBP_Port, &GPIO_InitStructure);

}/*}}}*/
