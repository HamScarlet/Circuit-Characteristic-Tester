#include "stm32h7xx.h"                  // Device header
#include "Core_Handle.h"
#include "TraitAnalysis.h"
#include "Community.h"


void Core_Init(void)
{
	SPI_Init();
}


void Core_Loop(void)
{
	Generate_Wave(Wave_Freq);

	
	
}

void Circuit_Switch(void)
{
	
}
