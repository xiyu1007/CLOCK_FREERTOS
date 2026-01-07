#ifndef __U_INITPAGE_H__
#define __U_INITPAGE_H__

#include "main.h"
#include "AT.h"
#include "DHT11.h"
#include "ST7789.h"
#include "TFT_LCD.h"
#include "u_log.h"
#include "stm32f4xx.h"

#include "font.h"
#include "image.h"

// 进度条数值坐标
// x: 186, y: 228
// 进度条坐标
// x: 23, y: 221, width: 188, height: 6
#define PROGRESS_BAR_WIDTH  188 // 进度条宽度
#define PROGRESS_BAR_HEIGHT 5 // 进度条高度
#define PROGRESS_BAR_X1     23 // 进度条起始 x 坐标
#define PROGRESS_BAR_Y1     221 // 进度条起始 y 坐标
#define PROGRESS_BAR_X2     (PROGRESS_BAR_X1 + PROGRESS_BAR_WIDTH)
#define PROGRESS_BAR_Y2     (PROGRESS_BAR_Y1 + PROGRESS_BAR_HEIGHT)
#define PROGRESS_BAR_COLOR  MKCOLOR(1, 157, 255)

#define PROGRESS_NUM_X     186 // 进度条数值 x 坐标
#define PROGRESS_NUM_Y     230 // 进度条数值 y 坐标
#define PROGRESS_NUM_COLOR  MKCOLOR(180, 180, 180)
#define PROGRESS_NUM        5 // 进度条任务数
#ifndef BACKGROUND
#define BACKGROUND          BLACK
#endif

void u_initpage(void *pvParameters);


#endif /* __U_INITPAGE_H__ */

