/*
 * =====================================================================================
 *
 *       Filename:  button.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/30 16:47:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __H_BUTTON_H__
#define __H_BUTTON_H__

#include "stm32f10x.h"
#include "hardwareDef.h"

enum BtnCode{BC_NULL,BC_Left,BC_Right,BC_Center,BC_Up,BC_Down};
extern enum BtnCode btnCode;

void btnExtiInit(void);
void btnGPIOinit(void);
void Tim3IntrInit(void);


#endif
