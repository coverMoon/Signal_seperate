/**
  ******************************************************************************
  * @file      LCDAPI.h
  * @brief     This file provides all the function prototypes and definitions
  *            for the LCDAPI.c file.
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
#ifndef __LCDAPI_H
#define __LCDAPI_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ILI9341.h"
#include "LCDFONT.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Defines -------------------------------------------------------------------*/
#define LCD_NUM_UPPERDIGITS 1
#define LCD_NUM_LOWERDIGITS 0

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Function prototypes -------------------------------------------------------*/
void LCD_Init(void);
void LCD_InitBlack(void);
void LCD_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Color);
void LCD_Draw_Line_Horizontal(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color);
void LCD_Draw_Line_Vertical(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color);
void LCD_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color);
void LCD_FillScreen(uint16_t Color);
void LCD_Draw_Line(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color);
void LCD_Draw_Box_Hollow(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color);
void LCD_Draw_Box_Filled(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color);
void LCD_Draw_Circle_Hollow(uint16_t XCenter, uint16_t YCenter, uint16_t Radius, uint16_t Color);
void LCD_Draw_Circle_Filled(uint16_t XCenter, uint16_t YCenter, uint16_t Radius, uint16_t Color);
void LCD_Draw_Horizontal_Arrow(uint16_t XOri, uint16_t YOri, int16_t Length, uint8_t ArrowSize, uint16_t Color);
void LCD_Draw_Vertical_Arrow(uint16_t XOri, uint16_t YOri, int16_t Length, uint8_t ArrowSize, uint16_t Color);
void LCD_Draw_Char(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint8_t Char);
void LCD_Disp_Text(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, const char* Text);
void LCD_Disp_NumUpp(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits);
void LCD_Disp_NumLow(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits);
void LCD_Disp_Num(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits, uint8_t Type);
void LCD_Disp_Decimal(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, double Num, uint16_t intDigits, uint16_t deciDigits);
void LCD_Disp_Axis_Quadrant(int16_t XOri, int16_t YOri, int16_t XRange, int16_t YRange, uint8_t ArrowSize, uint16_t Color);
uint32_t LCD_FUNC_Power(uint32_t a, uint32_t n);
void LCD_FUNC_SwapU16(uint16_t *X, uint16_t *Y);
void LCD_Disp_DotGrid(uint16_t X, uint16_t Y, const char* DotGrid, uint16_t Width_Bytes, uint16_t Height, uint16_t Color);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#endif /* __LCDAPI_H */

/* by HSSE@xjtu */
