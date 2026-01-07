#include "u_header.h"

/*----------------------------- 全局变量 -----------------------------*/
EventGroupHandle_t g_sys_event;
/* AT命令互斥信号量 */
SemaphoreHandle_t  semAT;
/* ST7789互斥量 */
SemaphoreHandle_t  semST7789;
char ssid[128]     = "";
char password[128] = "12345678";

TaskHandle_t xLogTaskHandle = NULL;

weather_info_t  g_weather_info = {
	.temp_outdoor = 0.0,
	.humidity = 0.0,
	.city = "毕节",
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

void vTaskRun_DHT11(void* pvParameters);
void vTaskRun_WIFI(void* pvParameters);
void vTaskRun_AT_Get_Time(void* pvParameters);
void vTaskRun_AT_HTTP(void* pvParameters);
void vTaskRun_Time_tick(void* pvParameters);
void vTaskRun_UI(void* pvParameters);
void vTaskRun_Exception(void* pvParameters);
void vTaskRun_AT_init(void* pvParameters);

void sys_init(void *pvParameters);
extern void vTaskRun_LogRx(void *pvParameters);
extern void u_initpage(void* pvParameters);
extern void u_homepage(void* pvParameters);

void sys_init(void *pvParameters)
{
	(void)pvParameters;
	EventBits_t init_bits = xEventGroupWaitBits(g_sys_event, EVT_HOMEPAGE_DONE,
	                                            pdFALSE,  // 不清除
	                                            pdTRUE,   // 等待所有位
	                                            portMAX_DELAY);

	// 创建任务
    xTaskCreate(vTaskRun_Time_tick, "TimeTick", 512, NULL, BASE_PRIORITY + 7, NULL);
    xTaskCreate(vTaskRun_Exception, "Exception", 512, NULL, BASE_PRIORITY + 7, NULL);

	if (!(xEventGroupGetBits(g_sys_event) & EVT_AT_INITED))
		xTaskCreate(vTaskRun_AT_init, "AT_Init", 512, NULL, BASE_PRIORITY + 6, NULL);

    xTaskCreate(vTaskRun_UI, "UI", 512, NULL, BASE_PRIORITY + 5, NULL);

    xTaskCreate(vTaskRun_LogRx, "LogRx", 512, NULL, BASE_PRIORITY + 4, &xLogTaskHandle);

    xTaskCreate(vTaskRun_WIFI, "WIFI", 512, NULL, BASE_PRIORITY + 3, NULL);
   xTaskCreate(vTaskRun_DHT11, "DHT11", 512, NULL, BASE_PRIORITY + 3, NULL);

    xTaskCreate(vTaskRun_AT_Get_Time, "AT_Time", 512, NULL, BASE_PRIORITY + 2, NULL);
    xTaskCreate(vTaskRun_AT_HTTP, "AT_HTTP", 512, NULL, BASE_PRIORITY + 2, NULL);

    // 初始化完成，自删任务
    vTaskDelete(NULL);
}

int main(void)
{

	/* 系统事件组初始化 */
	g_sys_event = xEventGroupCreate();
	semAT       = xSemaphoreCreateMutex();  // 创建即可用的互斥信号量
	semST7789   = xSemaphoreCreateMutex();
	xTaskCreate(u_initpage, "Init", 1024, NULL, BASE_PRIORITY, NULL);
	xTaskCreate(u_homepage, "Home", 1024, NULL, BASE_PRIORITY, NULL);
	xTaskCreate(sys_init, "sys_init", 1024, NULL, BASE_PRIORITY, NULL);
	vTaskStartScheduler();

	while (1)
	{
	}

	// return 0;
}

/* ---------------------------------所有任务---------------------------------------*/
/*------------------------ 室内环境更新 ------------------------*/
void vTaskRun_DHT11(void* pvParameters)
{
	(void)pvParameters;
	// TickType_t last_wake = xTaskGetTickCount();
	while (1)
	{
		// vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(TIME_DHT11)); // 会last_wake把更新成下一次应该唤醒的时刻
		vTaskDelay(pdMS_TO_TICKS(TIME_DHT11));

		float tmp, humi;
		if (DHT11_Get(&tmp, &humi))
		{
			// 更新状态
			g_weather_info.tmp_indoor = tmp;
			g_weather_info.humidity   = humi;
			log("DHT11: tmp = %.2f, humi = %.2f", tmp, humi);
			xEventGroupSetBits(g_sys_event, EVT_DHT11_UPDATED);
		}
	}
}

/*------------------------ WiFi任务 ------------------------*/
void vTaskRun_WIFI(void* pvParameters)
{
	(void)pvParameters;

	const EventBits_t wifi_wait_bits = EVT_WIFI_NEED_CONNECT;
	static bool is_changed_status = false;
	while (1)
	{
		// 等待 WiFi 相关事件触发（连接成功或信息变化）
		EventBits_t bits = xEventGroupWaitBits(g_sys_event, wifi_wait_bits,
		                                       pdFALSE,  // 等待后清除这些位
		                                       pdFALSE,  // 等待任意一个位
		                                       portMAX_DELAY);

		// xEventGroupClearBits(g_sys_event, EVT_WIFI_STATUS);
		if (xSemaphoreTake(semAT, TIME_SEM_TAKE))
		{
			// 获取 WiFi 状态
			if (AT_WIFI_Connect(ssid, password, NULL) == AT_WIFI_CONNECTED)
			{
				xEventGroupClearBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
				xEventGroupSetBits(g_sys_event, EVT_WIFI_STATUS);
				is_changed_status = false;
			}else{
				if(!is_changed_status){
					xEventGroupSetBits(g_sys_event, EVT_WIFI_STATUS); //如果被唤醒表明断开
					is_changed_status = true;
				}
				log("WIFI disconnect, SSID: %s, PASSWORD: %s", ssid, password);
				vTaskDelay(pdMS_TO_TICKS(TIME_WIFI));  // 休眠一会再试
			}
			xSemaphoreGive(semAT);
		}
	}
}

/*------------------------ AT获取时间任务 ------------------------*/
void vTaskRun_AT_Get_Time(void* pvParameters)
{
	(void)pvParameters;

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(TIME_GET_TIME));

		if (!xSemaphoreTake(semAT, TIME_SEM_TAKE)) continue;

		if (AT_WIFI_Info(ssid) != AT_WIFI_CONNECTED)
		{
			xEventGroupSetBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
			xSemaphoreGive(semAT);
			continue;
		}

		time_t recv_tm = g_weather_info.time;
		if (AT_Get_Time(&recv_tm))
		{
			// if (memcmp(&recv_tm, &g_weather_info.time, sizeof(time_t)) != 0) {
			if (1)
			{
				xEventGroupSetBits(g_sys_event, EVT_TIME_UPDATED);
				xEventGroupSetBits(g_sys_event, EVT_DATE_UPDATED);
				g_weather_info.time = recv_tm;
			}
		}

		xSemaphoreGive(semAT);
	}
}

