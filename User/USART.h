#ifndef __USART_CONFIG_H
#define __USART_CONFIG_H

#include <stdio.h>
#include "globalDef.h"

void USART_Config(void); //��������
void USART_IT_Config(void);//�����ж����ã���ѡ
int fputc(int ch, FILE *f);    //fputc�ض���
int fgetc(FILE *f); //fgetc�ض���

#endif
