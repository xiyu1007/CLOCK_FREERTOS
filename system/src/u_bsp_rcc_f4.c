#include "stm32f4xx.h"
#include "u_bsp_rcc.h"

#if defined(STM32F4xx) || defined(STM32F40_41xxx) || defined(STM32F40XX)
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


static uint8_t GPIO_PinToSource(uint16_t pin)
{
	uint8_t source = 0;
	while (pin >>= 1)
		source++;
	return source;
}

uint8_t BSP_GPIO_ConfigAF(void* port, uint16_t pin, void* peripheral)
{
	uint8_t pin_source = GPIO_PinToSource(pin);

	for (uint32_t i = 0; i < sizeof(af_map) / sizeof(af_map[0]); i++)
	{
		if (af_map[i].port == port && af_map[i].pin == pin && af_map[i].peripheral == peripheral)
		{
			GPIO_PinAFConfig((GPIO_TypeDef*)port, pin_source, af_map[i].af);
			return RCC_ENABLE_SUCCESS;
		}
	}
	return RCC_ENABLE_FAILURE;
}


// uint8_t BSP_GPIO_ConfigAF(void* port, uint8_t pin, BSP_PinFunction_t function)
// {
// 	for (uint32_t i = 0; i < sizeof(af_map) / sizeof(af_map[0]); i++)
// 	{
// 		if (af_map[i].port == port && af_map[i].pin == pin && af_map[i].function == function)
// 		{
// 			GPIO_PinAFConfig((GPIO_TypeDef*)port, pin, af_map[i].af);
// 			return RCC_ENABLE_SUCCESS;
// 		}
// 	}
// 	/* Î´ÕÒµ½Ó³Éä£¬ËµÃ÷ÅäÖÃ´íÎó */
// 	return RCC_ENABLE_FAILURE;
// }

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
