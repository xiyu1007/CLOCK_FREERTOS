#include "u_initpage.h"
#include "u_homepage.h"

static void update_progress_bar(uint8_t inum);

/**
 * @brief 系统初始化入口
 *
 * @details
 * 初始化显示、日志及传感器模块，并更新系统状态标志。
 */
void u_initpage(void* pvParameters)
{
	(void)pvParameters;

	xEventGroupSetBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
	xEventGroupSetBits(g_sys_event, EVT_HTTP_REQUEST);
	xEventGroupSetBits(g_sys_event, EVT_TIME_UPDATED);

	/* TFT LCD HAL 初始化 */
	TFT_LCD_Init(0);
	/* ST7789 初始化并显示启动画面 */
	ST7789_Init();
	ST7789_Draw_Image(0, 0, &img_init);

	xEventGroupSetBits(g_sys_event, EVT_TFT_INITED | EVT_ST7789_INITED);
	update_progress_bar(1);

	/* 日志系统初始化 */
	log_init();
	xEventGroupSetBits(g_sys_event, EVT_LOG_INITED);
	update_progress_bar(2);

	/* DHT11 传感器初始化（GPIO 配置见 DHT11.h） */
	DHT11_Init();
	xEventGroupSetBits(g_sys_event, EVT_DHT11_INITED);
	update_progress_bar(3);

	/* AT 模块初始化（GPIO 配置见 AT.h） */
	if (AT_Init()) xEventGroupSetBits(g_sys_event, EVT_AT_INITED);

	update_progress_bar(4);

	/* WiFi 连接 */
	if (AT_WIFI_Connect(SSID, PASSWORD, 0) == AT_WIFI_CONNECTED)
	{
		xEventGroupClearBits(g_sys_event, EVT_WIFI_NEED_CONNECT);
		xEventGroupSetBits(g_sys_event, EVT_WIFI_STATUS);
		xEventGroupSetBits(g_sys_event, EVT_AT_INITED);
	}

	update_progress_bar(5);

	/* 初始化流程完成 */
	xEventGroupSetBits(g_sys_event, EVT_INITPAGE_DONE);

	/* Init 任务自毁 */
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
	ST7789_Fill_Color(PROGRESS_BAR_X1, PROGRESS_BAR_Y1, PROGRESS_BAR_X1 + progress_width, PROGRESS_BAR_Y2,
	                  PROGRESS_BAR_COLOR);
	// 进度条数值
	char temp[5];
	sprintf(temp, "%2d%%", (inum * 100) / PROGRESS_NUM);
	ST7789_Show(PROGRESS_NUM_X, PROGRESS_NUM_Y, temp, PROGRESS_NUM_COLOR, BACKGROUND, &font16);
}
