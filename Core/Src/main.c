/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dac.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Delay.h"
#include "math.h"
#include "LCDAPI.h"
#include "FFT.h"
#include "DDS.h"
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
extern uint16_t *ADCbuff;						// 采样数据
extern bin_prev_t prev[2];
extern tone_t tones[2];
extern DDS_TypeDef DDS;
uint16_t ADCbuff_2frame[FFT_SIZE * 2];
uint8_t frame_ready = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  delay_init(168);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  HAL_Delay(500);
  
  LCD_FillScreen(LCD_COLOR_WHITE);
  LCD_Disp_Text(10, 10, LCD_COLOR_BLACK, 3, ASCII5x7, "Succeed!");
  LCD_Disp_Text(15, 50, LCD_COLOR_BLACK, 3, ASCII5x7, "U1");
  LCD_Disp_Text(30, 80, LCD_COLOR_BLACK, 2, ASCII5x7, "FREQ: ");
  LCD_Disp_Text(30, 105, LCD_COLOR_BLACK, 2, ASCII5x7, "Vopp: ");
  LCD_Disp_Text(15, 150, LCD_COLOR_BLACK, 3, ASCII5x7, "U2");
  LCD_Disp_Text(30, 180, LCD_COLOR_BLACK, 2, ASCII5x7, "FREQ: ");
  LCD_Disp_Text(30, 205, LCD_COLOR_BLACK, 2, ASCII5x7, "Vopp: ");
  
//  DDS.amp = 2.0;
//  DDS.freq = 1000;
//  DDS.phase = 0;
//  DDS.duty = 0.5;
//  DDS.waveType = SINE_WAVE;
//  DDS.offset = 0.5;
//  DDS_Start();
  
  printf("start\r\n");
  
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCbuff_2frame, FFT_SIZE * 2);
  HAL_TIM_Base_Start(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 if (frame_ready) 
	 {
        ADCbuff = &ADCbuff_2frame[0]; 
		process_signal();
		 
		printf("\r\nf1=%8.3f Hz  A1=%6.3f  phi1=%7.3f  |  f2=%8.3f Hz  A2=%6.3f  phi2=%7.3f\r\n",
               tones[0].f, tones[0].A, tones[0].phi,
               tones[1].f, tones[1].A, tones[1].phi);
		 
		ADCbuff = &ADCbuff_2frame[4096];
		process_signal();
        frame_ready = 0;
           
        printf("\r\nf1=%8.3f Hz  A1=%6.3f  phi1=%7.3f  |  f2=%8.3f Hz  A2=%6.3f  phi2=%7.3f\r\n",
                tones[0].f, tones[0].A, tones[0].phi,
                tones[1].f, tones[1].A, tones[1].phi);
		 
		 
		// 装填DDS波形参数
		DDS.amp = tones[0].A * 2;
		DDS.freq = tones[0].f;
		DDS.phase = tones[0].phi;
		DDS.duty = 0.5;
		DDS.waveType = SINE_WAVE;
		DDS.offset = 0;
		DDS_Start();
		
		// 判断频率整数位数
		int intnum_1 = 0;
		int intnum_2 = 0;
		int temp_1 = (int)tones[0].f;
		int temp_2 = (int)tones[1].f;
		
		while(temp_1 > 0)
		{
			intnum_1++;
			temp_1 /= 10;
		}
		
		while(temp_2 > 0)
		{
			intnum_2++;
			temp_2 /= 10;
		}
			
		LCD_Disp_Decimal(100, 80, LCD_COLOR_BLACK, 2, ASCII5x7, (double)tones[0].f, intnum_1, 2);
		LCD_Disp_Text(145 + intnum_1 * 10, 80, LCD_COLOR_BLACK, 2, ASCII5x7, "Hz");

		LCD_Disp_Decimal(100, 105, LCD_COLOR_BLACK, 2, ASCII5x7, (double)tones[0].A * 10, 1, 2);
		LCD_Disp_Text(155, 105, LCD_COLOR_BLACK, 2, ASCII5x7, "V");
		
		LCD_Disp_Decimal(100, 180, LCD_COLOR_BLACK, 2, ASCII5x7, (double)tones[1].f, intnum_2, 2);
		LCD_Disp_Text(145 + intnum_2 * 10, 180, LCD_COLOR_BLACK, 2, ASCII5x7, "Hz");

		LCD_Disp_Decimal(100, 205, LCD_COLOR_BLACK, 2, ASCII5x7, (double)tones[1].A * 10, 1, 2);
		LCD_Disp_Text(155, 205, LCD_COLOR_BLACK, 2, ASCII5x7, "V");
		 
		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
