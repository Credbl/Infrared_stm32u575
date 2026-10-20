#ifndef __LEPTOND_DELAY__H__
#define __LEPTOND_DELAY__H__

#include "main.h"




void SYSTEM_DelayInit(uint16_t SYSCLK);
void Delay_Us(uint32_t nus);
void Delay_Ms(uint16_t nms);

#endif



