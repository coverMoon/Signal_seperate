/**
  ******************************************************************************
  * @file      ILI9341.c
  * @brief     This file provides code for operating a TFTLCD screen with
  *            ILI9341 as display driver.
  *            This file is part of driver for XJTU_MCU on-board screen.
  ******************************************************************************
  * @attention
  *
  * This software is developed by a Xi'an Jiaotong University student
  * for educational purposes. 
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  * This software is based on HAL. 
  * 
  * Required peripherals:
  * GPIO:
  * | Pin Name | GPIO Mode     | GPIO PU/PD | Max Out Speed | User Label  |
  * | PB12     | Out Push/Pull | No PU/PD   | Very High     | ILI9341_CS  |
  * | PC4      | Out Push/Pull | No PU/PD   | Very High     | ILI9341_DC  |
  * | PC5      | Out Push/Pull | No PU/PD   | Very High     | ILI9341_RST |
  * SPI:
  * Instance:       SPI2
  * Mode:           Transmit-Only Master
  * Data Size:      8 bit
  * First Bit:      MSB
  * Clock Polarity: Low
  * Clock Phase:    1 Edge
  * SPI_MOSI Pin:   PB15
  * SPI_SCK  Pin:   PB13
  * 
  * Required Configuration Definitions:
  * HSPI_INSTANCE &hspi2
  * ILI9341_Block_Size_Maximum Maximum size of SPI data burst. Only valid when
  *                            USE_LARGE_RAM is defined.
  *                            Should be no larger than heap size configured in 
  *                            starter file. Reduce if display is abnormal.
  * ILI9341_SPI_TimeoutDuration Timeout duration of SPI for ILI9341.
  * 
  * Optional Configuration Definitions :
  * USE_BUILTIN_CONFIG When defined, the required configurations above will
  *                    use built-in definitions, thus no longer required in
  *                    user code.
  * USE_LARGE_RAM When defined, color data of large areas are sent in bursts,
  *               rather than repeated bytes. Improves performance but requires
  *               more RAM.
  * USE_HORIZONTAL When defined, display orientation is horizontal (320x240). 
  *                Default orientation is vertical (240x320).
  * 
  * All definitions above should be defined in main.h.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "ILI9341.h"

/* Function Implementations --------------------------------------------------*/
/**
  * @brief  Send hardware reset to ILI9341.
  * @retval none
 */
