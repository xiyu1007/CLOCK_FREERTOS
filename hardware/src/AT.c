#include "AT.h"

static uint8_t            at_usartc_ready = 0;
USARTConfig               g_at_usartc;
static char               g_at_buf[AT_RECV_BUF_SIZE];
static char*              buf_ptr   = g_at_buf;
static volatile AT_Status at_status = AT_BUSY;
static volatile uint8_t   at_echo   = 0;

static const AT_StatusMap at_status_map[] = {
	{ "OK",        AT_OK    },
	{ "ready",     AT_READY },
	{ "ERROR",     AT_ERROR },
	{ "busy p...", AT_BUSY  },
};

uint8_t AT_Init(void)
{
	if (!at_usartc_ready)
	{
		usart_default_config(&g_at_usartc);
		g_at_usartc.usartx = AT_USART;
		g_at_usartc.port   = AT_USART_PORT;
		g_at_usartc.tx     = AT_USART_TX;
		g_at_usartc.rx     = AT_USART_RX;
		u_usart_init(&g_at_usartc);
		at_usartc_ready = 1;

		// while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_RXNE) != RESET)
		// 	(void)USART_ReceiveData(g_at_usartc.usartx);
		AT_LOG("AT Init...");
		if (!AT_Wait_Send(AT_RECV_TIMEOUT))
			goto at_init_fail;

//		AT_Send("AT+RESTORE", AT_RECV_TIMEOUT);
//		AT_Parse();
//		AT_Callback();
//		if (!AT_Wait_Status(AT_READY, AT_INIT_TIMEOUT))
//			goto at_init_fail;
	}

	AT_Set_Echo(at_echo);
	// AT_Parse();
	// AT_Callback();
	AT_LOG("AT Initialized.");
	return 1;

at_init_fail:
	AT_LOG("AT Init Failed.");
	return 0;
}

void AT_SendCRLF(void)
{
	while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(g_at_usartc.usartx, (uint8_t)'\r');
	while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(g_at_usartc.usartx, (uint8_t)'\n');
	while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_TC) == RESET)
		;
}

void AT_Set_Echo(uint8_t echo)
{
	if (echo)
		AT_Send("ATE1", AT_RECV_TIMEOUT);
	else
		AT_Send("ATE0", AT_RECV_TIMEOUT);
}

void AT_Recv(uint32_t timeout)
{
	if (!at_usartc_ready)
	{
		AT_LOG("AT Recv: usart not ready.");
		return;
	}
	uint32_t tick = NOW();
	buf_ptr       = g_at_buf;
	while (1)
	{
		if (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_RXNE) != RESET)
		{
			*buf_ptr++ = USART_ReceiveData(g_at_usartc.usartx);
			if (buf_ptr == g_at_buf + AT_RECV_BUF_SIZE - 1)
			{
				AT_LOG("AT Recv: buf full.");
				break;
			}
			// \r\nERROR\r\n
			if (buf_ptr == g_at_buf + 9 && *(buf_ptr - 1) == '\n' &&
			    buf_ptr[-2] == '\r' && buf_ptr[-3] == 'R' &&
			    buf_ptr[-4] == 'O' && buf_ptr[-5] == 'R' &&
			    buf_ptr[-6] == 'R' && buf_ptr[-7] == 'E' &&
			    buf_ptr[-8] == '\n' && buf_ptr[-9] == '\r')
			{
				at_status = AT_ERROR;
				break;
			}
			// OK\r\n
			if (*(buf_ptr - 1) == '\n' && *(buf_ptr - 2) == '\r' &&
			    *(buf_ptr - 3) == 'K' && *(buf_ptr - 4) == 'O')
			{
				at_status = AT_OK;
				break;
			}
			tick = NOW();
		}
		if (IS_TIMEOUT(tick, timeout))
		{
			break;
		}
	}
	*buf_ptr = '\0';

#ifdef __AT_DEBUG__
	AT_Callback();
#endif
	// buf_ptr  = g_at_buf;
}

