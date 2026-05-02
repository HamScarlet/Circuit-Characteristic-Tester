#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H

#include "gpio.h"
#include <stdio.h>

#define DEBUG 	0

extern UART_HandleTypeDef huart3;

typedef struct {
	float Rin;
	float Rout;
	float Gain;
	float CutOff_Freq;
	float DCout;
	float ACout;
}TRAIT;

typedef enum {
	BREAK = 0,
	CLOSED
}KEY_STATUE;



typedef struct{
	
}FAULT;

void Core_Init(void);
void Core_Loop(void);
void Circuit_Switch(void);
void Set_Key_Statue(KEY_STATUE statue);

#endif
