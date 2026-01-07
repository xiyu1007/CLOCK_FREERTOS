#include "u_log.h"

/* ----------------------- 配置 ----------------------- */

static char        log_buf[LOG_BUF_SIZE + 1];
static int         LOG_INITED = 0;
static USARTConfig usartc;
uint16_t           recv_len;

static void LOG_DMA_Init(void);
static void log_process(char* line);

extern TaskHandle_t xLogTaskHandle;


void debug_dma_detailed_status(void)
{
    DMA_Stream_TypeDef *s = usartc.dma_rx_stream;
    USART_TypeDef *u = usartc.usartx;

    uint32_t cr = s->CR;

    log("========== DMA RX 详细诊断 ==========");

    log("DMA_SxCR = 0x%08lX", cr);

    log("使能 EN                : %lu", (cr >> 0) & 0x1);
    log("直连模式错误中断 DMEIE : %lu", (cr >> 1) & 0x1);
    log("传输错误中断 TEIE     : %lu", (cr >> 2) & 0x1);
    log("半传输中断 HTIE       : %lu", (cr >> 3) & 0x1);
    log("传输完成中断 TCIE     : %lu", (cr >> 4) & 0x1);

    log("外设流控制 PFCTRL     : %lu (0=DMA控制)", (cr >> 5) & 0x1);

    log("方向 DIR              : %lu (0=P2M,1=M2P,2=M2M)", (cr >> 6) & 0x3);
    log("循环模式 CIRC         : %lu", (cr >> 8) & 0x1);
    log("外设地址递增 PINC     : %lu", (cr >> 9) & 0x1);
    log("内存地址递增 MINC     : %lu", (cr >> 10) & 0x1);

    log("外设数据宽度 PSIZE    : %lu (0=8bit,1=16bit,2=32bit)", (cr >> 11) & 0x3);
    log("内存数据宽度 MSIZE    : %lu (0=8bit,1=16bit,2=32bit)", (cr >> 13) & 0x3);

    log("优先级 PL             : %lu (0=低,1=中,2=高,3=非常高)", (cr >> 16) & 0x3);
    log("双缓冲 DBM            : %lu", (cr >> 18) & 0x1);
    log("当前内存 CT           : %lu", (cr >> 19) & 0x1);

    log("NDTR(剩余计数)         : %lu", s->NDTR);
    log("外设地址 PAR          : 0x%08lX (USART->DR=0x%08lX)",
        s->PAR, (uint32_t)&u->DR);
    log("内存地址 M0AR         : 0x%08lX (log_buf=0x%08lX)",
        s->M0AR, (uint32_t)log_buf);

    log("USART_SR               : 0x%08lX", u->SR);
    log("USART_CR3              : 0x%08lX", u->CR3);
    log("DMA接收使能 DMAR       : %lu", (u->CR3 >> 6) & 0x1);

    log("========== 诊断结束 ==========");
}


/* ----------------------- 初始化 ----------------------- */
void log_init(void)
{
	usart_default_config(&usartc);
	u_usart_init(&usartc);
	LOG_INITED = 1;

	LOG_DMA_Init();
	/* 启用 USART IDLE 中断 */
	USART_ITConfig(usartc.usartx, USART_IT_IDLE, ENABLE);
	// USART_ITConfig(usartc.usartx, USART_IT_RXNE, ENABLE);

	NVIC_SetPriority(usartc.IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	//    NVIC_SetPriority(usartc.IRQn, 6);
	NVIC_EnableIRQ(usartc.IRQn);
}

static void LOG_DMA_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	// #define SERIAL_USART      USART2
    // DMA_DeInit(usartc.dma_rx_stream);
	DMA_InitTypeDef dma_init;
	DMA_StructInit(&dma_init);
	dma_init.DMA_Channel            = usartc.dma_rx_channel;
	dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
	dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
	dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
	dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
	dma_init.DMA_Mode               = DMA_Mode_Normal;
	dma_init.DMA_Priority           = DMA_Priority_High;
	dma_init.DMA_PeripheralBaseAddr = (uint32_t)&usartc.usartx->DR;
	dma_init.DMA_Memory0BaseAddr    = (uint32_t)log_buf;
	dma_init.DMA_BufferSize         = LOG_BUF_SIZE;

	DMA_Init(usartc.dma_rx_stream, &dma_init);
	USART_DMACmd(usartc.usartx, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(usartc.dma_rx_stream, ENABLE);
}

/* ----------------------- 串口 DMA + IDLE 中断 ----------------------- */
void USART2_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (USART_GetITStatus(usartc.usartx, USART_IT_IDLE) ==SET)
	{

		debug_dma_detailed_status();
		(void)usartc.usartx->SR;
		(void)usartc.usartx->DR;

		recv_len = LOG_BUF_SIZE - DMA_GetCurrDataCounter(usartc.dma_rx_stream);

		if (recv_len > 0 && recv_len <= LOG_BUF_SIZE)
		{
			// 通知日志任务处理（携带长度信息）
			vTaskNotifyGiveFromISR(xLogTaskHandle, &xHigherPriorityTaskWoken);
		}
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ----------------------- Log 接收任务 ----------------------- */
void vTaskRun_LogRx(void* pvParameters)
{
	(void)pvParameters;

	while (1)
	{
		// 阻塞等待中断通知
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        DMA_Cmd(usartc.dma_rx_stream, DISABLE);
        while (usartc.dma_rx_stream->CR & DMA_SxCR_EN);
		// 确保末尾 '\0'
		if (recv_len > 0 && recv_len < LOG_BUF_SIZE)
			log_buf[recv_len + 1] = '\0';
		else
			log_buf[LOG_BUF_SIZE] = '\0';

		log_process(log_buf);
		recv_len = 0;

		DMA_SetCurrDataCounter(usartc.dma_rx_stream, LOG_BUF_SIZE);
		DMA_Cmd(usartc.dma_rx_stream, ENABLE);
	}
}

static void log_process(char* line)
{
	if (!line || line[0] == '\0') return;

	char ssid_buf[128]     = { 0 };
	char password_buf[128] = { 0 };
	log("%s", line);
	if (sscanf(line, "WIFI: \"%127[^\"]\" \"%127[^\"]\"", ssid_buf, password_buf) != 2)
	{

		log("WIFI Info Error. Received SSID='%s', PASSWORD='%s'.", ssid_buf, password_buf);
		log("Correct format: WIFI: \"SSID\" \"PASSWORD\". Example: WIFI: \"MyWiFi\" \"12345678\"");
	}
	else
	{
		// 拷贝到全局变量
		strncpy(ssid, ssid_buf, sizeof(ssid) - 1);
		ssid[sizeof(ssid) - 1] = '\0';
		strncpy(password, password_buf, sizeof(password) - 1);
		password[sizeof(password) - 1] = '\0';

		log("WIFI Info Changed. SSID='%s', PASSWORD='%s'", ssid, password);
		xEventGroupSetBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
	}
}

/* ----------------------- printf 重定向 ----------------------- */
PUTCHAR_PROTOTYPE
{
	if (!LOG_INITED) return ch;  // 丢弃，不阻塞、不死机

	while (USART_GetFlagStatus(LOG_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(LOG_USART, (uint8_t)ch);
	while (USART_GetFlagStatus(LOG_USART, USART_FLAG_TC) == RESET);
	return ch;
}

