#ifndef __DDS_H
#define __DDS_H

#include "main.h"


#define LUT_LENGTH      	(uint32_t)256      // 查找表长度，即一个周期采样点
#define DAC_MAX_AMP			(uint32_t)4095      // DAC寄存器写入最大值
#define DDS_MAX_AMP 		(float)3.30f        // DDS输出最大幅值
#define NULL_DUTY           1                   // 无占空比标志
#define TIM_INITIAL_CLK     168000000           // 定时器时钟

//	Wave types listed below
enum 
{
    SINE_WAVE = 0,          // 正弦波				
    SQUARE_WAVE = 1,        // 方波
    TRIANGLE_WAVE = 2,		// 三角波  
    RECT_WAVE = 3,			// 矩形波  						
};


//	DDS Type Define
typedef struct 
{
    uint8_t         waveType;   // 波形
    uint32_t        freq;       // 频率 
    float        	amp;        // 幅值
	float			phase;		// 相位
    float           duty;       // 占空比，取值0~1
	float			offset;		// 直流偏置电压
}   DDS_TypeDef;


//	Start DDS
void DDS_Start(void);
void DDS_Stop(void);
void DDS_setWaveParams(uint32_t freq, float amplitude, float phase, uint8_t type, float duty, float offset);
void getNewWaveLUT(uint32_t length, uint32_t freq, float amplitude, float phase, uint8_t type, float duty, float offset);

#endif
