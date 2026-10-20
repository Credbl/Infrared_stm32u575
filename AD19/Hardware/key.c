/*!
\file       key.c
\brief      key.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2022-07-12
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

/* C库的.h */

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */
#include "bsp.h"
#include "key.h"
#include "gpio.h"

/*!
\defgroup KEY GROUP_KEY
\brief    KEY概述
\details  KEY详细描述
\{
*/

/* PRIVATE DEFINES ************************************************************/
/* 实体按键个数 */
#define HARD_KEY_NUM			(1)
/* 8个独立建 + 2个组合按键 */
#define KEY_COUNT				(HARD_KEY_NUM + 0)

/* 使能GPIO时钟 */
#define ALL_KEY_GPIO_CLK_ENABLE() do {		\
        __HAL_RCC_GPIOC_CLK_ENABLE();		\
    } while (0);


/*!自动关机等待时间（秒）*/
#if defined(DEBUG)
    #define AUTO_POWEROFF_DELAY_SEC				60
#else
    #define AUTO_POWEROFF_DELAY_SEC				30
#endif

/* PRIVATE TYPE DEFINITIONS ***************************************************/
typedef struct X_GPIO_T X_GPIO_T;
struct X_GPIO_T
{
    GPIO_TypeDef *gpio;
    uint16_t pin;
    uint8_t ActiveLevel;			/* 激活电平 */
};

/* PRIVATE FUNCTION DECLARATIONS **********************************************/

/* PRIVATE DATA DECLARATIONS **************************************************/

uint32_t _auto_poweroff_count_10ms = 0;

/* GPIO和PIN定义 */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] =
{
    {GPIOC, GPIO_PIN_10,  0},		/*!< K1   PA8 */
};

static KEY_T s_tBtn[KEY_COUNT] = { 0 };

static KEY_FIFO_T s_tKey;			/* 按键FIFO变量,结构体 */

static WWDG_HandleTypeDef IWDG_Handler;

/* EXPORTED PUBLIC DATA *******************************************************/

/* PRIVATE MODULE FUNCTIONS ***************************************************/


/*!设置按键参数
\param  _ucKeyID : 按键ID，从0开始
\param  _LongTime : 长按事件时间
\param  _RepeatSpeed : 连发速度*/
void _setKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed)
{
    s_tBtn[_ucKeyID].LongTime = _LongTime;          /* 长按时间 0 表示不检测长按键事件 */
    s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;    /* 按键连发的速度，0表示不支持连发 */
    s_tBtn[_ucKeyID].RepeatCount = 0;               /* 连发计数器 */
}


/*!初始化按键变量*/
static void _initKeyVar(void)
{
    uint8_t i;

    /* 对按键FIFO读写指针清零 */
    s_tKey.Read = 0;
    s_tKey.Write = 0;
    s_tKey.Read2 = 0;

    /* 给每个按键结构体成员变量赋一组缺省值 */
    for (i = 0; i < KEY_COUNT; i++)
    {
        s_tBtn[i].LongTime    = KEY_LONG_TIME;          /* 长按时间 0 表示不检测长按键事件 */
        s_tBtn[i].Count       = KEY_FILTER_TIME / 2;    /* 计数器设置为滤波时间的一半 */
        s_tBtn[i].State       = 0;                      /* 按键缺省状态，0为未按下 */
        s_tBtn[i].RepeatSpeed = 0;                      /* 按键连发的速度，0表示不支持连发 */
        s_tBtn[i].RepeatCount = 0;                      /* 连发计数器 */
    }
}

/*!配置按键对应的GPIO*/
static void _initKeyHard(void)
{
    uint8_t i;
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* 第1步：打开GPIO时钟 */
    ALL_KEY_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
    for (i = 0; i < HARD_KEY_NUM; i++)
    {
        GPIO_InitStruct.Pin = s_gpio_list[i].pin;
        HAL_GPIO_Init(s_gpio_list[i].gpio, &GPIO_InitStruct);
    }
}

