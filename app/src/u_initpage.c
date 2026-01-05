#include "u_initpage.h"
#include "u_homepage.h"

// extern system_status_t g_s_s;
// extern weather_info_t  g_weather_info;

/**
 * @brief 系统初始化入口
 *
 * @details
 * 初始化显示、日志及传感器模块，并更新系统状态标志。
 */
void u_initpage(void *pvParameters)
{
    (void)pvParameters;
	
	/* TFT LCD HAL 初始化（SPI 默认配置） */
	TFT_LCD_Init(0);

	/* ST7789 显示控制器初始化并显示启动画面 */
	ST7789_Init();
	ST7789_Draw_Image(0, 0, &img_init);

	g_s_s.tft_lcd_initiated = 1;
	update_progress_bar(1);

	/* 日志系统初始化（USART 默认通道） */
	log_init();
	g_s_s.log_initiated = 1;
	update_progress_bar(2);
	/* DHT11 传感器初始化（GPIO 配置见 DHT11.h） */
	DHT11_Init();
	g_s_s.dht11_initiated = 1;
	update_progress_bar(3);

	/* AT 模块初始化（GPIO 配置见 AT.h） */
	if (AT_Init()){
		g_s_s.at_initiated = 1;
	}
	update_progress_bar(4);
	if (AT_WIFI_Connect(SSID, PASSWORD, 0) == AT_WIFI_CONNECTED){
		g_s_s.wifi_connected = 1;
	}
	update_progress_bar(5);
	g_s_s.init_page_over = 1;

	xTaskCreate(u_homepage, "Home", 1024, NULL, 1, NULL);
    /* 自己结束 */
    vTaskDelete(NULL);
}

static void update_progress_bar(uint8_t inum)
{
	// 进度条数值坐标
	// x: 186, y: 228
	// 进度条坐标
	// x: 23, y: 221, width: 188, height: 6
	// 进度条宽度
	uint16_t progress_width = (inum * PROGRESS_BAR_WIDTH) / PROGRESS_NUM;
	ST7789_Fill_Color(PROGRESS_BAR_X1, PROGRESS_BAR_Y1,
	                  PROGRESS_BAR_X1 + progress_width, PROGRESS_BAR_Y2,
	                  PROGRESS_BAR_COLOR);
	// 进度条数值
	// void ST7789_Show_ASCII(uint16_t x, uint16_t y, char ch, uint16_t color,
	//                        uint16_t bg_color, const font_t* font)
	char temp[5];
	sprintf(temp, "%2d%%", (inum * 100) / PROGRESS_NUM);
	ST7789_Show(PROGRESS_NUM_X, PROGRESS_NUM_Y, temp, PROGRESS_NUM_COLOR,
	            BACKGROUND, &font16);
}