void AT_Send(const char* str, uint32_t timeout)
{
	if (!at_usartc_ready)
	{
		AT_LOG("AT Send: usart not ready.");
		return;
	}

	const char* p = str;
	while (*p != '\0')
	{
		while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_TXE) == RESET)
			;
		USART_SendData(g_at_usartc.usartx, (uint8_t)(*p++));
	}
	AT_SendCRLF();
	if (timeout)
		AT_Recv(timeout);
	else
		AT_Recv(AT_RECV_TIMEOUT);
}

AT_WIFI_Status AT_WIFI_Connect(char* ssid, const char* password,
                               const char* mac)
{
	uint8_t len = 0;
	if (AT_Is_Busy())
		goto go_at_wifi_busy;

	AT_Send("AT+CWMODE=1", AT_RECV_TIMEOUT);
	AT_Parse();
	if (at_status != AT_OK)
		goto go_at_wifi_err;

	if (AT_WIFI_Info(ssid) == AT_WIFI_CONNECTED)
		goto go_at_wifi_connected;

	if (AT_Is_Busy())
		goto go_at_wifi_busy;

	buf_ptr = g_at_buf;
	len = snprintf(g_at_buf, sizeof(g_at_buf), "AT+CWJAP=\"%s\",\"%s\"", ssid,
	               password);
	if (mac)
		snprintf(buf_ptr + len, sizeof(g_at_buf) - len, ",\"%s\"", mac);

	AT_Send(buf_ptr, AT_RECV_TIMEOUT);
	if (AT_Wait_WIFI_Connected(ssid, AT_WIFI_TIMEOUT))
		goto go_at_wifi_connected;

go_at_wifi_err:
	// AT_LOG("WIFI Connect Error: %s.", ssid);
	return AT_WIFI_ERROR;
go_at_wifi_connected:
	// AT_LOG("WIFI Connect: %s.", ssid);
	return AT_WIFI_CONNECTED;
go_at_wifi_busy:
	// AT_LOG("WIFI Connect Busy: %s.", ssid);
	return AT_WIFI_BUSY;
}

uint8_t AT_Wait_WIFI_Connected(char* sid, uint32_t timeout)
{
	uint32_t tick = NOW();
	while (AT_WIFI_Info(sid) != AT_WIFI_CONNECTED)
	{
		if (IS_TIMEOUT(tick, timeout))
		{
			return 0;
		}
	}
	return 1;
}

AT_WIFI_Status AT_WIFI_Info(char* sid)
{
	if (AT_Is_Busy())
		return AT_WIFI_BUSY;
	AT_WIFI_Status status = AT_WIFI_UNKNOWN;

	AT_Send("AT+CWSTATE?", AT_RECV_TIMEOUT);

	char* p = strstr(g_at_buf, "CWSTATE:");
	if (!p)
		return AT_WIFI_UNKNOWN;

	char parsed_sid[256];
	if (sscanf(p, "CWSTATE:2,\"%256[^\"]\"", parsed_sid) == 1)
	{
		if (sid[0] == '\0')
		{
			strcpy(sid, parsed_sid);
			status = AT_WIFI_CONNECTED;
		}
		else if (strcmp(parsed_sid, sid) == 0)
		{
			status = AT_WIFI_CONNECTED;
		}
	}
	return status;
}

uint8_t AT_Is_Busy(void)
{
	if (at_status != AT_BUSY)
	{
		return 0;
	}

	AT_Send("AT", AT_RECV_TIMEOUT);
	AT_Parse();
	if (at_status == AT_BUSY)
	{
		return 1;
	}
	return 0;
}

uint8_t AT_HTTP_Request(const char* url, weather_info_t* info)
{
	if (AT_Is_Busy())
		return 0;
	// 下载 HTTP 资源
	AT_LOG("HTTP Request.");
	at_status = AT_IDLE;
	uint8_t len =
	  snprintf(g_at_buf, sizeof(g_at_buf), "AT+HTTPCLIENT=2,1,\"%s\",,,2", url);
	AT_Send(g_at_buf, AT_HTTP_TIMEOUT);
	if (at_status != AT_OK)  // AT_Recv置位 OK
		return 0;

	if (!parse_weather(info))
		return 0;

	AT_LOG(
	  "city: %s, province: %s, weather: %2d, temperature: %3.1f, update: %s",
	  info->city, info->province, info->weather, info->temp_outdoor,
	  info->update);
	return 1;
}

