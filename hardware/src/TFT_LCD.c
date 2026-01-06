#include "TFT_LCD.h"

TFT_LCD_InitTypeDef g_TFT_LCD = {
	.width  = TFT_LCD_Width,
	.height = TFT_LCD_Height,
	.rst    = GPIO_Pin_2,
	.dc     = GPIO_Pin_3,
};

void TFT_LCD_Init(SPIConfig* spic)
{

	if (!spic)
	{
		spi_default_config(&g_TFT_LCD.spic);
		g_TFT_LCD.spic.port                                 = TFT_LCD_PORT;
		g_TFT_LCD.spic.spix                                 = TFT_LCD_SPIx;
		g_TFT_LCD.spic.sck                                  = TFT_LCD_SCK;
		g_TFT_LCD.spic.mosi                                 = TFT_LCD_MOSI;
		g_TFT_LCD.spic.miso                                 = TFT_LCD_MISO;
		g_TFT_LCD.spic.cs                                   = TFT_LCD_CS;
		g_TFT_LCD.spic.spi_initstruct.SPI_CPOL              = SPI_CPOL_Low;
		g_TFT_LCD.spic.spi_initstruct.SPI_CPHA              = SPI_CPHA_1Edge;
		g_TFT_LCD.spic.spi_initstruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

		spic = &g_TFT_LCD.spic; // Important
	}
	else
		g_TFT_LCD.spic = *spic;

	if (BSP_GPIO_EnableClock(g_TFT_LCD.spic.port))
		printf("TFT_LCD_Init: gpio enable clock failed, port: %d\r\n", g_TFT_LCD.spic.port);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin   = g_TFT_LCD.rst | g_TFT_LCD.dc;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;

	GPIO_Init(g_TFT_LCD.spic.port, &GPIO_InitStruct);

	u_spi_init(&g_TFT_LCD.spic);

	GPIO_SetBits(g_TFT_LCD.spic.port, g_TFT_LCD.rst | g_TFT_LCD.dc | g_TFT_LCD.spic.cs);
}
