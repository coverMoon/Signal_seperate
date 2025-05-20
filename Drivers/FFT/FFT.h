#ifndef _FFT_H
#define _FFT_H

#include "main.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_SIZE            4096			// 采样数量
#define SAMPLE_RATE         40000           // 采样率

enum{
    HANNING = 1,            // 汉宁窗
    HAMMING = 2,            // 汉明窗
    BLACKMAN = 3,           // 布莱克曼窗
    BLACKMAN_HARRIS = 4     // 布莱克曼-哈里斯窗
};

// 前帧状态结构体，在相位差算法精确中使用
typedef struct{
	float32_t re;
    float32_t im;
	uint32_t k;
} bin_prev_t;

// 信号状态结构体，记录频率、幅值与相位
typedef struct{
	float32_t f;
	float32_t A;
	float32_t phi;
} tone_t;

void process_signal(void);
void Init_window(uint8_t window_type);
void FFT_start(uint8_t window_type);
void find_peaks(uint32_t *k1, uint32_t *k2);
float32_t interp_parabolic(float32_t left, float32_t center, float32_t right);
void corr_amp_phase(float32_t freq, const float32_t *x, float32_t *A_out, float32_t *phi_out);
void least_square(float32_t f1, float32_t f2, const float32_t *x, float32_t *I1, float32_t *Q1, float32_t *I2, float32_t *Q2);
void window_hanning(void);
void window_hamming(void);
void window_blackman(void);
void window_blackmanHarris(void);

#endif
