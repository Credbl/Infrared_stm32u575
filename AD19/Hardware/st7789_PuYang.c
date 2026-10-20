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
\attention  =
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

#define ST7789_NOP        0x00
#define ST7789_SWRESET    0x01
#define ST7789_RDDID      0x04
#define ST7789_RDDST      0x09

#define ST7789_RDDPM      0x0A  // read display power mode
#define ST7789_RDD_MADCTL 0x0B  // read display MADCTL
#define ST7789_RDD_COLMOD 0x0C  // read display pixel format
#define ST7789_RDDIM      0x0D  // read display image mode
#define ST7789_RDDSM      0x0E  // read display signal mode
#define ST7789_RDDSR      0x0F  // read display self-diagnostic result (ST7789V)

#define ST7789_SLPIN      0x10  // enable sleep mode
#define ST7789_SLPOUT     0x11  // disable sleep mode
#define ST7789_PTLON      0x12
#define ST7789_NORON      0x13

#define ST7789_INVOFF     0x20  // non-invert color
#define ST7789_INVON      0x21  // invert color 反色
#define ST7789_GAMSET     0x26  // gamma set
#define ST7789_DISPOFF    0x28  // display off
#define ST7789_DISPON     0x29  // display on
#define ST7789_CASET      0x2A  // column address set
#define ST7789_RASET      0x2B  // row address set
#define ST7789_RAMWR      0x2C
#define ST7789_RGBSET     0x2D  // color setting for 4096, 64K and 262K colors
#define ST7789_RAMRD      0x2E

#define ST7789_PTLAR      0x30
#define ST7789_VSCRDEF    0x33  // vertical scrolling definition (ST7789V)
#define ST7789_TEOFF      0x34  // tearing effect line off
#define ST7789_TEON       0x35  // tearing effect line on
#define ST7789_MADCTL     0x36  // memory data access control
#define ST7789_IDMOFF     0x38  // idle mode off
#define ST7789_IDMON      0x39  // idle mode on
#define ST7789_RAMWRC     0x3C  // memory write continue (ST7789V)
#define ST7789_RAMRDC     0x3E  // memory read continue (ST7789V)
#define ST7789_COLMOD     0x3A

#define ST7789_RAMCTRL    0xB0  // RAM control
#define ST7789_RGBCTRL    0xB1  // RGB control
#define ST7789_PORCTRL    0xB2  // Porch control
#define ST7789_FRCTRL1    0xB3  // Frame rate control
#define ST7789_PARCTRL    0xB5  // Partial mode control
#define ST7789_GCTRL      0xB7  // Gate control
#define ST7789_GTADJ      0xB8  // Gate on timing adjustment
#define ST7789_DGMEN      0xBA  // Digital gamma enable
#define ST7789_VCOMS      0xBB  // VCOMS setting
#define ST7789_LCMCTRL    0xC0  // LCM control
#define ST7789_IDSET      0xC1  // ID setting
#define ST7789_VDVVRHEN   0xC2  // VDV and VRH command enable
#define ST7789_VRHS       0xC3  // VRH set
#define ST7789_VDVSET     0xC4  // VDV setting
#define ST7789_VCMOFSET   0xC5  // VCOMS offset set
#define ST7789_FRCTR2     0xC6  // FR Control 2
#define ST7789_CABCCTRL   0xC7  // CABC control
#define ST7789_REGSEL1    0xC8  // Register value section 1
#define ST7789_REGSEL2    0xCA  // Register value section 2
#define ST7789_PWMFRSEL   0xCC  // PWM frequency selection
#define ST7789_PWCTRL1    0xD0  // Power control 1
#define ST7789_VAPVANEN   0xD2  // Enable VAP/VAN signal output
#define ST7789_CMD2EN     0xDF  // Command 2 enable
#define ST7789_PVGAMCTRL  0xE0  // Positive voltage gamma control
#define ST7789_NVGAMCTRL  0xE1  // Negative voltage gamma control
#define ST7789_DGMLUTR    0xE2  // Digital gamma look-up table for red
#define ST7789_DGMLUTB    0xE3  // Digital gamma look-up table for blue
#define ST7789_GATECTRL   0xE4  // Gate control
#define ST7789_SPI2EN     0xE7  // SPI2 enable
#define ST7789_PWCTRL2    0xE8  // Power control 2
#define ST7789_EQCTRL     0xE9  // Equalize time control
#define ST7789_PROMCTRL   0xEC  // Program control
#define ST7789_PROMEN     0xFA  // Program mode enable
#define ST7789_NVMSET     0xFC  // NVM setting
#define ST7789_PROMACT    0xFE  // Program action

