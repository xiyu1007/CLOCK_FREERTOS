#ifndef __U_KEY_H__
#define __U_KEY_H__

#include "stm32f4xx.h"

#define U_KEY_PIN0 GPIO_Pin_4
#define U_KEY_PIN1 GPIO_Pin_3

#define KEY_FILTER_CNT  5   // 5 ms 消抖

typedef struct {
    uint8_t stable;      // 稳定状态（0/1）
    uint8_t cnt;         // 积分计数
    uint16_t pin;        // 引脚号
} key_t;


void u_key_init(void);
uint8_t u_key_scan(key_t *u_key);


#endif /* __U_KEY_H__ */


