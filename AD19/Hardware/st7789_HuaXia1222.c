/*!
\file       st7789.c
\brief      st7789.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2023-07-24
\see        =
\version    =
\pre        =
\bug        =
\attention  ST7789 的 SPI 接口，MCU作为master进行数据传输，SPI配置中只需要配置 SCK 和 MOSI 。
            使用普通 GPIO 控制芯片的 CS, D/C, RESET, BACKLIGHT 。
\warning    =

\copyright
<h2><center>Copyright &copy; \f$\text{2023}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/

/* INCLUDE FILES **************************************************************/
/* 相关头文件的.h */

/* C库的.h */

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */
#include "bsp.h"

/*!
\defgroup ST7789 GROUP_ST7789
\brief    ST7789概述
\details  ST7789详细描述
\{
*/

/* PRIVATE DEFINES ************************************************************/

#define ST7789_135X240 0
#define ST7789_240X240 1
#define ST7789_170X320 2
#define ST7789_240X320 3
#define ST7789_240X280 4

#define LCD_SIZE ST7789_240X280

//#define LCD_ROTATION 0
//#define LCD_ROTATION 1
#define LCD_ROTATION 2
//#define LCD_ROTATION 3

///////////////////

/* resolution & offset */

#if LCD_SIZE == ST7789_240X240
    #if LCD_ROTATION == 0
        #define ST7789_WIDTH  135
        #define ST7789_HEIGHT 240
        #define ST7789_XSTART 53
        #define ST7789_YSTART 40
    #elif LCD_ROTATION == 1
        #define ST7789_WIDTH  240
        #define ST7789_HEIGHT 135
        #define ST7789_XSTART 40
        #define ST7789_YSTART 52
    #elif LCD_ROTATION == 2
        #define ST7789_WIDTH  135
        #define ST7789_HEIGHT 240
        #define ST7789_XSTART 52
        #define ST7789_YSTART 40
    #elif LCD_ROTATION == 3
        #define ST7789_WIDTH  240
        #define ST7789_HEIGHT 135
        #define ST7789_XSTART 40
        #define ST7789_YSTART 53
    #endif
#elif LCD_SIZE == ST7789_170X320
    #if LCD_ROTATION == 0
        #define ST7789_WIDTH  170
        #define ST7789_HEIGHT 320
        #define ST7789_XSTART 35
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 1
        #define ST7789_WIDTH  320
        #define ST7789_HEIGHT 170
        #define ST7789_XSTART 0
        #define ST7789_YSTART 35
    #elif LCD_ROTATION == 2
        #define ST7789_WIDTH  170
        #define ST7789_HEIGHT 320
        #define ST7789_XSTART 35
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 3
        #define ST7789_WIDTH  320
        #define ST7789_HEIGHT 170
        #define ST7789_XSTART 0
        #define ST7789_YSTART 35
    #endif
#elif LCD_SIZE == ST7789_240X240
    #define ST7789_WIDTH  240
    #define ST7789_HEIGHT 240
    #if LCD_ROTATION == 0
        #define ST7789_XSTART 0
        #define ST7789_YSTART 80
    #elif LCD_ROTATION == 1
        #define ST7789_XSTART 80
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 2
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 3
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #endif
#elif LCD_SIZE == ST7789_240X320
    #define ST7789_WIDTH  240
    #define ST7789_HEIGHT 320
    #if LCD_ROTATION == 0
        #define ST7789_XSTART 0
        #define ST7789_YSTART 80
    #elif LCD_ROTATION == 1
        #define ST7789_XSTART 80
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 2
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 3
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #endif
#elif LCD_SIZE == ST7789_240X280
    #define ST7789_WIDTH  240
    #define ST7789_HEIGHT 280
    #if LCD_ROTATION == 0
        #define ST7789_XSTART 0
        #define ST7789_YSTART 80
    #elif LCD_ROTATION == 1
        #define ST7789_XSTART 80
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 2
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #elif LCD_ROTATION == 3
        #define ST7789_XSTART 0
        #define ST7789_YSTART 0
    #endif
#endif

#if LCD_ROTATION == 0
    #define ST7789_ROTATION ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB
#elif LCD_ROTATION == 1
    #define ST7789_ROTATION ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB
#elif LCD_ROTATION == 2
    #define ST7789_ROTATION ST7789_MADCTL_RGB
#elif LCD_ROTATION == 3
    #define ST7789_ROTATION ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB
#else
    #warning "LCD_ROTATION is not be defined or range not in [0,3]"
#endif


// #define ST7789_COLOR_65K   0x50
// #define ST7789_COLOR_262K  0x60
// #define ST7789_COLOR_12BIT 0x03
// #define ST7789_COLOR_16BIT 0x05
// #define ST7789_COLOR_18BIT 0x06
// #define ST7789_COLOR_16M   0x07

// #define ST7789_COLOR_MODE_16BIT (ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT)   // 0x55
// #define ST7789_COLOR_MODE_18BIT (ST7789_COLOR_MODE_262K | ST7789_COLOR_MODE_18BIT)  // 0x66


#define ST7789_COLOR_MODE ST7789_COLOR_MODE_16BIT

/* 接口引脚定义 */
#define LCD_CS_PORT             GPIOB
#define LCD_CS_PIN              GPIO_PIN_0
#define LCD_DC_PORT             GPIOB
#define LCD_DC_PIN              GPIO_PIN_1
#define LCD_REST_PORT           GPIOC
#define LCD_REST_PIN            GPIO_PIN_5
#define LCD_BACKLIGHT_PORT      GPIOA
#define LCD_BACKLIGHT_PIN       GPIO_PIN_10


#define st7789_select()     HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define st7789_unselect()   HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET)

#define st7789_start_cmd()  HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define st7789_start_data() HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET)

#define st7789_rst_l()      HAL_GPIO_WritePin(LCD_REST_PORT, LCD_REST_PIN, GPIO_PIN_RESET)
#define st7789_rst_h()      HAL_GPIO_WritePin(LCD_REST_PORT, LCD_REST_PIN, GPIO_PIN_SET)


/*!是否手动控制NSS信号*/
#define ST7789_NSS_SIGNAL 1

#define st7789_delay(n)                 HAL_Delay(n)

#define DELAY 0x80

#define st7789_transmit(buff, size)     HAL_SPI_Transmit(&_hspi, buff, size, 1000)

/* PRIVATE TYPE DEFINITIONS ***************************************************/

/* PRIVATE FUNCTION DECLARATIONS **********************************************/

/* PRIVATE DATA DECLARATIONS **************************************************/

static SPI_HandleTypeDef _hspi;
static DMA_HandleTypeDef _spi_dma_tx;
static __IO uint8_t _dma_tx_done = 0;
static uint16_t _lcd_buff[ST7789_WIDTH * 80];

/* BS30 配置命令， PuYang 屏幕可以使用 */
#if 1
static const uint8_t st7789_init_cmd[] =
{
    18, // count of commands
    //////////////////////////////////////////////////////////////////////////
    ST7789_SWRESET,     DELAY,  50,                             // soft reset
    ST7789_COLMOD,      1,      ST7789_COLOR_MODE,              // color mode
    ST7789_MADCTL,      1,      ST7789_ROTATION,                // display rotation
    ST7789_PORCTRL,     5,      0x7F, 0x7F, 0x00, 0x33, 0x33,//0x0C, 0x0C, 0x00, 0x33, 0x33,   // porch control
    ST7789_GCTRL,       1,      0x35,                           // gate control
    ST7789_VCOMS,       1,      0x19,                           // default 0.75v for 0x20
    ST7789_LCMCTRL,     1,      0x2C,
    ST7789_VDVVRHEN,    1,      0x01,
    ST7789_VRHS,        1,      0x12,   // +-4.45v, defalut +-4.1v for 0x0B
    ST7789_VDVSET,      1,      0x20,
    ST7789_FRCTR2,      1,      0x1F,   // frame rate = 60hz
    ST7789_PWCTRL1,     2,      0xA4, 0XA1,
    ST7789_PVGAMCTRL,   14,     0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    ST7789_NVGAMCTRL,   14,     0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    ST7789_INVON,       0,              //   inverst color
    ST7789_SLPOUT,      DELAY,  50,     //   out of sleep mode
    ST7789_DISPON,      DELAY,  50,     //   screen turned on
    ST7789_NORON,       0,              //   normal display on
    //////////////////////////////////////////////////////////////////////////
    // end
    0
};
#endif