uint8_t AT_Wait_Status(AT_Status status, uint32_t timeout)
{

	AT_Recv(timeout);
	AT_Parse();
	if (at_status == status)
		return 1;
	return 0;
}

uint8_t AT_Wait_Send(uint32_t timeout)
{

	uint32_t tick = NOW();
	while (AT_Is_Busy() || at_status != AT_OK)
	{
		at_status = AT_BUSY;
		if (IS_TIMEOUT(tick, timeout))
		{
			return 0;
		}
	}
	return 1;
}

uint8_t AT_Is_Echo(void)
{
	AT_Send("AT", AT_RECV_TIMEOUT);
	if (strcmp(g_at_buf, "\r\nOK\r\n") == 0)
		return 0;
	return 1;
}

void AT_Callback(void)
{
	AT_LOG("%s", g_at_buf);
}

AT_Status AT_Parse(void)
{
	/* 2. 缓冲区为空 / 数据过短 */
	if (g_at_buf[0] == '\0' || buf_ptr <= g_at_buf + 2)
		return AT_INCOMPLETE;

	/* 3. 缓冲区满但未遇到 CRLF */
	if ((buf_ptr == g_at_buf + AT_RECV_BUF_SIZE - 1) && buf_ptr[-1] != '\n' &&
	    buf_ptr[-2] != '\r')
	{
		return AT_BUF_FULL;
	}

	/* 1. 判断是否 AT 回显 */
	at_echo = (g_at_buf[0] == 'A' && g_at_buf[1] == 'T');

	at_status    = AT_UNKNOWN;
	char*  start = g_at_buf;
	char*  end   = buf_ptr;
	size_t i;

	end  -= 2;
	*end  = '\0';

	if (at_echo)
	{
		while (!(*start == '\r' && *(start + 1) == '\n') && (start < end - 2))
			start++;
		start += 2;
	}

	if ((*start == '\r' && *(start + 1) == '\n') && (start < end - 2))
		start += 2;

	if (start == end)
		return at_status;

	/* 4. 表驱动匹配 */
	if (start != g_at_buf)
		memmove(g_at_buf, start, end - start + 1);

	/* 8. 表驱动匹配 */
	for (i = 0; i < AT_STATUS_MAP_SIZE; ++i)
	{
		if (strcmp(g_at_buf, at_status_map[i].str) == 0)
		{
			at_status = at_status_map[i].status;
			break;
		}
	}
	if (at_status == AT_UNKNOWN && strstr(g_at_buf, "OK"))
		at_status = AT_OK;

	return at_status;
}

static uint8_t parse_weather(weather_info_t* info)
{

	/*
	+HTTPCLIENT:266,{"results":[{"location":{"id":"WM6N2PM3WY2K","name":"成都",
	"country":"CN","path":"成都,成都,四川,中国","timezone":"Asia/Shanghai",
	"timezone_offset":"+08:00"},"now":{"text":"多云","code":"4","temperature":"10"},
	"last_update":"2025-12-23T21:20:21+08:00"}]}
	
	*/
	char* p = g_at_buf;
	char  path[64];

	if (!json_next_string(&p, "\"name\"", info->city, sizeof(info->city)))
		return 0;

	if (!json_next_string(&p, "\"path\"", path, sizeof(path)))
		return 0;

	extract_province_from_path(path, info->province, sizeof(info->province));

	if (!json_next_string(&p, "\"code\"", path, sizeof(path)))
		return 0;

	info->weather = atoi(path);

	if (!json_next_string(&p, "\"temperature\"", path, sizeof(path)))
		return 0;

	info->temp_outdoor = atof(path);

	if (!json_next_string(&p, "\"last_update\"", info->update,
	                      sizeof(info->update)))
		return 0;

	return 1;
}

