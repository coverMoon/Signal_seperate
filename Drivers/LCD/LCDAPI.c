/**
  ******************************************************************************
  * @file	   LCDAPI.c
  * @brief     This file provides APIs for rendering shapes and text on 
  *            the XJTU_MCU on-board screen.
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
  * This file requires ILI9341.c by same author as screen driver.
  * This file requires LCDFONT.h by same author as font library.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "LCDAPI.h"

/* Function Implementations --------------------------------------------------*/
/**
  * @brief  Initialize screen.
  * @retval none
 */
void LCD_Init()
{
    ILI9341_Init();
    ILI9341_FillScreen(0xFFFF);
}

 /**
  * @brief  Initialize screen with black background.
  * @retval none
 */
void LCD_InitBlack()
{
    ILI9341_Init();
    ILI9341_FillScreen(0x0000);
}

/**
  * @brief  Draw a pixel at position (X,Y) with selected color.
  * @retval none
 */
void LCD_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Color)
{
    ILI9341_Draw_Pixel(X, Y, Color);
}

/**
  * @brief  Draw a horizontal line from (X,Y) to (X+Width-1,Y) with selected color.
  * @retval none
 */
void LCD_Draw_Line_Horizontal(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color)
{
    ILI9341_Draw_Horizontal_Line(X, Y, Width, Color);
}

/**
  * @brief  Draw a vertical line from (X,Y) to (X,Y+Height-1) with selected color.
  * @retval none
 */
void LCD_Draw_Line_Vertical(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color)
{
    ILI9341_Draw_Vertical_Line(X, Y, Height, Color);
}

/**
  * @brief  Draw a rectangle of Width and Height with selected color.
  *         The upper left corner is located at (X,Y).
  * @retval none
 */
void LCD_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color)
{
    ILI9341_Draw_Rectangle(X, Y, Width, Height, Color);
}

/**
  * @brief  Fill whole screen with selected color.
  * @retval none
 */
void LCD_FillScreen(uint16_t Color)
{
    ILI9341_FillScreen(Color);
}

/**
  * @brief  Draw a line from (X1,Y1) to (X2,Y2) using Bresenham's algorithm.
  * @retval none
 */
void LCD_Draw_Line(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color)
{
    uint16_t deltaX,deltaY,X,Y;
    int8_t steep,downwards; 
    int16_t erfD;
    uint8_t optimizationCase = 0; //Must initialize. Used for later judgement.
    
    /* Handle Endpoint Relative Position */
    if(X1>X2){LCD_FUNC_SwapU16(&X1,&X2);LCD_FUNC_SwapU16(&Y1,&Y2);}//Make sure line is drawn from left to right

    /* Calculate Line Dimentions */
    deltaX = X2 - X1;
    deltaY = (Y2 < Y1) ? Y1 - Y2 : Y2 - Y1;

    /* Optimization for Special Cases */
    optimizationCase = (X1==X2) + ((Y1==Y2)<<1);
    if(optimizationCase)
    {
        switch(optimizationCase)
        {
            case 3: ILI9341_Draw_Pixel(X1,Y1,Color); return;
            case 2: ILI9341_Draw_Horizontal_Line(X1,Y1,deltaX+1,Color); return;
            case 1: ILI9341_Draw_Vertical_Line(X1,(Y2<Y1)?Y2:Y1,deltaY+1,Color); return;
            default: return;
        }
    }

    /* Check Slope*/
    downwards = (Y2 < Y1); //Check if slope is downwards
    
    steep = (deltaY > deltaX);
    //If absolute value of slope is larger than 1, then calculation is 
    //reflected across line y=x. Thus, X and Y are swapped.
    if(steep)
    {
        LCD_FUNC_SwapU16(&deltaX,&deltaY);
        LCD_FUNC_SwapU16(&X1,&Y1);
        LCD_FUNC_SwapU16(&X2,&Y2);
        if(downwards){LCD_FUNC_SwapU16(&X1,&X2);LCD_FUNC_SwapU16(&Y1,&Y2);}
    }

    

    /*Initialize Error Factor erfD */
    erfD = deltaX / 2;

    /* Do Line Drawing */
    Y = Y1;
    for(X = X1; X <= X2; X++)
    {
        if(steep){ILI9341_Draw_Pixel(Y,X,Color);}else{ILI9341_Draw_Pixel(X,Y,Color);}
        erfD = erfD - deltaY;
        if(erfD < 0)
        {
            if(downwards){Y--;}else{Y++;}
            erfD = erfD + deltaX;
        }
    }
}

/**
  * @brief  Draw a hollow box with upper left corner at (X1,Y1) and
  *         lower right corner at (X2,Y2) and selected color.
  * @retval none
 */
