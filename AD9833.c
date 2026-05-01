#include "ad9833.h"

/* ------------------------------------------------------------
 * 内部函数：硬件 SPI 发送 16 位数据（MSB first，8 位 × 2）
 * ------------------------------------------------------------ */
static void AD9833_WriteSPI(uint16_t data) {
    uint8_t tx_buf[2];
    tx_buf[0] = (uint8_t)((data >> 8) & 0xFF);
    tx_buf[1] = (uint8_t)(data & 0xFF);

    HAL_GPIO_WritePin(AD9833_CS_PORT, AD9833_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&AD9833_SPI, tx_buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(AD9833_CS_PORT, AD9833_CS_PIN, GPIO_PIN_SET);
}

/* ------------------------------------------------------------
 * 写 16 位控制字（与原版 AD9833_SetRegisterValue 功能相同）
 * ------------------------------------------------------------ */
void AD9833_SetRegisterValue(unsigned short regValue) {
    AD9833_WriteSPI(regValue);
}

/* ------------------------------------------------------------
 * 初始化模块（IO 和寄存器）
 * ------------------------------------------------------------ */
void AD9833_Init(void) {
    // 复位序列
    AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
    HAL_Delay(10);
    AD9833_ClearReset();
    // 默认：B28 连续写入模式，输出正弦波，频率 0
    AD9833_SetRegisterValue(AD9833_B28 | AD9833_OUT_SINUS);
}

/* ------------------------------------------------------------
 * 置位复位
 * ------------------------------------------------------------ */
void AD9833_Reset(void) {
    AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
    HAL_Delay(10);
}

/* ------------------------------------------------------------
 * 清除复位
 * ------------------------------------------------------------ */
void AD9833_ClearReset(void) {
    AD9833_SetRegisterValue(AD9833_REG_CMD);
}

/* ------------------------------------------------------------
 * 设置频率（完整版，可指定 FREQ0/FREQ1）
 * ------------------------------------------------------------ */
void AD9833_SetFrequency(unsigned short reg, float fout, unsigned short type) {
    unsigned long val = (unsigned long)(FREQ_CALC_FACTOR * fout);
    unsigned short freqLo = reg | (val & 0x3FFF);
    unsigned short freqHi = reg | ((val >> 14) & 0x3FFF);

    AD9833_SetRegisterValue(AD9833_B28 | type);
    AD9833_SetRegisterValue(freqLo);
    AD9833_SetRegisterValue(freqHi);
}

/* ------------------------------------------------------------
 * 快捷频率设置（使用 FREQ0）
 * ------------------------------------------------------------ */
void AD9833_SetFrequencyQuick(float fout, unsigned short type) {
    AD9833_SetFrequency(AD9833_REG_FREQ0, fout, type);
}

/* ------------------------------------------------------------
 * 设置相位（指定 PHASE0/PHASE1）
 * ------------------------------------------------------------ */
void AD9833_SetPhase(unsigned short reg, unsigned short val) {
    unsigned short phase = reg | (val & 0x0FFF);   // 12 位相位
    AD9833_SetRegisterValue(phase);
}

/* ------------------------------------------------------------
 * 组合设置：选择频率、相位寄存器和波形
 * ------------------------------------------------------------ */
void AD9833_Setup(unsigned short freq, unsigned short phase, unsigned short type) {
    unsigned short val = freq | phase | type;
    AD9833_SetRegisterValue(val);
}

/* ------------------------------------------------------------
 * 仅切换波形（不改变频率/相位）
 * ------------------------------------------------------------ */
void AD9833_SetWave(unsigned short type) {
    // 只改写控制寄存器的波形位，保留 B28 等常用位
    AD9833_SetRegisterValue(AD9833_B28 | type);
}