#if 0
const uint8_t st7789_init_cmd[] =
{
    16, // count of commands
    //////////////////////////////////////////////////////////////////////////
    ST7789_SLPOUT,     0,
    ST7789_COLMOD,     1,  ST7789_COLOR_MODE,
    ST7789_PORCTRL,    5,  0x0C, 0x0C, 0x00, 0x33, 0x33,   // Standard porch
    ST7789_GCTRL,      1,  0x35,                           // Gate Control, Default value
    ST7789_VCOMS,      1,  0x19,                           // VCOM setting 0.725v (default 0.75v for 0x20)
    ST7789_LCMCTRL,    1,  0X2C,                           // LCMCTRL, Default value
    ST7789_VDVVRHEN,   1,  0x01,                           // VDV and VRH command Enable, Default value
    ST7789_VRHS,       1,  0x12,                           // VRH set, +-4.45v (default +-4.1v for 0x0B)
    ST7789_VDVSET,     1,  0x20,                           // VDV set, Default value
    ST7789_FRCTR2,     1,  0x0F,                           // Frame rate control in normal mode, Default refresh rate (60Hz)
    ST7789_PWCTRL1,    2,  0xA4, 0xA1,
    ST7789_MADCTL,     1,  ST7789_ROTATION,
    ST7789_PVGAMCTRL,  14, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    ST7789_NVGAMCTRL,  14, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    ST7789_INVON,      0,
    ST7789_NORON,      0,
    //////////////////////////////////////////////////////////////////////////
    // end
    0
};
#endif


/* EXPORTED PUBLIC DATA *******************************************************/

/* PRIVATE MODULE FUNCTIONS ***************************************************/
static void _write_cmd(uint8_t cmd)
{
    st7789_select();
    st7789_start_cmd();
    st7789_transmit(&cmd, 1);
    st7789_unselect();
}

static void _write_byte(uint8_t dat)
{
    st7789_select();
    st7789_start_data();
    st7789_transmit(&dat, 1);
    st7789_unselect();
}

static void _write_buffer(const uint8_t *dat, uint16_t len)
{
    st7789_select();
    st7789_start_data();
    if (HAL_OK != st7789_transmit(dat, len))
    {
        LOG_D("spi trans failed!");
        while (1);
    }
    st7789_unselect();
}


void _reset(void)
{
    st7789_delay(25);
    st7789_rst_l();
    st7789_delay(25);
    st7789_rst_h();
    st7789_delay(50);
}

void _set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4] = { 0x00 };

    x1 += ST7789_XSTART;
    x2 += ST7789_XSTART;
    y1 += ST7789_YSTART;
    y2 += ST7789_YSTART;

    // column address set
    data[0] = x1 >> 8;
    data[1] = x1 & 0xFF;
    data[2] = x2 >> 8;
    data[3] = x2 & 0xFF;
    _write_cmd(ST7789_CASET);
    _write_buffer(data, 4);

    // row address set
    data[0] = y1 >> 8;
    data[1] = y1 & 0xFF;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xFF;
    _write_cmd(ST7789_RASET);
    _write_buffer(data, 4);

    // write to RAM
    _write_cmd(ST7789_RAMWR);
}


