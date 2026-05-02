#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "math.h"
#include "stdio.h"
#include "AD9833.h"
#include <string.h>
#include "arm_math.h"
#include "arm_const_structs.h"

extern float f;        
extern float fs;     
extern uint16_t ADC1_input_data[];       // ADC1数据数组
extern uint16_t ADC2_input_data[];       // ADC2数据数组
extern uint16_t ADC3_input_data [];       // ADC3数据数组

extern float FFT_buffer[];

extern float FFT_mag[];
extern uint32_t FFT_mag_data_index;
extern float FFT_phase;
extern float FFT_Ampl;

extern float ADC1_Ampl;          //ADC1采集数据测量结果
extern float ADC2_Ampl;         //ADC2采集数据测量结果
extern float ADC3_Ampl_T ;         //ADC3采集数据测量结果(通)
extern float ADC3_Ampl_D ;         //ADC3采集数据测量结果（断）
extern float ADC1_phase; 
extern float ADC3_phase; 
extern float ADC13_phase_difference ;       //输入输出信号相位差

extern float int_R;    //输入电阻
extern float out_R;    //输出电阻
extern float gain;     //增益
extern float f_gain_half[];   //扫幅频特性曲线
extern float f_gain[];   //幅频特性曲线
extern float f_H;       //上限频率

void Generate_data();
void apply_hanning(uint16_t *input_data);
void FFT();
void energy_centroid_correction(uint32_t peak_idx, float* freq, float* amp, float* phase);
void Process_FFT_mag();
float Amp_calculation(uint16_t *intput_data);
void ProcessAmplifierGain(float *input); //插值函数

void Show_data(float* buffer, uint16_t n);