void LCD_Draw_Box_Hollow(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color)
{
    if(X1 > X2){LCD_FUNC_SwapU16(&X1,&X2);}
    if(Y1 > Y2){LCD_FUNC_SwapU16(&Y1,&Y2);}
    ILI9341_Draw_Horizontal_Line(X1,Y1,X2 - X1,Color);
    ILI9341_Draw_Horizontal_Line(X1,Y2,X2 - X1,Color);
    ILI9341_Draw_Vertical_Line(X1,Y1,Y2 - Y1,Color);
    ILI9341_Draw_Vertical_Line(X2,Y1,Y2 - Y1,Color);
}

/**
  * @brief  Draw a filled box with upper left corner at (X1,Y1) and
  *         lower right corner at (X2,Y2) and selected color.
  * @retval none
 */
void LCD_Draw_Box_Filled(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t Color)
{
    if(X1 > X2){LCD_FUNC_SwapU16(&X1,&X2);}
    if(Y1 > Y2){LCD_FUNC_SwapU16(&Y1,&Y2);}
    ILI9341_Draw_Rectangle(X1,Y1,X2 - X1,Y2 - Y1,Color);
}

/**
  * @brief  Draw a hollow circle at (XCenter,YCenter) with specified Radius
  *         using Midpoint circle algorithm.
  * @retval none
 */
void LCD_Draw_Circle_Hollow(uint16_t XCenter, uint16_t YCenter, uint16_t Radius, uint16_t Color)
{
    uint16_t X,Y;
    int16_t erfD;
    X = 0;
    Y = Radius;
    erfD = 3 - Radius*2;
    while(X<Y)
    {
        ILI9341_Draw_Pixel(XCenter + X, YCenter + Y, Color);
        ILI9341_Draw_Pixel(XCenter + X, YCenter - Y, Color);
        ILI9341_Draw_Pixel(XCenter - X, YCenter + Y, Color);
        ILI9341_Draw_Pixel(XCenter - X, YCenter - Y, Color);
        ILI9341_Draw_Pixel(XCenter + Y, YCenter + X, Color);
        ILI9341_Draw_Pixel(XCenter + Y, YCenter - X, Color);
        ILI9341_Draw_Pixel(XCenter - Y, YCenter + X, Color);
        ILI9341_Draw_Pixel(XCenter - Y, YCenter - X, Color);
        if(erfD < 0)
        {
            erfD = erfD + 4*X + 6;
        }
        else
        {
            erfD = erfD + 4*(X-Y) + 6;
            Y--;
        }
        X++;
    }
    if(X==Y)
    {
        ILI9341_Draw_Pixel(XCenter + X, YCenter + Y, Color);
        ILI9341_Draw_Pixel(XCenter + X, YCenter - Y, Color);
        ILI9341_Draw_Pixel(XCenter - X, YCenter + Y, Color);
        ILI9341_Draw_Pixel(XCenter - X, YCenter - Y, Color);
        ILI9341_Draw_Pixel(XCenter + Y, YCenter + X, Color);
        ILI9341_Draw_Pixel(XCenter + Y, YCenter - X, Color);
        ILI9341_Draw_Pixel(XCenter - Y, YCenter + X, Color);
        ILI9341_Draw_Pixel(XCenter - Y, YCenter - X, Color); 
    }
}

/**
  * @brief  Draw a filled circle at (XCenter,YCenter) with specified Radius
  *         using Midpoint circle algorithm.
  * @retval none
 */
void LCD_Draw_Circle_Filled(uint16_t XCenter, uint16_t YCenter, uint16_t Radius, uint16_t Color)
{
    uint16_t X,Y;
    int16_t erfD;
    X = 0;
    Y = Radius;
    erfD = 3 - Radius*2;
    while(X<Y)
    {
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter + Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter - Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter + X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter - X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - X, YCenter + Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - X, YCenter - Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - Y, YCenter + X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - Y, YCenter - X, Y+1,  Color);
        if(erfD < 0)
        {
            erfD = erfD + 4*X + 6;
        }
        else
        {
            erfD = erfD + 4*(X-Y) + 6;
            Y--;
        }
        X++;
    }
    if(X==Y)
    {
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter + Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter - Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter + X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter, YCenter - X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - X, YCenter + Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - X, YCenter - Y, X+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - Y, YCenter + X, Y+1,  Color);
        ILI9341_Draw_Horizontal_Line(XCenter - Y, YCenter - X, Y+1,  Color);
    }
}

/**
  * @brief  Draw a horizontal arrow with starting point (XOri,YOri)
  *         and specified length, arrowhead size and color.
  * @retval none
 */
