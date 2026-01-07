#include "AT.h"

/* ================= 内部状态 ================= */

static uint8_t   at_ready  = 0;
static uint8_t   at_echo   = 0;
static AT_Status at_status = AT_IDLE;

USARTConfig g_at_usartc;
static char g_at_buf[AT_RECV_BUF_SIZE];

/* ================= AT 状态映射 ================= */

typedef struct
{
	const char* str;
	AT_Status   status;
} AT_StatusMap;

static const AT_StatusMap at_status_map[] = {
	{ "OK",        AT_OK    },
	{ "ready",     AT_READY },
	{ "ERROR",     AT_ERROR },
	{ "busy p...", AT_BUSY  },
};


/* ================= 内部函数声明 ================= */
static void      at_usart_send_str(const char* s);
static void      AT_Send(const char *cmd);
static void      AT_Recv(uint32_t timeout);
static uint8_t AT_Transceive(const char* cmd, uint32_t timeout);
static void      AT_SendCRLF(void);
static uint8_t AT_Wait_Send(uint32_t timeout);
static uint8_t   AT_Is_Busy(void);
static AT_Status AT_Parse(void);

static int     json_next_string(char** pp, const char* key, char* out, size_t out_len);
static void    extract_province_from_path(const char* path, char* province, size_t len);
static uint8_t parse_weather(weather_info_t* info);
static uint8_t parse_time(time_t* t_tm);
static void AT_Show_Time(time_t* tm);


/* ================= AT 初始化 ================= */
uint8_t AT_Init(void)
{
	if (at_ready)
		return 1;

	usart_default_config(&g_at_usartc);
	g_at_usartc.usartx = AT_USART;
	g_at_usartc.port   = AT_USART_PORT;
	g_at_usartc.tx     = AT_USART_TX;
	g_at_usartc.rx     = AT_USART_RX;
	u_usart_init(&g_at_usartc);

	at_ready = 1;

	if (!AT_Wait_Send(AT_INIT_TIMEOUT))
	{
		AT_LOG("AT init failed");
		return 0;
	}

	if (at_echo)
		AT_Transceive("ATE1", AT_RECV_TIMEOUT);
	else
		AT_Transceive("ATE0", AT_RECV_TIMEOUT);

	AT_LOG("AT initialized");
	return 1;
}

void AT_Reset(void)
{
	AT_Send("AT+RST");
}

/* ================= WiFi ================= */
AT_WIFI_Status AT_WIFI_Info(char* ssid)
{
	if (AT_Is_Busy())
		return AT_WIFI_BUSY;

	AT_Send("AT+CWSTATE?");
	AT_Recv(AT_RECV_TIMEOUT);

	char* p = strstr(g_at_buf, "CWSTATE:");  // p 指向 C
	if (!p)
		return AT_WIFI_UNKNOWN;

	char parsed[64];
	if (sscanf(p, "CWSTATE:2,\"%63[^\"]\"", parsed) == 1)
	{
		if (ssid[0] == '\0' || strcmp(parsed, ssid) == 0)
		{
			strcpy(ssid, parsed);
			return AT_WIFI_CONNECTED;
		}
	}
	return AT_WIFI_UNKNOWN;
}

AT_WIFI_Status AT_WIFI_Connect(char* ssid, const char* password, const char* mac)
{
	if (AT_Is_Busy())
		return AT_WIFI_BUSY;

	if (AT_WIFI_Info(ssid) == AT_WIFI_CONNECTED)
		return AT_WIFI_CONNECTED;

	if (!AT_Transceive("AT+CWMODE=1", AT_RECV_TIMEOUT))
		return AT_WIFI_ERROR;

	snprintf(g_at_buf, sizeof(g_at_buf), mac ? "AT+CWJAP=\"%s\",\"%s\",\"%s\"" : "AT+CWJAP=\"%s\",\"%s\"", ssid,
	         password, mac);

	if (!AT_Transceive(g_at_buf, AT_WIFI_TIMEOUT))
		return AT_WIFI_ERROR;

	return AT_WIFI_CONNECTED;
}

/* ================= HTTP ================= */
uint8_t AT_HTTP_Request(const char* url, weather_info_t* info)
{
	if (AT_Is_Busy())
		return 0;

	snprintf(g_at_buf, sizeof(g_at_buf), "AT+HTTPCLIENT=2,1,\"%s\",,,2", url);

	if (!AT_Transceive(g_at_buf, AT_HTTP_TIMEOUT))
		return 0;

	AT_LOG("AT_HTTP: \r\n%s", g_at_buf);
	return parse_weather(info);
}

/* ================= 时间 ================= */
uint8_t AT_Get_Time(time_t* tm)
{
	if (AT_Is_Busy())
		return 0;

	if (!AT_Transceive("AT+CIPSNTPCFG=1,8", AT_RECV_TIMEOUT))
		return 0;

	AT_Send("AT+CIPSNTPTIME?");
	AT_Recv(AT_RECV_TIMEOUT);

	if (!parse_time(tm))
		return 0;

	AT_Show_Time(tm);
	return 1;
}

