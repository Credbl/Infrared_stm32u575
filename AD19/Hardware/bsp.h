/*!
\file       bsp.h
\brief      bsp.h 文件概述
\details    =
\author     EmbeddedLab Of SWUST
\date       2022-05-23
\see        =
\version    =
\pre        =
\bug        =
\attention  =
\warning    =

STM32U575RGT6  SRAM 768K, FLASH 768+256=1024K

AC5
--diag_suppress=167,550,177,111
warning:  #111-D: statement is unreachable
warning:  #167-D: argument of type "uint8_t *" is incompatible with parameter of type "char *restrict"
warning:  #177-D: variable "xxx"  was declared but never referenced
warning:  #550-D: variable "xxx"  was set but never used

AC6
-Wno-invalid-source-encoding

\copyright
<h2><center>Copyright &copy; \f$\text{2021}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/
#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup BSP
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>





/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup BSP_Exported_Macros BSP 导出的宏定义
\brief 导出的宏定义
\{
*/
//#pragma anon_unions

#define EVENT_RECORDER                      0

#if EVENT_RECORDER
#include "EventRecorder.h"
#endif

/*! 定义中断优先级
HAL 库中初始化参数为
    NVIC_PRIORITYGROUP_4 初始化    0-15 级抢占优先级     0 级子优先级

HAL_CONF 中配置 SysTick 抢占优先级 TICK_INT_PRIORITY 为 0
系统中其他中断统一使用 \ref NVIC_PREEMPT 作为抢占优先级*/
#define NVIC_PREEMPT                        (4)
#define NVIC_SUB                            (0)


/*!用于防止变量被优化，主要用于测试存储空间容量*/
/*#define DEBUG_USED      __attribute__ ((__used__))*/
#define DEBUG_USED

/*!定义PI用于数学运算*/
#ifndef PI
#define PI                  3.14159265358979f
#endif


#ifndef MIN
#define MIN(a, b)           ((a) < (b) ? (a) : (b) )
#endif

/** \} end of group BSP_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup BSP_Exported_Types BSP 导出的数据类型
\brief 导出的数据类型
\{
*/
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;

typedef volatile unsigned char  vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int   vu32;

typedef unsigned char           U08;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned __int64        U64;
typedef   signed char           S08;
typedef   signed short          S16;
typedef   signed int            S32;
typedef   signed __int64        S64;

typedef const unsigned char     UC08;
typedef const unsigned short    UC16;
typedef const unsigned int      UC32;
typedef const unsigned __int64  UC64;
typedef const   signed char     SC08;
typedef const   signed short    SC16;
typedef const   signed int      SC32;
typedef const   signed __int64  SC64;

typedef volatile unsigned char  VU08;
typedef volatile unsigned short VU16;
typedef volatile unsigned int   VU32;
typedef volatile   signed char  VS08;
typedef volatile   signed short VS16;
typedef volatile   signed int   VS32;



/*!APP_INFO 软件版本信息*/
typedef struct APP_INFO
{
    char version[32];       /*!<软件程序版本*/
    char build_date[32];    /*!<软件程序编译日期*/
    char build_time[32];    /*!<软件程序编译时间*/
} APP_INFO;



typedef enum
{
    FALSE = false, TRUE = true,
} BOOL;

/** \} end of group BSP_Exported_Types */


#include "stm32u5xx.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_it.h"


#include "fonts/fonts.h"
#include "images/images.h"
#include "images/colormap.h"
#include "log/swo_log.h"
#include "st7789.h"
//#include "lcd.h"
#include "i2c_cst816t.h"

//#include "spi.h"
#include "main.h"
#include "icache.h"
#include "gpio.h"
// #include "battery.h"

/*
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_pbs.h"



#include "Log/SwoPrintf.h"
#include "easylogger/inc/elog.h"
#include "fonts/fonts.h"



#include "adc.h"
#include "battery.h"
#include "command.h"
#include "beep.h"
#include "key.h"
#include "ds18b20.h"
#include "fpga_spi.h"
#include "image_process.h"
#include "timer_pwm.h"
#include "uart_bt.h"
*/
#include "main.h"


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup BSP_Exported_Datas BSP 导出的数据定义
\brief 导出的数据定义
\{
*/
extern uint8_t  GUID[2];
extern char     g_strbuff[128];
/** \} end of group BSP_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup BSP_Exported_Functions
\{
*/
void BSP_Init(void);
void BSP_DelayInit(uint8_t clk_mhz);
void BSP_DelayUs(uint32_t nus);
void BSP_DelayMs(uint32_t nms);
void BSP_GetUID(uint8_t *);
const APP_INFO *BSP_GetAppInfo(void);

/** \} BSP_Exported_Functions */



/** \} end of group BSP */
#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