static void _init_gpio(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = LCD_BACKLIGHT_PIN;
    HAL_GPIO_Init(LCD_BACKLIGHT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_DC_PIN;
    HAL_GPIO_Init(LCD_DC_PORT, &GPIO_InitStruct);


    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = LCD_REST_PIN;
    HAL_GPIO_Init(LCD_REST_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_CS_PIN;
    HAL_GPIO_Init(LCD_CS_PORT, &GPIO_InitStruct);
}


#define hspi1 _hspi
#define _hspi4 _hspi

static void _init_spi(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 0x7;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }


    SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = { 0 };
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
    if (HAL_SPIEx_SetConfigAutonomousMode(&hspi1, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
    {
        Error_Handler();
    }



#if 0
    _hspi4.Instance = SPI1;
    _hspi4.Init.Mode = SPI_MODE_MASTER;
    _hspi4.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;// SPI_DIRECTION_2LINES_TXONLY;// SPI_DIRECTION_2LINES;
    _hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
    _hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
    _hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
    _hspi4.Init.NSS = SPI_NSS_SOFT;
    _hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    _hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
    _hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
    _hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    _hspi4.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&_hspi4) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

#define handle_GPDMA1_Channel7 _spi_dma_tx
static void _init_spi_dma(void)
{
#if 1
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(GPDMA1_Channel7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel7_IRQn);

    /* SPI1 DMA Init */
    /* GPDMA1_REQUEST_SPI1_TX Init */
    handle_GPDMA1_Channel7.Instance = GPDMA1_Channel7;
    handle_GPDMA1_Channel7.Init.Request = GPDMA1_REQUEST_SPI1_TX;
    handle_GPDMA1_Channel7.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel7.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_GPDMA1_Channel7.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_GPDMA1_Channel7.Init.DestInc = DMA_DINC_FIXED;
    handle_GPDMA1_Channel7.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel7.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel7.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel7.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel7.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel7.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel7.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel7.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel7) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&_hspi, hdmatx, handle_GPDMA1_Channel7);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel7, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);

    return;
#endif

#if 0
    _spi_dma_tx.Instance                 = GPDMA1_Channel7;
    _spi_dma_tx.Init.Request             = DMA_REQUEST_SW;
    _spi_dma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    _spi_dma_tx.Init.DestInc             = DMA_DINC_FIXED;
    _spi_dma_tx.Init.SrcInc              = DMA_SINC_INCREMENTED;
    _spi_dma_tx.Init.DestDataWidth       = DMA_SRC_DATAWIDTH_BYTE;
    _spi_dma_tx.Init.SrcDataWidth        = DMA_SRC_DATAWIDTH_BYTE;
    _spi_dma_tx.Init.Mode                = DMA_NORMAL;
    _spi_dma_tx.Init.Priority            = DMA_LOW_PRIORITY_LOW_WEIGHT;
    //_spi_dma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
#endif


    if (HAL_DMA_Init(&_spi_dma_tx) != HAL_OK)
    {
        Error_Handler();
    }


    if (HAL_DMA_ConfigChannelAttributes(&_spi_dma_tx, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }
}



static void _init_cmd(void)
{
    _reset();


    /* BS30 */
#if 1
    const uint8_t *addr = st7789_init_cmd;
    uint8_t cmd_cnt, arg_cnt, ms;

    while (1)
    {
        // count of command
        if ((cmd_cnt = *addr++) == 0)
        {
            break;
        }
        while (cmd_cnt--)
        {
            // write comand
            // printf("cmd = 0x%x\r\n", *addr);
            _write_cmd(*addr++);
            // counf of arguments
            if ((arg_cnt = *addr++) > 0)
            {
                // if high bit set, delay follows args
                if (arg_cnt & DELAY)
                {
                    ms = *addr++;
                    st7789_delay((ms == 255) ? 500 : ms);
                }
                else
                {
                    _write_buffer(addr, arg_cnt);
                    addr += arg_cnt;
                }
            }
        }
    }
#endif

    /*  华夏彩光H0183S009T009
        SPI 配置
            hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
            hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
    */
#if 0

#define WriteComm   _write_cmd
#define WriteData   _write_byte
#define Delay       HAL_Delay

    WriteComm(0x11);
    Delay(200);
    WriteComm(0xB2);
    WriteData(0x0C);
    WriteData(0x0C);
    WriteData(0x00);
    WriteData(0x33);
    WriteData(0x33);
    WriteComm(0x35);
    WriteData(0x00);
    WriteComm(0x36);
    WriteData(0x00);
    WriteComm(0x3A);
    WriteData(0x05);
    WriteComm(0xB7);
    WriteData(0x66);//VGH=14.5V VGL=-11.38V
    WriteComm(0xBB);
    WriteData(0x22);
    WriteComm(0xC0);
    WriteData(0x2C);
    WriteComm(0xC2);
    WriteData(0x01);
    WriteComm(0xC3);
    WriteData(0x19);//4.8V
    WriteComm(0xC6);
    WriteData(0x0F);
    WriteComm(0xD0);
    WriteData(0xA7);
    WriteComm(0xD0);
    WriteData(0xA4);
    WriteData(0xA1);
    WriteComm(0xD6);
    WriteData(0xA1);
    WriteComm(0xE0);
    WriteData(0xF0);
    WriteData(0x00);
    WriteData(0x04);
    WriteData(0x03);
    WriteData(0x03);
    WriteData(0x10);
    WriteData(0x2C);
    WriteData(0x43);
    WriteData(0x43);
    WriteData(0x38);
    WriteData(0x14);
    WriteData(0x13);
    WriteData(0x2A);
    WriteData(0x30);
    WriteComm(0xE1);
    WriteData(0xF0);
    WriteData(0x08);
    WriteData(0x0B);
    WriteData(0x0E);
    WriteData(0x0D);
    WriteData(0x08);
    WriteData(0x2B);
    WriteData(0x33);
    WriteData(0x43);
    WriteData(0x3A);
    WriteData(0x15);
    WriteData(0x15);
    WriteData(0x2C);
    WriteData(0x32);
    WriteComm(0x21);
    WriteComm(0x29);
    //WriteComm(0x2C);
#endif
}




/*!带 FIFO 功能的 DMA 传输。

内存数据为 16bit 数据，SPI 为 8bit 数据，
数据宽度不匹配，需要使用 FIFO 功能实现 DMA 传输。*/
#if 0
static void _init_spi_dma_with_fifo(void)
{
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    _spi_dma_tx.Instance                 = GPDMA1_Channel7;
    _spi_dma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    _spi_dma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    _spi_dma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    _spi_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    _spi_dma_tx.Init.MemDataAlignment    = DMA_PDATAALIGN_HALFWORD;
    _spi_dma_tx.Init.Mode                = DMA_NORMAL;
    _spi_dma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    _spi_dma_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    _spi_dma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    _spi_dma_tx.Init.MemBurst            = DMA_MBURST_INC8;
    _spi_dma_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&_spi_dma_tx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&_hspi, hdmatx, _spi_dma_tx);

    HAL_NVIC_SetPriority(GPDMA1_Channel7_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel7_IRQn);
}
#endif

void GPDMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(_hspi.hdmatx);
}

void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&_hspi);
}

/*!
启动 SPI DMA 传输时，会将 hdmatx->XferCpltCallback 绑定到 SPI_DMATransmitCplt 。
HAL_DMA_IRQHandler 中断处理中会调用 SPI_DMATransmitCplt 。
    1. 如果 DMA 模式为 DMA_LINKEDLIST_CIRCULAR ，那么 SPI_DMATransmitCplt 会回调 HAL_SPI_TxCpltCallback
    2. 否则 触发 SPI 的 EOT 中断，由 HAL_SPI_IRQHandler 回调 HAL_SPI_TxCpltCallback

不适用 LINKEDLIST 时，必须同时开启 SPI 和 DMA 中断，才会回调该函数
*/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    _dma_tx_done = 1;
}


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\defgroup ST7789_Exported_Functions ST7789 导出的函数接口
\brief    导出的函数接口
\{
*/

void st7789_init(void)
{
    _init_gpio();

    _init_spi();

    //_init_spi_dma();

    _init_cmd();
}



// invert color
void st7789_invert(uint8_t enable)
{
    _write_cmd(enable ? ST7789_INVON : ST7789_INVOFF);
}

// //////////////////////////////////////////////////////////////

void st7789_clear(uint16_t color)
{
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

/*!
使用DMA传输方式执行填充操作。

HAL库DMA最大传输 uint16_t 字节，因此，可能需要多次传输。
*/
void st7789_fill_rect_dma(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    st7789_fill_rect(x, y, w, h, color);
    return;

    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT) || (0 == w) || (0 == h))
    {
        return;
    }
    if ((x + w) > ST7789_WIDTH)
    {
        w = ST7789_WIDTH - x;
    }
    if ((y + h) > ST7789_HEIGHT)
    {
        h = ST7789_HEIGHT - y;
    }

    /*填充颜色缓冲区*/
    color = REVERSE_RGB565(color);

    uint16_t size = MIN(sizeof(_lcd_buff) / 2, w * h);
    for (uint16_t i = 0; i < size; i++)
    {
        _lcd_buff[i] = color;
    }

    uint32_t rem_bytes, trans_bytes;
    uint16_t trans_blk = size * 2;
    /*uint16_t trans_blk = 0xFFF0;*/

    _set_window(x, y, x + w - 1, y + h - 1);

    /*DMA传输禁用内存地址增长*/
    /*__HAL_DMA_DISABLE(_hspi4.hdmatx);
    _hspi4.hdmatx->Instance->CR &= ~DMA_MINC_ENABLE;*/

    rem_bytes = w * h * 2;
    while (1)
    {
        if (rem_bytes > trans_blk)
        {
            trans_bytes = trans_blk;
        }
        else
        {
            trans_bytes = rem_bytes;
        }

        _dma_tx_done = 0;

        st7789_select();
        if (HAL_OK != HAL_SPI_Transmit_DMA(&_hspi, (uint8_t *)&_lcd_buff, trans_bytes))
        {
            Error_Handler();
        }
        st7789_unselect();

        while (!_dma_tx_done)
        {
            HAL_Delay(10);
        }

        rem_bytes -= trans_bytes;

        if (0 == rem_bytes)
        {
            break;
        }
    }

    /*DMA传输启用内存地址增长*/
    /*__HAL_DMA_DISABLE(_hspi4.hdmatx);
    _hspi4.hdmatx->Instance->CR |= DMA_MINC_ENABLE;*/
}

