#ifndef __U_USART_H__
#define __U_USART_H__

#include "stm32f4xx.h"
#include "u_bsp_rcc.h"
#include <stdio.h>

// #define SERIAL_USART      USART1
// #define SERIAL_USART_PORT GPIOA
// #define SERIAL_USART_TX   GPIO_Pin_9
// #define SERIAL_USART_RX   GPIO_Pin_10
#define SERIAL_USART          USART2
#define SERIAL_USART_PORT     GPIOD
#define SERIAL_USART_TX       GPIO_Pin_5
#define SERIAL_USART_RX       GPIO_Pin_6
#define SERIAL_IRQn           USART2_IRQn
#define SERIAL_DMA_RX_STREAM  DMA1_Stream5
#define SERIAL_DMA_RX_CHANNEL DMA_Channel_4
#define SERIAL_DMA_RX_RCC     RCC_AHB1Periph_DMA1


#define USART_LOG(fmt, ...)   printf("[USART] " fmt "\r\n", ##__VA_ARGS__)

typedef struct
{
	USART_TypeDef*      usartx;
	USART_InitTypeDef   usart_initstruct;
	GPIO_TypeDef*       port;
	uint16_t            tx;
	uint16_t            rx;
	IRQn_Type           IRQn;
	DMA_Stream_TypeDef* dma_rx_stream;
	uint32_t            dma_rx_channel;
	uint32_t            dma_rx_rcc;
} USARTConfig;

void u_usart_init(USARTConfig* usartc);
void usart_default_config(USARTConfig* usartc);

#endif /* __U_USART_H__ */
