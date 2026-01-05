#include "u_log.h"

static uint8_t     log_ready = 0;
static USARTConfig usartc;

static log_t log = {
	.ready  = 0,
	.active = 0,
	.ptr    = NULL,
	.buf    = { 0 },
};

PUTCHAR_PROTOTYPE
{
	if (!log_ready)
		return ch;  // 丢弃，不阻塞、不死机

	while (USART_GetFlagStatus(LOG_USART, USART_FLAG_TXE) == RESET)
		;

	USART_SendData(LOG_USART, (uint8_t)ch);

	while (USART_GetFlagStatus(LOG_USART, USART_FLAG_TC) == RESET)
		;

	return ch;
}

void log_init(void)
{
	usart_default_config(&usartc);
	u_usart_init(&usartc);
	log_ready = 1;

	NVIC_InitTypeDef nvic_initstruct;
	nvic_initstruct.NVIC_IRQChannel                   = usartc.IRQn;
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 6;
	nvic_initstruct.NVIC_IRQChannelSubPriority        = 0;
	nvic_initstruct.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&nvic_initstruct);

	USART_ClearFlag(usartc.usartx, USART_FLAG_RXNE);
	USART_ITConfig(usartc.usartx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(usartc.usartx, USART_IT_IDLE, ENABLE);
	log.ptr = log.buf;
}

char* log_buf_get(void)
{

	if (log.ready)
	{
		log.ready = 0;
		return log.buf;
	}
	return 0;
}

void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(usartc.usartx, USART_IT_RXNE) != RESET)
	{
		while (USART_GetITStatus(usartc.usartx, USART_IT_RXNE) != RESET)
		{
			*log.ptr++ = USART_ReceiveData(usartc.usartx);
			if (log.ptr - log.buf >= LOG_BUF_SIZE - 1)
			{
				log.ptr = log.buf;
			}
		}
		log.active = 1;
	}

	if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		// 清空中断需要读取状态寄存器和数据寄存器
		(void)usartc.usartx->SR;  // 读取状态寄存器
		(void)usartc.usartx->DR;  // 读取数据寄存器

		if (log.active)  // 只有本次接收有字节时才触发
		{
			*log.ptr   = '\0';
			log.ptr    = log.buf;
			log.active = 0;
			log.ready  = 1;

			g_s_s.wifi_info_change = 1;
		}
	}
}
