/*!
\file       i2c_cst816t.c
\brief      i2c_cst816t.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2024-12-18
\see        =
\version    =
\pre        =
\bug        =
\attention  =
\warning    =


// 读 shipID
drv_Touch_Read(0xA7, &chip_id, 1);//值 cst816s：0xB4    cst716：0x20


//读 TP 固件版本号
drv_Touch_Read(0xA9, &g_tp_version, 1);//每更新一版固件，版本号递增 1


//设置报点率，拉中断时间间隔
data[0] = 0x02;//单位 10ms, 设置时间= data[0]*10ms
drv_Touch_Write(0xEE, data, 1);//默认报点率为 100HZ，10ms 拉一次中断，


//设置为报点模式
data[0] = 0x60;
drv_Touch_Write(0xFA, data, 1);// 触摸 TP 时每隔 10ms 左右拉一次中断。时时上报坐标


//设置为手势模式
data[0] = 0x11;
drv_Touch_Write(0xFA, data, 1);// 生成手势后拉一次中断。


//设置为报点+手势模式
data[0] = 0x71;
drv_Touch_Write(0xFA, data, 1);


//设置自动复位时间
data[0] = 0x05;//单位 1S， 为 0 时不启用此功能。 默认为 5
drv_Touch_Write(0xFC, data, 1);// x 秒内有触摸但无有效手势时， 自动复位
data[0] = 0x10; //单位 1S， 为 0 时不启用此功能。 默认为 10
drv_Touch_Write(0xFD, data, 1);     // 长按 x 秒后自动复位

// 禁止自动进入低功耗模式
data[0] = 0x01; //为非 0 值时， 禁止自动进入低功耗模式。
drv_Touch_Write(0xFE, data, 1);// 默认为 0， 使能自动进入低功耗模式

\copyright
<h2><center>Copyright &copy; \f$\text{2024}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/

/* INCLUDE FILES **************************************************************/
/* 相关头文件的.h */
#include "bsp.h"
#include "Lepton_Stm32_Control.h"
#include "bsp_gc2145.h"
#include "Lepton_Stm32_SPI_DMA.h"
#include "Lepton_Common_Data.h"
#include "bsp_flash.h"
#include "sd_diskio.h"
/* C库的.h */

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */

/*!
\defgroup I2C_CST816T GROUP_I2C_CST816T
\brief    I2C_CST816T概述
\details  I2C_CST816T详细描述
\{
*/

/* PRIVATE DEFINES ************************************************************/

#define I2C_WR      0                       /* 写控制bit */
#define I2C_RD      1                       /* 读控制bit */


#define CST816T_TYPE_T      (0xB5)
#define CST816T_TYPE        (0xB6)
// 地址是：0x15(7 位)，加上读写为对应是：0x2A 写，0x2B 读
#define DEV_ADDR        (0x15)              /* I2C从机地址 */
#define DEV_ADDR_WR     (0x2A)  //((DEV_ADDR << 1) | I2C_WR)
#define DEV_ADDR_RD     (0x2B)  //((DEV_ADDR << 1) | I2C_RD)
#define BOOT_DEV_ADDR   (0x6A)

/* 定义I2C总线连接的GPIO端口, 用户只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C_SCL_GPIO    GPIOA               /* 连接到SCL时钟线的GPIO */
#define I2C_SDA_GPIO    GPIOA               /* 连接到SDA数据线的GPIO */

#define I2C_SCL_PIN     GPIO_PIN_2          /* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN     GPIO_PIN_3          /* 连接到SDA数据线的GPIO */

#define ALL_I2C_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()

/* 定义读写SCL和SDA的宏 */
// #define I2C_SCL_1()  I2C_SCL_GPIO->BSRR = I2C_SCL_PIN              /* SCL = 1 */
// #define I2C_SCL_0()  I2C_SCL_GPIO->BRR  = I2C_SCL_PIN              /* SCL = 0 */
//
// #define I2C_SDA_1()  I2C_SDA_GPIO->BSRR = I2C_SDA_PIN              /* SDA = 1 */
// #define I2C_SDA_0()  I2C_SDA_GPIO->BRR  = I2C_SDA_PIN              /* SDA = 0 */

