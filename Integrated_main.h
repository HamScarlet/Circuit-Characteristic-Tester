#ifndef __INTEGRATED_MAIN_H
#define __INTEGRATED_MAIN_H


void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);

void int_R_calculation();
void out_R_calculation();
void gain_calculation();
void phase_calculation();

void Integrated_Init(void);
void Integrated_Loop(void);


#endif
