#ifndef __AT_H__
#define __AT_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "stm32f4xx.h"
#include "u_delay.h"
#include "u_timer.h"
#include "u_usart.h"

#include "u_log.h"

/* USART 配置 */
#define AT_USART           USART1
#define AT_USART_PORT      GPIOA
#define AT_USART_TX        GPIO_Pin_9
#define AT_USART_RX        GPIO_Pin_10

/* 参数 */
#define AT_RECV_BUF_SIZE   2048
#define AT_RECV_TIMEOUT    1000
#define AT_INIT_TIMEOUT    3000
#define AT_WIFI_TIMEOUT    3000
#define AT_HTTP_TIMEOUT    10000

// #define AT_LOG(fmt, ...) LOG("[AT] %s:%d " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define AT_LOG(fmt, ...) LOG("[AT] " fmt "\r\n", ##__VA_ARGS__)

/* WiFi 状态 */
typedef enum
{
	AT_WIFI_NONE = 0,
	AT_WIFI_ERROR,
	AT_WIFI_UNKNOWN,
	AT_WIFI_DISCONNECT,
	AT_WIFI_CONNECTED,
	AT_WIFI_BUSY,
} AT_WIFI_Status;

/* AT 状态 */
typedef enum
{
	AT_OK = 0,
	AT_READY,
	AT_ERROR,
	AT_BUSY,
	AT_IDLE,
	AT_BUF_FULL,
	AT_INCOMPLETE,
	AT_UNKNOWN,
} AT_Status;

/* AT 接口 */
uint8_t   AT_Init(void);
void      AT_Send(const char *cmd);
void      AT_SendCRLF(void);
uint8_t   AT_Is_Busy(void);
AT_Status AT_Parse(void);

AT_WIFI_Status AT_WIFI_Connect(char *ssid,
                               const char *password,
                               const char *mac);
AT_WIFI_Status AT_WIFI_Info(char *ssid);

uint8_t AT_HTTP_Request(const char *url, weather_info_t *info);

uint8_t AT_Get_Time(time_t *tm);

#endif /* __AT_H__ */
