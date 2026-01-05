// #include "u_delay.h"

// void u_delay_us(uint32_t us)
// {
//     while (us)
//     {
//         uint32_t cur = (us > SYSTICK_MAX_US) ? SYSTICK_MAX_US : us;
//         us -= cur;

//         SysTick->CTRL = 0;
//         SysTick->LOAD = (SystemCoreClock / 1000000U) * cur - 1;
//         SysTick->VAL  = 0;
//         SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

//         while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
//         SysTick->CTRL = SysTick_CTRL_ENABLE_Pos;
//     }
// }

// void u_delay_ms(uint32_t ms)
// {
//     while (ms--)
//     {
//         u_delay_us(1000);
//     }
// }

// void u_delay_s(uint32_t s)
// {
//     while (s--)
//     {
//         u_delay_ms(1000);
//     }
// }

