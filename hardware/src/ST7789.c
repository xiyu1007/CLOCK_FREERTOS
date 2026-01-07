#include "ST7789.h"

static DMA_InitTypeDef dma;
static void st7789_spi_set_8bit(void);
static void st7789_spi_set_16bit(void);
static void ST7789_DMA_Init(void);
static void ST7789_DMA_TX(const uint16_t *data, uint32_t len_bytes, bool single);
static void ST7789_Write_Register(uint8_t reg, const uint8_t* data, uint16_t len);
static inline bool ST7789_IN_Screen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static void ST7789_Set_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static void ST7789_Set_Pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
					   const uint8_t* model, uint16_t color, uint16_t bg_color);
static void ST7789_Show_ASCII(uint16_t x, uint16_t y, char ch, uint16_t color,
							  uint16_t bg_color, const font_t* font);
static void ST7789_Show_Chinese(uint16_t x, uint16_t y, char* ch, uint16_t color,
                         uint16_t bg_color, const font_t* font);
static inline bool IS_GB2312(char ch);

/* 外部接口 */
void ST7789_Init(void)
{
	ST7789_DMA_Init();
	ST7789_Reset();

	// 退出休眠模式（SLPOUT）
	ST7789_Write_Register(0x11, NULL, 0);
	u_delay_ms(5);
	// 内存访问控制（MADCTL）
	// 0x00 = RGB顺序，正常扫描方向
	ST7789_Write_Register(0x36, (uint8_t[]){ 0x00 }, 1);
	// 像素格式设置（COLMOD）
	// 0x55 = 16-bit RGB565
	ST7789_Write_Register(0x3A, (uint8_t[]){ 0x55 }, 1);
	// 功能控制 B（Frame Rate & Porch Control）
	// 0xB7 = 设置行刷新周期、门控电压等
	ST7789_Write_Register(0xB7, (uint8_t[]){ 0x46 }, 1);
	// 功能控制 C（VCOM & VCOMH Voltage）
	// 0xBB = 设置VCOMH电压
	ST7789_Write_Register(0xBB, (uint8_t[]){ 0x1B }, 1);
	// 功能控制 D（VGH / VGL设置）
	// 0xC0 = VGH/VGL电压设置
	ST7789_Write_Register(0xC0, (uint8_t[]){ 0x2C }, 1);
	// 功能控制 2（电源控制）
	// 0xC2 = 电源控制，设置电流等
	ST7789_Write_Register(0xC2, (uint8_t[]){ 0x01 }, 1);
	// 功能控制 4（驱动能力）
	// 0xC4 = 驱动电流/功率控制
	ST7789_Write_Register(0xC4, (uint8_t[]){ 0x20 }, 1);
	// 功能控制 6（源驱动电流）
	// 0xC6 = 源驱动能力配置
	ST7789_Write_Register(0xC6, (uint8_t[]){ 0x0F }, 1);
	// D0: 电源控制相关设置
	ST7789_Write_Register(0xD0, (uint8_t[]){ 0xA4, 0xA1 }, 2);
	// D6: 功能控制，模组内部寄存器配置
	ST7789_Write_Register(0xD6, (uint8_t[]){ 0xA1 }, 1);
	// E0: 正极伽马校正
	ST7789_Write_Register(0xE0,
	                      (uint8_t[]){ 0xF0, 0x00, 0x06, 0x04, 0x05, 0x05, 0x31,
	                                   0x44, 0x48, 0x36, 0x12, 0x12, 0x2B,
	                                   0x34 },
	                      14);
	// E0: 正极伽马校正（第二组值）
	ST7789_Write_Register(0xE0,
	                      (uint8_t[]){ 0xF0, 0x0B, 0x0F, 0x0F, 0x0D, 0x26, 0x31,
	                                   0x43, 0x47, 0x38, 0x14, 0x14, 0x2C,
	                                   0x32 },
	                      14);
	// 颜色反相控制
	// ST7789_Write_Register(0x21, NULL, 0); // INVON(开启颜色反相)
	// INVOFF（关闭颜色反相，正常颜色显示）
	ST7789_Write_Register(0x20, NULL, 0);
	// 打开显示
	ST7789_Write_Register(0x29, NULL, 0);

//	ST7789_Fill_Color(0, 0, SCREEN.width - 1, SCREEN.height - 1, BACKGROUND);
}


