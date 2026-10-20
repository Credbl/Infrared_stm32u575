#include "Lepton_Delay.h"


static uint32_t fac_us = 0;



void SYSTEM_DelayInit(uint16_t  SYSCLK)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    fac_us = SYSCLK;
}


void Delay_Us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;
    ticks = nus * fac_us;
    told = SysTick->VAL;
    while(1)
    {
        tnow = SysTick->VAL;
        if(tnow != told)
        {
            if(tnow < told)
                tcnt += told - tnow;
            else 
                tcnt += reload - tnow + told;
            told = tnow;
            if(tcnt >= ticks)
                break;
        }
    }
}

void Delay_Ms(uint16_t nms)
{
    while((nms--) > 0)
    {
        Delay_Us(1000);
    }
}

