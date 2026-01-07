#ifndef __MAIN_H__
#define __MAIN_H__

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "task.h"

// #define URL "https://api.seniverse.com/v3/weather/now.json?key=S60PH5MTeThy6NgFn&location=Chengdu&language=zh-Hans&unit=c"
#define URL  "https://api.seniverse.com/v3/weather/now.json?key=S60PH5MTeThy6NgFn&location=Chengdu&language=en&unit=c"

#define SSID           ssid
#define PASSWORD       password

#define TIME_CHECK_AT  3000
#define TIME_EXCEPTION 30000
#define TIME_IDLE      1000
#define TIME_DHT11     2000   // 1.5秒
#define TIME_WIFI      2000   // 5秒
#define TIME_HTTP      30000  // 30秒
#define TIME_GET_TIME  10000  // 1分钟

#define CHECK_DELAY    1000  // 1000 ms

#define TIME_SEM_TAKE  portMAX_DELAY

#define BASE_PRIORITY  1

extern char ssid[128];
extern char password[128];

typedef enum
{
	EVT_AT_INITED     = (1 << 0),
	EVT_DHT11_INITED  = (1 << 1),
	EVT_LOG_INITED    = (1 << 2),
	EVT_TFT_INITED    = (1 << 3),
	EVT_ST7789_INITED = (1 << 4),
	EVT_INITPAGE_DONE = (1 << 5),
	EVT_HOMEPAGE_DONE = (1 << 6),

	EVT_WIFI_NEED_CONNECT = (1 << 7),
	EVT_WIFI_STATUS       = (1 << 8),

	EVT_HTTP_REQUEST  = (1 << 9),
	EVT_DHT11_UPDATED = (1 << 10),
	EVT_TIME_UPDATED  = (1 << 11),
	EVT_DATE_UPDATED  = (1 << 12),
	EVT_LOG_RX_READY  = (1 << 13),  // 串口接收到一帧完整数据
	EVT_EXCEPTION     = (1 << 14),
	EVT_COLON_TOGGLE  = (1 << 15),
	EVT_COLON_TOGGLE2 = (1 << 16),
} system_event_t;

typedef struct
{
	uint8_t week;
	uint8_t month;
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint16_t year;

} time_t;

typedef struct
{
	char    city[32];      // location.name
	char    province[32];  // location.path 最后一层之前倒数第二个
	uint8_t weather;       // now.text
	float   temp_outdoor;  // now.temperature
	char    update[32];    // last_update
	float   tmp_indoor;    // 室内温度（℃）
	float   humidity;      // now.humidity
	time_t  time;          // time
} weather_info_t;

extern weather_info_t     g_weather_info;
extern EventGroupHandle_t g_sys_event;
extern TaskHandle_t xLogTaskHandle;

#endif /* __MAIN_H__ */
