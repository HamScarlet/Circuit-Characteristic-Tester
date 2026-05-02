#include "stm32h7xx.h"                  // Device header
#include "arm_math.h"                   // ARM::CMSIS:DSP
#include "TraitAnalysis.h"
#include "Core_Handle.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern TIM_HandleTypeDef htim3;

uint16_t ADC1_raw_buf[ADC_LEN] = {0};
uint16_t ADC2_raw_buf[ADC_LEN] = {0};
uint16_t ADC3_raw_buf[ADC_LEN] = {0};
float ADC1_buf[ADC_LEN] = {0};
float ADC2_buf[ADC_LEN] = {0};
float ADC3_buf[ADC_LEN] = {0};
uint8_t Whole_Wave_Num = 11; 	//一个采样周期内整周期个数（最好是奇数）

float Wave_Freq = 1000.0f;
float Sample_Rate = 10000.0f;
uint8_t ADC1_flag = 0;
uint8_t ADC2_flag = 0;
uint8_t ADC3_flag = 0;

MEAS_DATA Measure_Para;

//FFT数组

float FFT_Buffer[ADC_LEN*2] = {0};
float FFT_mag[ADC_LEN] = {0};
float FFT_mag_max = 0;
uint32_t FFT_mag_max_index = 0;
float FFT_Base_Freq = 0;

//

void ADC_Start(void)
{
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc2,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc3,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC1_raw_buf,ADC_LEN);
	HAL_ADC_Start_DMA(&hadc2,(uint32_t*)ADC2_raw_buf,ADC_LEN);
	HAL_ADC_Start_DMA(&hadc3,(uint32_t*)ADC3_raw_buf,ADC_LEN);
	HAL_TIM_Base_Start(&htim3);
}

void Set_SampleRate(float Freq)
{
	uint32_t APB2_Clock = 240000000;
	
	uint16_t new_arr = APB2_Clock/Freq - 1;
	
	__HAL_TIM_SET_AUTORELOAD(&htim3,new_arr);
}

float Cal_Sample_Rate(float Freq)
{
	float fs = Freq*1024/Whole_Wave_Num;
	return fs;
}

void Generate_Wave(float Freq)
{
	Set_DDS(Freq);
}

void Convert_ADC_data(uint16_t *pData,float* pAnalogy_Data)
{
	float ADC_Coefficient = 3.3f/4095;
	arm_mult_f32((float *)pData,&ADC_Coefficient,pAnalogy_Data,ADC_LEN);
}

void Split_DC_AC(float* pData,float* pAC_Data,float* pDC) //好像不需要
{
	arm_mean_f32(pData,ADC_LEN,pDC);
	arm_sub_f32(pData,pDC,pAC_Data,ADC_LEN);
}  

void Resistence_Cal(TRAIT* trait) 	//输入输出电阻及增益计算主逻辑
{
	Wave_Freq = 1000.0f;
	Set_SampleRate(Cal_Sample_Rate(Wave_Freq));
	ADC_Start();
	if (ADC1_flag == 1 && ADC2_flag == 1 && ADC3_flag == 1)
	{
		Convert_ADC_data(ADC1_raw_buf,ADC1_buf);
		Convert_ADC_data(ADC2_raw_buf,ADC2_buf);
		Convert_ADC_data(ADC3_raw_buf,ADC3_buf);
		FFT(ADC1_buf,&Measure_Para.DCin_s,&Measure_Para.Uin_s);
		FFT(ADC2_buf,&Measure_Para.DCin,&Measure_Para.Uin);
		FFT(ADC3_buf,&Measure_Para.DCout,&Measure_Para.Uout);
	}
	ADC1_flag = 0;
	ADC2_flag = 0;
	ADC3_flag = 0;
	Circuit_Switch();
	ADC_Start();
	if (ADC1_flag == 1 && ADC2_flag == 1 && ADC3_flag == 1)
	{
		Convert_ADC_data(ADC3_raw_buf,ADC3_buf);
		FFT(ADC3_buf,&Measure_Para.DCout_s,&Measure_Para.Uout_s);
	}
	ADC1_flag = 0;
	ADC2_flag = 0;
	ADC3_flag = 0;
	trait->Rin =  Measure_Para.Uin*RS_IN/(Measure_Para.Uin_s - Measure_Para.Uin);
	trait->Rout = Measure_Para.DCout*RS_OUT/Measure_Para.DCout_s - RS_OUT;
	trait->Gain = Measure_Para.DCout/Measure_Para.DCin;
}

void FFT(float* input_Data,float* pDC,float* pVol)
{
	
//	float mean = 0;
//	arm_mean_f32(input_Data,ADC_LEN,&mean);
//	
//	for(int i = 0;i<ADC_LEN;i++)
//	{
//		input_Data[i] -= mean;
//	}

	//arm_mult_f32(input_Data,hanWindow,input_Data,FFT_LEN);
	
	
	for(int i = 0;i < ADC_LEN;i++)
	{
		FFT_Buffer[i*2] = input_Data[i];
		FFT_Buffer[i*2+1] = 0;
	}
	
	#if (DEBUG == 1)
	Show_Data(input_Data,ADC_LEN);
	#endif
	arm_cfft_f32(&arm_cfft_sR_f32_len1024,FFT_Buffer,0,1);
	
	arm_cmplx_mag_f32(FFT_Buffer,FFT_mag,ADC_LEN);
	#if (DEBUG == 1)
	Show_Data(FFT_Buffer,ADC_LEN);
	#endif
	Process_FFT(pVol,pDC);
	#if (DEBUG == 1)
	printf("Freq:%f\n",FFT_Base_Freq);
	#endif
}

void Process_FFT(float* pVol,float* pDC)
{
	arm_max_f32(FFT_mag + 1,ADC_LEN/2 - 1,&FFT_mag_max,&FFT_mag_max_index);
	
	FFT_Base_Freq = (float)FFT_mag_max_index * Sample_Rate / ADC_LEN;
	
	//FFT_Ampl[0] = FFT_mag_max * 2.0f /(float)FFT_LEN;
	
	*pDC = FFT_mag[0]*2.0f/(float)ADC_LEN;
	*pVol = FFT_mag_max*2.0f/(float)ADC_LEN;
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if(hadc ->Instance == ADC1)
	{
//		HAL_TIM_Base_Stop(&htim3);
//		HAL_ADC_Stop_DMA(&hadc1);
		
		ADC1_flag = 1;
	}
	else if(hadc ->Instance == ADC2)
	{
		ADC2_flag = 1;
	}
	else if(hadc ->Instance == ADC3)
	{
		ADC3_flag = 1;
	}
}