void LCD_Draw_Horizontal_Arrow(uint16_t XOri, uint16_t YOri, int16_t Length, uint8_t ArrowSize, uint16_t Color)
{
    uint8_t Direction,i;
    Direction = (Length >= 0); //Rightwards
    Length = (Direction) ? Length : -Length;
    if(Direction)//Rightwards
    {
        ILI9341_Draw_Horizontal_Line(XOri,YOri,Length,Color);
        for(i = 1; i <= ArrowSize; i++)
        {
            ILI9341_Draw_Pixel(XOri + Length - i,YOri - i,Color);
            ILI9341_Draw_Pixel(XOri + Length - i,YOri + i,Color);
        }
    } 
    else//Leftwards
    {
        ILI9341_Draw_Horizontal_Line(XOri-Length,YOri,Length,Color);
        for(i = 1; i <= ArrowSize; i++)
        {
            ILI9341_Draw_Pixel(XOri - Length + i,YOri - i,Color);
            ILI9341_Draw_Pixel(XOri - Length + i,YOri + i,Color);
        }
    }
}

/**
  * @brief  Draw a vertical arrow with starting point (XOri,YOri)
  *         and specified length, arrowhead size and color.
  * @retval none
 */
void LCD_Draw_Vertical_Arrow(uint16_t XOri, uint16_t YOri, int16_t Length, uint8_t ArrowSize, uint16_t Color)
{
    uint8_t Direction,i;
    Direction = (Length >= 0); //Rightwards
    Length = (Direction) ? Length : -Length;
    if(Direction)//Upwards
    {
        ILI9341_Draw_Vertical_Line(XOri,YOri-Length,Length,Color);
        for(i = 1; i <= ArrowSize; i++)
        {
            ILI9341_Draw_Pixel(XOri - i, YOri - Length + i, Color);
            ILI9341_Draw_Pixel(XOri + i, YOri - Length + i, Color);
        }
    }
    else//Downwards
    {
        ILI9341_Draw_Vertical_Line(XOri,YOri,Length,Color);
        for(i = 1; i <= ArrowSize; i++)
        {
            ILI9341_Draw_Pixel(XOri - i, YOri + Length - i, Color);
            ILI9341_Draw_Pixel(XOri + i, YOri + Length - i, Color);
        }
    }
}

/**
  * @brief  Draw a character at (X,Y) with selected color and TRANSPARENT background.
  * @param  X Horizontal location of chatacter.
  * @param  Y Vertical location of chatacter.
  * @param  Color Color of character.
  * @param  Size Character will be xSize larger than font data. (e.g. Font is 5x5
  *              pixel, Size is 2, and the character drawn will take up 10x10 pixel)
  * @param  Font Name of font used. See LCDFONT.h for font data.
  * @param  Char Character to draw
  * @retval none
 */
void LCD_Draw_Char(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint8_t Char)
{
    if(Size==0){return;}
    uint8_t i,j;
    Char = (Char<=' ') ? 0 : Char - 32;
    uint8_t GlyphData[MAX_CHARHEIGHT] = {0};
    for(i = 0; i<font_charHeight[Font]; i++)
    {
        switch(Font)
        {
            case ASCII5x5: GlyphData[i] = font_ASCII5x5[Char][i];break;
            case GRAZIA: GlyphData[i] = font_GRAZIA[Char][i];break;
            case ASCII5x7: GlyphData[i] = font_ASCII5x7[Char][i];break;
            case ASCII16: GlyphData[i] = font_ASCII16[Char][i];break;
            default: GlyphData[i] = 0x15;
        }
    }
    for(i=0; i<font_charHeight[Font]; i++)
    {
        for(j=0; j<font_charWidth[Font]; j++)
        {
            if(GlyphData[i] & (0x01<<j))
            {
                if(Size == 1){ILI9341_Draw_Pixel(X+j,Y+i,Color);}
                else{ILI9341_Draw_Rectangle(X+(j*Size),Y+(i*Size),Size,Size,Color);}
            }
        }
    }
}

/**
  * @brief  Display a line of text at specified location with selected color,
  *         size, font and TRANSPARENT background.
  * @retval none
 */
void LCD_Disp_Text(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, const char* Text)
{
    while(*Text)
    {
        LCD_Draw_Char(X, Y, Color, Size, Font, *Text++);
        X = X + font_charWidth[Font]*Size;
    }
}

/**
  * @brief  Display the upper digits of an integer at specified location 
  *         with selected color, size, font and TRANSPARENT background.
  *         e.g. LCD_Disp_Num(X,Y,Color,Size,Font,4321,2) will display 43
  * @retval none
 */
void LCD_Disp_NumUpp(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits)
{
    uint16_t Digit_Whole_Num = 0;
    uint32_t Temp_Num = Num / 10;
    while(Temp_Num)
    {
        Digit_Whole_Num++;
        Temp_Num = Temp_Num / 10;
    }
    for(uint16_t i = 0; i<Digits; i++)
    {
        LCD_Draw_Char(X, Y, Color, Size, Font, (Num/(LCD_FUNC_Power(10,Digit_Whole_Num--)))%10 + 48);
        X = X + font_charWidth[Font]*Size;
    }
}

