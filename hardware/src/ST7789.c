#include "ST7789.h"

void ST7789_Reset(void)
{
	ST7789_RST_L;
	ST7789_TRW;
	ST7789_RST_H;
	ST7789_TRT;
}

static DMA_InitTypeDef dma_init;

void ST7789_DMA_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	// DMA_DeInit(DMA2_Stream3);
	// DMA_StructInit(&dma_init);
	dma_init.DMA_Channel            = DMA_Channel_3;
	dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
	dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	dma_init.DMA_PeripheralBaseAddr = (uint32_t)&SCREEN.spic.spix->DR;
	// 每次 DMA 对外设寄存器搬运的数据单位大小
	dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
	// dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	// dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	// DMA_NORMAL：搬运一次，完成后停止
	// DMA_CIRCULAR：循环模式，搬运完成自动回到起始地址
	dma_init.DMA_Mode               = DMA_Mode_Normal;
	dma_init.DMA_Priority           = DMA_Priority_High;
	// DMA_FIFOMODE_ENABLE：启用 FIFO 缓冲，优化大数据搬运
	dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
	dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
	dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_INC8;
	dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream3, &dma_init);

	SPI_DMACmd(SCREEN.spic.spix, SPI_I2S_DMAReq_Tx, ENABLE);
}

void ST7789_Write_Register(uint8_t reg, const uint8_t* data, uint16_t len)
{
	SPI_Cmd(SCREEN.spic.spix, DISABLE);
	SPI_DataSizeConfig(SCREEN.spic.spix, SPI_DataSize_8b);
	SPI_Cmd(SCREEN.spic.spix, ENABLE);
	
	ST7789_CS_L;
	ST7789_DC_L;  // 发送命令，而不是数据
	SPI_SendData(SCREEN.spic.spix, reg);

	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_TXE) == RESET)
		;
	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) == SET)
		;

	if (len > 0)
	{
		ST7789_DC_H;  // 发送像素数据
		for (uint16_t i = 0; i < len; i++)
		{
			SPI_SendData(SCREEN.spic.spix, data[i]);
			while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_TXE) == RESET)
				;
		}
		// 等待 BSY 清零
		while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) == SET)
			;
	}
	ST7789_CS_H;
}

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

bool ST7789_IN_Screen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (x1 >= SCREEN.width || y1 >= SCREEN.height)
		return false;
	if (x2 >= SCREEN.width || y2 >= SCREEN.height)
		return false;
	if (x1 > x2 || y1 > y2)
		return false;

	return true;
}

void ST7789_Ajust_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (x1 >= SCREEN.width)
		x1 = x1 % SCREEN.width;
	if (y1 >= SCREEN.height)
		y1 = y1 % SCREEN.height;
	if (x2 >= SCREEN.width)
		x2 = SCREEN.width - 1;
	if (y2 >= SCREEN.height)
		y2 = SCREEN.height - 1;
}

void ST7789_Set_Range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	// 0x2A : 列地址设置(Column Address Set)
	// 设置显示区域的起始列和结束列(x方向范围)
	// 参数顺序?
	//   高字: (x1), 低字: (x1), 高字: (x2), 低字: (x2)
	ST7789_Write_Register(
	  0x2A,
	  (uint8_t[]){ (x1 >> 8) & 0xff, x1 & 0xff, (x2 >> 8) & 0xff, x2 & 0xff },
	  4);
	// 0x2B : 行地址设置 (Row Address Set)
	// 设置显示区域的起始行和结束行(y方向范围)?
	// 参数顺序?
	//   高字: (y1), 低字: (y1), 高字: (y2), 低字: (y2)
	ST7789_Write_Register(
	  0x2B,
	  (uint8_t[]){ (y1 >> 8) & 0xff, y1 & 0xff, (y2 >> 8) & 0xff, y2 & 0xff },
	  4);
	// 0x2C : 内存写入命令(Memory Write)
	// 表示接下来写入的连续数据会填充刚才设置的显示区域
	ST7789_Write_Register(0x2C, NULL, 0);
}

void ST7789_DMA_TX(const uint8_t* data, uint32_t len_bytes, bool is_single)
{

	SPI_Cmd(SCREEN.spic.spix, DISABLE);
	SPI_DataSizeConfig(SCREEN.spic.spix, SPI_DataSize_16b);
	SPI_Cmd(SCREEN.spic.spix, ENABLE);



	// memory write command
	ST7789_CS_L;
	ST7789_DC_H;
	len_bytes >>= 1;
	const uint16_t* pdata = (const uint16_t*)data;

	do
	{
		// 必须在循环内部重新计算 chunk_size，因为 len 会在循环中改变
		uint32_t chunk_size = len_bytes < 65535 ? len_bytes : 65535;

		// DMA_Cmd(DMA2_Stream  3, DISABLE);

		DMA2_Stream3->M0AR = (uint32_t)pdata;
		DMA2_Stream3->NDTR = chunk_size;

		if (is_single)
			DMA2_Stream3->CR &= ~DMA_SxCR_MINC;
		else
			DMA2_Stream3->CR |= DMA_SxCR_MINC;

		DMA_Cmd(DMA2_Stream3, ENABLE);
		while (DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3) == RESET)
			;
		DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);
		// SPI1->DR = *pdata;
		// 等待发送完成
		len_bytes -= chunk_size;

		// !!! 注意：如果是单字节传输，pdata 指针不需要增加
		if (!is_single)
			pdata += chunk_size;

	} while (len_bytes > 0);

	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_TXE) != SET)
		;
	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) != RESET)
		;

	ST7789_CS_H;
}

