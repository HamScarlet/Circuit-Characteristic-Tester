#ifndef __TRAITANALYSIS_H
#define __TRAITANALYSIS_H

#include "Core_Handle.h"
#include "arm_const_structs.h"          // ARM::CMSIS:DSP
#include "Community.h"

#define ADC_LEN		1024

typedef struct{
	float Uin; 		//未加采样电阻的输入电压
	float Uin_s;	//加采样电阻的输入电压
	float Uout;		//未加采样电阻的输出电压
	float Uout_s;   //加采样电阻的输出电压
	float DCin; 	//输入直流偏置
	float DCout;	//输出直流偏置
	float DCin_s;
	float DCout_s;	
}MEAS_DATA;

extern float Wave_Freq;

void Generate_Wave(float Freq);
void ADC_Start(void);
void Process_FFT(float* pVol,float* pDC);
void Resistence_Cal(TRAIT* trait);
void FFT(float* input_Data,float* pDC,float* pVol);

#endif
