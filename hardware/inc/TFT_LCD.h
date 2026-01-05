#ifndef __TFT_LCD_H__
#define __TFT_LCD_H__

#include "stm32f4xx.h"
#include "u_spi.h"
#include "u_bsp_rcc.h"

#define TFT_LCD_Width       240
#define TFT_LCD_Height      320
#define TFT_LCD_RST         GPIO_Pin_2
#define TFT_LCD_DC          GPIO_Pin_3
#define TFT_LCD_CS          GPIO_Pin_4
#define TFT_LCD_PORT        GPIOA
#define TFT_LCD_SPIx        SPI1
#define TFT_LCD_SCK         GPIO_Pin_5
#define TFT_LCD_MISO        GPIO_Pin_6
#define TFT_LCD_MOSI        GPIO_Pin_7

typedef struct
{
    uint16_t rst;
    uint16_t dc;
    SPIConfig spic;
    uint16_t width;
    uint16_t height;
} TFT_LCD_InitTypeDef;

void TFT_LCD_Init(SPIConfig *spic);

extern TFT_LCD_InitTypeDef g_TFT_LCD;

#endif /* __TFT_LCD_H__ */
