#include <fun.h>
#define FFT_LEN 1024                     //FFT点数

float f = 10000.0f;                     // 信号频率
float fs = 512000.0f;                   // 采样率
uint16_t ADC1_input_data[FFT_LEN] = {0};       // ADC1数据数组
uint16_t ADC2_input_data[FFT_LEN] = {0};       // ADC2数据数组
uint16_t ADC3_input_data[FFT_LEN] = {0};       // ADC3数据数组

float input_hanning_data [FFT_LEN] = {0};   // hanning窗处理数据数组
float FFT_buffer[FFT_LEN*2] = {0};     // FFT函数输入输出数组 

float FFT_mag[FFT_LEN] = {0};         // 幅度谱
uint32_t FFT_mag_data_index = 0;      // 基波幅值的索引
float FFT_phase = 0;                   // 基波的相位
float FFT_Freq = 0;                   // FFT计算得到的基波频率
float FFT_Ampl = 0;                   // FFT计算得到基波幅值

float ADC1_Ampl = 0;            //ADC1采集数据测量结果
float ADC2_Ampl = 0;            //ADC2采集数据测量结果
float ADC3_Ampl_T = 0;          //ADC3采集数据测量结果(通)
float ADC3_Ampl_D = 0;          //ADC3采集数据测量结果（断）
float ADC1_phase = 0;                   //输入信号相位
float ADC3_phase = 0;                   //输出信号相位
float ADC13_phase_difference = 0;       //输入输出信号相位差

float int_R = 0;    //输入电阻
float out_R = 0;    //输出电阻
float gain = 0;    //增益
float f_gain_half[110] = {0};   //扫幅频特性曲线
float f_gain[220] = {0};   //幅频特性曲线
float f_H = 0;  //上限频率
     
void apply_hanning(uint16_t *input_data)
{
    // 1. 计算直流分量（均值）
    float mean = 0.0f;
    for (int i = 0; i < FFT_LEN; i++) 
	{
        mean += (float)input_data[i];
    }
    mean /= FFT_LEN;

    // 2. 去除直流并加窗
    for (int i = 0; i < FFT_LEN; i++)
    {
        float win = 0.5f * (1.0f - cosf(2.0f * PI * (float)i / (float)(FFT_LEN - 1)));
        float ac_val = (float)input_data[i] - mean;   // 交流分量
        input_hanning_data[i] = ac_val * win;
    }
}

void FFT()      // 复数FFT变换
{
    //复数据转换
    for(int i=0; i<FFT_LEN; i++)
    {
        FFT_buffer[2*i] = input_hanning_data [i];  //实部
        FFT_buffer[2*i+1] = 0;           //虚部
    }

    //显示数据
//    Show_data(FFT_buffer, FFT_LEN*2);
//    HAL_Delay(2000);

    //FFT计算
    arm_cfft_f32(&arm_cfft_sR_f32_len1024 , FFT_buffer, 0, 1);

	//清除直流量
	FFT_buffer[0]=0;
	FFT_buffer[1]=0;
	FFT_buffer[2]=0;
	FFT_buffer[3]=0;
	
    //计算复数幅值谱
    arm_cmplx_mag_f32(FFT_buffer, FFT_mag, FFT_LEN);
	
    //显示数据
//    Show_data(FFT_mag, FFT_LEN);
//    HAL_Delay(2000);
}

