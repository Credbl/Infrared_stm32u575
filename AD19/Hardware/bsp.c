/*!
\file       bsp.c
\brief      bsp.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2022-05-23
\see        =
\version    =
\pre        =
\bug        =
\attention  =
\warning    =

\copyright
<h2><center>Copyright &copy; \f$\text{2022}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/

/* INCLUDE FILES **************************************************************/
/* 相关头文件的.h */
#include "bsp.h"

/* C库的.h */

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */

/*!
\defgroup BSP GROUP_BSP
\brief    BSP概述
\details  BSP详细描述
\{
*/

/* PRIVATE DEFINES ************************************************************/

#define MAJOR               0
#define MINOR               1
#define SUFFIX              1


/* PRIVATE TYPE DEFINITIONS ***************************************************/


/* PRIVATE FUNCTION DECLARATIONS **********************************************/

/* PRIVATE DATA DECLARATIONS **************************************************/
/*static const APP_INFO _app_info __attribute__((at(0x08050000))) =*/
static APP_INFO _app_info =
{
    .version    = "V0.1.0.20230725_1",
    .build_date = __DATE__,
    .build_time = __TIME__,
};
/* EXPORTED PUBLIC DATA *******************************************************/
uint8_t  GUID[2] = {0};
char     g_strbuff[128] = { 0 };

/* PRIVATE MODULE FUNCTIONS ***************************************************/

/*!根据程序编译时间生成软件版本号*/
static void _generate_firmware_version(void)
{
    char *months[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    char temp[] = __DATE__;

    uint8_t i;
    uint8_t month, day, year;

    year = atoi(temp + 9);
    *(temp + 6) = 0;
    day = atoi(temp + 4);
    *(temp + 3) = 0;

    for (i = 0; i < 12; i++)
    {
        if (!strcmp(temp, months[i]))
        {
            month = i + 1;
            break;
        }
    }

    snprintf((char *)_app_info.version, sizeof(_app_info.version),
        "V%d.%02d.%d%02d%02d%d", MAJOR, MINOR, year, month, day, SUFFIX);
}




/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\defgroup BSP_Exported_Functions BSP 导出的函数接口
\brief    导出的函数接口
\{
*/
void BSP_Init(void)
{
#if EVENT_RECORDER
    EventRecorderInitialize(EventRecordAll, 1U);
    EventRecorderStart();
#endif

    HAL_Init();

    BSP_GetUID(GUID);

    _generate_firmware_version();
}

static uint8_t fac_us = 0;
/*!根据当前系统时钟频率，自动计算延时因数。

SysTick 系统定时器是一个 `24bit` 的向下 **递减** 的计数器，
计数器每计数一次的时间为 $\dfrac{1}{\text{SYSCLK}}$，
一般我们设置 `SYSCLK` 等于系统时钟 `SystemCoreClock`。

\p clk_mhz 是当前系统时钟频率 MHz，
SysTick 计数 clk_mhz 次即为 1us。
*/
void BSP_DelayInit(uint8_t clk_mhz)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    fac_us = clk_mhz;
}

void BSP_DelayUs(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        /*LOAD的值*/
    ticks = nus * fac_us;                   /*需要的节拍数*/
    told = SysTick->VAL;                    /*刚进入时的计数器值*/
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                /*这里注意 SYSTICK 是一个递减的计数器.*/
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;    //时间超过/等于要延迟的时间,则退出.
            }
        }
    }
}

void BSP_DelayMs(uint32_t nms)
{
    while (nms--)
    {
        BSP_DelayUs(1000);
    }
}

/*!读取 96bit 芯片ID，从中提取3字节数据作为UID。
算法一：
1. 读取12字节芯片ID后，从固定索引提取3字节数据，分别对12取模，得到的结果作为二次索引
2. 返回二次索引指向的芯片ID字节

算法二：
1. 随机生成3个数，作为提取数据索引，该随机数存储于 E2ROM 中
2. 从96bit/12字节中，以随机数为索引提取3字节数据作为UID
*/
void BSP_GetUID(uint8_t *buf)
{
    uint8_t idx[2] = { 8, 0 };
    uint8_t idbytes[12] = {0};
    uint32_t *pid = (uint32_t *)idbytes;

    pid[0] = *(uint32_t *)(UID_BASE + 0);
    pid[1] = *(uint32_t *)(UID_BASE + 4);
    pid[2] = *(uint32_t *)(UID_BASE + 8);

    buf[0] = idbytes[idx[0]];
    buf[1] = idbytes[idx[1]];
}

const APP_INFO *BSP_GetAppInfo(void)
{
    return &_app_info;
}

/** \} end of group BSP_Exported_Functions */
/** \} end of group BSP */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