// 单色填充
void ST7789_Fill_Color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                       const uint16_t color)
{
	if (!ST7789_IN_Screen(x1, y1, x2, y2))
		return;

	ST7789_Set_Range(x1, y1, x2, y2);
	uint32_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);

	// 直接 DMA 发送单一 16-bit 数据
	ST7789_DMA_TX((uint8_t*)&color, pixels * 2, true);
}

void ST7789_Set_Pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
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

	ST7789_DMA_TX(pixel_data, (pixel_ptr - pixel_data), false);
}

void ST7789_Show_ASCII(uint16_t x, uint16_t y, char ch, uint16_t color,
                       uint16_t bg_color, const font_t* font)
{
	if (font == NULL)
	{
		ST7789_LOG("ST7789_Show_ASCII: font is NULL");
		return;
	}

	uint16_t fheight = font->size, fwidth = font->size / 2;
	if (!ST7789_IN_Screen(x, y, x + fwidth - 1, y + fheight - 1))
	{
		ST7789_LOG("ST7789_Show_ASCII: x = %d, y = %d, ch = %c", x, y, ch);
		return;
	}

	if (ch < 0x20 || ch > 0x7E)
	{
		ST7789_LOG("ST7789_Show_ASCII: ch = %c is not supported", ch);
		return;
	}

	uint16_t bytes_per_row = (fwidth + 7) / 8;

	const uint8_t* model =
	  font->ascii_model + (ch - ' ') * fheight * bytes_per_row;
	ST7789_Set_Pixels(x, y, fwidth, fheight, model, color, bg_color);
}

void ST7789_Show_Chinese(uint16_t x, uint16_t y, char* ch, uint16_t color,
                         uint16_t bg_color, const font_t* font)
{
	if (ch == NULL || font == NULL)
		return;

	uint16_t fheight = font->size, fwidth = font->size;
	if (!ST7789_IN_Screen(x, y, x + fwidth - 1, y + fheight - 1))
	{
		ST7789_LOG("ST7789_Show_Chinese: x = %d, y = %d, ch = %s", x, y, ch);
		return;
	}
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
	{
		c = fallback;
	}

	ST7789_Set_Pixels(x, y, fwidth, fheight, c->model, color, bg_color);
}

bool IS_GB2312(char ch)
{
	return ((unsigned char)ch >= 0xA1 && (unsigned char)ch <= 0xF7);
}

// static int utf8_char_length(const char *str)
//{
//     if ((*str & 0x80) == 0) return 1; // 1 byte
//     if ((*str & 0xE0) == 0xC0) return 2; // 2 bytes
//     if ((*str & 0xF0) == 0xE0) return 3; // 3 bytes
//     if ((*str & 0xF8) == 0xF0) return 4; // 4 bytes
//     return -1; // Invalid UTF-8
// }

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
	if (x >= SCREEN.width || y >= SCREEN.height ||
	    x + image->width - 1 >= SCREEN.width ||
	    y + image->height - 1 >= SCREEN.height)
	{
		ST7789_LOG("ST7789_Draw_Image: x = %d, y = %d, width = %d, height = %d",
		           x, y, image->width, image->height);
		return;
	}

	ST7789_Set_Range(x, y, x + image->width - 1, y + image->height - 1);

	ST7789_CS_L;
	ST7789_DC_H;

	uint32_t       size = image->width * image->height * 2;  // 每个像素2字节
	const uint8_t* data = image->data;

	ST7789_DMA_TX(data, size, false);
}

uint8_t ST7789_Get_Pixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint16_t* buffer)
{
	uint16_t width  = x2 - x1 + 1;
	uint16_t height = y2 - y1 + 1;
	if (!buffer)
		return 0;

	ST7789_Set_Range(x1, y1, x2, y2);

	ST7789_CS_L;  // 发送读取RAM命令
	ST7789_DC_L;  // 命令模式

	SPI_SendData(SCREEN.spic.spix, 0x2E);
	while (SPI_GetFlagStatus(SCREEN.spic.spix, SPI_FLAG_BSY) == SET)
		;

	ST7789_DC_H;  // 数据模式

	for (uint16_t row = 0; row < height; row++)
	{
		for (uint16_t col = 0; col < width; col++)
		{
			uint8_t r = SPI_ReceiveData(SCREEN.spic.spix);
			uint8_t g = SPI_ReceiveData(SCREEN.spic.spix);
			uint8_t b = SPI_ReceiveData(SCREEN.spic.spix);

			buffer[row * width + col] =
			  ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}
	}

	ST7789_CS_H;

	return 1;  // 使用完记得 free(buffer)
}

// 判断是否是 UTF-8 编码
bool is_utf8(const char* str)
{
	unsigned char* bytes = (unsigned char*)str;
	while (*bytes)
	{
		if ((*bytes & 0x80) == 0x00)
		{
			// 1 字节 ASCII
			bytes += 1;
		}
		else if ((*bytes & 0xE0) == 0xC0)
		{
			// 2 字节 UTF-8
			if ((bytes[1] & 0xC0) != 0x80)
				return false;
			bytes += 2;
		}
		else if ((*bytes & 0xF0) == 0xE0)
		{
			// 3 字节 UTF-8
			if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80)
				return false;
			bytes += 3;
		}
		else if ((*bytes & 0xF8) == 0xF0)
		{
			// 4 字节 UTF-8
			if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 ||
			    (bytes[3] & 0xC0) != 0x80)
				return false;
			bytes += 4;
		}
		else
		{
			// 不合法的 UTF-8 起始字节
			return false;
		}
	}
	return true;
}
