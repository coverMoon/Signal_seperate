#include "DDS.h"
#include "dac.h"
#include "tim.h"
#include "math.h"

DDS_TypeDef     		DDS;
volatile uint16_t  		DDS_lut[LUT_LENGTH];

/**
 * @brief       设置偏置电压
 * @param       offset:    		电压值
 * @retval      无
 */
void setOffset(float offset)
{
	uint16_t temp = offset / DDS_MAX_AMP * DAC_MAX_AMP;
	
	for(uint16_t i = 0; i < LUT_LENGTH; ++i)
	{
		DDS_lut[i] += temp;
	}
}

/**
 * @brief       开始DDS输出
 * @param       freq:    		输出频率
 * @param       amplitude:    	输出幅值
 * @param       type:    		输出波形
 * @param		duty:			占空比
 * @param		offset:         直流偏置电压
 * @retval      无
 */
void DDS_Start()
{
	DDS_setWaveParams(DDS.freq, DDS.amp, DDS.waveType, DDS.duty, DDS.offset);
	
	HAL_TIM_Base_Start(&htim8);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)DDS_lut, LUT_LENGTH, DAC_ALIGN_12B_R);
}

/**
 * @brief       停止DDS输出
 * @param       
 * @retval      无
 */
void DDS_Stop(void)
{
	HAL_TIM_Base_Stop(&htim8);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
}

/**
 * @brief       设置波形参数
 * @param       freq:    		输出频率
 * @param       amplitude:    	输出幅值
 * @param       type:    		输出波形
 * @param		duty:			占空比: 0~1
 * @param		offset:			直流偏置电压
 * @retval      无
 */
void DDS_setWaveParams(uint32_t freq, float amplitude, uint8_t type, float duty, float offset)
{
	// 停止DAC输出
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	// 关闭定时器
	HAL_TIM_Base_Stop(&htim8);

	// 设定波形
	DDS.waveType = type;
	DDS.freq = freq;
	DDS.amp = amplitude;
	DDS.duty = duty;
	
	
	// 设定频率，修改定时器寄存器参数，使arr值都稳定在三位数以上，减少误差
	if (freq >= 1 && freq <= 100)
	{
		// 频率为1~100Hz，arr为156~15625
		TIM8 -> PSC = 42 - 1;
		TIM8 -> ARR = 2 * TIM_INITIAL_CLK / LUT_LENGTH / (TIM8 -> PSC + 1) / freq - 1;
	}
	else if (freq > 100 && freq <= 1000)
	{
		//	频率为100~1000Hz，arr为218~2187
		TIM8 -> PSC = 3 - 1;
		TIM8 -> ARR = 2 * TIM_INITIAL_CLK / LUT_LENGTH / (TIM8 -> PSC + 1) / freq - 1;
	}
	else if (freq > 1000)
	{
		//	频率为1kHz以上，最高可达到65kHz左右
		TIM8 -> PSC = 0;
		TIM8 -> ARR = 2 * TIM_INITIAL_CLK / LUT_LENGTH / (TIM8 -> PSC + 1) / freq - 1;
	}
	
	getNewWaveLUT(LUT_LENGTH, DDS.freq, DDS.amp, DDS.waveType, DDS.duty, DDS.offset);
	setOffset(DDS.offset);
	
	
	// 重启定时器和DAC
	HAL_TIM_Base_Start(&htim8);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)DDS_lut, LUT_LENGTH / 2, DAC_ALIGN_12B_R);
}

/**
 * @brief       计算波形查找表
 * @param       length:			查找表长度
 * @param       freq:    		输出频率
 * @param       amplitude:    	输出幅值
 * @param       type:    		输出波形
 * @param		duty:			占空比
 * @param		offset:			直流偏置电压
 * @retval      无
 */
void getNewWaveLUT(uint32_t length, uint32_t freq, float amplitude, uint8_t type, float duty, float offset)
{
	switch(type){
		// 正弦波
		case SINE_WAVE:
			{
				float sin_step = 2.0f * 3.14159f / (float)(length-1);
				for (uint16_t i = 0; i < length; ++i)
				{
					DDS_lut[i] = (uint16_t) (DAC_MAX_AMP * ((float)amplitude * (sinf(sin_step * (float)i) + 1) / 2) / (float)DDS_MAX_AMP);
				}
				
				break;
			}
		// 方波
		case SQUARE_WAVE:
			{
				for(uint16_t i = 0; i < length / 2; ++i)
				{
					DDS_lut[i] = DAC_MAX_AMP * amplitude / DDS_MAX_AMP;
					DDS_lut[i + (length / 2)] = 0;
				}
				
				break;
			}
		// 矩形波
		case RECT_WAVE:
			{
				uint16_t flag = length * duty;
				for(uint16_t i = 0; i < flag; ++i)
				{
					DDS_lut[i] = DAC_MAX_AMP * amplitude / DDS_MAX_AMP;
				}
				for(uint16_t i = flag; i < length; ++i)
				{
					DDS_lut[i] = 0;
				}
				
				break;
			}
		// 三角波
		case TRIANGLE_WAVE:
			{
				uint16_t flag = length * duty;
				uint16_t tri_step_1 = DAC_MAX_AMP * amplitude / DDS_MAX_AMP / flag;
				uint16_t tri_step_2 = DAC_MAX_AMP * amplitude / DDS_MAX_AMP / (length - flag);
				
				for(uint16_t i = 0; i < flag; ++i)
				{
					DDS_lut[i] = tri_step_1 * i;
				}
				for(uint16_t i = flag; i < length; ++i)
				{
					DDS_lut[i] = DDS_lut[flag - 1] - tri_step_2 * (i - flag);
				}
				
				break;
			}
		default: break;
	}
}