// #define I2C_SDA_READ()  ((I2C_SDA_GPIO->IDR & I2C_SDA_PIN) != 0)    /* 读SDA口线状态 */
// #define I2C_SCL_READ()  ((I2C_SCL_GPIO->IDR & I2C_SCL_PIN) != 0)    /* 读SCL口线状态 */



#define I2C_SCL_1()     HAL_GPIO_WritePin(I2C_SCL_GPIO, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_0()     HAL_GPIO_WritePin(I2C_SCL_GPIO, I2C_SCL_PIN, GPIO_PIN_RESET)

#define I2C_SDA_1()     HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_0()     HAL_GPIO_WritePin(I2C_SDA_GPIO, I2C_SDA_PIN, GPIO_PIN_RESET)

#define I2C_SDA_READ()  HAL_GPIO_ReadPin(I2C_SDA_GPIO, I2C_SDA_PIN)
#define I2C_SCL_READ()  HAL_GPIO_ReadPin(I2C_SCL_GPIO, I2C_SCL_PIN)



#define SDA_IN()    do {                \
        GPIOA->MODER    &= ~(3<<(3*2)); \
        GPIOA->MODER    |=   0<<(3*2);  \
        GPIOA->PUPDR    &= ~(3<<(3*2)); \
        GPIOA->PUPDR    |=   1<<(3*2);  \
    } while (0)

#define SDA_OUT()   do {                \
        GPIOA->MODER    &= ~(3<<(3*2)); \
        GPIOA->MODER    |=   1<<(3*2);  \
    }while(0)



#define REG_CHIP_ID             (0xA7)
#define REG_PROJ_ID             (0xA8)
#define REG_FW_VER              (0xA9)



/* PRIVATE TYPE DEFINITIONS ***************************************************/

/* PRIVATE FUNCTION DECLARATIONS **********************************************/

/* PRIVATE DATA DECLARATIONS **************************************************/

static char str_buff[128] = { 0 };

static char *gesture_str[] =
{
    [0x00] = "无手势",
    [0x01] = "上滑",
    [0x02] = "下滑",
    [0x03] = "左滑",
    [0x04] = "右滑",
    [0x05] = "单击",
    [0x0B] = "双击",
    [0x0C] = "长按",
};

static __IO uint8_t _gesture = 0x00;
static __IO uint16_t _touch_x = 0;
static __IO uint16_t _touch_y = 0;

/* EXPORTED PUBLIC DATA *******************************************************/

/* PRIVATE MODULE FUNCTIONS ***************************************************/
static uint8_t i2c_cst816t_enter_boootmode(void);


//#define i2c_delay()     BSP_DelayUs(2)
static void i2c_delay(void)
{
    /*　
        CPU主频168MHz时，在内部Flash运行, MDK工程不优化。用台式示波器观测波形。
        循环次数为5时，SCL频率 = 1.78MHz (读耗时: 92ms, 读写正常，但是用示波器探头碰上就读写失败。时序接近临界)
        循环次数为10时，SCL频率 = 1.1MHz (读耗时: 138ms, 读速度: 118724B/s)
        循环次数为30时，SCL频率 = 440KHz， SCL高电平时间1.0us，SCL低电平时间1.2us

        上拉电阻选择2.2K欧时，SCL上升沿时间约0.5us，如果选4.7K欧，则上升沿约1us

        实际应用选择400KHz左右的速率即可
    */
    //for (i = 0; i < 30; i++);
    //for (i = 0; i < 60; i++);
    //bsp_DelayUS(2); 229.57KHz时钟
    BSP_DelayUs(5);
}