// 能量重心矫正函数（优先使用四点法，条件不足时回退到三点法）
// 输入：粗略峰值索引 peak_idx（需在1~FFT_LEN/2-2范围内）
// 输出：矫正后的频率、幅值、相位
void energy_centroid_correction(uint32_t peak_idx, float* freq, float* amp, float* phase)
{
    // 确保峰值索引在有效范围内
    if (peak_idx < 1 || peak_idx >= FFT_LEN/2) {
        *freq = peak_idx * fs / FFT_LEN;
        *amp = 0;
        *phase = 0;
        return;
    }

    // 尝试四点法（需要索引 ≥2 且 ≤ N/2-2）
    if (peak_idx >= 2 && peak_idx <= FFT_LEN/2 - 2) {
        // 提取四条谱线的实部和虚部
        float real_m2 = FFT_buffer[2*(peak_idx-2)];
        float imag_m2 = FFT_buffer[2*(peak_idx-2)+1];
        float real_m1 = FFT_buffer[2*(peak_idx-1)];
        float imag_m1 = FFT_buffer[2*(peak_idx-1)+1];
        float real_m  = FFT_buffer[2*peak_idx];
        float imag_m  = FFT_buffer[2*peak_idx+1];
        float real_p1 = FFT_buffer[2*(peak_idx+1)];
        float imag_p1 = FFT_buffer[2*(peak_idx+1)+1];

        // 计算幅值（模值）
        float mag_m2 = sqrtf(real_m2*real_m2 + imag_m2*imag_m2);
        float mag_m1 = sqrtf(real_m1*real_m1 + imag_m1*imag_m1);
        float mag_m  = sqrtf(real_m*real_m + imag_m*imag_m);
        float mag_p1 = sqrtf(real_p1*real_p1 + imag_p1*imag_p1);

        float sum_mag = mag_m2 + mag_m1 + mag_m + mag_p1;
        if (sum_mag > 0) {
            // 频率矫正：加权平均索引
            float weight_idx = (mag_m2*(peak_idx-2) + mag_m1*(peak_idx-1) + mag_m*peak_idx + mag_p1*(peak_idx+1)) / sum_mag;
            *freq = weight_idx * fs / FFT_LEN;

            // 幅值矫正（四点法公式，系数基于Hanning窗能量恢复，已包含单边谱修正）
            *amp = (mag_m2 + 2.0f*mag_m1 + 2.0f*mag_m + mag_p1) * 2.0f / FFT_LEN;

            // 相位矫正（取中心谱线相位）
            *phase = atan2f(imag_m, real_m);
            return;
        }
    }

    // 四点法条件不满足，尝试三点法（需要索引 ≥1 且 ≤ N/2-1）
    if (peak_idx >= 1 && peak_idx <= FFT_LEN/2 - 1) {
        // 提取三条谱线的实部和虚部
        float real_m1 = FFT_buffer[2*(peak_idx-1)];
        float imag_m1 = FFT_buffer[2*(peak_idx-1)+1];
        float real_m  = FFT_buffer[2*peak_idx];
        float imag_m  = FFT_buffer[2*peak_idx+1];
        float real_p1 = FFT_buffer[2*(peak_idx+1)];
        float imag_p1 = FFT_buffer[2*(peak_idx+1)+1];

        float mag_m1 = sqrtf(real_m1*real_m1 + imag_m1*imag_m1);
        float mag_m  = sqrtf(real_m*real_m + imag_m*imag_m);
        float mag_p1 = sqrtf(real_p1*real_p1 + imag_p1*imag_p1);

        float sum_mag = mag_m1 + mag_m + mag_p1;
        if (sum_mag > 0) {
            float weight_idx = (mag_m1*(peak_idx-1) + mag_m*peak_idx + mag_p1*(peak_idx+1)) / sum_mag;
            *freq = weight_idx * fs / FFT_LEN;
            *amp = (mag_m1 + 2.0f*mag_m + mag_p1) * 2.0f / FFT_LEN;
            *phase = atan2f(imag_m, real_m);
            return;
        }
    }

    // 均失败，回退到原始值
    *freq = peak_idx * fs / FFT_LEN;
    *amp = FFT_mag[peak_idx] * 2.0f / FFT_LEN;
    *phase = atan2f(FFT_buffer[2*peak_idx+1], FFT_buffer[2*peak_idx]);
}

// 处理基波（能量重心矫正）
void Process_FFT_mag()
{
    // 在幅度谱前一半数据中找最大值（跳过索引0，避免直流残留）
    // 注意：arm_max_f32 会返回最大值和相对索引，需加1补偿
    arm_max_f32(&FFT_mag[1], FFT_LEN/2 - 1, &FFT_Ampl, &FFT_mag_data_index);
    FFT_mag_data_index += 1;   // 转换为真实索引

    uint32_t peak_idx = FFT_mag_data_index;
    float freq_corr, amp_corr, phase_corr;
    energy_centroid_correction(peak_idx, &freq_corr, &amp_corr, &phase_corr);

    // 保存基波矫正结果
    FFT_Freq = freq_corr;
    FFT_Ampl = amp_corr;         // 基波幅值存入数组（覆盖原未矫正值）
    FFT_phase = phase_corr;      // 基波相位存入数组
	FFT_phase += PI/2;
}