#define ST7789_RDID1      0xDA
#define ST7789_RDID2      0xDB
#define ST7789_RDID3      0xDC
#define ST7789_RDID4      0xDD


#define ST7789_135X240 0
#define ST7789_240X240 1
#define ST7789_170X320 2
#define ST7789_240X320 3
#define ST7789_240X280 4

#define LCD_SIZE ST7789_240X320

/* rotation */

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

/* rotation */

/**
* Memory Data Access Control Register (0x36H)
* MAP:     D7  D6  D5  D4  D3  D2  D1  D0
* param:   MY  MX  MV  ML  RGB MH  -   -
*
*/
/* Page Address Order ('0': Top to Bottom, '1': the opposite) */
#define ST7789_MADCTL_MY 0x80
/* Column Address Order ('0': Left to Right, '1': the opposite) */
#define ST7789_MADCTL_MX 0x40
/* Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode) */
#define ST7789_MADCTL_MV 0x20
/* Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite) */
#define ST7789_MADCTL_ML 0x10
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
#define ST7789_MADCTL_RGB 0x00

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

#define ST7789_COLOR_MODE_16BIT 0x55  //  RGB565 (16bit)
#define ST7789_COLOR_MODE_18BIT 0x66  //  RGB666 (18bit)

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
static uint16_t _lcd_buff[ST7789_WIDTH * 20];
static const uint8_t st7789_init_cmd[] =
{
    // count of commands
    18,
    ST7789_SWRESET, DELAY, 50,                          // soft reset
    ST7789_COLMOD, 1, ST7789_COLOR_MODE,                 // color mode
    ST7789_MADCTL, 1, ST7789_ROTATION,                   // display rotation
    ST7789_PORCTRL, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,    // porch control
    ST7789_GCTRL, 1, 0x35,                                // gate control
    ST7789_VCOMS, 1, 0x19,                                // default 0.75v for 0x20
    ST7789_LCMCTRL, 1, 0x2C,
    ST7789_VDVVRHEN, 1, 0x01,
    ST7789_VRHS, 1, 0x12,       // +-4.45v, defalut +-4.1v for 0x0B
    ST7789_VDVSET, 1, 0x20,
    ST7789_FRCTR2, 1, 0x0F,  // frame rate = 60hz
    ST7789_PWCTRL1, 2, 0xA4, 0XA1,
    ST7789_PVGAMCTRL, 14, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    ST7789_NVGAMCTRL, 14, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    ST7789_INVON, 0,               //   inverst color
    ST7789_SLPOUT, DELAY, 50,     //   out of sleep mode
    ST7789_DISPON, DELAY, 50,     //   screen turned on
    ST7789_NORON, 0,               //   normal display on
    // end
    0
};

/* EXPORTED PUBLIC DATA *******************************************************/

/* PRIVATE MODULE FUNCTIONS ***************************************************/
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

#if 0
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    PeriphClkInit.Spi1ClockSelection = RCC_SPI1CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_RCC_SPI1_CLK_ENABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA7     ------> SPI1_MOSI
    PB0     ------> SPI1_NSS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//     GPIO_InitStruct.Pin = GPIO_PIN_0;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
}


#define hspi1 _hspi
#define _hspi4 _hspi

