/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <fun.h>
#include "AD9833.h"

#include "Integrated_main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
void int_R_calculation();
void out_R_calculation();
void gain_calculation();
void phase_calculation();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t switch_kg = 0;

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
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_ADC2_Init();
  MX_SPI1_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3);
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&aRxBuffer, 1);
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)input_data, 4096);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  AD9833_Init();                            // DDS初始化  
  AD9833_SetFrequencyQuick (1000.0f, AD9833_OUT_TRIANGLE);  // 输出 1kHz 正弦波



  // 可以简化为：
  Integrated_Init();
  // ----------- //



  while (1)
  {
    // 可以集成为：
    Integrated_Loop();
    // ---------------- //






	  if(switch_kg == 1)                //一、基础部分测量
	  {
		  //1.测量输入电阻
		  int_R_calculation();
		  
          printf("t15.txt=\"%.3f\"\xff\xff\xff" , int_R);	
		  
		  //2.测量输出电阻		  
		  out_R_calculation();
		  
          printf("t17.txt=\"%.2f\"\xff\xff\xff" , out_R);			  
		  
		  //3.测量增益
		  gain_calculation();
		  
          printf("t12.txt=\"%.2f\"\xff\xff\xff", gain);	
          
		  //4.测量相位差
          phase_calculation();   
		  
		  printf("t5.txt=\"%.2f\"\xff\xff\xff", ADC13_phase_difference);
	  }

	  else if(switch_kg == 2)           //二、绘制幅频特性曲线，测量上限频率
	  {		  
		  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);       //关闭继电器
		  HAL_Delay(10);
		  int s = 0;
		  for(int i=2; i<222; i += 2)
		  {
			  AD9833_SetFrequencyQuick ((float)i*1000.0f, AD9833_OUT_TRIANGLE);  // 输出 ikHz 正弦波
			  if(i*1000 <= 50000)
			  {
				  Change_Sample_Frequency (0, 99);              //采样率512k
				  fs = 512000.0f;
			  }
			  else if(50000 < i*1000 && i*1000 <= 100000)
			  {
				  Change_Sample_Frequency (0, 49);              //采样率1024k
				  fs = 1024000.0f;
			  }
			  else if(100000 < i*1000)
			  {
				  Change_Sample_Frequency (0, 24);              //采样率2048k
				  fs = 2048000.0f;
			  }
			  HAL_Delay(1);
			  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_input_data, 1024);    //启动ADC1采集没有采样电阻下的电压值			  
			  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC3_input_data , 1024);   //启动ADC3采集没有采样电阻下的电压值
			  while(ADC1_dmaComp == 0 || ADC3_dmaComp == 0)                    //在两个ADC采集完成之前，一直留在死循环
			  {
			  }
			  ADC1_dmaComp = 0,ADC3_dmaComp = 0;       //标志位重新置0
		  			  
			  ADC1_Ampl = Amp_calculation(ADC1_input_data);       //FFT计算ADC1测量结果		  
			  ADC3_Ampl_D = Amp_calculation(ADC3_input_data);     //FFT计算ADC3测量结果
			  f_gain_half[s] = (ADC3_Ampl_D) / ADC1_Ampl;
			  printf("j0.val=%d\xff\xff\xff", (uint8_t)((float)s / 110.0f *100.0f) + 1);	   //进度条
			  s++;
		  }
		  ProcessAmplifierGain(f_gain_half);
		  for(int a=0; a<220; a++)
		  {        
			  printf("add 1,0,%d\xff\xff\xff", (uint8_t)f_gain[a]);    
		  }
		  printf("t20.txt=\"%.1f\"\xff\xff\xff" , f_H);
		  printf("t1.txt=\"%d\"\xff\xff\xff", s-1);	
		  switch_kg = 0;                                 //只绘制一次，通过串口屏操作再次绘制
		  AD9833_SetFrequencyQuick (1000.0f, AD9833_OUT_TRIANGLE);  // 输出 1kHz 正弦波
	  }

	  else if(switch_kg == 3)           //三、发挥部分测量
	  {
		  int_R_calculation();     //测量输入电阻
		  out_R_calculation();     //测量输出电阻
		  gain_calculation();      //测量增益
		  phase_calculation();     //测量相位差
		  
		  printf("t3.txt=\"3\"\xff\xff\xff");		
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 512;
  RCC_OscInitStruct.PLL.PLLP = 10;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 4;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_0;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
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
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x08000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
