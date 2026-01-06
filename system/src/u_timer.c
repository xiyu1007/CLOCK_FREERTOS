#include "u_timer.h"

volatile uint32_t g_ms_tick = 0;  // 毫秒计数


/*---------------------------------------
 * 获取 TIM4 的真实输入时钟
 *--------------------------------------*/
static uint32_t tim4_get_clk(void)
{
	uint32_t pclk1;
	uint32_t prescaler;

	/* APB1 prescaler */
	prescaler = (RCC->CFGR >> 10) & 0x7;  // PPRE1[2:0]

	/* 计算 PCLK1 */
	if (prescaler < 4)
		pclk1 = SystemCoreClock;  // /1
	else
		pclk1 = SystemCoreClock >> (prescaler - 3);  // /2 /4 /8 /16

	/* 定时器时钟规则 */
	if (prescaler < 4)
		return pclk1;  // APB1 prescaler = 1
	else
		return pclk1 * 2;  // APB1 prescaler != 1
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		g_ms_tick++;  // 每 1 ms 增加一次
	}
}

/*---------------------------------------
 * u_timer 初始化：1 ms 中断一次
 *--------------------------------------*/
__attribute__((constructor)) static void u_timer_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	uint32_t timclk = tim4_get_clk();  // 真实 TIM4 输入时钟 84 MHz

	TIM_TimeBaseInitTypeDef tim4;
	TIM_TimeBaseStructInit(&tim4);
	tim4.TIM_Prescaler     = (timclk / 1000000UL) - 1;  // 1 MHz
	tim4.TIM_CounterMode   = TIM_CounterMode_Up;
	tim4.TIM_Period        = 1000 - 1;
	tim4.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM4, &tim4);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	// 5. 配置 NVIC
	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel                   = TIM4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 4;
	nvic.NVIC_IRQChannelSubPriority        = 0;
	nvic.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&nvic);
	// 开始计数
	TIM_Cmd(TIM4, ENABLE);
}
