#ifndef __U_SYSTICK_H__
#define __U_SYSTICK_H__

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"


#define SYSTICK_MAX_US (0xFFFFFF / (SystemCoreClock / 1000000U))





// #define u_delay_ms(m) vTaskDelay((m) / portTICK_PERIOD_MS)
// #define u_delay_s(s) vTaskDelay((s)* 1000 / portTICK_PERIOD_MS)

#define u_delay_us(m) vTaskDelay( pdMS_TO_TICKS(1))
#define u_delay_ms(m) vTaskDelay( pdMS_TO_TICKS(m))
#define u_delay_s(s) vTaskDelay( pdMS_TO_TICKS((s)* 1000))

#define delay_us(u) u_delay_us(u)
#define delay_ms(m) u_delay_ms(m)
#define delay_s(s) u_delay_s(s)



#endif /* __U_SYSTICK_H__ */

