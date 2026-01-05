#ifndef __U_SPI_H__
#define __U_SPI_H__

#include <stdio.h>
#include "stm32f4xx.h"
#include "u_bsp_rcc.h"

typedef struct
{
	SPI_TypeDef*    spix;
	GPIO_TypeDef*   port;
	uint16_t        sck;
	uint16_t        mosi;
	uint16_t        cs;
	uint16_t        miso; /* ¿ÉÑ¡ */
	SPI_InitTypeDef spi_initstruct;
} SPIConfig;

typedef enum
{
	SPI_CHECK_OK = 0,
	SPI_CHECK_ERR_SPE,
	SPI_CHECK_ERR_MODE,
	SPI_CHECK_ERR_TXE_TIMEOUT,
	SPI_CHECK_ERR_BSY_TIMEOUT
} SPI_CheckStatus;

void            u_spi_init(SPIConfig* spic);
SPI_CheckStatus spi_enable_check(SPI_TypeDef* spix);
void            spi_default_config(SPIConfig* spic);

#endif /* __U_SPI_H__ */