static void extract_province_from_path(const char* path, char* province,
                                       size_t len)
{
	char buf[64];
	strncpy(buf, path, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	char* last = strrchr(buf, ',');
	if (!last)
	{
		province[0] = '\0';
		return;
	}

	*last = '\0'; /* 去掉最后一段（中国） */

	char* second = strrchr(buf, ',');
	if (second)
		second++;
	else
		second = buf;

	strncpy(province, second, len - 1);
	province[len - 1] = '\0';
}

static int json_next_string(char** pp, const char* key, char* out,
                            size_t out_len)
{
	char* p = strstr(*pp, key);
	if (!p)
		return 0;

	/* 跳到 value 的第一个 " */
	p = strchr(p, ':');
	if (!p)
		return 0;

	p = strchr(p, '"');
	if (!p)
		return 0;
	p++; /* value 起始 */

	char* end = strchr(p, '"');
	if (!end)
		return 0;

	size_t len = (size_t)(end - p);
	if (len >= out_len)
		len = out_len - 1;

	memcpy(out, p, len);
	out[len] = '\0';

	/* 推进解析指针 */
	*pp = end + 1;
	return 1;
}

uint8_t AT_Get_Time(time_t* tm)
{

	if (AT_Is_Busy())
		return 0;

	AT_Send("AT+CIPSNTPCFG=1,8", AT_RECV_TIMEOUT);
	AT_Parse();
	if (at_status != AT_OK)
		return 0;

	AT_Send("AT+CIPSNTPTIME?", AT_RECV_TIMEOUT);
	if (!parse_time(tm))
	{
		at_status = AT_ERROR;
		return 0;
	}
	AT_Show_Time(tm);
	return 1;
}
void AT_Show_Time(time_t* tm)
{
	// 打印格式: 年月日 时:分:秒
	AT_LOG("%04d-%2d-%02d %02d:%02d:%02d", tm->year, tm->month, tm->day,
	       tm->hour, tm->min, tm->sec);
}

static uint8_t parse_time(time_t* t_tm)
{
	// +CIPSNTPTIME:Mon Dec 22 01:47:53 2025
	char* p = g_at_buf;
	// 跳过前缀
	p       = strstr(p, "+CIPSNTPTIME:");
	if (!p)
		return 0;
	p += strlen("+CIPSNTPTIME:");  // 跳过 "+CIPSNTPTIME:"
	// 使用 sscanf 解析
	char week[16];
	char month[16];
	if (sscanf(p, "%15[^ ] %15[^ ] %hhu %hhu:%hhu:%hhu %hu", week, month,
	           &t_tm->day, &t_tm->hour, &t_tm->min, &t_tm->sec,
	           &t_tm->year) != 7)
	{
		return 0;
	}
	// 转换月份
	// 从英文简写转换
	switch (month[0])
	{
		case 'J': // Jan, Jun, Jul
			if (month[1] == 'a')       t_tm->month = 1; // Jan
			else if (month[2] == 'n')  t_tm->month = 6; // Jun
			else                        t_tm->month = 7; // Jul
			break;
		case 'F': t_tm->month = 2; break; // Feb
		case 'M': t_tm->month = (month[2] == 'r') ? 3 : 5; break; // Mar / May
		case 'A': t_tm->month = (month[1] == 'p') ? 4 : 8; break; // Apr / Aug
		case 'S': t_tm->month = 9; break;  // Sep
		case 'O': t_tm->month = 10; break; // Oct
		case 'N': t_tm->month = 11; break; // Nov
		case 'D': t_tm->month = 12; break; // Dec
		default: t_tm->month = 0; break;   // 非法
	}

	// 转换星期
	if (week[0] == 'M')               // Mon
		t_tm->week = 1;
	else if (week[0] == 'T')
		t_tm->week = (week[1] == 'u') ? 2 : 4; // Tue / Thu
	else if (week[0] == 'W')          // Wed
		t_tm->week = 3;
	else if (week[0] == 'F')          // Fri
		t_tm->week = 5;
	else if (week[0] == 'S')
		t_tm->week = (week[1] == 'a') ? 6 : 7; // Sat / Sun
	else
		t_tm->week = 0;               // 非法

	return 1;
}