void ILI9341_Reset()
{
    HAL_GPIO_WritePin(ILI9341_RST_GPIO_Port, ILI9341_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(ILI9341_RST_GPIO_Port, ILI9341_RST_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Send 1 byte of data to ILI9341 via SPI interface.
  * @param  Data data about to be sent (NOT pointer to data)
  * @retval none
 */
void ILI9341_SPI_Transmit(uint8_t Data)
{
    HAL_SPI_Transmit(HSPI_ILI9341, &Data, 1, ILI9341_SPI_TimeoutDuration);
}

/**
  * @brief  Send 1 command byte to ILI9341.
  * @retval none
 */
void ILI9341_Write_Command(uint8_t Command)
{
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
    ILI9341_SPI_Transmit(Command);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Send 1 data byte to ILI9341.
  * @retval none
 */
void ILI9341_Write_Data(uint8_t Data)
{
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    ILI9341_SPI_Transmit(Data);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Send 2 data bytes to ILI9341. This function is 
  *         intended to be used to send data for commands
  *         that require 16-bit data.
  *         | Command Function            | Data                                |
  *         | Column Address Write (0x2A) | Column Address SC[15:0] or EC[15:0] |
  *         | Page Address Write   (0x2B) | Page Address   SP[15:0] or EP[15:0] |
  *         | Memory Write         (0x2C) | 16-bit Color Data D[15:0]           |
  * @retval none
 */
void ILI9341_Write_Data16(uint16_t Data)
{
    uint8_t TempBuffer[2] = {Data>>8, Data};
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(HSPI_ILI9341, TempBuffer, 2, ILI9341_SPI_TimeoutDuration);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Set the memory address area to write into.
  *         This function sets the area to draw, and starts
  *         memory writing process.
  * @retval none
 */
void ILI9341_Set_Address(uint16_t XStart, uint16_t YStart, uint16_t XEnd, uint16_t YEnd)
{
    ILI9341_Write_Command(0x2A);
    ILI9341_Write_Data16(XStart);
    ILI9341_Write_Data16(XEnd);

    ILI9341_Write_Command(0x2B);
    ILI9341_Write_Data16(YStart);
    ILI9341_Write_Data16(YEnd);

    ILI9341_Write_Command(0x2C);
}

/**
  * @brief  Send 16-bit data to ILI9341 repeatedly in bursts.
  *         This function is intended to send color data for
  *         large areas.
  * @attention REQUIRE LARGE RAM. Configure the definition
  *            Block_Size_Maximum in ILI9341.h to adjust
  *            required RAM size.
  * @retval none
 */
void ILI9341_Write_Data16Burst(uint16_t Data, uint32_t Size)
{
    uint32_t Buffer_Size = 0;
    Buffer_Size = ((Size*2) < ILI9341_Block_Size_Maximum) ? (Size*2) : ILI9341_Block_Size_Maximum;
    uint8_t Data_UpperByte = Data>>8;
    uint8_t BurstBuffer[Buffer_Size];
    for(uint32_t i = 0; i < Buffer_Size; i+=2)
    {
        BurstBuffer[i] = Data_UpperByte;
        BurstBuffer[i+1] = Data;
    }
    uint32_t Block_Count = (Size*2)/Buffer_Size;
    uint32_t Remainder_Size = (Size*2)%Buffer_Size;
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    if(Block_Count)
    {
        for(uint32_t i = 0; i < Block_Count; i++)
        {
            HAL_SPI_Transmit(HSPI_ILI9341, BurstBuffer, Buffer_Size, ILI9341_SPI_TimeoutDuration);
        }
    }
    HAL_SPI_Transmit(HSPI_ILI9341, BurstBuffer, Remainder_Size, ILI9341_SPI_TimeoutDuration);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Send 16-bit data to ILI9341 repeatedly.
  *         This function is intended to send color data for
  *         large areas using less RAM.
  * @attention This implementation is slower than
  *            ILI9341_Write_Data16Burst.
  * @retval none
 */
void ILI9341_Write_Data16Repeat(uint16_t Data, uint32_t Size)
{
    uint8_t TempBuffer[2] = {Data>>8, Data};
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
    for(uint32_t i = 0; i < Size; i++)
    {
        HAL_SPI_Transmit(HSPI_ILI9341, TempBuffer, 2, ILI9341_SPI_TimeoutDuration);
    }
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief  Initialize ILI9341.
  * @retval none
 */
void ILI9341_Init()
{
    ILI9341_Reset();

    //Software Reset
    ILI9341_Write_Command(0x01);
    HAL_Delay(200);

    //Sleep OUT
    ILI9341_Write_Command(0x11);

    //Power Control A
    ILI9341_Write_Command(0xCB);
    ILI9341_Write_Data(0x39);
    ILI9341_Write_Data(0x2C);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x34); //VCore 1.6V
    ILI9341_Write_Data(0x05); //DDVDH 5.5V

    //Power Control B
    ILI9341_Write_Command(0xCF);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0xC1);
    ILI9341_Write_Data(0x30);

    //Driver Timing Control A
    ILI9341_Write_Command(0xE8);
    ILI9341_Write_Data(0x85);
    ILI9341_Write_Data(0x11);
    ILI9341_Write_Data(0x78);

    //Driver Timing Control B
    ILI9341_Write_Command(0xEA);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x00);

    //Power On Sequence Control
    ILI9341_Write_Command(0xED);
    ILI9341_Write_Data(0x64);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x12);
    ILI9341_Write_Data(0x81);

    //Pump Ratio Control
    ILI9341_Write_Command(0xF7);
    ILI9341_Write_Data(0x20);

    //Power Control 1
    ILI9341_Write_Command(0xC0);
    ILI9341_Write_Data(0x20);

    //Power Control 2
    ILI9341_Write_Command(0xC1);
    ILI9341_Write_Data(0x11);

    //VCOM Control 1
    ILI9341_Write_Command(0xC5);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0x3C);

    //VCOM Control 2
    ILI9341_Write_Command(0xC7);
    ILI9341_Write_Data(0xA9);

    //Interface Control
    ILI9341_Write_Command(0xF6);
    ILI9341_Write_Data(0x01);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x00);

    //Memory Access Control
    ILI9341_Write_Command(0x36);
    #ifdef USE_HORIZONTAL
        ILI9341_Write_Data(0xE8);
    #else
        ILI9341_Write_Data(0x48);
    #endif

    //Pixel Format
    ILI9341_Write_Command(0x3A);
    ILI9341_Write_Data(0x55); //use 16-bit RGB565

    //Frame Rate Control Normal Mode
    ILI9341_Write_Command(0xB1);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x18);

    //Display Inversion Control
    ILI9341_Write_Command(0xB4);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x00);

    //Disable 3Gamma
    ILI9341_Write_Command(0xF2);
    ILI9341_Write_Data(0x00);

    //Gamma Set
    ILI9341_Write_Command(0x26);
    ILI9341_Write_Data(0x01);

    //Positice Gamma Correction
    ILI9341_Write_Command(0xE0);  
    ILI9341_Write_Data(0x0F);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0x2B);
    ILI9341_Write_Data(0x0C);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x08);
    ILI9341_Write_Data(0x4E);
    ILI9341_Write_Data(0xF1);
    ILI9341_Write_Data(0x37);
    ILI9341_Write_Data(0x07);
    ILI9341_Write_Data(0x10);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x09);
    ILI9341_Write_Data(0x00);

    //Negative Gamma Correction
    ILI9341_Write_Command(0xE1);  
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x14);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x11);
    ILI9341_Write_Data(0x07);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0xC1);
    ILI9341_Write_Data(0x48);
    ILI9341_Write_Data(0x08);
    ILI9341_Write_Data(0x0F);
    ILI9341_Write_Data(0x0C);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0x36);
    ILI9341_Write_Data(0x0F);

    //Display ON
    ILI9341_Write_Command(0x29);
}