void i2c_Start(void)
{
    SDA_OUT();
    /* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
    I2C_SDA_1();
    I2C_SCL_1();
    i2c_delay();
    I2C_SDA_0();
    i2c_delay();

    I2C_SCL_0();
    //i2c_delay();
}

void i2c_Stop(void)
{
    SDA_OUT();
    /* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
    I2C_SDA_0();
    I2C_SCL_0();
    i2c_delay();
    I2C_SCL_1();
    i2c_delay();
    I2C_SDA_1();
    i2c_delay();
}

void i2c_SendByte(uint8_t _ucByte)
{
    uint8_t i;

    SDA_OUT();
    I2C_SDA_0();
    I2C_SCL_0();
    /* 先发送字节的高位bit7 */
    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
        {
            I2C_SDA_1();
        }
        else
        {
            I2C_SDA_0();
        }
        i2c_delay();
        I2C_SCL_1();
        i2c_delay();
        I2C_SCL_0();

        _ucByte <<= 1;  /* 左移一个bit */
    }
}

uint8_t i2c_ReadByte(void)
{
    uint8_t i;
    uint8_t value;

    SDA_IN();
    /* 读到第1个bit为数据的bit7 */
    value = 0;
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_0();
        i2c_delay();
        I2C_SCL_1();
        value <<= 1;
        if (I2C_SDA_READ())
        {
            value++;
        }
        i2c_delay();
    }
    return value;
}

/*! 等待 ACK，成功返回0 */
uint8_t i2c_WaitAck(void)
{
    uint8_t retry = 0;

    I2C_SDA_1();
    i2c_delay();
    SDA_IN();
    I2C_SCL_1();
    i2c_delay();

    while (I2C_SDA_READ())
    {
        retry++;
        if (250 < retry)
        {
            i2c_Stop();
            return 1;
        }
    }

    I2C_SCL_0();
    return 0;
}


void i2c_Ack(void)
{
    I2C_SCL_0();    /* CPU产生1个时钟 */
    SDA_OUT();
    I2C_SDA_0();    /* CPU驱动SDA = 0 */
    i2c_delay();
    I2C_SCL_1();    /* CPU产生1个时钟 */
    i2c_delay();
    I2C_SCL_0();
}

void i2c_NAck(void)
{
    I2C_SCL_0();
    SDA_OUT();
    I2C_SDA_1();    /* CPU驱动SDA = 1 */
    i2c_delay();
    I2C_SCL_1();    /* CPU产生1个时钟 */
    i2c_delay();
    I2C_SCL_0();
}


static void i2c_gpio_init(void)
{
    GPIO_InitTypeDef gpio_init;

    /* 第1步：打开GPIO时钟 */
    ALL_I2C_GPIO_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   /* 设置开漏输出 */
    gpio_init.Pull = GPIO_PULLUP;           /* 上下拉电阻不使能 */
    gpio_init.Speed = GPIO_SPEED_HIGH;  // GPIO_SPEED_FREQ_HIGH;  /* GPIO速度等级 */

    gpio_init.Pin = I2C_SCL_PIN;
    HAL_GPIO_Init(I2C_SCL_GPIO, &gpio_init);

    gpio_init.Pin = I2C_SDA_PIN;
    HAL_GPIO_Init(I2C_SDA_GPIO, &gpio_init);

    // irq 输入中断，上升/下降都支持
    gpio_init.Pin = GPIO_PIN_1;
    gpio_init.Mode = GPIO_MODE_IT_RISING;// GPIO_MODE_IT_RISING_FALLING;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    // reset 引脚
    gpio_init.Pin = GPIO_PIN_4;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &gpio_init);

    // 执行一次复位
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(100);

    /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
    //i2c_Stop();
}

static void i2c_cst816t_reset(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(10);
}

/*! 连续读取
    0xA7 芯片型号
    0xA8 工程编号
    0xA9 软件版本号
    三个寄存器*/ 
static uint8_t read_chip_info()
{
    static uint8_t DAT[8];

    i2c_Start();
    i2c_SendByte(DEV_ADDR_WR);
    i2c_WaitAck();
    i2c_SendByte(REG_CHIP_ID);
    i2c_WaitAck();
    i2c_Stop();

    i2c_Start();
    i2c_SendByte(DEV_ADDR_RD);
    i2c_WaitAck();
    DAT[0] = i2c_ReadByte();
    i2c_Ack();
    DAT[1] = i2c_ReadByte();
    i2c_Ack();
    DAT[2] = i2c_ReadByte();
    i2c_Ack();
    i2c_Stop();

    if (CST816T_TYPE != DAT[0])
    {
        return 1;
        //Error_Handler();
    }
    return 0;
}

