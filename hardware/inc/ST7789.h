#ifndef __ST7789_H__
#define __ST7789_H__

#include "TFT_LCD.h"
#include "font.h"
#include "image.h"
#include "u_delay.h"
#include "u_log.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ST7789_LOG(fmt, ...) LOG("[ST7789] " fmt "\r\n", ##__VA_ARGS__)

#define SCREEN               g_TFT_LCD
#define WIDTH                SCREEN.width
#define HEIGHT               SCREEN.height

/* RGB565 */
#define MKCOLOR(r, g, b)     (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

/* 基本颜色 */
#define BLACK                MKCOLOR(0, 0, 0)
#define WHITE                MKCOLOR(255, 255, 255)
#define RED                  MKCOLOR(255, 0, 0)
#define GREEN                MKCOLOR(0, 255, 0)
#define BLUE                 MKCOLOR(0, 0, 255)
#define BACKGROUND           BLACK
#define TRANSPARENT          BACKGROUND  // 透明

/* 组合色 */
#define YELLOW               MKCOLOR(255, 255, 0)
#define CYAN                 MKCOLOR(0, 255, 255)
#define MAGENTA              MKCOLOR(255, 0, 255)
/* 扩展常用色 */
#define ORANGE               MKCOLOR(255, 165, 0)
#define PURPLE               MKCOLOR(128, 0, 128)
#define PINK                 MKCOLOR(255, 192, 203)
#define BROWN                MKCOLOR(165, 42, 42)
/* 灰度 */
#define GRAY                 MKCOLOR(128, 128, 128)
#define DARK_GRAY            MKCOLOR(64, 64, 64)
#define LIGHT_GRAY           MKCOLOR(192, 192, 192)
/* 显示调试常用 */
#define NAVY                 MKCOLOR(0, 0, 128)
#define TEAL                 MKCOLOR(0, 128, 128)
#define OLIVE                MKCOLOR(128, 128, 0)
#define DARK_ORANGE          MKCOLOR(255, 140, 0)

#define ST7789_TRW           u_delay_us(20)
#define ST7789_TRT           u_delay_ms(120)
#define ST7789_CS_L          GPIO_ResetBits(SCREEN.spic.port, SCREEN.spic.cs)
#define ST7789_CS_H          GPIO_SetBits(SCREEN.spic.port, SCREEN.spic.cs)
#define ST7789_DC_L          GPIO_ResetBits(SCREEN.spic.port, SCREEN.dc)
#define ST7789_DC_H          GPIO_SetBits(SCREEN.spic.port, SCREEN.dc)
#define ST7789_RST_L         GPIO_ResetBits(SCREEN.spic.port, SCREEN.rst)
#define ST7789_RST_H         GPIO_SetBits(SCREEN.spic.port, SCREEN.rst)

/* API */
void ST7789_Init(void);
void ST7789_Reset(void);
void ST7789_Fill_Color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void ST7789_Show(uint16_t x, uint16_t y, char* str, uint16_t color, uint16_t bg, const font_t* font);
void ST7789_Draw_Image(uint16_t x, uint16_t y, const image_t* image);

#endif /* __ST7789_H__ */
