#ifndef __CORE_HANDLE_H
#define __CORE_HANDLE_H



#define DEBUG 	0

typedef struct {
	float Rin;
	float Rout;
	float Gain;
	float CutOff_Freq;
}TRAIT;


typedef struct{
	
}FAULT;

void Core_Init(void);
void Core_Loop(void);
void Circuit_Switch(void);

#endif
