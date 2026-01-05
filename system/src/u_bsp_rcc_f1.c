#include "u_bsp_rcc.h"
#include "stm32f4xx.h"

#if defined(STM32F10X_MD) 
uint8_t BSP_GPIO_EnableClock(void* gpio)
{
	for (uint32_t i = 0; i < sizeof(gpio_map) / sizeof(gpio_map[0]); i++)
	{
		if (gpio_map[i].instance == gpio)
		{
			gpio_map[i].enable(gpio_map[i].clock, ENABLE);
			return RCC_ENABLE_SUCCESS;
		}
	}
    return RCC_ENABLE_FAILURE;
}

uint8_t BSP_SPI_EnableClock(void* spi)
{
	for (uint32_t i = 0; i < sizeof(spi_map) / sizeof(spi_map[0]); i++)
	{
		if (spi_map[i].instance == spi)
		{
			spi_map[i].enable(spi_map[i].clock, ENABLE);
			return RCC_ENABLE_SUCCESS;
		}
	}
    return RCC_ENABLE_FAILURE;
}
uint8_t BSP_USART_EnableClock(void* usart)
{
	for (uint32_t i = 0; i < sizeof(usart_map) / sizeof(usart_map[0]); i++)
	{
		if (usart_map[i].instance == usart)
		{
			usart_map[i].enable(usart_map[i].clock, ENABLE);
			return RCC_ENABLE_SUCCESS;
		}
	}
    return RCC_ENABLE_FAILURE;
}

// uint8_t BSP_RCC_Enable(void *instance)
// {
//     for (int i = 0; i < RCC_MAP_SIZE; i++)
//     {
//         if (rcc_map[i].instance == instance)
//         {
//             switch (rcc_map[i].bus)
//             {
//             case RCC_BUS_AHB1:
//                 RCC_AHB1PeriphClockCmd(rcc_map[i].clock, ENABLE);
//                 break;

//             case RCC_BUS_APB1:
//                 RCC_APB1PeriphClockCmd(rcc_map[i].clock, ENABLE);
//                 break;

//             case RCC_BUS_APB2:
//                 RCC_APB2PeriphClockCmd(rcc_map[i].clock, ENABLE);
//                 break;
//             }
//             return RCC_ENABLE_SUCCESS;
//         }
//     }
//     return RCC_ENABLE_FAILURE;
// }

#endif

