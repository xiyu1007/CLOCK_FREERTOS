#ifndef __DHT11_H
#define __DHT11_H

#include "u_delay.h"
#include <stdio.h>
#include <string.h>
#include "u_bsp_rcc.h"
#include "u_timer.h"

/* ===== 硬件配置 ===== */
#define DHT11_PORT           GPIOE
#define DHT11_PIN            GPIO_Pin_2
#define DHT11_RCC            RCC_AHB1Periph_GPIOE

/* ===== 时序参数 ===== */
#define DHT11_START_TIME     20   // ms
#define DHT11_START_END_TIME 30   // us
#define DHT11_BIT_SAMPLE_US  50   // us（关键）
#define DHT11_TIMEOUT_US     100  // us
#define DHT11_BITS           40   // 必须是 40 bit

/* ===== 返回值 ===== */
#define FLAG_DHT11_OK        0
#define FLAG_DHT11_ERROR     1

/* ===== IO 操作 ===== */
#define DHT11_SDA_H          GPIO_SetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_SDA_L          GPIO_ResetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_SDA_READ       GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)

#define DHT11_LOG(fmt, ...)  printf("[DHT11] " fmt "\r\n", ##__VA_ARGS__)

/* ===== 接口 ===== */
void    DHT11_Init(void);
uint8_t DHT11_Read(uint8_t* data);
uint8_t DHT11_Parse(uint8_t* data, float* temp, float* humi);
uint8_t DHT11_Get(float* temp, float* humi);

#endif
