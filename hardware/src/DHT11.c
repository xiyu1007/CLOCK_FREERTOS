#include "DHT11.h"

static GPIO_InitTypeDef DHT11_GPIO;

/* ===== GPIO 模式切换 ===== */
static void DHT11_OUT(void)
{
    DHT11_GPIO.GPIO_Pin   = DHT11_PIN;
    DHT11_GPIO.GPIO_Mode  = GPIO_Mode_OUT;
    DHT11_GPIO.GPIO_OType = GPIO_OType_PP;
    DHT11_GPIO.GPIO_PuPd  = GPIO_PuPd_UP;
    DHT11_GPIO.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(DHT11_PORT, &DHT11_GPIO);
}

static void DHT11_IN(void)
{
    DHT11_GPIO.GPIO_Pin   = DHT11_PIN;
    DHT11_GPIO.GPIO_Mode  = GPIO_Mode_IN;
    DHT11_GPIO.GPIO_PuPd  = GPIO_PuPd_UP;
    DHT11_GPIO.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(DHT11_PORT, &DHT11_GPIO);
}

/* ===== 初始化 ===== */
void DHT11_Init(void)
{
    // RCC_AHB1PeriphClockCmd(DHT11_RCC, ENABLE);
    BSP_GPIO_EnableClock(DHT11_PORT);
    GPIO_StructInit(&DHT11_GPIO);

    DHT11_OUT();
    DHT11_SDA_H;
    u_delay_ms(100);
}

/* ===== 等待电平变化（带超时） ===== */
static uint8_t DHT11_WaitLevel(uint8_t level, uint32_t timeout_us)
{
    uint32_t tick = NOW();
    while (DHT11_SDA_READ == level) {
        if(IS_TIMEOUT(tick, timeout_us)){
            return FLAG_DHT11_ERROR;
        }
    }
    return FLAG_DHT11_OK;
}

/* ===== 读取原始 40 bit 数据 ===== */
uint8_t DHT11_Read(uint8_t *data)
{
    uint8_t i;

    memset(data, 0, 5);

    /* 1. 起始信号 */
    DHT11_OUT();
    DHT11_SDA_L;
    u_delay_ms(DHT11_START_TIME);
    DHT11_SDA_H;
    u_delay_us(DHT11_START_END_TIME);
    DHT11_IN();

    /* 2. DHT11 响应 */
    if (DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US)) {
        DHT11_LOG("DHT11 response timeout, check wiring!"); // DHT11 响应超时,检查连接是否正常
        return FLAG_DHT11_ERROR;
    }
    if (DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US)) {
        DHT11_LOG("DHT11 response reset timeout, check wiring!"); // DHT11 响应重置超时,检查连接是否正常
        return FLAG_DHT11_ERROR;
    }

    /* 3. 读取 40 bit */
    for (i = 0; i < DHT11_BITS; i++) {

        /* 等待 发送起始低信号*/
        if (DHT11_WaitLevel(Bit_SET, DHT11_TIMEOUT_US)){
            DHT11_LOG("Start-low signal timeout, check wiring!"); // 等待起始低信号超时,检查连接是否正常
            return FLAG_DHT11_ERROR;
        }

        /* 等待起始低信号结束（进入高电平） */
        if (DHT11_WaitLevel(Bit_RESET, DHT11_TIMEOUT_US)) {
            DHT11_LOG("Start-high signal timeout, check wiring!"); // 等待起始高信号超时,检查连接是否正常
            return FLAG_DHT11_ERROR;
        }

        /* 在高电平中段采样 */
        u_delay_us(DHT11_BIT_SAMPLE_US);

        if (DHT11_SDA_READ == Bit_SET) {
            data[i / 8] |= (0x80 >> (i % 8));

        }
    }

    /* 释放总线 */
    DHT11_OUT();
    DHT11_SDA_H;

    return FLAG_DHT11_OK;
}

/* ===== 数据解析 ===== */
uint8_t DHT11_Parse(uint8_t *data, float *temp, float *humi)
{

    if ((data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return FLAG_DHT11_ERROR;
    }
    *humi = data[0] + data[1] * 0.1f;
    *temp = data[2] + data[3] * 0.1f;
    if ( data[3] & 0x80){
        *temp = -(*temp);
    }
    return FLAG_DHT11_OK;
}

/* ===== 对外接口 ===== */
uint8_t DHT11_Get(float *temp, float *humi)
{
    uint8_t data[5];

    if (DHT11_Read(data) == FLAG_DHT11_OK) {
        DHT11_Parse(data, temp, humi);
        return 1;
    } else {
        // DHT11_LOG("Get data failed."); // 获取数据失败
        return 0;
    }
}
