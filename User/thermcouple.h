/*
 * =====================================================================================
 *
 *       Filename:  thermcouple.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/3/23 20:10:47
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

void thermcoupleIOinit(void);
uint16_t thermcoupleReadTemp(void);
void thermistorIOinit(void);
uint16_t thermistorReadTemp(void);