static void ST7789_DMA_Init(void)
{
    RCC_AHB1PeriphClockCmd(SCREEN.spic.DMA_RCC, ENABLE);

    dma.DMA_Channel            = SCREEN.spic.DMA_Channel;
    dma.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_High;
    dma.DMA_PeripheralBaseAddr = (uint32_t)&SCREEN.spic.spix->DR;

    DMA_Init(SCREEN.spic.DMA_Stream, &dma);
    SPI_DMACmd(SCREEN.spic.spix, SPI_I2S_DMAReq_Tx, ENABLE);
}

void ST7789_Show(uint16_t x, uint16_t y, char* str, uint16_t color,
                 uint16_t bg_color, const font_t* font)
{
	while (*str)
	{
		// int len = utf8_char_length(*str);
		int len = IS_GB2312(*str) ? 2 : 1;
		if (len <= 0)
		{
			str++;
			continue;
		}
		else if (len == 1)
		{
			ST7789_Show_ASCII(x, y, *str, color, bg_color, font);
			str++;
			x += font->size / 2;
		}
		else
		{
			char ch[5];
			strncpy(ch, str, len);
			ST7789_Show_Chinese(x, y, ch, color, bg_color, font);
			str += len;
			x   += font->size;
		}
	}
}

void ST7789_Draw_Image(uint16_t x, uint16_t y, const image_t* image)
{
	if (!ST7789_IN_Screen(x, y, x + image->width - 1, y + image->height - 1))
		return;

	ST7789_Set_Range(x, y, x + image->width - 1, y + image->height - 1);

	uint32_t       size = image->width * image->height;  // 每个像素2字节
	const uint16_t* data = (const uint16_t *)image->data;

	ST7789_DMA_TX(data, size, false);
}

// 单色填充
void ST7789_Fill_Color(uint16_t x1, uint16_t y1,uint16_t x2, uint16_t y2, uint16_t color)
{
    if (!ST7789_IN_Screen(x1, y1, x2, y2))
        return;

    ST7789_Set_Range(x1, y1, x2, y2);
    uint32_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);

    ST7789_DMA_TX(&color, pixels, true);
}

void ST7789_Reset(void)
{
	ST7789_RST_L;
	ST7789_TRW;
	ST7789_RST_H;
	ST7789_TRT;
}

/* 内部接口 */
static inline void st7789_spi_set_8bit(void)
{
    SPI_Cmd(SCREEN.spic.spix, DISABLE);
    SPI_DataSizeConfig(SCREEN.spic.spix, SPI_DataSize_8b);
    SPI_Cmd(SCREEN.spic.spix, ENABLE);
}

static inline void st7789_spi_set_16bit(void)
{
    SPI_Cmd(SCREEN.spic.spix, DISABLE);
    SPI_DataSizeConfig(SCREEN.spic.spix, SPI_DataSize_16b);
    SPI_Cmd(SCREEN.spic.spix, ENABLE);
}

static void ST7789_DMA_TX(const uint16_t *data, uint32_t len, bool single)
{
    st7789_spi_set_16bit();

    ST7789_CS_L;
    ST7789_DC_H;
    while (len)
    {
        uint32_t chunk = len > 65535 ? 65535 : len;

        DMA2_Stream3->M0AR = (uint32_t)data;
        DMA2_Stream3->NDTR = chunk;

        if (single)
            DMA2_Stream3->CR &= ~DMA_SxCR_MINC;
        else
            DMA2_Stream3->CR |= DMA_SxCR_MINC;

        DMA_Cmd(DMA2_Stream3, ENABLE);
        while (!DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3));
        DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);

        len -= chunk;
        if (!single) data += chunk;
    }

    while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY));
    ST7789_CS_H;
}

static void ST7789_Write_Register(uint8_t reg, const uint8_t* data, uint16_t len)
{
	st7789_spi_set_8bit();
	
	ST7789_CS_L;
	ST7789_DC_L;  // 发送命令，而不是数据

	SPI_SendData(SCREEN.spic.spix, reg);

	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) == SET);

	if (len > 0)
	{
		ST7789_DC_H;  // 发送像素数据
		for (uint16_t i = 0; i < len; i++)
		{
			SPI_SendData(SCREEN.spic.spix, data[i]);
			while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_TXE) == RESET);
		}
		// 等待 BSY 清零
		while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) == SET);
	}
	ST7789_CS_H;
}