/*------------------------ AT HTTP任务 ------------------------*/
void vTaskRun_AT_HTTP(void* pvParameters)
{
	while (1)
	{
		if (!xSemaphoreTake(semAT, TIME_SEM_TAKE)) continue;

		// 保证WiFi已连接
		if (AT_WIFI_Info(ssid) != AT_WIFI_CONNECTED)
		{
			xEventGroupSetBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
			xSemaphoreGive(semAT);
			continue;
		}

		if (AT_HTTP_Request(URL, &g_weather_info))
		{
			// 设置事件标志
			xEventGroupSetBits(g_sys_event, EVT_HTTP_REQUEST);
		}

		xSemaphoreGive(semAT);

		vTaskDelay(pdMS_TO_TICKS(TIME_HTTP));
	}
}

/*------------------------ 时间自增 & 冒号 ------------------------*/
void vTaskRun_Time_tick(void* pvParameters)
{
	TickType_t last_wake   = xTaskGetTickCount();
	uint8_t    colon_state = 0;

	while (1)
	{
		vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(500));

		colon_state ^= 1;

		if (colon_state == 0)
		{
			g_weather_info.time.sec++;
			if (g_weather_info.time.sec >= 60)
			{
				g_weather_info.time.sec = 0;
				g_weather_info.time.min++;
			}
			if (g_weather_info.time.min >= 60)
			{
				g_weather_info.time.min = 0;
				g_weather_info.time.hour++;
			}
			if (g_weather_info.time.hour >= 24)
			{
				g_weather_info.time.hour = 0;
				g_weather_info.time.day++;
				g_weather_info.time.week = (g_weather_info.time.week + 1) % 7;
				xEventGroupSetBits(g_sys_event, EVT_DATE_UPDATED);
			}
			xEventGroupSetBits(g_sys_event, EVT_COLON_TOGGLE);
			xEventGroupSetBits(g_sys_event, EVT_TIME_UPDATED);
		}
		else
		{
			xEventGroupSetBits(g_sys_event, EVT_COLON_TOGGLE2);
		}
	}
}

