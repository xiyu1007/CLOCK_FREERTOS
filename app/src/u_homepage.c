#include "u_homepage.h"

static char time_buf[64];
static char home_page_buf[256];

void u_homepage(void* pvParameters)
{
	(void)pvParameters;

	// 等待核心显示初始化完成（保证首页显示）
	EventBits_t init_bits = xEventGroupWaitBits(g_sys_event, EVT_TFT_INITED | EVT_ST7789_INITED | EVT_INITPAGE_DONE,
	                                            pdFALSE,  // 不清除
	                                            pdTRUE,   // 等待所有位
	                                            portMAX_DELAY);

	// 显示首页静态 UI
	ST7789_Draw_Image(0, 0, &img_home);
	ST7789_Show(HOME_INFO_X, HOME_INFO_Y, "室内环境", HOME_INFO_COLOR, HOME_INNER_COLOR, &FONT24);

	// WiFi 名称显示（缓存SSID）
	u_update_wifi_name(SSID);
	// 根据事件位显示 WiFi 状态
	u_update_wifi_img();

	// 显示已有天气信息缓存
	u_update_time(&g_weather_info.time);
	u_update_colon(0);
	u_update_date(&g_weather_info.time);
	u_update_city(g_weather_info.city);
	u_update_outdoor_environment(g_weather_info.temp_outdoor);
	u_update_tmp_img(&g_weather_info);
	u_update_weather_img(&g_weather_info);

	u_update_indoor_environment(g_weather_info.tmp_indoor, g_weather_info.humidity);

	if (AT_Get_Time(&g_weather_info.time))
	{
		u_update_time(&g_weather_info.time);
		u_update_colon(HOME_TIME_COLON_COLOR2);
		u_update_date(&g_weather_info.time);

		// 标记时间更新事件
		// xEventGroupSetBits(g_sys_event, EVT_TIME_UPDATED);

		if (!(xEventGroupGetBits(g_sys_event) & EVT_AT_INITED))
		{
			xEventGroupSetBits(g_sys_event, EVT_AT_INITED);
		}
		// 如果 WiFi 还未连接，则显示已连接
		if ((xEventGroupGetBits(g_sys_event) & EVT_WIFI_NEED_CONNECT))
		{
			xEventGroupClearBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
			xEventGroupSetBits(g_sys_event, EVT_WIFI_STATUS);
			u_update_wifi_img();
		}
	}

	// if ((AT_HTTP_Request(URL, &g_weather_info)))
	// {
	// 	u_update_city(g_weather_info.city);
	// 	u_update_outdoor_environment(g_weather_info.temp_outdoor);
	// 	u_update_tmp_img(&g_weather_info);
	// 	u_update_weather_img(&g_weather_info);

	// 	// 标记 HTTP 请求完成事件
	// 	// xEventGroupSetBits(g_sys_event, EVT_HTTP_REQUEST);

	// 	if (!(xEventGroupGetBits(g_sys_event) & EVT_AT_INITED))
	// 	{
	// 		xEventGroupSetBits(g_sys_event, EVT_AT_INITED);
	// 	}

	// 	// 如果 WiFi 还未连接，则显示已连接
	// 	if ((xEventGroupGetBits(g_sys_event) & EVT_WIFI_NEED_CONNECT))
	// 	{
	// 		xEventGroupClearBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
	// 		xEventGroupSetBits(g_sys_event, EVT_WIFI_STATUS);
	// 		u_update_wifi_img();
	// 	}
	// }

	// 标记首页显示完成事件
	xEventGroupSetBits(g_sys_event, EVT_HOMEPAGE_DONE);

	vTaskDelete(NULL);
}
/* 温度格式化接口，多任务可安全调用 */
static void u_check_tmp(int tmp)
{
	if (tmp < -10)
		sprintf(home_page_buf, "-9");
	else if (tmp < 0)
		sprintf(home_page_buf, "%1d", tmp);
	else if (tmp < 10)
		sprintf(home_page_buf, "%02d", tmp);
	else if (tmp >= 100)
		sprintf(home_page_buf, "99");
	else
		sprintf(home_page_buf, "%2d", tmp);
}

