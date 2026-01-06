#ifndef __U_LOG_H__
#define __U_LOG_H__

#include "main.h"
#include "stm32f4xx.h"
#include "u_timer.h"
#include "u_usart.h"
#include <stdio.h>
#include "stream_buffer.h"
#include <string.h>

#define LOG_USART         SERIAL_USART
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)

#define LOG               printf
#define log(fmt, ...)     LOG("[LOG] " fmt "\r\n", ##__VA_ARGS__)
// #define log(fmt, ...)     LOG("[LOG] FILE: %s LINE: %d\r\n" fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)


#define LOG_BUF_SIZE      128

extern char            log_buf[LOG_BUF_SIZE];
extern weather_info_t  g_weather_info;


void log_init(void);
void vTaskRun_LogRx(void *pvParameters);

#endif /* __U_LOG_H__ */

