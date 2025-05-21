#include "main.h"
#include "FFT.h"
#include "usart.h"

uint16_t *ADCbuff;								// 采样数据
static float32_t ADC_ConvData[FFT_SIZE];		// ADC模拟值
static float32_t fft_inputbuf[FFT_SIZE];  		// 用于FFT的输入数据
static float32_t fft_outputbuf[FFT_SIZE];		// 保存FFT结果
static float32_t mag[FFT_SIZE / 2];				// 保存FFT频域幅值
static float32_t window[FFT_SIZE]; 				// 窗函数
static float window_cg = 0.0f;					// 窗函数相干增益

bin_prev_t prev[2] = { 0 };
tone_t tones[2] = { 0 };


/**
 * @brief       信号处理
 * @note		暂时放在 FFT.c 中，后续可考虑更高层次的封装
 * @param       无
 * @retval      无
 */
void process_signal(void)
{
	// 开启FFT
	FFT_start(BLACKMAN_HARRIS);
	
	// 找到两信号粗估计bin下标
	uint32_t k1 = 0, k2 = 0;
	find_peaks(&k1, &k2);

	float32_t f1 = (float32_t)k1 * (float32_t)SAMPLE_RATE / (float32_t)FFT_SIZE;
	float32_t f2 = (float32_t)k2 * (float32_t)SAMPLE_RATE / (float32_t)FFT_SIZE;

	// 相位差法进一步精确
	float32_t *c1 = &fft_outputbuf[k1 * 2U];	// 得到复数频率点
	float32_t *c2 = &fft_outputbuf[k2 * 2U];
	float32_t phi1_now, phi2_now;				// 计算当前相位
	arm_atan2_f32(c1[1], c1[0], &phi1_now);
	arm_atan2_f32(c2[1], c2[0], &phi2_now);

	float32_t frameT = (float32_t)FFT_SIZE / (float32_t)SAMPLE_RATE;	// 频移周期，即每帧间隔 4096 / 40k = 0.1024 s
	if (prev[0].k == k1) {
        float32_t phi_prev;
		arm_atan2_f32(prev[0].im, prev[0].re, &phi_prev);
        float32_t delta_phi = phi1_now - phi_prev;
        if (delta_phi >  M_PI) 
			delta_phi -= 2.0f * M_PI;
        if (delta_phi < -M_PI) 
			delta_phi += 2.0f * M_PI;

        f1 += delta_phi / (2.0f * M_PI * frameT);
    }
    if (prev[1].k == k2) {
        float32_t phi_prev;
		arm_atan2_f32(prev[1].im, prev[1].re, &phi_prev);
        float32_t delta_phi = phi2_now - phi_prev;
        if (delta_phi >  M_PI) 
			delta_phi -= 2.0f * M_PI;
        if (delta_phi < -M_PI) 
			delta_phi += 2.0f * M_PI;

        f2 += delta_phi / (2.0f * M_PI * frameT);
    }

	// 更新前帧结构体
	prev[0] = (bin_prev_t){c1[0], c1[1], k1};
	prev[1] = (bin_prev_t){c2[0], c2[1], k2};
	
	if(!k2)
		f2 = 0.001f;
		
	tones[0].f = f1;  tones[1].f = f2;

	// 最小二乘法计算幅度与相位
	float32_t I1, Q1, I2, Q2;
	least_square(tones[0].f, tones[1].f, ADC_ConvData, &I1, &Q1, &I2, &Q2);
	tones[0].f = f1;
	arm_sqrt_f32(I1 * I1 + Q1 * Q1, &tones[0].A);
	arm_atan2_f32(Q1, I1, &tones[0].phi);
	tones[1].f = f2;
	if(f2 > 100)
	{
		arm_sqrt_f32(I2 * I2 + Q2 * Q2, &tones[1].A);
		arm_atan2_f32(Q2, I2, &tones[1].phi);
	}		
}

/**
 * @brief       最小二乘法计算幅度与相位
 * @note		得到的幅度为Vop，非Vopp
 * @param       f1, f2：			已确定频率
 * @param		x：					采样序列
 * @param		I1, Q1, I2, Q2：	求解参数组
 * @retval      无
 */
