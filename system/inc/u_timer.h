#ifndef __U_TIMER_H__
#define __U_TIMER_H__

#include "stm32f4xx.h"

// extern volatile uint32_t g_ms_tick;  // TIM4 毫秒计数全局变量
/* 当前时间 tick */
// #define NOW() ((uint32_t)(g_ms_tick))
#define NOW() xTaskGetTickCount()
/* 判断是否超时 */
#define IS_TIMEOUT(start, ms) ((ms) ? ((uint32_t)((NOW()) - (uint32_t)(start)) >= (uint32_t)(ms)) : 0)

// u_delay_us(m) // 定义在 u_timer.c 中， void u_delay_us(uint32_t us)
void u_delay_us(uint32_t us);
#define u_delay_ms(m) vTaskDelay( pdMS_TO_TICKS(m))
#define u_delay_s(s) vTaskDelay( pdMS_TO_TICKS((s)* 1000))

#define delay_us(u) u_delay_us(u)
#define delay_ms(m) u_delay_ms(m)
#define delay_s(s) u_delay_s(s)

#endif /* __U_TIMER_H__ */

