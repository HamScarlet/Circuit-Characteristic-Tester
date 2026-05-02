#include "stm32h7xx.h"                  // Device header
#include "Core_Handle.h"
#include "TraitAnalysis.h"
#include "Community.h"

TRAIT Circuit_Trait;


void Core_Init(void)
{
	SPI_Init();
}


void Core_Loop(void)
{
	Generate_Wave(Wave_Freq);
	Resistence_Cal(&Circuit_Trait);
	Circuit_Trait.CutOff_Freq = MeasCut_offFreq();
	printf("Rin = %f,Rout = %f,Gain = %f,Cutoff Freq = %f\n",Circuit_Trait.Rin,Circuit_Trait.Rout,Circuit_Trait.Gain,Circuit_Trait.CutOff_Freq);
}

void Circuit_Switch(void)
{
	static uint8_t statue = 1;
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,statue);
	if(statue == 1)
	{
		statue = 0;
	}
	else
	{
		statue = 1;
	}
}

void Set_Key_Statue(KEY_STATUE statue)
{
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,(uint8_t)statue);
} 



int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&huart3,(uint8_t *)&ch,1,0xffff);
	return ch;
}