static void _init_spi(void)
{
    _hspi4.Instance = SPI1;
    _hspi4.Init.Mode = SPI_MODE_MASTER;
    _hspi4.Init.Direction = SPI_DIRECTION_2LINES;
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
}


static void _init_spi_dma(void)
{
    __HAL_RCC_GPDMA1_CLK_ENABLE();

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
    if (HAL_DMA_Init(&_spi_dma_tx) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_LINKDMA(&_hspi, hdmatx, _spi_dma_tx);

    HAL_NVIC_SetPriority(GPDMA1_Channel7_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel7_IRQn);
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


void GPDMA1_Channel7_IRQHandler()
{
    HAL_DMA_IRQHandler(_hspi.hdmatx);
}

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
void st7789_write_cmd(uint8_t cmd)
{
    st7789_select();
    st7789_start_cmd();
    st7789_transmit(&cmd, 1);
    st7789_unselect();
}

void st7789_write_data(uint8_t dat)
{
    st7789_select();
    st7789_start_data();
    st7789_transmit(&dat, 1);
    st7789_unselect();
}

void st7789_write_ndata(const uint8_t *dat, uint16_t len)
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

void st7789_reset(void)
{
    st7789_delay(25);
    st7789_rst_l();
    st7789_delay(25);
    st7789_rst_h();
    st7789_delay(50);
}

void st7789_init2(void)
{
    _init_gpio();
    _init_spi();
    //_init_spi_dma();


#if 0

    st7789_reset();  // hard reest

#if ST7789_NSS_SIGNAL
    st7789_select();
#endif

    const uint8_t *addr = st7789_init_cmd;
    uint8_t        cmd_cnt, arg_cnt, ms;

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
            st7789_write_cmd(*addr++);
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
                    st7789_write_ndata(addr, arg_cnt);
                    addr += arg_cnt;
                }
            }
        }
    }

#if ST7789_NSS_SIGNAL
    st7789_unselect();
#endif

#endif


    //LCD厂商配置
#if 0

#define WriteComm st7789_write_cmd
#define WriteData st7789_write_data
#define Delay HAL_Delay

    st7789_select();

    WriteComm(0x11);
    Delay(120);
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
    WriteComm(0x2C);

    st7789_unselect();
#endif


#if 1

#define ST7789_WriteCommand st7789_write_cmd
#define ST7789_WriteSmallData st7789_write_data
#define ST7789_WriteData st7789_write_ndata


    st7789_reset();
    st7789_select();

    ST7789_WriteCommand(ST7789_COLMOD);		//	Set color mode
    ST7789_WriteSmallData(ST7789_COLOR_MODE);
    ST7789_WriteCommand(0xB2);				//	Porch control
    {
        uint8_t data[] = { 0x0C, 0x0C, 0x00, 0x33, 0x33 };
        ST7789_WriteData(data, sizeof(data));
    }
    //ST7789_SetRotation(ST7789_ROTATION);	//	MADCTL (Display Rotation)

    /* Internal LCD Voltage generator settings */
    ST7789_WriteCommand(0XB7);				//	Gate Control
    ST7789_WriteSmallData(0x35);			//	Default value
    ST7789_WriteCommand(0xBB);				//	VCOM setting
    ST7789_WriteSmallData(0x19);			//	0.725v (default 0.75v for 0x20)
    ST7789_WriteCommand(0xC0);				//	LCMCTRL
    ST7789_WriteSmallData(0x2C);			//	Default value
    ST7789_WriteCommand(0xC2);				//	VDV and VRH command Enable
    ST7789_WriteSmallData(0x01);			//	Default value
    ST7789_WriteCommand(0xC3);				//	VRH set
    ST7789_WriteSmallData(0x12);			//	+-4.45v (defalut +-4.1v for 0x0B)
    ST7789_WriteCommand(0xC4);				//	VDV set
    ST7789_WriteSmallData(0x20);			//	Default value
    ST7789_WriteCommand(0xC6);				//	Frame rate control in normal mode
    ST7789_WriteSmallData(0x0F);			//	Default value (60HZ)
    ST7789_WriteCommand(0xD0);				//	Power control
    ST7789_WriteSmallData(0xA4);			//	Default value
    ST7789_WriteSmallData(0xA1);			//	Default value
    /**************** Division line ****************/

    ST7789_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23 };
        ST7789_WriteData(data, sizeof(data));
    }

    ST7789_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23 };
        ST7789_WriteData(data, sizeof(data));
    }
    ST7789_WriteCommand(ST7789_INVON);		//	Inversion ON
    ST7789_WriteCommand(ST7789_SLPOUT);	//	Out of sleep mode
    ST7789_WriteCommand(ST7789_NORON);		//	Normal Display on
    ST7789_WriteCommand(ST7789_DISPON);	//	Main screen turned on
