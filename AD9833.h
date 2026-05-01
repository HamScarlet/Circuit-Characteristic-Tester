#ifndef AD9833_H
#define AD9833_H

//#include "stm32h7xx_hal.h"
#include <math.h>
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* ========== 硬件定义（按你的实际连接修改） ========== */
#define AD9833_SPI            hspi1
#define AD9833_CS_PORT        GPIOA
#define AD9833_CS_PIN         GPIO_PIN_3        // FSYNC 片选引脚
#define AD9833_MCLK           25000000UL        // 参考时钟 25 MHz

/* ========== 寄存器常量（摘自原版 AD9833.h） ========== */
#define AD9833_REG_CMD        (0 << 14)
#define AD9833_REG_FREQ0      (1 << 14)
#define AD9833_REG_FREQ1      (2 << 14)
#define AD9833_REG_PHASE0     (6 << 13)
#define AD9833_REG_PHASE1     (7 << 13)

#define AD9833_B28            (1 << 13)
#define AD9833_HLB            (1 << 12)
#define AD9833_FSEL0          (0 << 11)
#define AD9833_FSEL1          (1 << 11)
#define AD9833_PSEL0          (0 << 10)
#define AD9833_PSEL1          (1 << 10)
#define AD9833_PIN_SW         (1 << 9)
#define AD9833_RESET          (1 << 8)
#define AD9833_SLEEP1         (1 << 7)
#define AD9833_SLEEP12        (1 << 6)
#define AD9833_OPBITEN        (1 << 5)
#define AD9833_SIGN_PIB       (1 << 4)
#define AD9833_DIV2           (1 << 3)
#define AD9833_MODE           (1 << 1)

/* 波形选择 */
#define AD9833_OUT_SINUS      ((0 << 5) | (0 << 1) | (0 << 3))
#define AD9833_OUT_TRIANGLE   ((0 << 5) | (1 << 1) | (0 << 3))
#define AD9833_OUT_MSB        ((1 << 5) | (0 << 1) | (1 << 3))
#define AD9833_OUT_MSB2       ((1 << 5) | (0 << 1) | (0 << 3))

/* 计算频率字的辅助常量 */
#define FREQ_CALC_FACTOR      (268435456.0f / AD9833_MCLK)

/* ========== 函数声明 ========== */
void AD9833_Init(void);
void AD9833_Reset(void);
void AD9833_ClearReset(void);
void AD9833_SetRegisterValue(unsigned short regValue);
void AD9833_SetFrequency(unsigned short reg, float fout, unsigned short type);
void AD9833_SetFrequencyQuick(float fout, unsigned short type);
void AD9833_SetPhase(unsigned short reg, unsigned short val);
void AD9833_Setup(unsigned short freq, unsigned short phase, unsigned short type);
void AD9833_SetWave(unsigned short type);

#endif