void st7789_clear_dma(uint16_t color)
{
    st7789_fill_rect_dma(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
    //st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
#if 0
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
    {
        return;
    }
#endif
#if ST7789_NSS_SIGNAL
    st7789_select();
#endif
    _set_window(x, y, x + 1, y + 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    _write_buffer(data, sizeof(data));
#if ST7789_NSS_SIGNAL
    st7789_unselect();
#endif
}

void st7789_draw_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;
    _set_window(x, y, x + font.width - 1, y + font.height - 1);
    for (i = 0; i < font.height; ++i)
    {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; ++j)
        {
            if ((b << j) & 0x8000)
            {
                uint8_t data[] = { color >> 8, color & 0xFF };
                _write_buffer(data, sizeof(data));
            }
            else
            {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                _write_buffer(data, sizeof(data));
            }
        }
    }
}

void st7789_draw_string(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    while (*str)
    {
        if (x + font.width >= ST7789_WIDTH)
        {
            x = 0;
            y += font.height;
            if (y + font.height >= ST7789_HEIGHT)
            {
                break;
            }
            if (*str == ' ')
            {
                // skip spaces in the beginning of the new line
                ++str;
                continue;
            }
        }

        st7789_draw_char(x, y, *str, font, color, bgcolor);
        x += font.width;
        ++str;
    }
}

void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)
        || (0 == w) || (0 == h))
    {
        return;
    }
    if ((x + w) > ST7789_WIDTH)
    {
        w = ST7789_WIDTH - x;
    }
    if ((y + h) > ST7789_HEIGHT)
    {
        h = ST7789_HEIGHT - y;
    }

    _set_window(x, y, x + w - 1, y + h - 1);

    /*填充颜色缓冲区*/
    color = REVERSE_RGB565(color);

    /* 使用缓冲区刷新 */
#if 1
    uint16_t size = MIN(sizeof(_lcd_buff) / 2, w * h);
    for (uint16_t i = 0; i < size; i++)
    {
        _lcd_buff[i] = color;
    }

    uint32_t rem_bytes, trans_bytes;
    uint16_t trans_blk = size * 2;

    rem_bytes = w * h * 2;
    while (1)
    {
        if (rem_bytes > trans_blk)
        {
            trans_bytes = trans_blk;
        }
        else
        {
            trans_bytes = rem_bytes;
        }


        _write_buffer((uint8_t *)&_lcd_buff, trans_bytes);

        rem_bytes -= trans_bytes;

        if (0 == rem_bytes)
        {
            break;
        }
    }
#endif

    /* 直接刷新 */
#if 0
    uint32_t size = w * h;
    uint8_t  data[2] = { color >> 8, color & 0xFF };
    while (size--)
    {
        st7789_write_ndata(data, 2);
    }
#endif
}

void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
    {
        return;
    }
    if ((x + w) > ST7789_WIDTH)
    {
        return;
    }
    if ((y + h) > ST7789_HEIGHT)
    {
        return;
    }
    _set_window(x, y, x + w - 1, y + h - 1);
    _write_buffer((const uint8_t *)data, sizeof(uint16_t) * w * h);
}

void st7789_draw_image_dma(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT) || (0 == w) || (0 == h))
    {
        return;
    }
    if ((x + w) > ST7789_WIDTH)
    {
        return;
    }
    if ((y + h) > ST7789_HEIGHT)
    {
        return;
    }

    uint32_t rem_bytes, trans_bytes;
    uint16_t trans_blk = 0xFFF0;

#if ST7789_NSS_SIGNAL
    st7789_select();
#endif

    _set_window(x, y, x + w - 1, y + h - 1);
    st7789_start_data();

    rem_bytes = w * h * 2;
    while (1)
    {
        if (rem_bytes > trans_blk)
        {
            trans_bytes = trans_blk;
        }
        else
        {
            trans_bytes = rem_bytes;
        }

        _dma_tx_done = 0;
        HAL_SPI_Transmit_DMA(&_hspi, (const uint8_t *)data, trans_bytes);
        while (!_dma_tx_done);

        rem_bytes -= trans_bytes;
        data += (trans_bytes / 2);

        if (0 == rem_bytes)
        {
            break;
        }
    }

