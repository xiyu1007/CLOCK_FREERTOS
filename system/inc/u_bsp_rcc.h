#ifndef __U_BSP_RCC_H__
#define __U_BSP_RCC_H__

#include "stm32f4xx.h"

#define RCC_ENABLE_SUCCESS 0  // 使能成功返回值
#define RCC_ENABLE_FAILURE 1  // 使能失败返回值

typedef enum
{
	RCC_BUS_AHB1,
	RCC_BUS_APB1,
	RCC_BUS_APB2
} RCC_Bus_t;

// #define RCC_MAP_SIZE (sizeof(rcc_map) / sizeof(rcc_map[0]))

// RCC 使能函数映射表
// 每个元素对应一个外设实例，包含其时钟使能函数、所在总线和时钟位定义
typedef void (*RCC_EnableFunc_t)(uint32_t, FunctionalState);

typedef struct
{
	void*            instance;  // GPIOA / SPI1 / USART1
	uint32_t         clock;     // RCC_xxxPeriph_xxx
	RCC_Bus_t        bus;       // 所在总线
	RCC_EnableFunc_t enable;    // 使能函数
} RCC_Map_t;

static const RCC_Map_t gpio_map[] = {
	{ GPIOA, RCC_AHB1Periph_GPIOA, RCC_BUS_AHB1, RCC_AHB1PeriphClockCmd },
	{ GPIOB, RCC_AHB1Periph_GPIOB, RCC_BUS_AHB1, RCC_AHB1PeriphClockCmd },
	{ GPIOC, RCC_AHB1Periph_GPIOC, RCC_BUS_AHB1, RCC_AHB1PeriphClockCmd },
	{ GPIOD, RCC_AHB1Periph_GPIOD, RCC_BUS_AHB1, RCC_AHB1PeriphClockCmd },
	{ GPIOE, RCC_AHB1Periph_GPIOE, RCC_BUS_AHB1, RCC_AHB1PeriphClockCmd },
};

static const RCC_Map_t spi_map[] = {
	{ SPI1, RCC_APB2Periph_SPI1, RCC_BUS_APB2, RCC_APB2PeriphClockCmd },
	{ SPI2, RCC_APB1Periph_SPI2, RCC_BUS_APB1, RCC_APB1PeriphClockCmd },
	{ SPI3, RCC_APB1Periph_SPI3, RCC_BUS_APB1, RCC_APB1PeriphClockCmd },
};

static const RCC_Map_t usart_map[] = {
	{ USART1, RCC_APB2Periph_USART1, RCC_BUS_APB2, RCC_APB2PeriphClockCmd },
	{ USART2, RCC_APB1Periph_USART2, RCC_BUS_APB1, RCC_APB1PeriphClockCmd },
	{ USART3, RCC_APB1Periph_USART3, RCC_BUS_APB1, RCC_APB1PeriphClockCmd },
};

// 引脚功能映射表2
// 引脚功能枚举
typedef enum
{
	BSP_AF_USART1_TX,
	BSP_AF_USART1_RX,
	BSP_AF_SPI1_SCK,
	BSP_AF_SPI1_MOSI,
	BSP_AF_SPI1_MISO,
} BSP_PinFunction_t;

typedef struct
{
	void*    port;
	uint16_t pin;
	void*    peripheral;
	uint8_t  af;
} BSP_AF_Map_t;

