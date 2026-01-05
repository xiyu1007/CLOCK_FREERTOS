#include "FreeRTOS.h"
#include "task.h"
#include "u_header.h"


// 原子变量
uint8_t atomic_flag = 0;

char ssid[128]     = "";
char password[128] = "12345678";

system_status_t g_s_s = { 0 };

weather_info_t  g_weather_info = {
	.temp_outdoor = 0.0,
	.humidity = 0.0,
	.city = "成都",
	.tmp_indoor = 0.0,
	.humidity = 0.0,
	.weather = 0,
	.time = {
		.week = 3,
		.month = 1,
		.sec = 0,
		.min = 0,
		.hour = 0,
		.day = 1,
		.year = 2025,
	},
};
char main_buff[1024] = { 0 };

static uint32_t dht11_counter = 0;
static uint32_t http_counter  = 0;
static uint32_t wifi_counter  = 0;
static uint32_t time_counter  = 0;

void vTaskRun(void *pvParameters)
{
    (void)pvParameters;

    while (1)
	{
        update_system_status();
        update_system();
        vTaskDelay(pdMS_TO_TICKS(TIME_IDLE));
	}
}

int main(void)
{
    xTaskCreate(u_initpage, "Init", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

	while (1)
	{

	}

	// return 0;
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == RESET)
		return;

	if (g_s_s.init_page_over)
	{
		static uint8_t  colon_state     = 0;
		static uint16_t half_second_cnt = 0;
		static uint16_t second_cnt      = 0;

		half_second_cnt++;
		second_cnt++;

		// 500ms任务
		if (half_second_cnt >= 499)
		{
			half_second_cnt  = 0;
			colon_state     ^= 1;
			u_update_colon(colon_state ? HOME_TIME_COLON_COLOR :
			                             HOME_TIME_COLON_COLOR2);
		}
		// 1000ms任务
		if (second_cnt >= 999)
		{
			second_cnt = 0;
			g_weather_info.time.sec++;
			u_update_time(&g_weather_info.time);
			if (!g_s_s.date_updated)
			{
				u_update_date(&g_weather_info.time);
				g_s_s.date_updated = 1;
			}
		}
	}
	g_ms_tick++;
	dht11_counter++;
	http_counter++;
	wifi_counter++;
	time_counter++;
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}

static void update_system_status(void)
{
	uint32_t tick = NOW();
	if (tick != 0)
	{
		// DHT11室内温度更新（每5秒）
		if (dht11_counter >= TIME_GET_TMP_INDOOR)
		{
			dht11_counter       = 0;
			g_s_s.dht11_updated = 0;  // 清除标志，准备重新获取
		}

		// HTTP室外温度更新（每30秒）
		if (http_counter >= TIME_GET_TMP_OUTDOOR)
		{
			http_counter         = 0;
			g_s_s.http_requested = 0;
		}

		// WiFi连接检查（每10秒）
		if (wifi_counter >= TIME_CHECK_WIFI)
		{
			wifi_counter         = 0;
			g_s_s.wifi_connected = (AT_WIFI_Info(SSID) == AT_WIFI_CONNECTED);
		}

		// 时间同步（每60秒）
		if (time_counter >= TIME_GET_TIME)
		{
			time_counter       = 0;
			g_s_s.time_updated = 0;
		}
	}
}

// 根据状态更新系统
static void update_system(void)
{

	if (!g_s_s.dht11_updated)
	{
		DHT11_Get(&g_weather_info.tmp_indoor, &g_weather_info.humidity);
		u_update_indoor_environment(g_weather_info.tmp_indoor,
		                            g_weather_info.humidity);
		g_s_s.dht11_updated = 1;
		dht11_counter       = 0;
		log("DHT11 updated. Temp=%.2f, Humidity=%.2f.",
		    g_weather_info.tmp_indoor, g_weather_info.humidity);
	}

	if (!g_s_s.at_initiated)
	{
		if (AT_Init())
			g_s_s.at_initiated = 1;
	}

	if (g_s_s.wifi_info_change)
	{
		char* p = log_buf_get();
		if (!p)
			return;
		char ssid_buf[128]     = { 0 };
		char password_buf[128] = { 0 };
		if (sscanf(p, "WIFI: \"%127[^\"]\" \"%127[^\"]\"", ssid_buf,
		           password_buf) != 2)
		{
			log("WIFI Info Error. Received SSID='%s', PASSWORD='%s'.", ssid_buf,
			    password_buf);
			log("Correct format: WIFI: \"SSID\" \"PASSWORD\". Example: WIFI: "
			    "\"MyWiFi\" \"12345678\"");
		}
		else
		{
			strcpy(ssid, ssid_buf);
			strcpy(password, password_buf);
			log("WIFI Info Changed. SSID='%s', PASSWORD='%s'", ssid, password);
			g_s_s.wifi_connected = 0;
		}
		g_s_s.wifi_info_change = 0;
	}

	if (!g_s_s.wifi_connected)
	{
		if (AT_WIFI_Connect(SSID, PASSWORD, 0) == AT_WIFI_CONNECTED)
		{
			g_s_s.wifi_connected = 1;
			wifi_counter         = 0;
			u_update_wifi_img(&g_s_s);
			log("WIFI connected: %s.", SSID);
		}
		else
		{
			log("WIFI connect error: %s.", SSID);
		}
	}

	if (!g_s_s.time_updated)
	{
		if (AT_WIFI_Info(SSID) != AT_WIFI_CONNECTED)
		{
			g_s_s.wifi_connected = 0;
			u_update_wifi_img(&g_s_s);
			log("Time update fail NO WIFI: %s.", SSID);
		}
		else
		{
			time_t recv_tm = g_weather_info.time;
			if (AT_Get_Time(&recv_tm))
			{
				__disable_irq();
				g_weather_info.time = recv_tm;
				// g_weather_info.time.sec++;
				u_update_time(&g_weather_info.time);
				u_update_date(&g_weather_info.time);
				TIM4->CNT = 0;
				// g_ms_tick = 0;
				// TIM_SetCounter(TIM4, 0);
				__enable_irq();
				g_s_s.time_updated = 1;
				g_s_s.date_updated = 1;
				time_counter       = 0;
				wifi_counter       = 0;
				log("Time updated. Time=%4d/%02d/%02d %02d:%02d:%02d.",
				    g_weather_info.time.year, g_weather_info.time.month,
				    g_weather_info.time.day, g_weather_info.time.hour,
				    g_weather_info.time.min, g_weather_info.time.sec);
			}
			else
			{
				log("Time update fail AT GET: %s.", SSID);
			}
		}
	}

	if (!g_s_s.http_requested)
	{
		if (AT_WIFI_Info(SSID) != AT_WIFI_CONNECTED)
		{
			g_s_s.wifi_connected = 0;
			u_update_wifi_img(&g_s_s);
			log("HTTP request error NO WIFI: %s.", SSID);
		}
		else if (AT_HTTP_Request(URL, &g_weather_info))
		{
			u_update_outdoor_environment(g_weather_info.temp_outdoor);
			u_update_tmp_img(&g_weather_info);
			u_update_weather_img(&g_weather_info);
			g_s_s.http_requested = 1;
			http_counter         = 0;
			wifi_counter         = 0;
			log("HTTP request success. City=%s, Temp=%.2f, Weather=%2d.",
			    g_weather_info.city, g_weather_info.temp_outdoor,
			    g_weather_info.weather);
		}
	}
}