#endif

}


static void _init_cmd(void)
{
    st7789_reset();


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
            st7789_write_cmd(*addr++);
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
                    st7789_write_ndata(addr, arg_cnt);
                    addr += arg_cnt;
                }
            }
        }
    }
#endif

    /* datasheet */
#if 0

#define WriteComm st7789_write_cmd
#define WriteData st7789_write_data
#define Delay HAL_Delay


    WriteComm(0x11);
    Delay(120);
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
    WriteComm(0x2C);

#endif

}

void st7789_init(void)
{
    _init_gpio();

    _init_spi();

    _init_cmd();
}


void st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
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
    st7789_write_cmd(ST7789_CASET);
    st7789_write_ndata(data, 4);

    // row address set
    data[0] = y1 >> 8;
    data[1] = y1 & 0xFF;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xFF;
    st7789_write_cmd(ST7789_RASET);
    st7789_write_ndata(data, 4);

    // write to RAM
    st7789_write_cmd(ST7789_RAMWR);
}

// invert color
void st7789_invert(uint8_t enable)
{
    st7789_write_cmd(enable ? ST7789_INVON : ST7789_INVOFF);
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


    st7789_set_window(x, y, x + w - 1, y + h - 1);


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

        HAL_SPI_Transmit_DMA(&_hspi, (uint8_t *)&_lcd_buff, trans_bytes);
        while (!_dma_tx_done);

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
    //st7789_fill_rect_dma(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
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
    st7789_set_window(x, y, x + 1, y + 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    st7789_write_ndata(data, sizeof(data));
#if ST7789_NSS_SIGNAL
    st7789_unselect();
#endif
}

void st7789_draw_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;
    st7789_set_window(x, y, x + font.width - 1, y + font.height - 1);
    for (i = 0; i < font.height; ++i)
    {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; ++j)
        {
            if ((b << j) & 0x8000)
            {
                uint8_t data[] = { color >> 8, color & 0xFF };
                st7789_write_ndata(data, sizeof(data));
            }
            else
            {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                st7789_write_ndata(data, sizeof(data));
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

    st7789_set_window(x, y, x + w - 1, y + h - 1);

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


        st7789_write_ndata((uint8_t *)&_lcd_buff, trans_bytes);

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
    st7789_set_window(x, y, x + w - 1, y + h - 1);
    st7789_write_ndata((const uint8_t *)data, sizeof(uint16_t) * w * h);
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

    st7789_set_window(x, y, x + w - 1, y + h - 1);
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

    //srand(SysTick->VAL);

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

        n = 0;

        st7789_draw_string(0, 0, "hello, world", Font_7x10, COLOR_RGB565_RED, COLOR_RGB565_WHITE);
        st7789_draw_string(0, 20, "hello, world", Font_11x18, COLOR_RGB565_RED, COLOR_RGB565_WHITE);
        st7789_draw_string(0, 40, "hello, world", Font_16x26, COLOR_RGB565_RED, COLOR_RGB565_WHITE);
        HAL_Delay(1000);

        st7789_invert(bInvert = !bInvert);
        HAL_Delay(1000);
    }
}

/** \} end of group ST7789_Exported_Functions */
/** \} end of group ST7789 */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
