/* 室外温度显示 */
void u_update_outdoor_environment(float tmp)
{
	u_check_tmp((int)tmp);
	ST7789_Show(HOME_TMP_X, HOME_TMP_Y, home_page_buf, HOME_TMP_COLOR, HOME_OUTDOOR_COLOR, &FONT44);
	ST7789_Show(HOME_TMP_X + (FONT44.size) + 4, HOME_TMP_Y + 8, "℃", HOME_TMP_COLOR, HOME_OUTDOOR_COLOR, &FONT32);
}

/* 室内温湿度显示 */
void u_update_indoor_environment(float tmp, float humi)
{
	u_check_tmp((int)tmp);
	ST7789_Show(HOME_TMP_INNER_X, HOME_TMP_INNER_Y, home_page_buf, HOME_TMP_INNER_COLOR, HOME_INNER_COLOR, &font44);
	ST7789_Show(HOME_TMP_INNER_X + font44.size + 2, HOME_TMP_INNER_Y + 8, "℃", HOME_TMP_INNER_COLOR, HOME_INNER_COLOR,
	            &FONT32);

	if (humi >= 10 && humi < 100)
		sprintf(home_page_buf, "%2d%%RH", (int)humi);
	else if (humi > 0 && humi < 10)
		sprintf(home_page_buf, "%02d%%RH", (int)humi);
	else
		sprintf(home_page_buf, "00%%RH");

	ST7789_Show(HOME_RH_X, HOME_RH_Y, home_page_buf, HOME_RH_COLOR, HOME_INNER_COLOR, &FONT24);
}

// /* 时间有效性检查 */
// static uint8_t u_check_time(time_t* tm)
// {
// 	return (tm->year <= 9999 && tm->day <= 31 && tm->hour <= 23 && tm->min <= 59 && tm->sec <= 59 && 1 <= tm->week &&
// 	        tm->week <= 7 && 1 <= tm->month && tm->month <= 12);
// }

/* 时分秒显示接口 */
void u_update_hour(uint8_t hour)
{
	sprintf(time_buf, "%02d", hour);
	ST7789_Show(HOME_TIME_X, HOME_TIME_Y, time_buf, HOME_TIME_COLOR, HOME_TIME_BG_COLOR, &FONT48);
}

void u_update_min(uint8_t min)
{
	sprintf(time_buf, "%02d", min);
	ST7789_Show(HOME_TIME_X + 3 * (FONT48.size / 2), HOME_TIME_Y, time_buf, HOME_TIME_COLOR, HOME_TIME_BG_COLOR,
	            &FONT48);
}

void u_update_sec(uint8_t sec)
{
	sprintf(time_buf, "%02d", sec);
	ST7789_Show(HOME_TIME_X + 6 * (FONT48.size / 2), HOME_TIME_Y, time_buf, HOME_TIME_COLOR, HOME_TIME_BG_COLOR,
	            &FONT48);
}

/* 冒号显示 */
void u_update_colon(bool start)
{
	if (start)
	{
		ST7789_Show(HOME_TIME_X + 2 * (FONT48.size / 2), HOME_TIME_Y, ":", HOME_TIME_COLON_COLOR, HOME_TIME_BG_COLOR,
					&FONT48);
		ST7789_Show(HOME_TIME_X + 5 * (FONT48.size / 2), HOME_TIME_Y, ":", HOME_TIME_COLON_COLOR, HOME_TIME_BG_COLOR,
					&FONT48);
	}
	else
	{
		ST7789_Show(HOME_TIME_X + 2 * (FONT48.size / 2), HOME_TIME_Y, ":", HOME_TIME_COLON_COLOR2, HOME_TIME_BG_COLOR,
					&FONT48);
		ST7789_Show(HOME_TIME_X + 5 * (FONT48.size / 2), HOME_TIME_Y, ":", HOME_TIME_COLON_COLOR2, HOME_TIME_BG_COLOR,
					&FONT48);
	}

}

