#include "u_spi.h"

void spi_default_config(SPIConfig* spic)
{
	spic->spix = SPI1;
	spic->port = GPIOA;
	spic->cs   = GPIO_Pin_4;
	spic->sck  = GPIO_Pin_5;
	spic->miso = GPIO_Pin_6;
	spic->mosi = GPIO_Pin_7;
	spic->DMA_Channel = DMA_Channel_3;
	spic->DMA_Stream = DMA2_Stream3;
	spic->DMA_RCC = RCC_AHB1Periph_DMA2;

	SPI_StructInit(&spic->spi_initstruct);
	(spic->spi_initstruct).SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
	(spic->spi_initstruct).SPI_Mode              = SPI_Mode_Master;
	(spic->spi_initstruct).SPI_DataSize          = SPI_DataSize_8b;
	(spic->spi_initstruct).SPI_CPOL              = SPI_CPOL_Low;
	(spic->spi_initstruct).SPI_CPHA              = SPI_CPHA_1Edge;
	(spic->spi_initstruct).SPI_NSS               = SPI_NSS_Soft;
	(spic->spi_initstruct).SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	(spic->spi_initstruct).SPI_FirstBit          = SPI_FirstBit_MSB;
	// (spic->spi_initstruct).SPI_CRCPolynomial = 7;
}

static void spi_config_AF(SPIConfig* spic)
{
	if (BSP_GPIO_ConfigAF(spic->port, spic->sck, spic->spix))
	{
		printf("spi_config_AF: gpio config af failed, port: %d, sck: %d, spix: %d\n", spic->port, spic->sck, spic->spix);
	}
	if (BSP_GPIO_ConfigAF(spic->port, spic->mosi, spic->spix))
	{
		printf("spi_config_AF: gpio config af failed, port: %d, mosi: %d, spix: %d\n", spic->port, spic->mosi, spic->spix);
	}
	if (BSP_GPIO_ConfigAF(spic->port, spic->miso, spic->spix))
	{
		printf("spi_config_AF: gpio config af failed, port: %d, miso: %d, spix: %d\n", spic->port, spic->miso, spic->spix);
	}
}

static void spi_config_GPIO(SPIConfig* spic)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Pin   = spic->mosi | spic->sck | spic->miso;
	GPIO_Init(spic->port, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	// GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin  = spic->cs;
	GPIO_Init(spic->port, &GPIO_InitStruct);
}

void u_spi_init(SPIConfig* spic)
{
	if (BSP_GPIO_EnableClock(spic->port))
	{
		printf("u_spi_init: gpio enable clock failed, port: %d\r\n", spic->port);
	}

	if (BSP_SPI_EnableClock(spic->spix))
	{
		printf("u_spi_init: spi enable clock failed, spix: %d\r\n", spic->spix);
	}

	spi_config_GPIO(spic);
	spi_config_AF(spic);

	SPI_Init(spic->spix, &spic->spi_initstruct);

	SPI_Cmd(spic->spix, ENABLE);
	SPI_DMACmd(spic->spix, SPI_I2S_DMAReq_Tx, ENABLE);

	// 检查是否正常启动SPI
	if (spi_enable_check(spic->spix) != SPI_CHECK_OK)
		printf("u_spi_init: spi enable check failed, spix: %d\r\n", spic->spix);
}

SPI_CheckStatus spi_enable_check(SPI_TypeDef* SPIx)
{
	uint32_t timeout;

	/* 1. 检查 SPI 使能 */
	if (!(SPIx->CR1 & SPI_CR1_SPE))
		return SPI_CHECK_ERR_SPE;

	/* 2. 检查主机模式 */
	if (!(SPIx->CR1 & SPI_CR1_MSTR))
		return SPI_CHECK_ERR_MODE;

	/* 3. 发送一个测试字 */
	SPIx->DR = 0xA5;

	/* 等待 TXE */
	timeout = 0xFFFF;
	while (!(SPIx->SR & SPI_SR_TXE))
	{
		if (--timeout <= 0)
			return SPI_CHECK_ERR_TXE_TIMEOUT;
	}

	/* 等待 BSY 清零 */
	timeout = 0xFFFF;
	while (SPIx->SR & SPI_SR_BSY)
	{
		if (--timeout <= 0)
			return SPI_CHECK_ERR_BSY_TIMEOUT;
	}

	return SPI_CHECK_OK;
}