/*! 从寄存器 reg 开始，连续读取 len 个字节，存储到 pbuff 中 */
static uint8_t read_reg(uint8_t reg, uint8_t *pbuff, uint8_t len)
{
    uint8_t cnt = 0;

    i2c_Start();
    i2c_SendByte(DEV_ADDR_WR);
    i2c_WaitAck();
    i2c_SendByte(reg);
    i2c_WaitAck();
    i2c_Stop();


    i2c_Start();
    i2c_SendByte(DEV_ADDR_RD);
    i2c_WaitAck();

    for (cnt = 0; cnt < len; cnt++)
    {
        *pbuff++ = i2c_ReadByte();
        i2c_Ack();
    }

    i2c_Stop();

    return 0;
}

uint8_t write_reg(uint8_t reg, uint8_t val)
{
    i2c_Start();
    i2c_SendByte(DEV_ADDR_WR);
    i2c_WaitAck();
    i2c_SendByte(reg);
    i2c_WaitAck();
    i2c_SendByte(val);
    i2c_WaitAck();
    i2c_Stop();

    return 0;
}

/*! 接收到中断后，读取触摸数据 */
void read_touch_data(void)
{
    uint8_t data[10] = { 0 };

    memset(&data[0], 0, sizeof(data));

    // 从 0x01 寄存器开始，连续读取 6 字节数据
//    if (read_reg(0x01, &data[0], 6))
    if (read_reg(0x00, &data[0], 7))
    {
        LOG_D("read error");
    }

    if (0x0C < data[1])
    {
        // 无效数据
        return;
    }

    _gesture = data[1];

    uint16_t tpx = ((data[3] & 0xF) << 8) | data[4];
    uint16_t tpy = ((data[5] & 0xF) << 8) | data[6];
    uint8_t finger_num = data[1];

    if (tpx > 240)
    {
        tpx = 240 - 1;
    }

    if (tpy > 280)
    {
        tpy = 280 - 1;
    }

    _touch_x = tpx;
    _touch_y = tpy;

    return;

    LOG_D("gesture %s, touch (%3d, %3d) with %d fingers",
        gesture_str[data[0]], tpx, tpy, finger_num);
}