static void AT_Show_Time(time_t* tm)
{
	AT_LOG("%04d-%02d-%02d %02d:%02d:%02d", tm->year, tm->month, tm->day, tm->hour, tm->min, tm->sec);
}

/* ================= USART 底层 ================= */
static void at_usart_send_str(const char* s)
{
	while (*s){
		while (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_TXE) == RESET){
			;
		}
		USART_SendData(g_at_usartc.usartx, (uint8_t)*s++);
	}
}

/* ================= AT 基础操作 ================= */
static void AT_SendCRLF(void)
{
    at_usart_send_str("\r\n");
}

static void AT_Send(const char* cmd)
{
	if (!at_ready)
		return;

	at_usart_send_str(cmd);
	AT_SendCRLF();
}

/* 唯一阻塞点（将来 RTOS 用 Queue/StreamBuffer 替换） */
static void AT_Recv(uint32_t timeout)
{
	uint32_t tick = NOW();
	char*    p    = g_at_buf;

//	memset(g_at_buf, 0, sizeof(g_at_buf));

	while (!IS_TIMEOUT(tick, timeout))
	{
		if (USART_GetFlagStatus(g_at_usartc.usartx, USART_FLAG_RXNE) != RESET)
		{
			*p++ = USART_ReceiveData(g_at_usartc.usartx);
			if (p >= g_at_buf + AT_RECV_BUF_SIZE - 1)
			{
				at_status = AT_BUF_FULL;
				break;
			}
			// \r\nERROR\r\n
			if (p == g_at_buf + 9 && *(p - 1) == '\n' && p[-2] == '\r' && p[-3] == 'R' && p[-4] == 'O' &&
			    p[-5] == 'R' && p[-6] == 'R' && p[-7] == 'E' && p[-8] == '\n' && p[-9] == '\r')
			{
				at_status = AT_ERROR;
				break;
			}
			// OK\r\n
			if (p - g_at_buf > 4 && *(p - 1) == '\n' && *(p - 2) == '\r' && *(p - 3) == 'K' && *(p - 4) == 'O')
			{
				at_status = AT_OK;
				break;
			}
			tick = NOW();
		}
	}
    *p = '\0';
}

/* 统一：发送 + 接收 + 解析 */
static uint8_t AT_Transceive(const char* cmd, uint32_t timeout)
{
	at_status = AT_BUSY;
	AT_Send(cmd);
	AT_Recv(timeout);
	AT_Parse();
	return (at_status == AT_OK);
}

/* ================= 状态判断 ================= */
static uint8_t AT_Is_Busy(void)
{
	if (at_status != AT_BUSY)
		return 0;

	AT_Transceive("AT", AT_RECV_TIMEOUT);

	return (at_status == AT_BUSY);
}

static uint8_t AT_Wait_Send(uint32_t timeout)
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

/* ================= AT 解析 ================= */
static AT_Status AT_Parse(void)
{
	if (g_at_buf[0] == '\0')
		return AT_INCOMPLETE;

	at_status = AT_UNKNOWN;

	for (size_t i = 0; i < sizeof(at_status_map) / sizeof(at_status_map[0]); i++)
	{
		if (strstr(g_at_buf, at_status_map[i].str))
		{
			at_status = at_status_map[i].status;
			break;
		}
	}

	// if (at_status == AT_UNKNOWN && strstr(g_at_buf, "OK"))
	// 	at_status = AT_OK;

	return at_status;
}


static int json_next_string(char** pp, const char* key, char* out, size_t out_len)
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

static void extract_province_from_path(const char* path, char* province, size_t len)
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

static uint8_t parse_weather(weather_info_t* info)
{
	/*
	+HTTPCLIENT:266,{"results":[{"location":{"id":"WM6N2PM3WY2K","name":"成都",
	"country":"CN","path":"成都,成都,四川,中国","timezone":"Asia/Shanghai",
	"timezone_offset":"+08:00"},"now":{"text":"多云","code":"4","temperature":"10"},
	"last_update":"2025-12-23T21:20:21+08:00"}]}
	
	*/
	char* p = g_at_buf;
	char  tmp[64];

	if (!json_next_string(&p, "\"name\"", info->city, sizeof(info->city)))
		return 0;

	if (!json_next_string(&p, "\"path\"", tmp, sizeof(tmp)))
		return 0;

	extract_province_from_path(tmp, info->province, sizeof(info->province));

	if (!json_next_string(&p, "\"code\"", tmp, sizeof(tmp)))
		return 0;

	info->weather = atoi(tmp);

	if (!json_next_string(&p, "\"temperature\"", tmp, sizeof(tmp)))
		return 0;

	info->temp_outdoor = atof(tmp);

	if (!json_next_string(&p, "\"last_update\"", info->update, sizeof(info->update)))
		return 0;

	return 1;
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