void least_square(float32_t f1, float32_t f2, const float32_t *x, 
				float32_t *I1, float32_t *Q1, 
				float32_t *I2, float32_t *Q2)
{
	// 计算一次角增量
	float32_t w1 = 2.0f * M_PI * f1 / SAMPLE_RATE;
    float32_t w2 = 2.0f * M_PI * f2 / SAMPLE_RATE;
	
	// 迭代生产sin, cos序列
	float32_t c1 = arm_cos_f32(w1), s1 = arm_sin_f32(w1);
    float32_t c2 = arm_cos_f32(w2), s2 = arm_sin_f32(w2);
    float32_t cn1 = 1, sn1 = 0, cn2 = 1, sn2 = 0;
	
	float32_t Scc1 = 0,Sss1 = 0,Scc2 = 0,Sss2 = 0;
    float32_t Scc12 = 0,Scs12 = 0,Ssc12 = 0,Sss12 = 0;
    float32_t SxC1 = 0,SxS1 = 0,SxC2 = 0,SxS2 = 0;
    for (uint32_t n = 0; n < FFT_SIZE; ++n) 
	{
        float32_t xn = x[n];
        // 同频能量
        Scc1 += cn1 * cn1;
        Sss1 += sn1 * sn1;
        Scc2 += cn2 * cn2;
        Sss2 += sn2 * sn2;
        // 跨频交叉项
        Scc12 += cn1 * cn2;
        Scs12 += cn1 * sn2;
        Ssc12 += sn1 * cn2;
        Sss12 += sn1 * sn2;
        // 数据投影
        SxC1 += xn * cn1;
        SxS1 += xn * sn1;
        SxC2 += xn * cn2;
        SxS2 += xn * sn2;
        // 迭代sin/cos
        float ncn1 = cn1 * c1 - sn1 * s1;
        float nsn1 = sn1 * c1 + cn1 * s1;
        float ncn2 = cn2 * c2 - sn2 * s2;
        float nsn2 = sn2 * c2 + cn2 * s2;
        cn1 = ncn1; 
		sn1 = nsn1; 
		cn2 = ncn2; 
		sn2 = nsn2;
    }
	
	// 构建正交矩阵和投影向量，部分交叉项可近似为0
	float32_t H[4][4] = {
        {Scc1,    0,      Scc12,  Scs12},
        {0,       Sss1,   Ssc12,  Sss12},
        {Scc12,   Ssc12,  Scc2,   0    },
        {Scs12,   Sss12,  0,      Sss2 }
    };	
	float32_t b[4] = {SxC1, SxS1, SxC2, SxS2};
	
	// 高斯消元
    for (int k = 0; k < 4; k++) 
	{
        // 主元选择
        int m = k;
        float_t maxv = fabsf(H[k][k]);
        for (int i = k + 1; i < 4; ++i) 
		{
			if(fabsf(H[i][k]) > maxv)
			{
				m = i;
				maxv = fabsf(H[i][k]);
			}
		}
        // 交换行		
        for(int j = k; j < 4; ++j) 
		{
			float_t t = H[k][j]; 
			H[k][j] = H[m][j]; 
			H[m][j] = t;
		} 
        float32_t tb = b[k]; 
		b[k] = b[m]; 
		b[m] = tb;
        
        // 标准化主行
        float32_t diag = H[k][k];
        for (int j = k; j < 4; ++j) 
			H[k][j] /= diag; 
		
		int test = 0;
		if(k == 3)
			test = 1;
		
		b[k] /= diag;
		
		// 消去下方得到上三角式
        for (int i = k + 1; i < 4; ++i) 
		{
            float_t factor = H[i][k];
            for(int j = k; j < 4; ++j) 
				H[i][j] -= factor * H[k][j];
			
            b[i] -= factor * b[k];
        }
    }
	
	// 回代求解
    float32_t theta[4];
    for(int i = 3; i >= 0; --i)
	{
        float32_t sum = b[i];
        for(int j = i + 1; j < 4; ++j) 
			sum -= H[i][j] * theta[j];
		
        theta[i] = sum;
    }
    
	// 缩放比例
//    float32_t scale = 2.0f/ (float32_t)FFT_SIZE ;
    *I1 = theta[0];
    *Q1 = theta[1];
    *I2 = theta[2];
    *Q2 = theta[3];
}

