#ifndef __U_HOMEPAGE_H__
#define __U_HOMEPAGE_H__

#include "main.h"
#include "AT.h"
#include "DHT11.h"
#include "ST7789.h"
#include "TFT_LCD.h"
#include "u_log.h"
#include "stm32f4xx.h"

#include "font.h"
#include "image.h"

#define HOME_WIFI_X          25
#define HOME_WIFI_Y          16

#define HOME_IMG_TMP_X       131
#define HOME_IMG_TMP_Y       254

#define HOME_IMG_WE_X        174
#define HOME_IMG_WE_Y        254
#define HOME_OUTDOOR_COLOR   MKCOLOR(201, 201, 201)
#define HOME_INNER_COLOR     MKCOLOR(209, 233, 249)
#define HOME_TIME_BG_COLOR   MKCOLOR(248, 248, 248)
// #define HOME_TIME_BG_COLOR   BLACK

#define HOME_TIME_X          23
#define HOME_TIME_Y          56
// #define HOME_TIME_COLOR      WHITE
#define HOME_TIME_COLOR      MKCOLOR(56, 89, 182)
#define HOME_TIME_COLON_COLOR      MKCOLOR(80, 162, 255)
#define HOME_TIME_COLON_COLOR2      MKCOLOR(56, 89, 182)
#define HOME_DATE_X          28
#define HOME_DATE_Y          118
// #define HOME_DATE_COLOR      MKCOLOR(87, 115, 132)
#define HOME_DATE_COLOR      MKCOLOR(80, 162, 255)

#define HOME_LOC_X           134
#define HOME_LOC_Y           177
#define HOME_LOC_COLOR       BLACK
#define HOME_INFO_X          16
#define HOME_INFO_Y          189
#define HOME_INFO_COLOR      BLACK

#define HOME_TMP_X           140
#define HOME_TMP_Y           210
#define HOME_TMP_COLOR       MKCOLOR(80, 131, 182)
#define HOME_TMP_INNER_X     24
#define HOME_TMP_INNER_Y     220
#define HOME_TMP_INNER_COLOR MKCOLOR(87, 115, 132)
#define HOME_RH_X            38
#define HOME_RH_Y            270
#define HOME_RH_COLOR        MKCOLOR(87, 115, 132)

#define HOME_WIFI_NAME_X     (HOME_WIFI_X + 26 + 20)
#define HOME_WIFI_NAME_Y     16
// ³ý×ÖÌå´óÐ¡
#define HOME_WIFI_NAME_SIZE  ((WIDTH - 14 - HOME_WIFI_NAME_X) / 13)
#define HOME_WIFI_NAME_COLOR MKCOLOR(146, 146, 146)

#define FONT22               font22 //
#define FONT24               font24 // 
#define FONT32               font32 //
#define FONT44               font44 //
#define FONT48               font48

void u_homepage(void *pvParameters);

void u_update_wifi_name(char* wifi_name);
void u_update_indoor_environment(float tmp, float humi);
void u_update_outdoor_environment(float tmp);
void u_update_city(char* city);
void u_update_date(time_t* tm);
void u_update_time(time_t* tm);
void u_update_hour(uint8_t hour);
void u_update_min(uint8_t min);
void u_update_sec(uint8_t sec);
void u_update_colon(bool start);
void u_update_wifi_img(void);
void u_update_tmp_img(weather_info_t* info);
void u_update_weather_img(weather_info_t* info);
extern char ssid[128];

#endif /* __U_HOMEPAGE_H__ */

