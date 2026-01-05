#ifndef __U_TIMER_H__
#define __U_TIMER_H__

#include "stm32f4xx.h"

extern volatile uint32_t g_ms_tick;  // TIM4 毫秒计数全局变量
/* 当前时间 tick */
#define NOW() ((uint32_t)(g_ms_tick))
/* 判断是否超时 */
#define IS_TIMEOUT(start, ms) ((ms) ? ((uint32_t)((NOW()) - (uint32_t)(start)) >= (uint32_t)(ms)) : 0)

// #define NOW() (xTaskGetTickCount())
// #define IS_TIMEOUT(start, ms) ((ms) ? ((uint32_t)((NOW()) - (uint32_t)(start)) >= (uint32_t)(ms)) : 0)


#endif /* __U_TIMER_H__ */