/**
 * @brief       相关法计算幅度与相位
 * @note		得到的幅度为Vop，非Vopp
 * @param       freq: 		已确定的频率
 * @param		x:			采样序列
 * @param		A_out:		幅度输出
 * @pqarm		phi_out:	相位输出
 * @retval      无
 */
void corr_amp_phase(float32_t freq, const float32_t *x, float32_t *A_out, float32_t *phi_out)
{
	// 计算一次角增量
	float32_t w  = 2.0f * M_PI * freq / SAMPLE_RATE;
	float32_t c = arm_cos_f32(w);
	float32_t s = arm_sin_f32(w);

	// CORDIC-like迭代生成sin,cos序列
	float32_t cos_n = 1.0f, sin_n = 0.0f;
	float32_t acc_cos = 0.0f, acc_sin = 0.0f;	

	int n;
	for(n = 0; n < FFT_SIZE; ++n)
	{
		float32_t xn = x[n];
		acc_cos += xn * cos_n;
		acc_sin += xn * sin_n;

		// 迭代下一角度
		float32_t cos_next = cos_n * c - sin_n * s;
		float32_t sin_next = sin_n * c + cos_n * s;
		cos_n = cos_next;
		sin_n = sin_next;
	}

//	float32_t scale = 2.0f / ((float32_t)FFT_SIZE * window_cg);		// 计算缩放比例
	float32_t scale = 2.0f / (float32_t)FFT_SIZE;
	float32_t a = acc_cos * scale;		// Asin(phi)
	float32_t b = acc_sin * scale;		// Acos(phi)

	arm_sqrt_f32(a * a + b * b, A_out);
	arm_atan2_f32(a, b, phi_out);
}

/**
 * @brief       抛物线插值算法
 * @note		通过三个点确定一条抛物线，抛物线峰值即为所求点
 * @param       无
 * @retval      无
 */
float32_t interp_parabolic(float32_t left, float32_t center, float32_t right)
{
	float32_t denom = 2.0f * (left - 2.0f * center + right);
	if(fabsf(denom) < 1e-12f)
		return 0.0f;
	else	
		return (left - right) / denom;
}

/**
 * @brief       开启FFT
 * @note		选择窗函数并做FFT处理
 * @param       window_type: 窗函数类型
 * @retval      无
 */
void FFT_start(uint8_t window_type)
{
	int i;						// 计数器
		
	Init_window(window_type);
	
	float32_t sum_avr = 0.0f;
	for(i = 0; i < FFT_SIZE; ++i)
	{
		ADC_ConvData[i] = (float32_t)ADCbuff[i] * 3.3f / 4096.0f;
		sum_avr += ADC_ConvData[i];
	}
	sum_avr = sum_avr / FFT_SIZE;
	
	// 初始化FFT输入数组
	for(i = 0; i < FFT_SIZE; ++i)
	{
		ADC_ConvData[i] = ADC_ConvData[i] - sum_avr;
		fft_inputbuf[i] = ADC_ConvData[i] * window[i];
	}

	// FFT
	arm_rfft_fast_instance_f32 Rfft;
	arm_rfft_fast_init_f32(&Rfft, FFT_SIZE);

	arm_rfft_fast_f32(&Rfft, fft_inputbuf, fft_outputbuf, 0);
	arm_cmplx_mag_f32(fft_outputbuf, mag, FFT_SIZE);
	
	
	
//	for(i = 0; i < FFT_SIZE; ++i)
//	{
//		fft_inputbuf[i * 2] = ((float)ADCbuff[i] * 3.3f / 4096.0f);	// 采样值填入实部部分
//		fft_inputbuf[i * 2 + 1] = 0;			// 虚部全部为0
//	}
//	
//	arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_inputbuf, 0, 1);  	//fft运算
//	arm_cmplx_mag_f32(fft_inputbuf,mag,FFT_SIZE);			//把运算结果复数求模得幅值

//	for(int i = 0; i < FFT_SIZE / 2; ++i)
//		printf("%.2f, ", mag[i]);
}

/**
 * @brief       找到模值最大的两个下标
 * @note		该函数仅针对本次分离两个信号的设计，更低耦合的方法应排序后根据需要选取前几项数据，但耗时更多一些
 * @param       k1: 能量较大信号的下标
 * @param		k2: 能量次之信号的下标
 * @retval      无
 */
