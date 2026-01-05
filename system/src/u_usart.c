#include "u_usart.h"


void usart_default_config(USARTConfig* usartc)
{
	usartc->usartx = SERIAL_USART;
	usartc->port = SERIAL_USART_PORT;
	usartc->tx = SERIAL_USART_TX;
	usartc->rx = SERIAL_USART_RX;
	usartc->IRQn = SERIAL_IRQn;

	USART_StructInit(&usartc->usart_initstruct);
	usartc->usart_initstruct.USART_BaudRate            = 115200;
	usartc->usart_initstruct.USART_WordLength          = USART_WordLength_8b;
	usartc->usart_initstruct.USART_StopBits            = USART_StopBits_1;
	usartc->usart_initstruct.USART_Parity              = USART_Parity_No;
	usartc->usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartc->usart_initstruct.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
}

void u_usart_init(USARTConfig* usartc)
{
	// 1. 先使能 GPIOA 时钟
	if (BSP_GPIO_EnableClock(usartc->port))
	{
		USART_LOG("u_usart_init: gpio clock enable failed, port: %d", usartc->port);
	}
	if (BSP_USART_EnableClock(usartc->usartx))
	{
		USART_LOG("u_usart_init: usart clock enable failed, usartx: %d", usartc->usartx);
	}

	// 2. 配置 PA9/PA10
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin   = usartc->tx | usartc->rx;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(usartc->port, &GPIO_InitStruct);

	if (BSP_GPIO_ConfigAF(usartc->port, usartc->tx, usartc->usartx))
	{
		USART_LOG("u_usart_init: gpio config af failed, port: %d, tx: %d, usartx: %d", usartc->port, usartc->tx, usartc->usartx);
	}

	if (BSP_GPIO_ConfigAF(usartc->port, usartc->rx, usartc->usartx))
	{
		USART_LOG("u_usart_init: gpio config af failed, port: %d, rx: %d, usartx: %d", usartc->port, usartc->rx, usartc->usartx);
	}

	// 4. 配置 USART1
	USART_Init(usartc->usartx, &usartc->usart_initstruct);

	// 5. 使能 USART
	USART_Cmd(usartc->usartx, ENABLE);
}

