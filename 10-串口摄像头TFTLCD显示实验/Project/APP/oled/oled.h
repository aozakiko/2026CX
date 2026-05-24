#ifndef _oled_H
#define _oled_H

#include "system.h"

#define OLED_WIDTH                 128
#define OLED_HEIGHT                64
#define OLED_PAGE_COUNT            8
#define OLED_FRAME_SIZE            (OLED_WIDTH * OLED_PAGE_COUNT)

#define OLED_I2C_PORT              GPIOB
#define OLED_I2C_RCC               RCC_APB2Periph_GPIOB
#define OLED_I2C_SCL_PIN           GPIO_Pin_6
#define OLED_I2C_SDA_PIN           GPIO_Pin_7

#define OLED_I2C_ADDR              0x78
#define OLED_I2C_ADDR_ALT          0x7A
#define OLED_SEND_TO_BOTH_ADDRS    1
#define OLED_COLUMN_OFFSET         0

void OLED_Init(void);
void OLED_Clear(void);
void OLED_DisplayFrame(const u8 *frame);
void OLED_TestPattern(void);

#endif
