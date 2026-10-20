#ifndef __GC2145_H
#define	__GC2145_H

/**
  ******************************************************************************
  * @file    bsp_gc2145.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    15-April-2025
  * @brief   Header for dcmi_ov2640.c module
  *******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/


/* Includes ------------------------------------------------------------------*/
#include "bsp.h"

#define GC2145_DEVICE_ADDRESS     0x78
#define GC2145_PID  0x2145
// #define CONFIG_IDF_TARGET_ESP32   1
#define CONFIG_GC_SENSOR_SUBSAMPLE_MODE 1
#define H8(v) ((v)>>8)
#define L8(v) ((v)&0xff)

//相机是否正确工作的标志
extern volatile uint8_t g_GC2145_state;


void GC2145_Test(void);
void GC2145_PowerOff(void);
uint8_t GC2145_Init(void);


#endif /* __GC2145_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