void find_peaks(uint32_t *k1, uint32_t *k2)
{
	float32_t m = 10;
	int i = 0;
	
	for(int k = 0; k < FFT_SIZE / 2; ++k)
	{
		float32_t v = mag[k];
		if(v > m)
		{
			m = v;
			i = k;
		}
	}
	
	*k1 = i;
	i = 0;
	m = 10;
	for(int k = 0; k < FFT_SIZE / 2; ++k)
	{
		if(k > *k1 - 4 && k <*k1 + 4)
			continue;
		
		float32_t v = mag[k];		
		if(v > m)
		{
			m = v;
			i = k;
		}
	}
	*k2 = i;
		
//	float32_t m1 = 0, m2 = 0; 
//	int i1 = 0, i2 = 0;
//    for (int k = 9; k < FFT_SIZE / 2; ++k) 
//	{
//        float32_t v = mag[k];
//        if (v > m1) 
//		{ 
//			if((k - i1 > 9) || (k - i1 < -9))
//			{
//				m2 = m1; 
//				i2 = i1;
//			}
//			m1 = v;
//			i1 = k;
//		}
//        else if (v > m2 && ((k - i1 > 9) || (k - i1 < -9))) 
//		{ 
//			m2 = v; 
//			i2 = k;
//		}
//    }
//	
//    *k1 = i1; 
//	*k2 = i2;
}

/**
 * @brief       初始化窗函数
 * @note		对采样数据加窗函数后作FFT，并得到频域幅值
 * @param       window_type:	选择窗函数类型
 * @retval      无
 */
void Init_window(uint8_t window_type)
{
	switch(window_type){
		case HANNING:
			window_hanning();
			break;
		case HAMMING:
			window_hamming();
			break;
		case BLACKMAN:
			window_blackman();
			break;
		case BLACKMAN_HARRIS:
			window_blackmanHarris();
			break;
		default: break;
	}
}

/**
 * @brief       汉宁窗函数
 * @note		无
 * @param       无
 * @retval      无
 */
void window_hanning(void)
{
	int i = 0;
	float32_t sum = 0.0f;
	for(i = 0; i < FFT_SIZE; ++i)
	{
		window[i] = 0.5f * (1.0f - arm_cos_f32(2.0f * M_PI * i / (FFT_SIZE - 1)));
		sum += window[i];	
	}

	window_cg = sum / (float32_t)FFT_SIZE;
}

/**
 * @brief       汉明窗函数
 * @note		无
 * @param       无
 * @retval      无
 */
void window_hamming(void)
{
	int i = 0;
	float32_t sum = 0.0f;
	for(i = 0; i < FFT_SIZE; ++i)
	{
		window[i] = 0.54f - 0.46f * arm_cos_f32(2.0f * M_PI * i / (FFT_SIZE - 1));
		sum += window[i];
	}

	window_cg = sum / (float32_t)FFT_SIZE;
}

/**
 * @brief       布莱克曼窗函数
 * @note		无
 * @param       无
 * @retval      无
 */
void window_blackman(void)
{
	int i = 0;
	float32_t sum = 0.0f;
	for(i = 0; i < FFT_SIZE; ++i)
	{
		window[i] = 0.42323f - 0.49755f * arm_cos_f32(2.0f * M_PI * i / (FFT_SIZE - 1)) + 0.07922f * arm_cos_f32(4.0f * M_PI * i / (FFT_SIZE - 1));
		sum += window[i];
	}

	window_cg = sum / (float32_t)FFT_SIZE;
}

/**
 * @brief       布莱克曼-哈里斯窗函数
 * @note		无
 * @param       无
 * @retval      无
 */
void window_blackmanHarris(void)
{
	int i = 0;
	float32_t sum = 0.0f;
	for(i = 0; i < FFT_SIZE; ++i)
	{	
		window[i] = 0.35875f - 0.48829f * arm_cos_f32(2.0f * M_PI * i / (FFT_SIZE - 1)) + 0.14128f * arm_cos_f32(4.0f * M_PI * i / (FFT_SIZE - 1)) - 0.01168f * arm_cos_f32(6.0f * M_PI * i / (FFT_SIZE - 1));
		sum += window[i];
	}

	window_cg = sum / (float32_t)FFT_SIZE;
}