static inline bool ST7789_IN_Screen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (x2 >= WIDTH || y2 >= HEIGHT || x1 > x2 || y1 > y2)
        return 0;
	return 1;
}

static void ST7789_Set_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	// 0x2A : 列地址设置(Column Address Set)
	// 设置显示区域的起始列和结束列(x方向范围)
	//   高字: (x1), 低字: (x1), 高字: (x2), 低字: (x2)
	uint8_t buf[4];
    buf[0] = x1 >> 8; buf[1] = x1;
    buf[2] = x2 >> 8; buf[3] = x2;
    ST7789_Write_Register(0x2A, buf, 4);

	// 0x2B : 行地址设置 (Row Address Set)
	// 设置显示区域的起始行和结束行(y方向范围)?
	//   高字: (y1), 低字: (y1), 高字: (y2), 低字: (y2)
    buf[0] = y1 >> 8; buf[1] = y1;
    buf[2] = y2 >> 8; buf[3] = y2;
    ST7789_Write_Register(0x2B, buf, 4);
	// 0x2C : 内存写入命令(Memory Write)
	// 表示接下来写入的连续数据会填充刚才设置的显示区域
    ST7789_Write_Register(0x2C, NULL, 0);
	
}

static void ST7789_Set_Pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       const uint8_t* model, uint16_t color, uint16_t bg_color)
{
	uint16_t bytes_per_row    = (width + 7) / 8;
	uint8_t  color_data[2]    = { (color >> 8) & 0xff, color & 0xff };
	uint8_t  bg_color_data[2] = { (bg_color >> 8) & 0xff, bg_color & 0xff };

	static uint8_t pixel_data[48 * 48 * 2];
	uint8_t*       pixel_ptr = pixel_data;

	ST7789_Set_Range(x, y, x + width - 1, y + height - 1);

	for (uint16_t row = 0; row < height; row++)
	{
		const uint8_t* row_data = model + row * bytes_per_row;
		for (uint16_t col = 0; col < width; col++)
		{
			uint8_t pixel = row_data[col / 8] & (1 << (7 - col % 8));
			if (pixel)
			{
				*pixel_ptr++ = color_data[1];
				*pixel_ptr++ = color_data[0];
			}
			else
			{
				*pixel_ptr++ = bg_color_data[1];
				*pixel_ptr++ = bg_color_data[0];
			}
		}
	}

	ST7789_DMA_TX((const uint16_t *)pixel_data, (pixel_ptr - pixel_data) >> 1, false);
}

static void ST7789_Show_ASCII(uint16_t x, uint16_t y, char ch, uint16_t color,
                       uint16_t bg_color, const font_t* font)
{
	if (font == NULL)
		return;

	uint16_t fheight = font->size, fwidth = font->size / 2;
	if (!ST7789_IN_Screen(x, y, x + fwidth - 1, y + fheight - 1))
		return;

	if (ch < 0x20 || ch > 0x7E)
		return;

	uint16_t bytes_per_row = (fwidth + 7) / 8;
	const uint8_t* model = font->ascii_model + (ch - ' ') * fheight * bytes_per_row;

	ST7789_Set_Pixels(x, y, fwidth, fheight, model, color, bg_color);
}

static void ST7789_Show_Chinese(uint16_t x, uint16_t y, char* ch, uint16_t color,
                         uint16_t bg_color, const font_t* font)
{
	if (ch == NULL || font == NULL)
		return;

	uint16_t fheight = font->size, fwidth = font->size;
	if (!ST7789_IN_Screen(x, y, x + fwidth - 1, y + fheight - 1))
		return;

	const font_chinese_t* c        = font->chinese;
	const font_chinese_t* fallback = c;

	for (; c->name != NULL; c++)
	{
		if (strcmp(c->name, ch) == 0)
			break;
		// 记录"_END_"作为回退
		if (strcmp(c->name, "_END_C") == 0)
			fallback = c;
	}
	if (c->name == NULL)
		c = fallback;

	ST7789_Set_Pixels(x, y, fwidth, fheight, c->model, color, bg_color);
}

static inline bool IS_GB2312(char ch)
{
	return ((unsigned char)ch >= 0xA1 && (unsigned char)ch <= 0xF7);
}


