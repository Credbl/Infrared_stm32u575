/*******************************************************************************
**
**      File NAME: swo_log.h
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 2019-08-07
**
**      DESCRIPTION:
**
**      (C) COPYRIGHT 2015 ~ 2025 EmbeddedLab Of SWUST
**      All rights reserved.
**
**  概念缩写
**      SWD：Serial Wire Debug，串行线调试
**      SWO：Serial Wire Output，串行线输出
**      SWV：Serial Wire Viewer，串行线查看器
**      ITM：Instrumentation Trace Macrocell，指令跟踪宏单元
**
**  前提条件
**      ST-LINK 或 JLink 与 MCU 的连线至少包含 SWCLK SWDIO SWO GND 四条信号线，
**      其中 SWO 是用于串行输出的，必不可少，输出的数据可以借助 SWV 查看，SWV
**      可以使用：
**      1. 基于Keil的 Debug (printf) Viewer
**      2. 基于ST-LINK Utility的 Serial Wire Viewer
**      3. 基于IAR的 Terminal IO
**      4. 基于J-Link的 SWO Viewer
**
**
**  在 Keil 中打开工程设置 Options
**      1. Target
**          1. 勾选 Use Micro-LIB
**      2. Debug->Trace
**          1. Core Clock 配置为硬件时钟
**          2. 勾选 Trace Enable
**          3. 下方 TIM Stimulus Ports 中， Enable 值设置为 0x01，
**             Privilege 值设置为0x00
**      3. 下载程序，启动调试后打开 Debug (printf) viewer 窗口即可查看输出
*******************************************************************************/
#ifndef __SWO_LOG_H__
#define __SWO_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/





// 将 #if 0 ... #endif 中的内容拷贝到工程的任一源文件中进行使用，否则，编译会报错误
#if 0
// ITM_SendChar说明
//      这个函数位于 core_cm3.h（或cm4 cm7）中，直接调用即可。
//  一般都使用 port0 无需修改
int fputc(int ch, FILE *f)
{
    return ITM_SendChar(ch);
}
#endif

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
#define LOG_ENABLE



#define _sl_(x)         #x
#define _strline_(x)    _sl_(x)
#define _STR_LINE_      _strline_(__LINE__)

#define TAG             __FILE__":"_STR_LINE_

#if defined (LOG_ENABLE)

#define LOG_D(...)      do {\
        printf("[%6s]%s:| ", "DEBUG", TAG); \
        printf(__VA_ARGS__);printf("\r\n"); \
    } while(0)

#define LOG_E(...)      do {\
        printf("%*s[%06d] [%6s] [%16s:%04d]| ", ESCAPE, "", E_COUNT++, "ERROR", __FILE__, __LINE__);\
        printf(__VA_ARGS__);printf("\r\n");\
    } while(0)

// #define LOG_D(...)      do {printf(__VA_ARGS__);} while(0)
// #define LOG_E(...)      do {printf(__VA_ARGS__);} while(0)
#define ENTER()         do {printf("%*s>>>>>>>>>>>> Enter: %48s\r\n", ESCAPE, "", __FUNCTION__);ESCAPE+=4;} while(0)
#define LEAVE()         do {ESCAPE-=4;printf("%*s<<<<<<<<<<<< Leave: %48s\r\n", ESCAPE, "", __FUNCTION__);} while(0)
#else
#define LOG_D(...)
#define LOG_E(...)
#define ENTER()
#define LEAVE()
#endif



// #ifndef LOG_LEVEL_LOWEST
// #define LOG_LEVEL_LOWEST    _LOG_LEVEL_DEBUG
// #endif
//
// /**
//  * Defines the default log level.
//  */
// #ifndef LOG_LEVEL_DEFAULT
// #define LOG_LEVEL_DEFAULT   _LOG_LEVEL_INFO
// #endif
//
// #if LOG_LEVEL_LOWEST <= _LOG_LEVEL_TRACE
// #define logTrace(MSG, ...)       __logLine(&_log, LOG_LEVEL_TRACE, MSG, ##__VA_ARGS__)
// #else
// #define logTrace(MSG, ...)
// #endif
//
// #if LOG_LEVEL_LOWEST <= _LOG_LEVEL_DEBUG
// #define logDebug(MSG, ...)      __logLine(&_log, LOG_LEVEL_DEBUG, MSG, ##__VA_ARGS__)
// #else
// #define logDebug(MSG, ...)
// #endif
//
// #if LOG_LEVEL_LOWEST <= _LOG_LEVEL_INFO
// #define logInfo(MSG, ...)       __logLine(&_log, LOG_LEVEL_INFO, MSG, ##__VA_ARGS__)
// #else
// #define logInfo(MSG, ...)
// #endif
//
// #if LOG_LEVEL_LOWEST <= _LOG_LEVEL_WARN
// #define logWarn(MSG, ...)       __logLine(&_log, LOG_LEVEL_WARN, MSG, ##__VA_ARGS__)
// #else
// #define logWarn(MSG, ...)
// #endif
//
// #if LOG_LEVEL_LOWEST <= _LOG_LEVEL_ERROR
// #define logError(MSG, ...)       __logLine(&_log, LOG_LEVEL_ERROR, MSG, ##__VA_ARGS__)
// #else
// #define logError(MSG, ...)
// #endif
//
//
// #ifdef DEBUG
//    dbgPrintf(MSG, ...)     printf(MSG, ##__VA_ARGS__)
// #else
//    dbgPrintf(MSG, ...)
// #endif



/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/



/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/



/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/





#ifdef __cplusplus
}
#endif

#endif /* __SWO_LOG_H__ */



/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/




