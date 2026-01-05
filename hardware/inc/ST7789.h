#ifndef __ST7789_H__
#define __ST7789_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "TFT_LCD.h"
#include "font.h"
#include "image.h"
#include "u_delay.h"

#define ST7789_LOG(fmt, ...) printf("[ST7789] FILE: %s LINE: %d\r\n" fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define SCREEN           g_TFT_LCD
#define WIDTH            SCREEN.width
#define HEIGHT           SCREEN.height

// RGB565（16 bit）
#define mkcolor(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

/* 基本颜色 */
#define BLACK            mkcolor(0, 0, 0)
#define WHITE            mkcolor(255, 255, 255)
#define RED              mkcolor(255, 0, 0)
#define GREEN            mkcolor(0, 255, 0)
#define BLUE             mkcolor(0, 0, 255)
#define BACKGROUND       BLACK
#define TRANSPARENT      BACKGROUND  // 透明

/* 组合色 */
#define YELLOW           mkcolor(255, 255, 0)
#define CYAN             mkcolor(0, 255, 255)
#define MAGENTA          mkcolor(255, 0, 255)
/* 扩展常用色 */
#define ORANGE           mkcolor(255, 165, 0)
#define PURPLE           mkcolor(128, 0, 128)
#define PINK             mkcolor(255, 192, 203)
#define BROWN            mkcolor(165, 42, 42)
/* 灰度 */
#define GRAY             mkcolor(128, 128, 128)
#define DARK_GRAY        mkcolor(64, 64, 64)
#define LIGHT_GRAY       mkcolor(192, 192, 192)
/* 显示调试常用 */
#define NAVY             mkcolor(0, 0, 128)
#define TEAL             mkcolor(0, 128, 128)
#define OLIVE            mkcolor(128, 128, 0)
#define DARK_ORANGE      mkcolor(255, 140, 0)

#define ST7789_TRW       u_delay_us(20)
#define ST7789_TRT       u_delay_ms(120)
#define ST7789_CS_L      GPIO_ResetBits(SCREEN.spic.port, SCREEN.spic.cs)
#define ST7789_CS_H      GPIO_SetBits(SCREEN.spic.port, SCREEN.spic.cs)
#define ST7789_DC_L      GPIO_ResetBits(SCREEN.spic.port, SCREEN.dc)
#define ST7789_DC_H      GPIO_SetBits(SCREEN.spic.port, SCREEN.dc)
#define ST7789_RST_L     GPIO_ResetBits(SCREEN.spic.port, SCREEN.rst)
#define ST7789_RST_H     GPIO_SetBits(SCREEN.spic.port, SCREEN.rst)

// Instruction
#define SLPOUT           0x11
#define SLPOUT_T         u_delay_ms(5)
/* MADCTL command */
#define MADCTL           0x36
/* Address order control */
#define MADCTL_MY        0x80 /* D7: Y-axis flip */
#define MADCTL_MX        0x40 /* D6: X-axis flip */
#define MADCTL_MV        0x20 /* D5: X/Y swap */
#define MADCTL_ML        0x10 /* D4: Vertical refresh order */
#define MADCTL_MH        0x04 /* D2: Horizontal refresh order */
/* Color order */
#define MADCTL_RGB       0x00 /* D3 = 0: RGB */
#define MADCTL_BGR       0x08 /* D3 = 1: BGR */

void ST7789_Set_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
bool ST7789_IN_Screen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_Reset(void);
void ST7789_Write_Register(uint8_t reg, const uint8_t* data, uint16_t len);
void ST7789_Init(void);
bool ST7789_IN_Screen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_Ajust_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_Set_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_Fill_Color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                       uint16_t color);
void ST7789_Set_Pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       const uint8_t* model, uint16_t color, uint16_t bg_color);
void ST7789_Show_ASCII(uint16_t x, uint16_t y, char ch, uint16_t color,
                       uint16_t bg_color, const font_t* font);
void ST7789_Show_Chinese(uint16_t x, uint16_t y, char* ch, uint16_t color,
                         uint16_t bg_color, const font_t* font);
bool IS_GB2312(char ch);
void ST7789_Show(uint16_t x, uint16_t y, char* str, uint16_t color,
                 uint16_t bg_color, const font_t* font);
void ST7789_Draw_Image(uint16_t x, uint16_t y, const image_t* image);
void ST7789_DMA_TX(const uint8_t* data, uint32_t len_bytes, bool is_single);

#endif /* __ST7789_H__ */
