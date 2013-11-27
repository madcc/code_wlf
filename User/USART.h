#ifndef __USART_CONFIG_H
#define __USART_CONFIG_H

#include <stdio.h>
#include "globalDef.h"

void USART_Config(void); //串口设置
void USART_IT_Config(void);//串口中断设置，可选
int fputc(int ch, FILE *f);    //fputc重定向
int fgetc(FILE *f); //fgetc重定向

#endif
