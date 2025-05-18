/**
  ******************************************************************************
  * @file      ILI9341.h
  * @brief     This file provides all the function prototypes and definitions
  *            for the ILI9341.c file.
  *            This file is part of driver for XJTU_MCU on-board screen.
  ******************************************************************************
  * @attention
  *
  * This software is developed by a Xi'an Jiaotong University student
  * for educational purposes. 
  * If no LICENSE file comes with this software, it is provided AS-IS.
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ILI9341_H
#define __ILI9341_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Defines -------------------------------------------------------------------*/
/* Built-in Essential Configurations */
#ifdef USE_BUILTIN_CONFIG
#define HSPI_ILI9341 &hspi2
#define ILI9341_Block_Size_Maximum 	200
#define ILI9341_SPI_TimeoutDuration 1
#endif

/* LCD Dimentions */
#ifdef USE_HORIZONTAL
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#else
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#endif

/* Color Palettes */
/* Color Palette Basic */
#define BLACK   0x0000
#define WHITE   0xffff
#define GRAY1   0x4228
#define GRAY2   0x8c51
#define GRAY3   0xce59
#define RED     0xf800
#define GREEN   0x07e0
#define BLUE    0x001f
#define YELLOW  0xffe0
#define MAGENTA 0xf81f
#define CYAN    0x07ff
/* Color Palette VSinger */
#if 0
#define TIANYI  0x665f
#define YZLING  0xe800
#define YANHE   0x07f9
#endif
/* Color Palette RW */
#if 1
#define SURVIVE 0xffff
#define MONK    0xffee
#define HUNTER  0xfb8e
#define ROTUND  0xee12
#define ARTI    0x7127
#define RIVULET 0x965d
#define SPEAR   0x516d
#define SAINT   0xaf8a
#define ENOT    0x192a
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Function prototypes -------------------------------------------------------*/
void ILI9341_Reset(void);
void ILI9341_SPI_Transmit(uint8_t Data);
void ILI9341_Write_Command(uint8_t Command);
void ILI9341_Write_Data(uint8_t Data);
void ILI9341_Write_Data16(uint16_t Data);
void ILI9341_Set_Address(uint16_t XStart, uint16_t YStart, uint16_t XEnd, uint16_t YEnd);
void ILI9341_Write_Data16Burst(uint16_t Data, uint32_t Size);
void ILI9341_Write_Data16Repeat(uint16_t Data, uint32_t Size);
void ILI9341_Init(void);
void ILI9341_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Color);
void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color);
void ILI9341_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color);
void ILI9341_Draw_Rectangle(int16_t X, uint16_t Y,uint16_t Width, uint16_t Height, uint16_t Color);
void ILI9341_FillScreen(uint16_t Color);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#endif /* __ILI9341_H */

/* by HSSE@xjtu */
