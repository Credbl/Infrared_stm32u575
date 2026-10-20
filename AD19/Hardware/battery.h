/*!
\file       battery.h
\brief      battery.h 文件概述
\details    =
\author     EmbeddedLab Of SWUST
\date       2024-12-24
\see        =
\version    =
\pre        =
\bug        =
\attention  =
\warning    =

\copyright
<h2><center>Copyright &copy; \f$\text{2021}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/
#ifndef __BATTERY_H__
#define __BATTERY_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup BATTERY
\{
*/
/* INCLUDE FILES **************************************************************/
#include "bsp.h"

/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup BATTERY_Exported_Macros BATTERY 导出的宏定义
\brief 导出的宏定义
\{
*/


/** \} end of group BATTERY_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup BATTERY_Exported_Types BATTERY 导出的数据类型
\brief 导出的数据类型
\{
*/

#define SYSTEM_POWER_PORT             GPIOC
#define SYSTEM_POWER_PIN              GPIO_PIN_12
#define SYSTEM_POWER_CLK_ENABLE      __HAL_RCC_GPIOC_CLK_ENABLE


#define SYSTEM_POWER_KEY_PORT               GPIOC
#define SYSTEM_POWER_KEY_PIN                GPIO_PIN_10
#define SYSTEM_POWER_KEY_CLK_ENABLE         __HAL_RCC_GPIOC_CLK_ENABLE

enum BATTERY_LEVEL_TAG{
    BATTERY_LEVEL_0 = 0x1,
    BATTERY_LEVEL_30,
    BATTERY_LEVEL_60,
    BATTERY_LEVEL_100,
    BATTERY_LEVEL_CHARGING,
    BATTERY_LEVEL_NUM,
    BATTERY_LEVEL_INVALID = 0x55,
};

/** \} end of group BATTERY_Exported_Types */


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup BATTERY_Exported_Datas BATTERY 导出的数据定义
\brief 导出的数据定义
\{
*/
extern const uint8_t* ICON_BATTERY[];


/** \} end of group BATTERY_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup ST7789_Exported_Functions
\{
*/
void Battery_GPIO_Init(void);
void Battery_PowerOn(void);
void Battery_PowerOff(void);
void Battery_KeyScan_Task(void);
void Battery_ADC_Init(void);
uint8_t Battery_Battery_ADC_Task(void);
/** \} BATTERY_Exported_Functions */



/** \} end of group BATTERY */
#ifdef __cplusplus
}
#endif

#endif /* __BATTERY_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/



