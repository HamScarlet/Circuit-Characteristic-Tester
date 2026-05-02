#include "stm32H7xx.h"

uint8_t switch_kg = 0;

void Integrated_Init(void)
{
    HAL_TIM_Base_Start(&htim3);
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);

    AD9833_Init();                                          // DDS初始化
    AD9833_SetFrequencyQuick(1000.0f, AD9833_OUT_TRIANGLE); // 输出 1kHz 正弦波
}

void Integrated_Loop(void)
{
    if (switch_kg == 1) // 一、基础部分测量
    {
        // 1.测量输入电阻
        int_R_calculation();

        printf("t15.txt=\"%.3f\"\xff\xff\xff", int_R);

        // 2.测量输出电阻
        out_R_calculation();

        printf("t17.txt=\"%.2f\"\xff\xff\xff", out_R);

        // 3.测量增益
        gain_calculation();

        printf("t12.txt=\"%.2f\"\xff\xff\xff", gain);

        // 4.测量相位差
        phase_calculation();

        printf("t5.txt=\"%.2f\"\xff\xff\xff", ADC13_phase_difference);
    }

    else if (switch_kg == 2) // 二、绘制幅频特性曲线，测量上限频率
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // 关闭继电器
        HAL_Delay(10);
        int s = 0;
        for (int i = 2; i < 222; i += 2)
        {
            AD9833_SetFrequencyQuick((float)i * 1000.0f, AD9833_OUT_TRIANGLE); // 输出 ikHz 正弦波
            if (i * 1000 <= 50000)
            {
                Change_Sample_Frequency(0, 99); // 采样率512k
                fs = 512000.0f;
            }
            else if (50000 < i * 1000 && i * 1000 <= 100000)
            {
                Change_Sample_Frequency(0, 49); // 采样率1024k
                fs = 1024000.0f;
            }
            else if (100000 < i * 1000)
            {
                Change_Sample_Frequency(0, 24); // 采样率2048k
                fs = 2048000.0f;
            }
            HAL_Delay(1);
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC1_input_data, 1024); // 启动ADC1采集没有采样电阻下的电压值
            HAL_ADC_Start_DMA(&hadc3, (uint32_t *)ADC3_input_data, 1024); // 启动ADC3采集没有采样电阻下的电压值
            while (ADC1_dmaComp == 0 || ADC3_dmaComp == 0)                // 在两个ADC采集完成之前，一直留在死循环
            {
            }
            ADC1_dmaComp = 0, ADC3_dmaComp = 0; // 标志位重新置0

            ADC1_Ampl = Amp_calculation(ADC1_input_data);   // FFT计算ADC1测量结果
            ADC3_Ampl_D = Amp_calculation(ADC3_input_data); // FFT计算ADC3测量结果
            f_gain_half[s] = (ADC3_Ampl_D) / ADC1_Ampl;
            printf("j0.val=%d\xff\xff\xff", (uint8_t)((float)s / 110.0f * 100.0f) + 1); // 进度条
            s++;
        }
        ProcessAmplifierGain(f_gain_half);
        for (int a = 0; a < 220; a++)
        {
            printf("add 1,0,%d\xff\xff\xff", (uint8_t)f_gain[a]);
        }
        printf("t20.txt=\"%.1f\"\xff\xff\xff", f_H);
        printf("t1.txt=\"%d\"\xff\xff\xff", s - 1);
        switch_kg = 0;                                          // 只绘制一次，通过串口屏操作再次绘制
        AD9833_SetFrequencyQuick(1000.0f, AD9833_OUT_TRIANGLE); // 输出 1kHz 正弦波
    }

    else if (switch_kg == 3) // 三、发挥部分测量
    {
        int_R_calculation(); // 测量输入电阻
        out_R_calculation(); // 测量输出电阻
        gain_calculation();  // 测量增益
        phase_calculation(); // 测量相位差

        printf("t3.txt=\"3\"\xff\xff\xff");
    }
}

uint8_t ADC1_dmaComp = 0, ADC2_dmaComp = 0, ADC3_dmaComp = 0;     //ADC123采样结束标志位
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  if(hadc == &hadc1)
  {
	  HAL_ADC_Stop_DMA(&hadc1);  //关闭ADC

	  ADC1_dmaComp = 1;  //采样结束标志位置1
  }
  if(hadc == &hadc2)
  {
	  HAL_ADC_Stop_DMA(&hadc2);  //关闭ADC

	  ADC2_dmaComp = 1;  //采样结束标志位置1
  }
  if(hadc == &hadc3)
  {
	  HAL_ADC_Stop_DMA(&hadc3);  //关闭ADC

	  ADC3_dmaComp = 1;  //采样结束标志位置1
  }
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ConvCpltCallback must be implemented in the user file.
   */
}

