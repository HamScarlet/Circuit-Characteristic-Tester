#include "stm32h7xx.h"                  // Device header
#include "FaultSearch.h"

float Trait_diff_Matrix[15][6] = {0};



char* FindFault(TRAIT trait)
{
	float ModLength = 0;
	float MinModLength = 1000000;
	// 归一化
	for(int i = 0;i < 15;i++)
	{
		Trait_diff_Matrix[i][0] = trait.Rin - FaultCase[0].Rin;
		Trait_diff_Matrix[i][1] = trait.Rout - FaultCase[1].Rout;
		Trait_diff_Matrix[i][2] = trait.Gain - FaultCase[2].Gain;
		Trait_diff_Matrix[i][3] = trait.CutOff_Freq - FaultCase[3].CutOff_Freq;
		Trait_diff_Matrix[i][4] = trait.DCout - FaultCase[4].DCout;
		Trait_diff_Matrix[i][5] = trait.ACout - FaultCase[5].ACout;
	}
	float col_max[6];
	float col_min[6];
	float temp_max = 0,temp_min = 1000000;
	for(int i = 0;i < 6;i++)
	{
		for(int j = 0;j < 15;j++)
		{
			if(Trait_diff_Matrix[j][i] > temp_max)
			{
				temp_max = Trait_diff_Matrix[j][i];
			}
			else if(Trait_diff_Matrix[j][i] < temp_min)
			{
				temp_min = Trait_diff_Matrix[j][i];
			}
		}
		col_max[i] = temp_max;
		col_min[i] = temp_min;
	}
	//计算最短模长
	uint8_t index;
	for(int i = 0;i < 15;i++)
	{
		for(int j = 0;j < 6;j++)
		{
			ModLength += Weight[j]*(Trait_diff_Matrix[i][j] - col_min[j]/(col_max[j] - col_min[j]))*Weight[j]
			*(Trait_diff_Matrix[i][j] - col_min[j]/(col_max[j] - col_min[j])); 		//计算模长
		}
		if(MinModLength > ModLength)
		{
			MinModLength = ModLength;
			index = i;
		}
	}
	return Case[index];
}