/**
  * @brief  Display the lower digits of an integer at specified location 
  *         with selected color, size, font and TRANSPARENT background.
  *         e.g. LCD_Disp_Num(X,Y,Color,Size,Font,4321,2) will display 21
  * @retval none
 */
void LCD_Disp_NumLow(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits)
{
    for(uint16_t i = 0; i<Digits; i++)
    {
        LCD_Draw_Char(X, Y, Color, Size, Font, (Num/(LCD_FUNC_Power(10,Digits-i-1)))%10 + 48);
        X = X + font_charWidth[Font]*Size;
    }
}

/**
  * @brief  Display several digits of an integer at specified location 
  *         with selected color, size, font and TRANSPARENT background.
  *         To display upper or lower digits is specified by Type.
  * @retval none
 */
void LCD_Disp_Num(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, uint32_t Num, uint16_t Digits, uint8_t Type)
{
    if(Type == LCD_NUM_UPPERDIGITS){LCD_Disp_NumUpp(X,Y,Color,Size,Font,Num,Digits);}
    else if(Type == LCD_NUM_LOWERDIGITS){LCD_Disp_NumLow(X,Y,Color,Size,Font,Num,Digits);}
    else{LCD_Disp_Text(X,Y,Color,Size,Font,"!ARG_Type");}
}

/**
  * @brief  Display digits near decimal point of a decimal at specified location 
  *         with selected color, size, font and TRANSPARENT background.
  *         e.g. LCD_Disp_Decimal(X,Y,C,S,F,4321.1234,2,3) will display 21.123
  * @retval none
 */
void LCD_Disp_Decimal(uint16_t X, uint16_t Y, uint16_t Color, uint16_t Size, FONT_NAME Font, double Num, uint16_t intDigits, uint16_t deciDigits)
{
    uint32_t integerPart,decimalPart = 0;
    integerPart = (int)Num;
    decimalPart = (int)((Num-integerPart)*LCD_FUNC_Power(10,deciDigits));
    if((int)(Num*LCD_FUNC_Power(10,deciDigits+1))%10>=5){decimalPart++;}
    integerPart = integerPart % LCD_FUNC_Power(10,intDigits);
    LCD_Disp_NumLow(X, Y, Color, Size, Font, integerPart, intDigits);
    LCD_Draw_Char(X+font_charWidth[Font]*Size*intDigits, Y, Color, Size, Font, '.');
    LCD_Disp_NumLow(X+font_charWidth[Font]*Size*(intDigits+1), Y, Color, Size, Font, decimalPart, deciDigits);
}

/**
  * @brief  Display axis of one quardrant with arrow ends and TRANSPRANT
  *         background.
  * @retval none
 */
void LCD_Disp_Axis_Quadrant(int16_t XOri, int16_t YOri, int16_t XRange, int16_t YRange, uint8_t ArrowSize, uint16_t Color)
{
    LCD_Draw_Horizontal_Arrow(XOri,YOri,XRange,ArrowSize,Color);
    LCD_Draw_Vertical_Arrow(XOri,YOri,YRange,ArrowSize,Color);
}


/**
  * @brief  Calculates a^n. This is an auxillary mathematical function. 
  * @retval uint32_t a^n.
 */
uint32_t LCD_FUNC_Power(uint32_t a, uint32_t n)
{
    uint32_t res = 1;
    for(uint32_t i = 0; i<n; i++)
    {
        res = res*a;
    }
    return res;
}

/**
  * @brief  Swap imputs. This is an auxillary function. 
  * @retval none
 */
void LCD_FUNC_SwapU16(uint16_t *X, uint16_t *Y)
{
    uint16_t Temp = *X;
    *X = *Y;
    *Y = Temp;
}

/**
  * @brief  Display a program-built-in dotgrid. Data is MSB on the left.
  *         This function does NOT have array out-of-bounds access prevention.
  * @retval none
 */
void LCD_Disp_DotGrid(uint16_t X, uint16_t Y, const char* DotGrid, uint16_t Width_Bytes, uint16_t Height, uint16_t Color)
{
    uint16_t i,j,b;
    for(j = 0; j < Height; j++)
    {
        for(i = 0; i < Width_Bytes; i++)
        {
            for(b = 0; b < 8; b++)
            {
                if((DotGrid[i+j*Width_Bytes]&(0x80>>b))!=0){ILI9341_Draw_Pixel(X+b+i*8,Y+j,Color);}
            }
        }
    }
}

/* by HSSE@xjtu */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
