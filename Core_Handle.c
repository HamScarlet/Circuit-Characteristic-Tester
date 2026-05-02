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
