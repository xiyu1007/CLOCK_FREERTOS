#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f4xx.h"

// #define URL "https://api.seniverse.com/v3/weather/now.json?key=S60PH5MTeThy6NgFn&location=Chengdu&language=zh-Hans&unit=c"

#define URL                                 \
	"https://api.seniverse.com/v3/weather/" \
	"now.json?key=S60PH5MTeThy6NgFn&location=Chengdu&language=en&unit=c"

#define SSID                 ssid
#define PASSWORD             password

#define TIME_IDLE            1000
#define TIME_GET_TMP_INDOOR  1500   // 1.5秒
#define TIME_CHECK_WIFI      5000   // 5秒
#define TIME_GET_TMP_OUTDOOR 30000  // 30秒
#define TIME_GET_TIME        10000  // 1分钟

extern char ssid[128];
extern char password[128];

typedef struct
{
	uint32_t at_initiated:1;
	uint32_t dht11_initiated:1;
	uint32_t log_initiated:1;
	uint32_t tft_lcd_initiated:1;
	uint32_t st7789_initiated:1;

	uint32_t wifi_connected:1;
	uint32_t wifi_info_change:1;
	uint32_t http_requested:1;
	uint32_t dht11_updated:1;
	uint32_t time_updated:1;
	uint32_t date_updated:1;
	uint32_t init_page_over:1;
	uint32_t idle:1;
	uint32_t:1;
} system_status_t;

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

extern weather_info_t  g_weather_info;
extern system_status_t g_s_s;
// extern char            home_page_buf[1024];

static void update_system_status(void);
static void update_system(void);
void vTaskRun(void *pvParameters);

#endif /* __MAIN_H__ */
