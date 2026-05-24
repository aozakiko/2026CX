#ifndef _tftlcd_H
#define _tftlcd_H

#include "system.h"

/* Default driver used by the added FSMC TFTLCD reference project. */
#define TFTLCD_HX8357DN

/* Camera frames are 2:1, so landscape gives a larger preview. Set to 0 for portrait. */
#define TFTLCD_DIR 1

#define LCD_LED PBout(0)

typedef struct
{
	volatile u16 LCD_CMD;
	volatile u16 LCD_DATA;
} TFTLCD_TypeDef;

/* Bank1_NORSRAM4, NE4, A10 as RS. */
#define TFTLCD_BASE ((u32)(0x6C000000 | 0x000007FE))
#define TFTLCD      ((TFTLCD_TypeDef *)TFTLCD_BASE)

typedef struct
{
	u16 width;
	u16 height;
	u16 id;
	u8 dir;
} _tftlcd_data;

extern _tftlcd_data tftlcd_data;
extern u16 FRONT_COLOR;
extern u16 BACK_COLOR;

#define WHITE      0xFFFF
#define BLACK      0x0000
#define BLUE       0x001F
#define BRED       0xF81F
#define GRED       0xFFE0
#define GBLUE      0x07FF
#define RED        0xF800
#define MAGENTA    0xF81F
#define GREEN      0x07E0
#define CYAN       0x7FFF
#define YELLOW     0xFFE0
#define BROWN      0xBC40
#define BRRED      0xFC07
#define GRAY       0x8430
#define DARKBLUE   0x01CF
#define LIGHTBLUE  0x7D7C
#define GRAYBLUE   0x5458
#define LIGHTGREEN 0x841F
#define LIGHTGRAY  0xEF5B
#define LGRAY      0xC618
#define LGRAYBLUE  0xA651
#define LBBLUE     0x2B12

void LCD_WriteCmd(u16 cmd);
void LCD_WriteData(u16 data);
void LCD_WriteCmdData(u16 cmd, u16 data);
void LCD_WriteData_Color(u16 color);
u16 LCD_ReadData(void);

void TFTLCD_Init(void);
void LCD_Display_Dir(u8 dir);
void LCD_Set_Window(u16 sx, u16 sy, u16 ex, u16 ey);
void LCD_Clear(u16 color);
void LCD_Fill(u16 xState, u16 yState, u16 xEnd, u16 yEnd, u16 color);
void LCD_DrawPoint(u16 x, u16 y);
void LCD_DrawFRONT_COLOR(u16 x, u16 y, u16 color);

void TFTLCD_DisplayColorFrame(const u8 *frame);

#endif
