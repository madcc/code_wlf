/*
 * =====================================================================================
 *
 *       Filename:  extruderOutput.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/23 15:47:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  RY (good lucky), madcc09@163.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "stm32f10x.h"
#include "hardwareDef.h"

void ExtruderIOinit(void);
void Tim1_config(void);
void Tim2_config(void);