/*!将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。*/
void _putKey(uint8_t _KeyCode)
{
    /*有按键事件发生，重置自动关机计数*/
    _auto_poweroff_count_10ms = 0;

    s_tKey.Buf[s_tKey.Write] = _KeyCode;

    if (++s_tKey.Write >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}

/*!判断按键是否按下
\retval 1 表示按下
\retval 0 表示未按下*/
static uint8_t _isKeyPinActive(uint8_t _id)
{
    uint8_t level;

    if (RESET == HAL_GPIO_ReadPin(s_gpio_list[_id].gpio, s_gpio_list[_id].pin))
    {
        level = 0;
    }
    else
    {
        level = 1;
    }

    if (level == s_gpio_list[_id].ActiveLevel)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*!判断按键是否按下。单键和组合键区分。单键事件不允许有其他键按下。
\retval 1 表示按下(导通）
\retval 0表示未按下（释放）*/
static uint8_t _isKeyDownFunc(uint8_t _id)
{
    /* 实体单键 */
    if (_id < HARD_KEY_NUM)
    {
        uint8_t i;
        uint8_t count = 0;
        uint8_t save = 255;

        /* 判断有几个键按下 */
        for (i = 0; i < HARD_KEY_NUM; i++)
        {
            if (_isKeyPinActive(i))
            {
                count++;
                save = i;
            }
        }

        if (count == 1 && save == _id)
        {
            return 1;   /* 只有1个键按下时才有效 */
        }

        return 0;
    }

#if 0
    /* 组合键 K1K2 */
    if (_id == HARD_KEY_NUM + 0)
    {
        if (KeyPinActive(KID_K1) && KeyPinActive(KID_K2))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* 组合键 K2K3 */
    if (_id == HARD_KEY_NUM + 1)
    {
        if (KeyPinActive(KID_K2) && KeyPinActive(KID_K3))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
#endif

    return 0;
}

/*!检测一个按键。非阻塞状态，必须被周期性的调用。*/
static void _detectKey(uint8_t i)
{
    KEY_T *pBtn = 0;

    pBtn = &s_tBtn[i];
    if (_isKeyDownFunc(i))
    {
        if (pBtn->Count < KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if (pBtn->Count < 2 * KEY_FILTER_TIME)
        {
            pBtn->Count++;
        }
        else
        {
            if (pBtn->State == 0)
            {
                pBtn->State = 1;

                /* 发送按钮按下的消息 */
                _putKey((uint8_t)(3 * i + 1));
            }

            if (pBtn->LongTime > 0)
            {
                if (pBtn->LongCount < pBtn->LongTime)
                {
                    /* 发送按钮持续按下的消息 */
                    if (++pBtn->LongCount == pBtn->LongTime)
                    {
                        /* 键值放入按键FIFO */
                        _putKey((uint8_t)(3 * i + 3));
                    }
                }
                else
                {
                    if (pBtn->RepeatSpeed > 0)
                    {
                        if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                        {
                            pBtn->RepeatCount = 0;
                            /* 常按键后，每隔10ms发送1个按键 */
                            _putKey((uint8_t)(3 * i + 1));
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (pBtn->Count > KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if (pBtn->Count != 0)
        {
            pBtn->Count--;
        }
        else
        {
            if (pBtn->State == 1)
            {
                pBtn->State = 0;

                /* 发送按钮弹起的消息 */
                _putKey((uint8_t)(3 * i + 2));
            }
        }

        pBtn->LongCount = 0;
        pBtn->RepeatCount = 0;
    }
}


/*!清空按键FIFO缓冲区*/
void _clearKey(void)
{
    s_tKey.Read = s_tKey.Write;
}



/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\defgroup KEY_Exported_Functions KEY 导出的函数接口
\brief    导出的函数接口
\{
*/

/*!初始化按键。*/
void KEY_Init(void)
{
    _initKeyVar();       /* 初始化按键变量 */
    _initKeyHard();      /* 初始化按键硬件 */
}

/*!扫描所有按键。非阻塞，被systick中断周期性的调用，10ms一次*/
void KEY_Scan10ms(void)
{
    uint8_t i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        _detectKey(i);
    }

    ++_auto_poweroff_count_10ms;
    if (AUTO_POWEROFF_DELAY_SEC * 100 < _auto_poweroff_count_10ms)
    {
        _auto_poweroff_count_10ms = 0;
        //POWER_OFF();
    }

    /* 喂狗 */
    //HAL_WWDG_Refresh(&IWDG_Handler);
}


/*!读取按键的状态
\param  _ucKeyID : 按键ID，从0开始
\retval 1 表示按下
\retval 0 表示未按下*/
uint8_t KEY_GetKeyState(KEY_ID_E _ucKeyID)
{
    return s_tBtn[_ucKeyID].State;
}



/*!从按键FIFO缓冲区读取一个键值。*/
uint8_t KEY_GetKey(void)
{
    uint8_t ret;

    if (s_tKey.Read == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
        return ret;
    }
}

/*!
初始化独立看门狗
prer:分频数:IWDG_PRESCALER_4~IWDG_PRESCALER_256
rlr:自动重装载值,0~0XFFF.
时间计算(大概):Tout=((4*2^prer)*rlr)/32 (ms).
*/
void WD_Init(void)
{
    /* 从 WD 复位后，清除标志 */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
    {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        printf("reset from iwdg\r\n");
    }

    IWDG_Handler.Instance = WWDG;
    IWDG_Handler.Init.Prescaler = WWDG_PRESCALER_8;
    //IWDG_Handler.Init.Reload = 500;        // 超时时间 1秒
    IWDG_Handler.Init.Counter = 500 * AUTO_POWEROFF_DELAY_SEC;        // 超时时间 32秒
    HAL_WWDG_Init(&IWDG_Handler);

    //HAL_IWDG_Refresh(&IWDG_Handler); 	//喂狗
}

void WD_Refresh(void)
{
    HAL_WWDG_Refresh(&IWDG_Handler);
}

void WD_Test(void)
{
    uint8_t cnt = 0;
    while (1)
    {
        HAL_Delay(900);
        HAL_WWDG_Refresh(&IWDG_Handler);
        printf("running....\r\n");
        ++cnt;
        if (cnt > 10)
        {
            printf("reach count, delay more to trigger wathdog....\r\n");
            cnt = 0;
            HAL_Delay(500);
        }
    }
}


/** \} end of group KEY_Exported_Functions */
/** \} end of group KEY */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/