void vTaskRun_UI(void* pvParameters)
{
	const EventBits_t ui_bits = EVT_COLON_TOGGLE | EVT_COLON_TOGGLE2 | EVT_TIME_UPDATED | EVT_DHT11_UPDATED |
	                            EVT_HTTP_REQUEST | EVT_WIFI_STATUS;

	while (1)
	{
		EventBits_t bits = xEventGroupWaitBits(g_sys_event, ui_bits, pdTRUE, pdFALSE, portMAX_DELAY);

		if (xSemaphoreTake(semST7789, portMAX_DELAY))
		{
			if (bits & EVT_COLON_TOGGLE2)
			{
				u_update_colon(1);
			}

			if (bits & EVT_COLON_TOGGLE)
			{
				u_update_colon(0);
			}

			if (bits & EVT_TIME_UPDATED)
			{
				u_update_time(&g_weather_info.time);
			}

			if (bits & EVT_DATE_UPDATED)
			{
				u_update_date(&g_weather_info.time);
			}

			if (bits & EVT_DHT11_UPDATED)
			{
				u_update_indoor_environment(g_weather_info.tmp_indoor, g_weather_info.humidity);
			}

			/* 
			EVT_WIFI_STATUS 由 vTaskRun_WIFI 任务设置，指示状态变化
			*/
			if (bits & (EVT_WIFI_STATUS))
			{
				u_update_wifi_img();
			}

			if (bits & EVT_HTTP_REQUEST)
			{
				u_update_city(g_weather_info.city);
				u_update_outdoor_environment(g_weather_info.temp_outdoor);
				u_update_tmp_img(&g_weather_info);
				u_update_weather_img(&g_weather_info);
			}

			xSemaphoreGive(semST7789);
		}
	}
}

/*------------------------ 异常任务 ------------------------*/
void vTaskRun_Exception(void* pvParameters)
{
	(void)pvParameters;
	while (1)
	{
		// 等待异常事件
		EventBits_t bits = xEventGroupWaitBits(g_sys_event, EVT_EXCEPTION,
		                                       pdTRUE,   // 自动清除
		                                       pdFALSE,  // 等待任意一个
		                                       portMAX_DELAY);
		if (!(bits & EVT_AT_INITED))
		{
			// AT初始化异常
			AT_Reset();
			log("EXCEPTION: AT_INIT failed, triggering reset!");
			vTaskDelay(pdMS_TO_TICKS(3000));  // 延时避免打印过快
		}
	}
}

/*------------------------ AT初始化任务 ------------------------*/
void vTaskRun_AT_init(void* pvParameters)
{
	TickType_t start_tick = xTaskGetTickCount();
	while (1)
	{
		if (xSemaphoreTake(semAT, pdMS_TO_TICKS(100)))
		{
			if (AT_Init())
			{
				// 初始化成功
				xEventGroupSetBits(g_sys_event, EVT_AT_INITED);
				xSemaphoreGive(semAT);
				vTaskDelete(NULL);  // 任务结束
			}
		}

		// 超时检查
		if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS >= TIME_EXCEPTION)
		{
			xEventGroupClearBits(g_sys_event, EVT_AT_INITED);  // 确保异常任务知道未成功
			xEventGroupSetBits(g_sys_event, EVT_EXCEPTION);    // 用作异常通知
			start_tick = xTaskGetTickCount();                  // 重置计时器，持续触发异常
		}

		vTaskDelay(pdMS_TO_TICKS(TIME_CHECK_AT));  // 重试间隔
	}
}