/**
  * @brief  Draw a pixel at position (X,Y) with selected color.
  * @retval none
 */
void ILI9341_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Color)
{
    if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
    ILI9341_Set_Address(X, Y, X, Y);
    ILI9341_Write_Data16(Color);
}

/**
  * @brief  Draw a horizontal line from (X,Y) to (X+Width-1,Y) with selected color.
  * @retval none
 */
void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color)
{
    if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
    Width = ((X+Width-1)>=LCD_WIDTH) ? (LCD_WIDTH - X) : Width;
    ILI9341_Set_Address(X, Y, X+Width-1, Y);
    #ifdef USE_LARGE_RAM
        ILI9341_Write_Data16Burst(Color,Width);
    #else
        ILI9341_Write_Data16Repeat(Color,Width);
    #endif
}

/**
  * @brief  Draw a vertical line from (X,Y) to (X,Y+Height-1) with selected color.
  * @retval none
 */
void ILI9341_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color)
{
    if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
    Height = ((Y+Height-1)>=LCD_HEIGHT) ? (LCD_HEIGHT - Y) : Height;
    ILI9341_Set_Address(X, Y, X, Y+Height-1);
    #ifdef USE_LARGE_RAM
        ILI9341_Write_Data16Burst(Color,Height);
    #else
        ILI9341_Write_Data16Repeat(Color,Height);
    #endif
}

/**
  * @brief  Draw a rectangle of Width and Height with selected color.
  *         The upper left corner is located at (X,Y).
  * @retval none
 */
void ILI9341_Draw_Rectangle(int16_t X, uint16_t Y,uint16_t Width, uint16_t Height, uint16_t Color)
{
    if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
    Width = ((X+Width-1)>=LCD_WIDTH) ? (LCD_WIDTH - X) : Width;
    Height = ((Y+Height-1)>=LCD_HEIGHT) ? (LCD_HEIGHT - Y) : Height;
    ILI9341_Set_Address(X, Y, X+Width-1, Y+Height-1);
    #ifdef USE_LARGE_RAM
        ILI9341_Write_Data16Burst(Color,Width*Height);
    #else
        ILI9341_Write_Data16Repeat(Color,Width*Height);
    #endif
}

/**
  * @brief  Fill whole screen with selected color.
  * @retval none
 */
void ILI9341_FillScreen(uint16_t Color)
{
    ILI9341_Set_Address(0, 0, LCD_WIDTH, LCD_HEIGHT);
    #ifdef USE_LARGE_RAM
        ILI9341_Write_Data16Burst(Color,86400);
    #else
        ILI9341_Write_Data16Repeat(Color,86400);
    #endif
}

/* by HSSE@xjtu */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
