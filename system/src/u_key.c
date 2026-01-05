#include "u_key.h"


void u_key_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Pin = U_KEY_PIN0 | U_KEY_PIN1;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
}


uint8_t u_key_scan(key_t *u_key)
{
    if (GPIO_ReadInputDataBit(GPIOE, u_key->pin) == u_key->stable){
        u_key->cnt = 0;
    } else {
        if (++u_key->cnt >= KEY_FILTER_CNT) {
            u_key->cnt = 0;
            if (u_key->stable == Bit_SET){
                u_key->stable = !u_key->stable;
                return 1;  
            }else{
                u_key->stable = !u_key->stable;
                // return 1;  
            }

        }
    }
    return 0;
}

