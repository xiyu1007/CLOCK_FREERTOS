#ifndef __U_SYSTICK_H__
#define __U_SYSTICK_H__

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"


#define SYSTICK_MAX_US (0xFFFFFF / (SystemCoreClock / 1000000U))


// #define u_delay_ms(m) vTaskDelay((m) / portTICK_PERIOD_MS)
// #define u_delay_s(s) vTaskDelay((s)* 1000 / portTICK_PERIOD_MS)


#endif /* __U_SYSTICK_H__ */

