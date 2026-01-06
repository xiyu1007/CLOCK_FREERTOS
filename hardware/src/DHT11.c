#include "DHT11.h"

/* ================= 内部 GPIO 描述 ================= */

static GPIO_InitTypeDef dht11_gpio;

static uint8_t DHT11_Read(uint8_t *data);
static uint8_t DHT11_Parse(uint8_t *data, float *temp, float *humi);

/* ================= GPIO 模式切换 ================= */

static inline void DHT11_SetOutput(void)
{
	dht11_gpio.GPIO_Pin   = DHT11_PIN;
	dht11_gpio.GPIO_Mode  = GPIO_Mode_OUT;
	dht11_gpio.GPIO_OType = GPIO_OType_PP;
	dht11_gpio.GPIO_PuPd  = GPIO_PuPd_UP;
	dht11_gpio.GPIO_Speed = GPIO_Medium_Speed;
	GPIO_Init(DHT11_PORT, &dht11_gpio);
}

static inline void DHT11_SetInput(void)
{
	dht11_gpio.GPIO_Pin   = DHT11_PIN;
	dht11_gpio.GPIO_Mode  = GPIO_Mode_IN;
	dht11_gpio.GPIO_PuPd  = GPIO_PuPd_UP;
	dht11_gpio.GPIO_Speed = GPIO_Medium_Speed;
	GPIO_Init(DHT11_PORT, &dht11_gpio);
}

/* ================= 初始化 ================= */
void DHT11_Init(void)
{
	BSP_GPIO_EnableClock(DHT11_PORT);
	GPIO_StructInit(&dht11_gpio);

	DHT11_SetOutput();
	DHT11_SDA_H;
	u_delay_ms(100);
}

/* ================= 对外接口 ================= */
uint8_t DHT11_Get(float *temp, float *humi)
{
	uint8_t raw[5];

	if (DHT11_Read(raw) != FLAG_DHT11_OK)
		return 0;

	if (DHT11_Parse(raw, temp, humi) != FLAG_DHT11_OK)
		return 0;

	return 1;
}

/* ================= 等待电平变化（带超时） ================= */
/*
 * 等待引脚“跳变为 != level”
 * 成功返回 FLAG_DHT11_OK
 */
static uint8_t DHT11_WaitLevel(uint8_t level, uint32_t timeout_us)
{
	uint32_t tick = NOW();

	while (DHT11_SDA_READ == level)
	{
		if (IS_TIMEOUT(tick, timeout_us))
			return FLAG_DHT11_ERROR;
	}
	return FLAG_DHT11_OK;
}

/* ================= 读取 40 bit 原始数据 ================= */
static uint8_t DHT11_Read(uint8_t *data)
{
	uint8_t i;

	memset(data, 0, 5);

	/* 1. 主机起始信号 */
	DHT11_SetOutput();
	DHT11_SDA_L;
	u_delay_ms(DHT11_START_TIME);
	DHT11_SDA_H;
	u_delay_us(DHT11_START_END_TIME);
	DHT11_SetInput();

	/* 2. 传感器响应 */
	if (DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US))
	{
		DHT11_LOG("Response timeout");
		return FLAG_DHT11_ERROR;
	}
	if (DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US))
	{
		DHT11_LOG("Response low timeout");
		return FLAG_DHT11_ERROR;
	}

	/* 3. 接收 40 bit 数据 */
	for (i = 0; i < DHT11_BITS; i++)
	{
		/* 等待起始低电平结束 */
		if (DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US))
		{
			DHT11_LOG("Bit start low timeout");
			return FLAG_DHT11_ERROR;
		}

		/* 等待进入高电平 */
		if (DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US))
		{
			DHT11_LOG("Bit start high timeout");
			return FLAG_DHT11_ERROR;
		}

		/* 在高电平中段采样 */
		u_delay_us(DHT11_BIT_SAMPLE_US);

		if (DHT11_SDA_READ == Bit_SET)
			data[i / 8] |= (0x80 >> (i % 8));
	}

	/* 4. 释放总线 */
	DHT11_SetOutput();
	DHT11_SDA_H;

	return FLAG_DHT11_OK;
}

/* ================= 数据解析 ================= */
static uint8_t DHT11_Parse(uint8_t *data, float *temp, float *humi)
{
	uint8_t checksum = data[0] + data[1] + data[2] + data[3];

	if (checksum != data[4])
		return FLAG_DHT11_ERROR;

	*humi = data[0] + data[1] * 0.1f;
	*temp = data[2] + data[3] * 0.1f;

	if (data[3] & 0x80)
		*temp = -(*temp);

	return FLAG_DHT11_OK;
}

