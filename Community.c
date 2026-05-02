#include "Community.h"
#include "Core_Handle.h"

void SPI_Init(void)
{
	AD9833_Init();
}

void Set_DDS(float Freq)
{
	AD9833_SetFrequencyQuick(Freq,AD9833_OUT_SINUS);
}