// 自定义函数：动态改变采样频率
// 参数 psc: 新的预分频器值 (TIM_Prescaler)
// 参数 arr: 新的自动重载值 (TIM_Period)
void Change_Sample_Frequency (uint16_t psc, uint16_t arr)
{
    // 1. 停止定时器（如果有正在进行的ADC采集，最好也暂停）
    HAL_TIM_Base_Stop(&htim3); // 假设你的定时器句柄是 htim

    // 2. 修改定时器的核心参数
    //    ⚠️ 重要：修改参数前必须关闭定时器，否则可能导致配置错误
    __HAL_TIM_SET_PRESCALER(&htim3, psc);
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);

    // 3. 强制产生一个更新事件，让新的ARR值立即生效
    __HAL_TIM_SET_COUNTER(&htim3, 0); // 计数器清零
	HAL_TIM_GenerateEvent(&htim3, TIM_EVENTSOURCE_UPDATE);

    // 4. 重新启动定时器
    HAL_TIM_Base_Start(&htim3);
}

uint8_t aRxBuffer = 0;			//接收中断缓冲
//串口中断接收回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
     if(huart == &huart1)
	{
		if(aRxBuffer == '0')
		{
			switch_kg = 0;
		}
		if(aRxBuffer == '1')
		{
			switch_kg = 1;
		}
		else if(aRxBuffer == '2')
		{
			switch_kg = 2;
		}		
		else if(aRxBuffer == '3')
		{
			switch_kg = 3;
		}
		HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);//循环使能，才能不断接收
    }
}

//测量输入电阻
void int_R_calculation()      
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_input_data, 1024);   //启动ADC1采集没有采样电阻下的电压值			  
	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)ADC2_input_data , 1024);   //启动ADC2采集有采样电阻下的电压值
	while(ADC1_dmaComp == 0 || ADC2_dmaComp == 0)        //在两个ADC采集完成之前，一直留在死循环
	{
	}
	ADC1_dmaComp = 0,ADC2_dmaComp = 0;       //标志位重新置0
		  
	ADC1_Ampl = Amp_calculation(ADC1_input_data);    //FFT计算ADC1测量结果
	ADC2_Ampl = Amp_calculation(ADC2_input_data);    //FFT计算ADC2测量结果
		  
    int_R = ADC1_Ampl / (ADC2_Ampl);
}
//测量输出电阻
void out_R_calculation()     
{		  
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);       //闭合继电器
	HAL_Delay(10);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC3_input_data , 1024);   //启动ADC3采集有采样电阻下的电压值		  
	while(ADC3_dmaComp == 0)        //在ADC3采集完成之前，一直留在死循环
	{
	}
	ADC3_dmaComp = 0;       //标志位重新置0

	ADC3_Ampl_T = Amp_calculation(ADC3_input_data);    //a.采样电阻接入时的测量结果  
		  
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);       //关闭继电器		  
	HAL_Delay(10);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC3_input_data , 1024);   //启动ADC3采集无采样电阻下的电压值		  
	while(ADC3_dmaComp == 0)        //在ADC3采集完成之前，一直留在死循环
	{
	}
	ADC3_dmaComp = 0;       //标志位重新置0

	ADC3_Ampl_D = Amp_calculation(ADC3_input_data);	    //b.采样电阻断开时的测量结果
		  
	out_R = ADC3_Ampl_T / ADC3_Ampl_D ;	
}
//测量增益
void gain_calculation()  
{		  
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_input_data, 1024);    //启动ADC1采集没有采样电阻下的电压值			  
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC3_input_data , 1024);   //启动ADC3采集没有采样电阻下的电压值
	while(ADC1_dmaComp == 0 || ADC3_dmaComp == 0)        //在两个ADC采集完成之前，一直留在死循环
	{
	}
	ADC1_dmaComp = 0,ADC3_dmaComp = 0;       //标志位重新置0
		  
	ADC1_Ampl = Amp_calculation(ADC1_input_data);    //FFT计算ADC1测量结果
	ADC1_phase = FFT_phase;
	ADC3_Ampl_D = Amp_calculation(ADC3_input_data );    //FFT计算ADC3测量结果
	ADC3_phase = FFT_phase;
		  
	gain = (ADC3_Ampl_D) / ADC1_Ampl;
}
//测量相位差
void phase_calculation()  
{		 
	ADC13_phase_difference = ADC3_phase - ADC1_phase;
	ADC13_phase_difference = ADC13_phase_difference * 360.0f / 3.1415926f / 2.0f;
	if(ADC13_phase_difference < 0)
	{
		ADC13_phase_difference = ADC13_phase_difference + 360;
	}
	else if(ADC13_phase_difference > 360)
	{
		ADC13_phase_difference = ADC13_phase_difference - 360;
	}
	else
	{}
}