static const BSP_AF_Map_t af_map[] = {
	/* ================= USART1 (AF7) ================= */
	{ GPIOA, GPIO_Pin_9, USART1, GPIO_AF_USART1 },  /* TX */
	{ GPIOA, GPIO_Pin_10, USART1, GPIO_AF_USART1 }, /* RX */
	{ GPIOB, GPIO_Pin_6, USART1, GPIO_AF_USART1 },  /* TX */
	{ GPIOB, GPIO_Pin_7, USART1, GPIO_AF_USART1 },  /* RX */

	/* ================= USART2 (AF7) ================= */
	{ GPIOA, GPIO_Pin_2, USART2, GPIO_AF_USART2 }, /* TX */
	{ GPIOA, GPIO_Pin_3, USART2, GPIO_AF_USART2 }, /* RX */
	{ GPIOD, GPIO_Pin_5, USART2, GPIO_AF_USART2 }, /* TX */
	{ GPIOD, GPIO_Pin_6, USART2, GPIO_AF_USART2 }, /* RX */

	/* ================= USART3 (AF7) ================= */
	{ GPIOB, GPIO_Pin_10, USART3, GPIO_AF_USART3 }, /* TX */
	{ GPIOB, GPIO_Pin_11, USART3, GPIO_AF_USART3 }, /* RX */
	{ GPIOD, GPIO_Pin_8, USART3, GPIO_AF_USART3 },  /* TX */
	{ GPIOD, GPIO_Pin_9, USART3, GPIO_AF_USART3 },  /* RX */

	/* ================= SPI1 (AF5) ================= */
	{ GPIOA, GPIO_Pin_5, SPI1, GPIO_AF_SPI1 }, /* SCK */
	{ GPIOA, GPIO_Pin_6, SPI1, GPIO_AF_SPI1 }, /* MISO */
	{ GPIOA, GPIO_Pin_7, SPI1, GPIO_AF_SPI1 }, /* MOSI */
	{ GPIOB, GPIO_Pin_3, SPI1, GPIO_AF_SPI1 }, /* SCK */
	{ GPIOB, GPIO_Pin_4, SPI1, GPIO_AF_SPI1 }, /* MISO */
	{ GPIOB, GPIO_Pin_5, SPI1, GPIO_AF_SPI1 }, /* MOSI */

	/* ================= SPI2 (AF5) ================= */
	{ GPIOB, GPIO_Pin_13, SPI2, GPIO_AF_SPI2 }, /* SCK */
	{ GPIOB, GPIO_Pin_14, SPI2, GPIO_AF_SPI2 }, /* MISO */
	{ GPIOB, GPIO_Pin_15, SPI2, GPIO_AF_SPI2 }, /* MOSI */
	{ GPIOC, GPIO_Pin_2, SPI2, GPIO_AF_SPI2 },  /* MISO */
	{ GPIOC, GPIO_Pin_3, SPI2, GPIO_AF_SPI2 },  /* MOSI */

	/* ================= SPI3 (AF6) ================= */
	{ GPIOB, GPIO_Pin_3, SPI3, GPIO_AF_SPI3 },  /* SCK */
	{ GPIOB, GPIO_Pin_4, SPI3, GPIO_AF_SPI3 },  /* MISO */
	{ GPIOB, GPIO_Pin_5, SPI3, GPIO_AF_SPI3 },  /* MOSI */
	{ GPIOC, GPIO_Pin_10, SPI3, GPIO_AF_SPI3 }, /* SCK */
	{ GPIOC, GPIO_Pin_11, SPI3, GPIO_AF_SPI3 }, /* MISO */
	{ GPIOC, GPIO_Pin_12, SPI3, GPIO_AF_SPI3 }, /* MOSI */
};

uint8_t BSP_GPIO_EnableClock(void* gpio);
uint8_t BSP_SPI_EnableClock(void* spi);
uint8_t BSP_USART_EnableClock(void* usart);

uint8_t BSP_GPIO_ConfigAF(void* port, uint16_t pin, void* peripheral);

// // 引脚功能映射表
// // 引脚功能枚举
// typedef enum
// {
// 	BSP_AF_USART1_TX,
// 	BSP_AF_USART1_RX,
// 	BSP_AF_SPI1_SCK,
// 	BSP_AF_SPI1_MOSI,
// 	BSP_AF_SPI1_MISO,
// } BSP_PinFunction_t;

// typedef struct
// {
// 	void*             port;
// 	uint8_t           pin;
// 	BSP_PinFunction_t function;
// 	uint8_t           af;
// } BSP_AF_Map_t;

// static const BSP_AF_Map_t af_map[] = {
// 	{ GPIOA, 9, BSP_AF_USART1_TX, GPIO_AF_USART1 }, { GPIOA, 10, BSP_AF_USART1_RX, GPIO_AF_USART1 },
// 	{ GPIOA, 5, BSP_AF_SPI1_SCK, GPIO_AF_SPI1 },    { GPIOA, 6, BSP_AF_SPI1_MISO, GPIO_AF_SPI1 },
// 	{ GPIOA, 7, BSP_AF_SPI1_MOSI, GPIO_AF_SPI1 },
// };

// uint8_t BSP_GPIO_ConfigAF(void* port, uint8_t pin, BSP_PinFunction_t function);
#endif /* __U_BSP_RCC_H__ */
