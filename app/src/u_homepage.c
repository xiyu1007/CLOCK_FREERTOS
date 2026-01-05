#include "u_homepage.h"

static char time_buf[64];
static char home_page_buf[256];

void u_homepage(void* pvParameters)
{
	(void)pvParameters;

	ST7789_Draw_Image(0, 0, &img_home);
	ST7789_Show(HOME_INFO_X, HOME_INFO_Y, "室内环境", HOME_INFO_COLOR,
	            HOME_INNER_COLOR, &FONT24);

	u_update_wifi_name(SSID);
	u_update_wifi_img(&g_s_s);

	u_update_time(&g_weather_info.time);
	u_update_colon(HOME_TIME_COLON_COLOR2);
	u_update_date(&g_weather_info.time);

	u_update_city(g_weather_info.city);
	u_update_outdoor_environment(g_weather_info.temp_outdoor);
	u_update_tmp_img(&g_weather_info);
	u_update_weather_img(&g_weather_info);

	if (DHT11_Get(&g_weather_info.tmp_indoor, &g_weather_info.humidity))
	{
		g_s_s.dht11_updated = 1;
	}
	u_update_indoor_environment(g_weather_info.tmp_indoor,
	                            g_weather_info.humidity);

	if (AT_Get_Time(&g_weather_info.time))
	{
		u_update_time(&g_weather_info.time);
		u_update_colon(HOME_TIME_COLON_COLOR2);
		u_update_date(&g_weather_info.time);
		g_s_s.time_updated = 1;

		if (!g_s_s.wifi_connected)
		{
			g_s_s.wifi_connected = 1;
			u_update_wifi_img(&g_s_s);
		}
	}
	if ((AT_HTTP_Request(URL, &g_weather_info)))
	{
		g_s_s.http_requested = 1;
		u_update_city(g_weather_info.city);
		u_update_outdoor_environment(g_weather_info.temp_outdoor);
		u_update_tmp_img(&g_weather_info);
		u_update_weather_img(&g_weather_info);

		if (!g_s_s.wifi_connected)
		{
			g_s_s.wifi_connected = 1;
			u_update_wifi_img(&g_s_s);
		}
	}

	xTaskCreate(vTaskRun, "Run", 1024, NULL, 1, NULL);
	vTaskDelete(NULL);
}

void u_check_tmp(int tmp)
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

void u_update_outdoor_environment(float tmp)
{
	u_check_tmp((int)tmp);
	ST7789_Show(HOME_TMP_X, HOME_TMP_Y, home_page_buf, HOME_TMP_COLOR,
	            HOME_OUTDOOR_COLOR, &FONT44);
	ST7789_Show(HOME_TMP_X + (FONT44.size) + 4, HOME_TMP_Y + (8), "℃",
	            HOME_TMP_COLOR, HOME_OUTDOOR_COLOR, &FONT32);
}

void u_update_indoor_environment(float tmp, float humi)
{
	// 室内温度
	u_check_tmp((int)tmp);
	ST7789_Show(HOME_TMP_INNER_X, HOME_TMP_INNER_Y, home_page_buf,
	            HOME_TMP_INNER_COLOR, HOME_INNER_COLOR, &font44);
	ST7789_Show(HOME_TMP_INNER_X + (font44.size) + 2, HOME_TMP_INNER_Y + (8),
	            "℃", HOME_TMP_INNER_COLOR, HOME_INNER_COLOR, &FONT32);
	// 室内湿度
	if (10 <= humi && humi < 100)
		sprintf(home_page_buf, "%2d%%RH", (int)humi);
	else if (0 < humi && humi < 10)
		sprintf(home_page_buf, "%02d%%RH", (int)humi);
	else
		sprintf(home_page_buf, "00%%RH");

	ST7789_Show(HOME_RH_X, HOME_RH_Y, home_page_buf, HOME_RH_COLOR,
	            HOME_INNER_COLOR, &FONT24);
}

uint8_t u_check_time(time_t* tm)
{
	if (tm->year <= 9999 && tm->day <= 31 && tm->hour <= 23 && tm->min <= 59 &&
	    tm->sec <= 59 && 1 <= tm->week && tm->week <= 7 && 1 <= tm->month &&
	    tm->month <= 12)
	{
		return 1;
	}
	return 0;
}

void u_update_hour(uint8_t hour)
{
	sprintf(time_buf, "%02d", hour);
	ST7789_Show(HOME_TIME_X, HOME_TIME_Y, time_buf, HOME_TIME_COLOR,
	            HOME_TIME_BG_COLOR, &FONT48);
}
void u_update_min(uint8_t min)
{
	sprintf(time_buf, "%02d", min);
	ST7789_Show(HOME_TIME_X + 3 * (FONT48.size / 2), HOME_TIME_Y, time_buf,
	            HOME_TIME_COLOR, HOME_TIME_BG_COLOR, &FONT48);
}