/* 时间更新接口，可多任务调用 */
void u_update_time(time_t* tm)
{
	u_update_sec(tm->sec);
	u_update_min(tm->min);
	u_update_hour(tm->hour);
}

/* 日期显示 */
static const char week[8][10] = { "一", "二", "三", "四", "五", "六", "日" };

void u_update_date(time_t* tm)
{
	sprintf(time_buf, "%04d.%02d.%02d 星期%s", tm->year, tm->month, tm->day, week[tm->week - 1]);
	ST7789_Show(HOME_DATE_X, HOME_DATE_Y, time_buf, HOME_DATE_COLOR, HOME_TIME_BG_COLOR, &FONT22);
}

/* 城市显示 */
void u_update_city(char* city)
{
	ST7789_Show(HOME_LOC_X, HOME_LOC_Y, city, HOME_LOC_COLOR, HOME_OUTDOOR_COLOR, &FONT24);
}

/* WiFi 名称显示 */
void u_update_wifi_name(char* wifi_name)
{
	uint8_t len = strlen(wifi_name);
	if (len > HOME_WIFI_NAME_SIZE)
	{
		strncpy(home_page_buf, wifi_name, HOME_WIFI_NAME_SIZE - 3);
		home_page_buf[HOME_WIFI_NAME_SIZE - 3]   = home_page_buf[HOME_WIFI_NAME_SIZE - 2] =
		  home_page_buf[HOME_WIFI_NAME_SIZE - 1] = '.';
	}
	else if (len == HOME_WIFI_NAME_SIZE)
	{
		strncpy(home_page_buf, wifi_name, HOME_WIFI_NAME_SIZE);
	}
	else
	{
		memset(home_page_buf, ' ', HOME_WIFI_NAME_SIZE - len);
		strncpy(&home_page_buf[HOME_WIFI_NAME_SIZE - len], wifi_name, len);
	}
	home_page_buf[HOME_WIFI_NAME_SIZE] = '\0';
	ST7789_Show(HOME_WIFI_NAME_X, HOME_WIFI_NAME_Y, home_page_buf, HOME_WIFI_NAME_COLOR, HOME_TIME_BG_COLOR, &FONT24);
}

void u_update_wifi_img(void)
{
	static uint8_t last_wifi_connected = 0xFF;  // 初始值非法，保证首次刷新

	EventBits_t bits    = xEventGroupGetBits(g_sys_event);
	uint8_t     current = !(bits & EVT_WIFI_NEED_CONNECT) ? 1 : 0;

	if (current != last_wifi_connected)
	{
		if (current)
			ST7789_Draw_Image(HOME_WIFI_X, HOME_WIFI_Y, &img_wifi_yes);
		else
			ST7789_Draw_Image(HOME_WIFI_X, HOME_WIFI_Y, &img_wifi_no);

		last_wifi_connected = current;
	}
}

/* 温度图标 */
void u_update_tmp_img(weather_info_t* info)
{
	if (info->temp_outdoor < 15)
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_cold);
	else if (info->temp_outdoor < 25)
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_warm);
	else
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_hot);
}

/* 天气图标 */
void u_update_weather_img(weather_info_t* info)
{
	if (info->time.hour && info->time.hour < 2)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_night);
	else if (2 <= info->time.hour && info->time.hour < 5)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_late_night);
	else if (info->weather <= 1)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_sunny);
	else if (4 <= info->weather && info->weather <= 9)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_cloudy);
	else if (info->weather == 14 || info->weather == 13 || info->weather == 11 || info->weather == 10)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_rainy);
	else if (15 <= info->weather && info->weather <= 18)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_heavy_rain);
	else if (20 <= info->weather && info->weather <= 25 || info->weather == 12)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_snowy);
	else
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_unknown);
}

