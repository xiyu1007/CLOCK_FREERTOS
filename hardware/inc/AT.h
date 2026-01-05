#ifndef __AT_H__
#define __AT_H__

#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "stm32f4xx.h"
#include "u_delay.h"
#include "u_timer.h"
#include "u_usart.h"



// #define __AT_DEBUG__
// #define AT_USART      USART2
// #define AT_USART_PORT GPIOD
// #define AT_USART_TX   GPIO_Pin_5
// #define AT_USART_RX   GPIO_Pin_6
#define AT_USART           USART1
#define AT_USART_PORT      GPIOA
#define AT_USART_TX        GPIO_Pin_9
#define AT_USART_RX        GPIO_Pin_10
#define AT_RECV_BUF_SIZE   2048
#define AT_STATUS_MAP_SIZE (sizeof(at_status_map) / sizeof(at_status_map[0]))

#define AT_RECV_TIMEOUT    1000  // 超时时间（ms）
#define AT_INIT_TIMEOUT    3000
#define AT_WIFI_TIMEOUT    3000
#define AT_HTTP_TIMEOUT    10000
#define AT_LOG(fmt, ...)   printf("[AT] FILE: %s LINE: %d\r\n" fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

typedef enum
{
	AT_WIFI_NONE = 0,
	AT_WIFI_ERROR,
	AT_WIFI_UNKNOWN,
	AT_WIFI_DISCONNECT,
	AT_WIFI_CONNECTED,
	AT_WIFI_BUSY,
} AT_WIFI_Status;

typedef enum
{
	AT_OK = 0,
	AT_READY,
	AT_ERROR,
	AT_BUSY,
	AT_IDLE,

	// AT_TIMEOUT,     // 超时
	AT_BUF_FULL,    // 缓冲区满
	AT_INCOMPLETE,  // 未接收完整（最后不是 \n）
	AT_UNKNOWN,     // 未知
} AT_Status;

typedef struct
{
	const char* str;
	AT_Status   status;
} AT_StatusMap;

extern USARTConfig g_at_usartc;

uint8_t   AT_Init(void);
void      AT_Send(const char* str, uint32_t timeout);
void      AT_Callback(void);
void      AT_Recv(uint32_t timeout);
void      AT_Set_Echo(uint8_t echo);
void      AT_SendCRLF(void);
AT_Status AT_Parse(void);
uint8_t   AT_Is_Busy(void);
uint8_t   AT_Is_Echo(void);
uint8_t   AT_Wait_Status(AT_Status status, uint32_t timeout);
uint8_t   AT_Wait_Send(uint32_t timeout);

AT_WIFI_Status AT_WIFI_Connect(char* ssid, const char* password,
                               const char* mac);
AT_WIFI_Status AT_WIFI_Info(char* ssid);
uint8_t        AT_Wait_WIFI_Connected(char* ssid, uint32_t timeout);

uint8_t        AT_HTTP_Request(const char* url, weather_info_t* info);
static uint8_t parse_weather(weather_info_t* info);
static void    extract_province_from_path(const char* path, char* province,
                                          size_t len);
static int     json_next_string(char** pp, const char* key, char* out,
                                size_t out_len);

uint8_t        AT_Get_Time(time_t* tm);
static uint8_t parse_time(time_t* tm);
void           AT_Show_Time(time_t* tm);

#endif /* __AT_H__ */
