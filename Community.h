#ifndef __COMMUNITY_H
#define __COMMUNITY_H

#include "stm32h7xx.h"                  // Device header
#include "Core_Handle.h"
#include "AD9833.h"

void SPI_Init(void);
void Set_DDS(float Freq);

#endif