uint8_t cst816t_check_device(void)
{
    uint8_t ucAck = 1;

    i2c_Start();

    /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
    i2c_SendByte(DEV_ADDR_WR);

    ucAck = i2c_WaitAck();

    i2c_Stop();
    return ucAck;
}


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\defgroup I2C_CST816T_Exported_Functions I2C_CST816T 导出的函数接口
\brief    导出的函数接口
\{
*/

uint8_t cst816t_init(void)
{
    i2c_gpio_init();
    if (cst816t_check_device())
    {
        return 1;
        //Error_Handler();
    }
    HAL_Delay(100);
    if(0 != read_chip_info())
        return 1;

    uint8_t mode = 0;

    // 配置为手势模式
    uint8_t cnt = 0;
    for (cnt = 0; cnt < 10; cnt++)
    {
        read_reg(0xFA, &mode, 1);
        if (0x11 == mode)
        {
            break;
        }
        else
        {
            write_reg(0xFA, 0x11);
        }
    }

    /* 为非 0 值时， 禁止自动进入低功耗模式 */
    write_reg(0xFE, 0x01);

    if (10 == cnt)
    {
        return 1;
        //Error_Handler();
    }
    
    return 0;
}

void cst816t_get_axis(void)
{
    static uint8_t data[8];

    i2c_Start();
    i2c_SendByte(DEV_ADDR_WR);
    i2c_WaitAck();
    i2c_SendByte(0);
    i2c_WaitAck();

    i2c_Start();
    i2c_SendByte(DEV_ADDR_RD);
    i2c_WaitAck();
    data[0] = i2c_ReadByte();
    i2c_Ack();
    data[1] = i2c_ReadByte();
    i2c_Ack();
    data[2] = i2c_ReadByte();
    i2c_Ack();
    data[3] = i2c_ReadByte();
    i2c_Ack();
    data[4] = i2c_ReadByte();
    i2c_Ack();
    data[5] = i2c_ReadByte();
    i2c_Ack();
    data[6] = i2c_ReadByte();
    i2c_NAck();
    i2c_Stop();

    uint16_t tpx = ((data[3] & 0xF) << 8) | data[4];
    uint16_t tpy = ((data[5] & 0xF) << 8) | data[6];
    uint8_t finger_num = data[2];

    snprintf(str_buff, sizeof(str_buff),
        "touch (%3d, %3d) with %d fingers",
        tpx, tpy, finger_num);

    LOG_D("touch (%3d, %3d) with %d fingers",
        tpx, tpy, finger_num);
}


void cst816t_test(void)
{
    uint8_t cnt = 0;

    i2c_gpio_init();
    if (cst816t_check_device())
    {
        //return 1;
        Error_Handler();
    }
    HAL_Delay(100);
    if(0 != read_chip_info())
        Error_Handler();
    i2c_cst816t_enter_boootmode();
    while(1);

    cst816t_init();

#if 0
    if (cst816t_check_device())
    {
        Error_Handler();
    }

    read_chip_info();

    uint8_t mode = 0;

    // 配置为手势模式
    for (cnt = 0; cnt < 10; cnt++)
    {
        read_reg(0xFA, &mode, 1);
        if (0x11 == mode)
        {
            break;
        }
        else
        {
            write_reg(0xFA, 0x11);
        }
    }

    if (10 == cnt)
    {
        Error_Handler();
    }
#endif

    while (1)
    {
        cst816t_process(MODE_NORMAL);
        //read_touch_data();
        //HAL_Delay(1000);
        //cst816t_get_axis();
        //HAL_Delay(300);
    }
}

uint32_t touch_count = 0;
// uint8_t strTemp[20] = {0};
void cst816t_exti_callback(uint16_t pin)
{
    if (GPIO_PIN_1 != pin)
    {
        return;
    }

    read_touch_data();
    touch_count++;

    /* 为非 0 值时， 禁止自动进入低功耗模式 */
    write_reg(0xFE, 0x01);
    //cst816t_get_axis();
}
extern volatile uint8_t g_Working_flag;
volatile uint8_t g_Infrared_Working_flag = 1; 
extern int8_t ff_format(void);
uint8_t press_state = 0;
uint16_t press_count = 0;
void cst816t_process(uint8_t mode)
{
    int8_t save_ret = 0;
    uint8_t strTemp[20] = {0};
    static uint8_t up_cnt = 0, dn_cnt = 0;

    sprintf((char *)strTemp, "cnt:%d,ges:%d", touch_count, _gesture);
    st7789_draw_string(20, 220, (const char *)strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);

    if (MODE_CAL == mode)
    {

        // uint8_t strTemp[10] = {0};
        sprintf((char *)strTemp, "CAL:%d", g_WindowTransimitRate);
        st7789_draw_string(20, 220, (const char *)strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
    }

    if (0x00 == _gesture)
    {
        if ((press_count++) > 1000)
        {
            press_count = 0;
            press_state = 0;
        }
        
        return;
    }

    if (MODE_NORMAL == mode)
    {
        // 上滑
        if (0x01 == _gesture && !g_GC2145_state)
        {
            // GC2145_Init();
            // Lepton_PowerOff();
            g_Working_flag = 1;
            g_Infrared_Working_flag = 0;
        }

        // 下滑
        if (0x02 == _gesture)
        {
            // Lepton_ExitStandby();
            // GC2145_PowerOff();
            g_Working_flag = 0;
            g_Infrared_Working_flag = 1;
        }

        // 左滑
        if ((0x03 == _gesture) && g_Infrared_Working_flag)
        {
            // if (COLOUR_IDX + 1 >= COLOUR_NUM)
            // {
            //     COLOUR_IDX = 0;
            // }
            COLOUR_IDX = ( COLOUR_IDX + 1) % COLOUR_NUM;
            colour_change(COLOUR_IDX);
        }
        // 右滑
        else if ((0x04 == _gesture) && g_Infrared_Working_flag)
        {
            if ( 0 == COLOUR_IDX)
            {
                colour_change(COLOUR_NUM - 1);
            }
            else
            {
                colour_change(COLOUR_IDX - 1);
            }
        }
 
        if ((0x0c == _gesture) && g_Infrared_Working_flag)
        {
            if (0 == press_state)
            {      
                g_TempUnit = !g_TempUnit;
                g_TempChangeFlag = 1;
                press_state = 1;
            }
            press_count = 0;
           // BSP_Flash_Write((g_TempUnit << 16) | g_WindowTransimitRate);
        }
        // else if (g_Infrared_Working_flag)   
        // {
        //      press_state = 0;
        // }

        // 正常模式下，长按进入到校准模式
        // if (0x0c == _gesture)
        // {
        //     win_mode = MODE_CAL;
        // }
        
        up_cnt = dn_cnt = 0;

    }
    else if (MODE_SAVING == mode)
    {
        if (0x05 == _gesture)
        {
            // st7789_fill_rect_dma(_touch_x, 280 - _touch_y, 10, 10, COLOR_RGB565_RED);

            if (_touch_y < 60)
            {
                if (120 > _touch_x)
                {
                    process_save(1);
                }
                else
                {
                    process_save(0);
                }
            }
        }
        // 上划
        if (0x01 == _gesture)
        {
            if (0 == dn_cnt)
            {
                ++up_cnt;
            }
            else
            {
                dn_cnt = 0;
                up_cnt = 0;
            }
        }
        // 下滑
        if (0x02 == _gesture)
        {
            if (5 == up_cnt)
            {
                ++dn_cnt;
                if (5 == dn_cnt)
                {
                    #ifdef LEPTON_25
                        // 若定义了LEPTON_25，使用"80_"作为前缀
                        sprintf(g_strbuff, "80_%s", __DATE__);
                    #elif defined(LEPTON_FS) 
                        // 若定义了LEPTON_FS，使用"FS_"作为前缀
                        sprintf(g_strbuff, "FS_%s", __DATE__);
                    #else
                        // 未定义上述宏时，默认使用"128_"作为前缀
                        sprintf(g_strbuff, "128_%s", __DATE__);
                    #endif
                    st7789_draw_string(20, 200, g_strbuff, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
                    // ff_format();
                    // ff_init();
                }
            }
            else
            {
                dn_cnt = 0;
                up_cnt = 0;
            }
        }
    }
    else if (MODE_CAL == mode)
    {
        // 上滑
        if (0x01 == _gesture)
        {
            if (g_WindowTransimitRate + 82 <= 8192)
            {
                Lepton_SetFluxParams(g_WindowTransimitRate + 82);
            }
        }
        // 下滑
        if (0x02 == _gesture)
        {
            if (g_WindowTransimitRate - 82 >= 82)
            {
                Lepton_SetFluxParams(g_WindowTransimitRate - 82);
            }
        }

        // 校准模式下，退出到正常模式
        if (0x05 == _gesture)
        {
            win_mode = MODE_NORMAL;
        }

        // 长按保存参数
        if (0x0C == _gesture)
        {
            BSP_Flash_Write(g_WindowTransimitRate);
            st7789_draw_string(20, 220, "Save success!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        }
    }

    _gesture = 0x00;
    _touch_x = 0;
    _touch_y = 0;
}


static uint8_t i2c_cs816t_write_updata(uint8_t device_addr, uint16_t RegAddr, 
    uint8_t *Data, uint16_t len, uint8_t lenth) {
    // 启动I2C通信
    i2c_Start(); 
    // 发送设备写地址
    i2c_SendByte(device_addr);
    if (!i2c_WaitAck()) {  // 若未收到ACK，通信失败
        i2c_Stop();
        return 1;
    }
    // 发送寄存器地址（根据lenth决定1字节还是2字节，通常高位在前）
    if (lenth == 2) {  // 16位地址：先发送高8位，再发送低8位
        i2c_SendByte((RegAddr >> 8) & 0xFF);  // 高8位
        if (!i2c_WaitAck()) {
            i2c_Stop();
            return 1;
        }
    }
    // 发送地址低8位（lenth=1时只发这部分）
    i2c_SendByte(RegAddr & 0xFF);
    if (!i2c_WaitAck()) {
        i2c_Stop();
        return 1;
    } 
    // 发送数据（连续发送len个字节）
    for (uint16_t i = 0; i < len; i++) {
        i2c_SendByte(Data[i]);
        if (!i2c_WaitAck()) {
            i2c_Stop();
            return 1;
        }
    }
    // 结束通信
    i2c_Stop();
    return 0;  // 成功
}


static uint8_t i2c_cs816t_read_updata(uint8_t device_addr, uint16_t RegAddr, uint8_t *Data, 
    uint8_t len, uint8_t lenth){
    // 第一步：先通过写操作设置要读取的寄存器地址
    i2c_Start();
    // 发送设备写地址（注意：读操作前需先写寄存器地址，因此用写地址）
    i2c_SendByte(device_addr);  // device_addr应为写地址（如0x6A<<1）
    if (!i2c_WaitAck()) {
        i2c_Stop();
        return 1;
    }
    // 发送寄存器地址（同写函数，支持16位/8位）
    if (lenth == 2) {  // 16位地址：高位在前
        i2c_SendByte((RegAddr >> 8) & 0xFF);
        if (!i2c_WaitAck()) {
            i2c_Stop();
            return 1;
        }
    }
    i2c_SendByte(RegAddr & 0xFF);  // 低8位
    if (!i2c_WaitAck()) {
        i2c_Stop();
        return 1;
    }
    // 第二步：重复启动I2C，切换到读操作
    i2c_Start();  // 重复启动（Repeated Start） 
    // 发送设备读地址（写地址最低位加1即为读地址）
    i2c_SendByte(device_addr | 0x01);  // 如写地址0xD4，读地址为0xD5
    if (!i2c_WaitAck()) {
        i2c_Stop();
        return 1;
    }
    // 读取数据（前len-1个字节发ACK，最后1个发NACK终止读取）
    for (uint8_t i = 0; i < len; i++) {
        Data[i] = i2c_ReadByte();  // 读取1字节
        if (i != len - 1) {
            i2c_Ack();  // 非最后1字节：发ACK让从机继续发送
        } else {
            i2c_NAck();  // 最后1字节：发NACK终止
        }
    } 
    // 结束通信
    i2c_Stop();
    return 0;  // 成功
}

#define REG_LEN_2B      (2)
// 触摸屏固件升级
static uint8_t i2c_cst816t_enter_boootmode(void)
{
    uint8_t retryCnt = 50;  // 重试次数
    
    // 进入bootmode需要拉复位：拉低10ms，拉高后10ms内发指令（7~15ms窗口）
    // while (retryCnt--) 
    while (1)
    {  // 循环重试
        i2c_cst816t_reset();
        uint8_t cmd[3];   // 指令/接收缓冲区
        
        // 不同IC的指令差异：CST816S/CST816D/CST816T使用0xAB
        cmd[0] = 0xAB;    // 进入编程模式指令
        
        // 写入指令：设备地址0x6A（左移1位为I2C从机地址），寄存器0xA001，数据长度1字节，地址长度2字节
        i2c_cs816t_write_updata((0x6A << 1), 0xA001, cmd, 1, REG_LEN_2B);
        HAL_Delay(2);  // 等待指令执行
        
        // 读取标志位：寄存器0xA003，读取1字节
        i2c_cs816t_read_updata((0x6A << 1), 0xA003, cmd, 1, REG_LEN_2B);
        HAL_Delay(2);  // 等待读取完成
        
        // 不同IC的返回值差异：CST816S等返回0x55表示成功
        if (cmd[0] != 0xC1) {
            HAL_Delay(2);  // 未成功，延时后重试
            continue;
        } else {
            return 0;  // 进入bootmode成功
        }
    }
    
    return 1;  // 重试次数耗尽，失败
}


// void HAL_GPIO_EXTI_Falling_Callback(uint16_t pin)
// {
//     if (GPIO_PIN_1 != pin)
//     {
//         return;
//     }
// }

/** \} end of group I2C_CST816T_Exported_Functions */
/** \} end of group I2C_CST816T */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/