float Amp_calculation(uint16_t *intput_data)
{
	apply_hanning (intput_data);       //FFT计算ADC1测量结果
	FFT();
	Process_FFT_mag();
	return FFT_Ampl;
}

/**
 * @brief 将 110 点原始增益数据插值、缩放并计算上限频率
 * @param input 原始增益数组，长度为 110，依次对应 2kHz, 4kHz, ..., 220kHz
 */
void ProcessAmplifierGain(float *input)
{
    // ========================
    // 1. 线性插值：110 点 -> 220 点 (频率 1kHz ~ 220kHz)
    // ========================
    
    // 1a. 首点：1kHz 直接用第一个原始值（2kHz 的增益）
    f_gain[0] = input[0];
    // 1b. 第二个点：2kHz（原始第一个值）
    f_gain[1] = input[0];

    // 1c. 中间处理：i 从 1 到 108，对应原始索引 1..108（频率 4kHz..218kHz）
    for (int i = 1; i <= 108; i++) {
        int idx_insert = 2 * i;       // 插入点位置，对应奇数频率 2*i+1 kHz
        int idx_even   = 2 * i + 1;   // 原始偶数频率点位置，对应 2*(i+1) kHz
        
        // 插入点：由前一个偶数点 (input[i-1]) 和当前偶数点 (input[i]) 线性插值
        f_gain[idx_insert] = (input[i-1] + input[i]) * 0.5f;
        // 原始偶数点
        f_gain[idx_even]   = input[i];
    }

    // 1d. 末点处理：219kHz 和 220kHz
    // 219kHz (插入点) 直接用最后一个原始值（220kHz 增益）
    f_gain[218] = input[109];
    // 220kHz (原始最后一个值)
    f_gain[219] = input[109];


    // ========================
    // 2. 缩放：每个值 / 最大值 * 200
    // ========================
    
    // 寻找最大值
    float max_gain = f_gain[0];
    for (int i = 1; i < 220; i++) {
        if (f_gain[i] > max_gain)
            max_gain = f_gain[i];
    }
    // 防止除零（极低频或无信号情况）
    if (max_gain < 1e-12f)
        max_gain = 1e-12f;

    // 缩放因子：乘以 200.0 / max_gain
    float scale = 200.0f / max_gain;
    for (int i = 0; i < 220; i++) {
        f_gain[i] *= scale;
    }


    // ========================
    // 3. 计算上限频率 f_H
    // ========================
    
    // -3 dB 阈值（相对最大增益 200 的 -3dB）
    const float threshold = 200.0f * powf(10.0f, -3.0f / 20.0f); // ≈141.589
    
    // 从高频向低频搜索（220kHz -> 1kHz）
    int i;
    for (i = 219; i >= 0; i--) {
        if (f_gain[i] >= threshold)
            break;
    }

    if (i == 219) {
        // 最高频 220kHz 的增益就已经 ≥ 阈值，说明上限频率 ≥ 220kHz
        f_H = 220000.0f;
    } else if (i < 0) {
        // 所有频率的增益都低于阈值，上限频率设为最低频率（通常不会出现）
        f_H = 1000.0f;
    } else {
        // 在 i 和 i+1 两个点之间进行线性插值，精确定位 -3dB 频率
        // 点 A: 频率 (i+1)*1000 Hz, 增益 f_gain[i]   (≥ threshold)
        // 点 B: 频率 (i+2)*1000 Hz, 增益 f_gain[i+1] ( < threshold)
        float yA = f_gain[i];
        float yB = f_gain[i+1];
        float t = (threshold - yA) / (yB - yA);  // 在 A、B 之间的比例位置
        
        // 上限频率 = A点频率 + t * 步长(1000 Hz)
        f_H = (i + 1.0f + t) * 1000.0f;
    }
}

void Show_data(float* buffer, uint16_t n)  //打印波形
{
    for(int i=0; i<n; i++)
    {
        printf("%.3f\n", buffer[i]);
    }
}