void u_update_sec(uint8_t sec)
{
	sec = sec % 60;
	sprintf(time_buf, "%02d", sec);
	ST7789_Show(HOME_TIME_X + 6 * (FONT48.size / 2), HOME_TIME_Y, time_buf,
	            HOME_TIME_COLOR, HOME_TIME_BG_COLOR, &FONT48);
}

void u_update_colon(uint16_t colon_c)
{
	ST7789_Show(HOME_TIME_X + 2 * (FONT48.size / 2), HOME_TIME_Y, ":", colon_c,
	            HOME_TIME_BG_COLOR, &FONT48);
	ST7789_Show(HOME_TIME_X + 5 * (FONT48.size / 2), HOME_TIME_Y, ":", colon_c,
	            HOME_TIME_BG_COLOR, &FONT48);
}

void u_update_time(time_t* tm)
{
	if (tm->sec >= 60)
	{
		tm->sec = 0;
		tm->min++;
	}
	if (tm->min >= 60)
	{
		tm->min = 0;
		tm->hour++;
	}
	if (tm->hour >= 24)
	{
		tm->hour = 0;
		tm->day++;
		tm->week           = (tm->week + 1) % 7;
		g_s_s.date_updated = 0;
	}
	// if (!u_check_time(tm))
	// {
	// 	g_s_s.time_updated = 0;
	// 	return;
	// }

	u_update_sec(tm->sec);
	u_update_min(tm->min);
	u_update_hour(tm->hour);
}

static const char week[8][10] = { "一", "二", "三", "四", "五", "六", "日" };
void              u_update_date(time_t* tm)
{
	// if (!u_check_time(tm))
	// {
	// 	g_s_s.time_updated = 0;
	// 	return;
	// }
	// 2023/10/16   星期五
	sprintf(time_buf, "%04d.%02d.%02d 星期%s", tm->year, tm->month, tm->day,
	        week[tm->week - 1]);
	ST7789_Show(HOME_DATE_X, HOME_DATE_Y, time_buf, HOME_DATE_COLOR,
	            HOME_TIME_BG_COLOR, &FONT22);
}

void u_update_city(char* city)
{
	ST7789_Show(HOME_LOC_X, HOME_LOC_Y, city, HOME_LOC_COLOR,
	            HOME_OUTDOOR_COLOR, &FONT24);
}

void u_update_wifi_name(char* wifi_name)
{
	uint8_t len = strlen(wifi_name);
	if (len > HOME_WIFI_NAME_SIZE)
	{
		strncpy(home_page_buf, wifi_name, HOME_WIFI_NAME_SIZE - 3);
		home_page_buf[HOME_WIFI_NAME_SIZE - 3] =
		  home_page_buf[HOME_WIFI_NAME_SIZE - 2] =
		    home_page_buf[HOME_WIFI_NAME_SIZE - 1] = '.';
	}
	else if (len == HOME_WIFI_NAME_SIZE)
	{
		strncpy(home_page_buf, wifi_name, HOME_WIFI_NAME_SIZE);
	}
	else
	{
		for (uint8_t i = 0; i < HOME_WIFI_NAME_SIZE - len; i++)
		{
			home_page_buf[i] = ' ';
		}
		strncpy(&home_page_buf[HOME_WIFI_NAME_SIZE - len], wifi_name, len);
	}

	home_page_buf[HOME_WIFI_NAME_SIZE] = '\0';
	ST7789_Show(HOME_WIFI_NAME_X, HOME_WIFI_NAME_Y, home_page_buf,
	            HOME_WIFI_NAME_COLOR, HOME_TIME_BG_COLOR, &FONT24);
}

void u_update_wifi_img(system_status_t* s_s)
{
	if (s_s->wifi_connected)
		ST7789_Draw_Image(HOME_WIFI_X, HOME_WIFI_Y, &img_wifi_yes);
	else
		ST7789_Draw_Image(HOME_WIFI_X, HOME_WIFI_Y, &img_wifi_no);
}

void u_update_tmp_img(weather_info_t* info)
{
	if (info->temp_outdoor < 15)
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_cold);
	else if (info->temp_outdoor < 25)
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_warm);
	else
		ST7789_Draw_Image(HOME_IMG_TMP_X, HOME_IMG_TMP_Y, &img_tmp_hot);
}

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

	else if (info->weather == 14 || info->weather == 13 ||
	         info->weather == 11 || info->weather == 10)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_rainy);

	else if (15 <= info->weather && info->weather <= 18)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_heavy_rain);

	else if (20 <= info->weather && info->weather <= 25 || info->weather == 12)
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_snowy);

	else
		ST7789_Draw_Image(HOME_IMG_WE_X, HOME_IMG_WE_Y, &img_we_unknown);
}