#if ST7789_NSS_SIGNAL
    st7789_unselect();
#endif
}

void st7789_show_image(IMAGE_ELEMENT *pimg)
{
    st7789_draw_image_dma(pimg->x, pimg->y,
        pimg->w, pimg->h, pimg->pdata);
}

/*!绘制数字字符串，函数将自动过滤非数字的字符*/
void st7789_show_num_str(uint16_t x, uint16_t y, char *str)
{
    IMAGE_ELEMENT *pimg = 0;
    while (*str)
    {
        if ((*str < '0' || *str > '9') && (' ' != *str) && ('>' != *str))
        {
            ++str;
            continue;
        }


        if (x + pimg->w >= ST7789_WIDTH)
        {
            x = 0;
            y += pimg->h;
            if (y + pimg->h >= ST7789_HEIGHT)
            {
                break;
            }
            if (*str == ' ')
            {
                ++str;
                continue;
            }
        }

        st7789_draw_image_dma(x, y, pimg->w, pimg->h, pimg->pdata);

        x += pimg->w;
        ++str;
    }
}
void st7789_show_num_str_s(uint16_t x, uint16_t y, char *str)
{
    IMAGE_ELEMENT *pimg = 0;
    while (*str)
    {
        if ((*str < '0' || *str > '9')
            && (' ' != *str)
            && ('.' != *str)
            && ('V' != *str))
        {
            ++str;
            continue;
        }


        if (x + pimg->w >= ST7789_WIDTH)
        {
            x = 0;
            y += pimg->h;
            if (y + pimg->h >= ST7789_HEIGHT)
            {
                break;
            }
            if (*str == ' ')
            {
                ++str;
                continue;
            }
        }

        st7789_draw_image_dma(x, y, pimg->w, pimg->h, pimg->pdata);

        x += pimg->w;
        ++str;
    }
}


void st7789_backlight_on(void)
{
    HAL_GPIO_WritePin(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, GPIO_PIN_SET);
}
void st7789_backlight_off(void)
{
    HAL_GPIO_WritePin(LCD_BACKLIGHT_PORT, LCD_BACKLIGHT_PIN, GPIO_PIN_RESET);
}


void st7789_test(void)
{
    st7789_init();
    st7789_backlight_on();

    uint16_t colors[] =
    {
        COLOR_RGB565_BLACK,
        COLOR_RGB565_WHITE,
        COLOR_RGB565_RED,
        COLOR_RGB565_GREEN,
        COLOR_RGB565_BLUE,
        COLOR_RGB565_BLACK,
    };
    uint8_t color_cnt = (sizeof(colors) / sizeof(colors[0]));

    uint8_t bInvert = 0;

    srand(SysTick->VAL);

    while (1)
    {
        for (uint8_t i = 0; i < color_cnt; ++i)
        {
            st7789_clear_dma(colors[i]);
            HAL_Delay(1000);
        }

        int n = 50;
        while (--n)
        {
            st7789_fill_rect_dma(
                rand() % ST7789_WIDTH,
                rand() % ST7789_HEIGHT,
                rand() % 100,
                rand() % 100,
                colors[rand() % color_cnt]);
            HAL_Delay(10);
        }

        //st7789_draw_image_dma(20, 20, 128, 128, (uint16_t *)image_saber);
        st7789_draw_image(20, 20, 128, 128, (uint16_t *)image_saber);
        HAL_Delay(1000);
        //st7789_draw_image(60, 60, 114, 77, (uint16_t *)fry_data);
        //HAL_Delay(1000);

        n = 0;

        st7789_draw_string(0, 0, "hello, world", Font_7x10, COLOR_RGB565_RED, COLOR_RGB565_WHITE);
        st7789_draw_string(10, 20, "hello, world", Font_11x18, COLOR_RGB565_YELLOW, COLOR_RGB565_BLUE);
        st7789_draw_string(20, 40, "hello, world", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_LGRAY);
        HAL_Delay(1000);

        //st7789_invert(bInvert = !bInvert);
        //HAL_Delay(1000);
    }
}

/** \} end of group ST7789_Exported_Functions */
/** \} end of group ST7789 */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
















