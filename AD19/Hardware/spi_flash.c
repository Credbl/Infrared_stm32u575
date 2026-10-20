/*!
\file       spi_flash.c
\brief      spi_flash.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2025-01-09
\see        =
\version    =
\pre        =
\bug        =
\attention  =
\warning    =

\copyright
<h2><center>Copyright &copy; \f$\text{2025}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/

/* INCLUDE FILES **************************************************************/
/* 相关头文件的.h */

/* C库的.h */
#include <stdio.h>

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */
#include "bsp.h"
#include "spi_flash.h"
#include "spi_flash_defs.h"

/*!
\defgroup SPI_FLASH GROUP_SPI_FLASH
\brief    SPI_FLASH概述
\details  SPI_FLASH详细描述
\{
*/

/* PRIVATE DEFINES ************************************************************/
//#define NOT_AD19

#define USE_SPI_DMA
#define SPI_BUFFER_SIZE                     (4 * 1024)

/* STM32U575 的 SPI3 单次传输数据不超过 1024 字节 */
#define TX_BUFF_SIZE                            (768)
#define RX_BUFF_SIZE                            (768)


#define CS_0()   do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); } while(0)
#define CS_1()   do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,   GPIO_PIN_SET); } while(0)

#define SF_CS_0()   do { GPIOA->BSRR = (uint32_t)GPIO_PIN_15 << 16; } while(0)
#define SF_CS_1()   do { GPIOA->BSRR = GPIO_PIN_15; } while(0)

/* PRIVATE TYPE DEFINITIONS ***************************************************/

enum
{
    TRANSFER_WAIT,
    TRANSFER_COMPLETE,
    TRANSFER_ERROR
};

/* PRIVATE FUNCTION DECLARATIONS **********************************************/

/* PRIVATE DATA DECLARATIONS **************************************************/

static SPI_HandleTypeDef hspi3;
static DMA_HandleTypeDef handle_GPDMA1_Channel2;
static DMA_HandleTypeDef handle_GPDMA1_Channel1;
#define g_hspi hspi3

static uint32_t g_spiLen;
//static uint8_t  g_spi_busy; /* SPI忙状态，0表示不忙，1表示忙 */
__IO uint32_t wSpiState = TRANSFER_WAIT;

/* DMA模式使用的SRAM4 */
__ALIGN_BEGIN uint8_t g_spiTxBuf[TX_BUFF_SIZE] __ALIGN_END;
__ALIGN_BEGIN uint8_t g_spiRxBuf[RX_BUFF_SIZE] __ALIGN_END;

/* 用于写函数，先读出整个扇区，修改缓冲区后，再整个扇区回写 */
//static uint8_t s_spiBuf[SPI_BUFFER_SIZE];
__ALIGN_BEGIN uint8_t s_spiBuf[SPI_BUFFER_SIZE] __ALIGN_END;

char str_buffer[128] = {0};

spi_flash_t g_tSF;

/* EXPORTED PUBLIC DATA *******************************************************/

/* PRIVATE MODULE FUNCTIONS ***************************************************/

static uint8_t _init_spi(void)
{
    SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = { 0 };

    hspi3.Instance = SPI3;
    hspi3.Init.Mode = SPI_MODE_MASTER;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 0x7;
    hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    hspi3.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    hspi3.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;

    if (HAL_SPI_Init(&hspi3) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
    if (HAL_SPIEx_SetConfigAutonomousMode(&hspi3, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
    {
        Error_Handler();
    }

    return HAL_OK;
}

void Init_SPI_buffer(void)
{
    // 新增：清零SPI缓冲区，避免首次使用残留数据
    memset(g_spiTxBuf, 0, TX_BUFF_SIZE);
    memset(g_spiRxBuf, 0, RX_BUFF_SIZE);
    memset(s_spiBuf, 0, SPI_BUFFER_SIZE);
    
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (SPI3 != hspi->Instance)
    {
        return;
    }

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    /* SPI3 时钟选择 sysclk = 160MHz */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
    PeriphClkInit.Spi3ClockSelection = RCC_SPI3CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*
        SPI3 GPIO Configuration
        PA15 (JTDI)             ------> SPI3_NSS
        PB4 (NJTRST)            ------> SPI3_MISO
        PB5                     ------> SPI3_MOSI
        PB3 (JTDO/TRACESWO)     ------> SPI3_SCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    /* SPI3 DMA Init */
    /* GPDMA1_REQUEST_SPI3_RX Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel2_IRQn);
    handle_GPDMA1_Channel2.Instance = GPDMA1_Channel2;
    handle_GPDMA1_Channel2.Init.Request = GPDMA1_REQUEST_SPI3_RX;
    handle_GPDMA1_Channel2.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel2.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel2.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_GPDMA1_Channel2.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel2.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel2.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel2.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel2.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel2.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel2.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel2.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel2) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hspi, hdmarx, handle_GPDMA1_Channel2);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel2, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }

    /* GPDMA1_REQUEST_SPI3_TX Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);
    handle_GPDMA1_Channel1.Instance = GPDMA1_Channel1;
    handle_GPDMA1_Channel1.Init.Request = GPDMA1_REQUEST_SPI3_TX;
    handle_GPDMA1_Channel1.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    handle_GPDMA1_Channel1.Init.SrcInc = DMA_SINC_INCREMENTED;
    handle_GPDMA1_Channel1.Init.DestInc = DMA_DINC_FIXED;
    handle_GPDMA1_Channel1.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel1.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    handle_GPDMA1_Channel1.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel1.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel1.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel1.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel1.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel1.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel1) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hspi, hdmatx, handle_GPDMA1_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(SPI3_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
}



void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (SPI3 == hspi->Instance)
    {
        wSpiState = TRANSFER_COMPLETE;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (SPI3 == hspi->Instance)
    {
        wSpiState = TRANSFER_ERROR;
    }
}


void GPDMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_hspi.hdmatx);
}
void GPDMA1_Channel2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(g_hspi.hdmarx);
}

void SPI3_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&g_hspi);
}


void _spi_transfer(void)
{
    if (g_spiLen > TX_BUFF_SIZE)
    {
        return;
    }

    wSpiState = TRANSFER_WAIT;

    if (HAL_OK != HAL_SPI_TransmitReceive_DMA(&g_hspi,
            (uint8_t *)g_spiTxBuf, (uint8_t *)g_spiRxBuf, g_spiLen))
    {
        Error_Handler();
    }

    while (wSpiState == TRANSFER_WAIT);
}

void _set_cs(uint8_t level)
{
    if (level == 0)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
        for (volatile uint32_t i = 0; i < 100; i++); 
    }
    else
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    }
}

static void _write_enable(void)
{
    _set_cs(0);                                     /* 使能片选 */
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = (CMD_WREN);            /* 发送命令 */
    _spi_transfer();
    _set_cs(1);                                     /* 禁能片选 */
}

static void _waitfor_write_end(void)
{
    _set_cs(0);                                     /* 使能片选 */
    g_spiTxBuf[0] = (CMD_RDSR);                     /* 发送命令， 读状态寄存器 */
    g_spiLen = 2;
    _spi_transfer();
    _set_cs(1);                                     /* 禁能片选 */

    while (1)
    {
        _set_cs(0);                                 /* 使能片选 */
        g_spiTxBuf[0] = (CMD_RDSR);                 /* 发送命令， 读状态寄存器 */
        g_spiTxBuf[1] = 0;                          /* 无关数据 */
        g_spiLen = 2;
        _spi_transfer();
        _set_cs(1);                                 /* 禁能片选 */

        if ((g_spiRxBuf[1] & WIP_FLAG) != SET)      /* 判断状态寄存器的忙标志位 */
        {
            break;
        }
    }
}


static uint8_t _is_need_erase(uint8_t *_ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
    uint16_t i;
    uint8_t ucOld;

    /*
        算法第1步：old 求反, new 不变
              old    new
              1101   0101
        ~     1111
            = 0010   0101

        算法第2步: old 求反的结果与 new 位与
              0010   old
        &     0101   new
             =0000

        算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
    */

    for (i = 0; i < _usLen; i++)
    {
        ucOld = *_ucpOldBuf++;
        ucOld = ~ucOld;

        /* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) */
        if ((ucOld & (*_ucpNewBuf++)) != 0)
        {
            return 1;
        }
    }
    return 0;
}

void spiflash_write_page(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
    uint32_t i, j;

    for (j = 0; j < _usSize / 256; j++)
    {
        _write_enable();                               /* 发送写使能命令 */
        _set_cs(0);                                    /* 使能片选 */
        g_spiLen = 0;
        g_spiTxBuf[g_spiLen++] = (CMD_PP);                              /* 发送AAI命令(地址自动增加编程) */
        g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF0000) >> 16);     /* 发送扇区地址的高8bit */
        g_spiTxBuf[g_spiLen++] = ((_uiWriteAddr & 0xFF00) >> 8);        /* 发送扇区地址中间8bit */
        g_spiTxBuf[g_spiLen++] = (_uiWriteAddr & 0xFF);                 /* 发送扇区地址低8bit */
        for (i = 0; i < 256; i++)
        {
            g_spiTxBuf[g_spiLen++] = (*_pBuf++);        /* 发送数据 */
        }
        _spi_transfer();
        _set_cs(1);                                     /* 禁止片选 */

        _waitfor_write_end();                           /* 等待串行Flash内部写操作完成 */

        _uiWriteAddr += 256;
    }

    /* 进入写保护状态 */
    _set_cs(0);
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = (CMD_DISWR);
    _spi_transfer();
    _set_cs(1);

    _waitfor_write_end();                               /* 等待串行Flash内部写操作完成 */
}


static uint8_t _datacmp(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
    uint16_t i, j;
    uint16_t rem;

    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((_uiSrcAddr + _uiSize) > g_tSF.TotalSize)
    {
        return 1;
    }

    if (_uiSize == 0)
    {
        return 0;
    }

    _set_cs(0);                                    /* 使能片选 */
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = (CMD_FAST_READ);                       /* 发送读命令 */
    g_spiTxBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF0000) >> 16);       /* 发送扇区地址的高8bit */
    g_spiTxBuf[g_spiLen++] = ((_uiSrcAddr & 0xFF00) >> 8);          /* 发送扇区地址中间8bit */
    g_spiTxBuf[g_spiLen++] = (_uiSrcAddr & 0xFF);                   /* 发送扇区地址低8bit */
    g_spiTxBuf[g_spiLen++] = (DUMMY_BYTE);
    _spi_transfer();

    /* 开始读数据，因为底层DMA缓冲区有限，必须分包读 */
    for (i = 0; i < _uiSize / RX_BUFF_SIZE; i++)
    {
        g_spiLen = RX_BUFF_SIZE;
        _spi_transfer();

        for (j = 0; j < RX_BUFF_SIZE; j++)
        {
            if (g_spiRxBuf[j] != *_ucpTar++)
            {
                goto NOTEQ;     /* 不相等 */
            }
        }
    }

    rem = _uiSize % RX_BUFF_SIZE;    /* 剩余字节 */
    if (rem > 0)
    {
        g_spiLen = rem;
        _spi_transfer();

        for (j = 0; j < rem; j++)
        {
            if (g_spiRxBuf[j] != *_ucpTar++)
            {
                goto NOTEQ;     /* 不相等 */
            }
        }
    }
    _set_cs(1);
    return 0;       /* 相等 */

NOTEQ:
    _set_cs(1);    /* 不相等 */
    return 1;
}

u8 _read_write_byte(u8 TxData)
{
    u8 Rxdata;
    HAL_SPI_TransmitReceive(&g_hspi, &TxData, &Rxdata, 1, 1000);
    return Rxdata;
}

uint8_t _auto_write_sector(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                 /* 用于延时 */
    uint32_t uiFirstAddr;       /* 扇区首址 */
    uint8_t ucNeedErase;        /* 1表示需要擦除 */
    uint8_t cRet;

    /* 长度为0时不继续操作,直接认为成功 */
    if (_usWrLen == 0)
    {
        return 1;
    }

    /* 如果偏移地址超过芯片容量则退出 */
    if (_uiWrAddr >= g_tSF.TotalSize)
    {
        return 0;
    }

    /* 如果数据长度大于扇区容量，则退出 */
    if (_usWrLen > g_tSF.SectorSize)
    {
        return 0;
    }

    /* 如果FLASH中的数据没有变化,则不写FLASH */
    spiflash_read_buffer(s_spiBuf, _uiWrAddr, _usWrLen);
    if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
    {
        return 1;
    }

    /* 判断是否需要先擦除扇区 */
    /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
    ucNeedErase = 0;
    if (_is_need_erase(s_spiBuf, _ucpSrc, _usWrLen))
    {
        ucNeedErase = 1;
    }

    uiFirstAddr = _uiWrAddr & (~(g_tSF.SectorSize - 1));

    if (_usWrLen == g_tSF.SectorSize)       /* 整个扇区都改写 */
    {
        for (i = 0; i < g_tSF.SectorSize; i++)
        {
            s_spiBuf[i] = _ucpSrc[i];
        }
    }
    else                        /* 改写部分数据 */
    {
        /* 先将整个扇区的数据读出 */
        spiflash_read_buffer(s_spiBuf, uiFirstAddr, g_tSF.SectorSize);

        /* 再用新数据覆盖 */
        i = _uiWrAddr & (g_tSF.SectorSize - 1);
        memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
    }

    /* 写完之后进行校验，如果不正确则重写，最多3次 */
    cRet = 0;
    for (i = 0; i < 3; i++)
    {

        /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
        if (ucNeedErase == 1)
        {
            spiflash_erase_sector(uiFirstAddr);        /* 擦除1个扇区 */
        }

        /* 编程一个扇区 */
        spiflash_write_page(s_spiBuf, uiFirstAddr, g_tSF.SectorSize);

        if (_datacmp(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
        {
            cRet = 1;
            break;
        }
        else
        {
            if (_datacmp(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
            {
                cRet = 1;
                break;
            }

            /* 失败后延迟一段时间再重试 */
            for (j = 0; j < 10000; j++);
        }
    }

    return cRet;
}



uint16_t _read_shortid_ll(void)
{
    uint16_t Temp = 0;
    CS_0();
    _read_write_byte(0x90);//发送读取ID命令
    _read_write_byte(0x00);
    _read_write_byte(0x00);
    _read_write_byte(0x00);
    Temp |= _read_write_byte(0xFF) << 8;
    Temp |= _read_write_byte(0xFF);
    CS_1();
    return Temp;
}

uint16_t _read_longid_ll(void)
{
    uint16_t Temp = 0;
    uint8_t cnt = 0;
    CS_0();
    g_spiRxBuf[cnt++] = _read_write_byte(0x9F);
    g_spiRxBuf[cnt++] = _read_write_byte(0x00);
    g_spiRxBuf[cnt++] = _read_write_byte(0x00);
    g_spiRxBuf[cnt++] = _read_write_byte(0x00);
    g_spiRxBuf[cnt++] = _read_write_byte(0xFF);
    g_spiRxBuf[cnt++] = _read_write_byte(0xFF);
    g_spiRxBuf[cnt++] = _read_write_byte(0xFF);
    g_spiRxBuf[cnt++] = _read_write_byte(0xFF);
    CS_1();
    return Temp;
}


/*! 读取芯片的 Manufacturer / Device ID */
uint16_t spiflash_read_shortid(void)
{
    uint16_t MID;
    uint8_t id1, id2;

    _set_cs(0);                             /* 使能片选 */
    g_spiLen = 0;
    g_spiTxBuf[0] = (CMD_RDSID);            /* 发送读ID命令 */
    g_spiLen = 6;

    _spi_transfer();

    id1 = g_spiRxBuf[4];                    /* 读ID的第1个字节 */
    id2 = g_spiRxBuf[5];                    /* 读ID的第2个字节 */
    _set_cs(1);                             /* 禁能片选 */

    MID = ((uint16_t)id1 << 8) | id2;

    return MID;
}

/*! 读取芯片的 Identification */
uint32_t spiflash_read_longid(void)
{
    uint32_t uiID;
    uint8_t id1, id2, id3;

    g_spiLen = 0;
    _set_cs(0);                             /* 使能片选 */
    g_spiTxBuf[g_spiLen++] = (CMD_RDLID);   /* 发送读ID命令 */
    g_spiTxBuf[g_spiLen++] = (DUMMY_BYTE);
    g_spiTxBuf[g_spiLen++] = (DUMMY_BYTE);
    g_spiTxBuf[g_spiLen++] = (DUMMY_BYTE);

    _spi_transfer();

    id1 = g_spiRxBuf[1];                    /* 读ID的第1个字节 */
    id2 = g_spiRxBuf[2];                    /* 读ID的第2个字节 */
    id3 = g_spiRxBuf[3];                    /* 读ID的第3个字节 */
    _set_cs(1);                             /* 禁能片选 */

    uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

    return uiID;
}

void spiflash_read_info(void)
{
    g_tSF.ChipID = spiflash_read_shortid(); /* 芯片ID */

    switch (g_tSF.ChipID)
    {
        case ZD25WQ32C_SHORT_ID:
        {
            strcpy(g_tSF.ChipName, "ZD25WQ32C");
            g_tSF.TotalSize = 4 * 1024 * 1024;  /* 总容量 = 4M, 32Mbit*/
            g_tSF.SectorSize = 4 * 1024;        /* 扇区大小 = 4K */
            g_tSF.SectorNum = 1024;
            break;
        }
        case NM25Q128:
        {
            strcpy(g_tSF.ChipName, "NM25Q128");
            g_tSF.TotalSize = 1 * 1024 * 1024;  /* 总容量 = 1M, 8Mbit*/
            g_tSF.SectorSize = 4 * 1024;        /* 扇区大小 = 4K */
            g_tSF.SectorNum = 256;
            break;
        }
        case GD25Q32EEIGR_ID:  // 新增分支
        {
            strcpy(g_tSF.ChipName, "GD25Q32C");
            g_tSF.TotalSize = 4 * 1024 * 1024;  // 4MB
            g_tSF.SectorSize = 4 * 1024;
            g_tSF.SectorNum = 1024;
            break;
        }
        default:
        {
            strcpy(g_tSF.ChipName, "Unknow Flash");
            g_tSF.TotalSize = 2 * 1024 * 1024;
            g_tSF.SectorSize = 4 * 1024;
            break;
        }
    }
}

uint8_t spiflash_write_buffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize)
{
    uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = _uiWriteAddr % g_tSF.SectorSize;
    count = g_tSF.SectorSize - Addr;
    NumOfPage = _usWriteSize / g_tSF.SectorSize;
    NumOfSingle = _usWriteSize % g_tSF.SectorSize;

    if (Addr == 0) /* 起始地址是扇区首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于扇区大小 */
        {
            if (_auto_write_sector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
            {
                return 0;
            }
        }
        else    /* 数据长度大于等于扇区大小 */
        {
            while (NumOfPage--)
            {
                if (_auto_write_sector(_pBuf, _uiWriteAddr, g_tSF.SectorSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr += g_tSF.SectorSize;
                _pBuf += g_tSF.SectorSize;
            }
            if (_auto_write_sector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
            {
                return 0;
            }
        }
    }
    else  /* 起始地址不是扇区首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于扇区大小 */
        {
            if (NumOfSingle > count)  /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
            {
                temp = NumOfSingle - count;

                if (_auto_write_sector(_pBuf, _uiWriteAddr, count) == 0)
                {
                    return 0;
                }

                _uiWriteAddr += count;
                _pBuf += count;

                if (_auto_write_sector(_pBuf, _uiWriteAddr, temp) == 0)
                {
                    return 0;
                }
            }
            else
            {
                if (_auto_write_sector(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
                {
                    return 0;
                }
            }
        }
        else    /* 数据长度大于等于扇区大小 */
        {
            _usWriteSize -= count;
            NumOfPage = _usWriteSize / g_tSF.SectorSize;
            NumOfSingle = _usWriteSize % g_tSF.SectorSize;
            if (_auto_write_sector(_pBuf, _uiWriteAddr, count) == 0)
            {
                return 0;
            }

            _uiWriteAddr += count;
            _pBuf += count;

            while (NumOfPage--)
            {
                if (_auto_write_sector(_pBuf, _uiWriteAddr, g_tSF.SectorSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr += g_tSF.SectorSize;
                _pBuf += g_tSF.SectorSize;
            }

            if (NumOfSingle != 0)
            {
                if (_auto_write_sector(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
                {
                    return 0;
                }
            }
        }
    }
    return 1;   /* 成功 */
}

void spiflash_read_buffer(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    uint16_t rem;
    uint16_t i;

    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((_uiSize == 0) || (_uiReadAddr + _uiSize) > g_tSF.TotalSize)
    {
        return;
    }

    g_spiLen = 0;
    _set_cs(0);                                                 /* 使能片选 */
    g_spiTxBuf[g_spiLen++] = (CMD_FAST_READ);                   /* 发送读命令 */
    g_spiTxBuf[g_spiLen++] = ((_uiReadAddr & 0xFF0000) >> 16);  /* 发送扇区地址的高8bit */
    g_spiTxBuf[g_spiLen++] = ((_uiReadAddr & 0xFF00) >> 8);     /* 发送扇区地址中间8bit */
    g_spiTxBuf[g_spiLen++] = (_uiReadAddr & 0xFF);              /* 发送扇区地址低8bit */
    g_spiTxBuf[g_spiLen++] = (DUMMY_BYTE);
    _spi_transfer();

    /* 开始读数据，因为底层DMA缓冲区有限，必须分包读 */
    for (i = 0; i < _uiSize / RX_BUFF_SIZE; i++)
    {
        g_spiLen = RX_BUFF_SIZE;

        _spi_transfer();

        memcpy(_pBuf, g_spiRxBuf, RX_BUFF_SIZE);
        _pBuf += RX_BUFF_SIZE;
    }

    rem = _uiSize % RX_BUFF_SIZE;    /* 剩余字节 */
    if (rem > 0)
    {
        g_spiLen = rem;
        _spi_transfer();

        memcpy(_pBuf, g_spiRxBuf, rem);
    }

    _set_cs(1);                                    /* 禁能片选 */
}

void spiflash_erase_sector(uint32_t _uiSectorAddr)
{
    _write_enable();                               /* 发送写使能命令 */

    /* 擦除扇区操作 */
    g_spiLen = 0;
    _set_cs(0);                                                     /* 使能片选 */
    g_spiTxBuf[g_spiLen++] = (CMD_SE);                              /* 发送擦除命令 */
    g_spiTxBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF0000) >> 16);    /* 发送扇区地址的高8bit */
    g_spiTxBuf[g_spiLen++] = ((_uiSectorAddr & 0xFF00) >> 8);       /* 发送扇区地址中间8bit */
    g_spiTxBuf[g_spiLen++] = (_uiSectorAddr & 0xFF);                /* 发送扇区地址低8bit */

    _spi_transfer();

    _set_cs(1);                                     /* 禁能片选 */

    _waitfor_write_end();                           /* 等待串行Flash内部写操作完成 */
}

void spiflash_erase_chip(void)
{
    _write_enable();                                /* 发送写使能命令 */

    g_spiLen = 0;
    _set_cs(0);                                     /* 使能片选 */
    g_spiTxBuf[g_spiLen++] = CMD_BE;                /* 发送整片擦除命令 */

    _spi_transfer();

    _set_cs(1);                                     /* 禁能片选 */

    _waitfor_write_end();                           /* 等待串行Flash内部写操作完成 */
}

/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\defgroup SPI_FLASH_Exported_Functions SPI_FLASH 导出的函数接口
\brief    导出的函数接口
\{
*/

/*
uint8_t spiflash_init(void)
{
    _init_spi();

    CS_1();
    HAL_Delay(1);
    CS_0();
    HAL_Delay(1);
    CS_1();

    spiflash_read_info();

    return 0;
}
*/

#define CMD_RSTEN       0x66    // 复位使能命令（多数SPI Flash通用）
#define CMD_RST         0x99    // 复位执行命令（多数SPI Flash通用）

uint8_t spiflash_init(void)
{
    uint8_t retry = 3;
    _init_spi();
    
    // 新增：清零SPI缓冲区，避免首次使用残留数据
    memset(g_spiTxBuf, 0, TX_BUFF_SIZE);
    memset(g_spiRxBuf, 0, RX_BUFF_SIZE);
    memset(s_spiBuf, 0, SPI_BUFFER_SIZE);

    // 发送Flash复位命令（根据芯片手册确认命令）
    CS_0();
    g_spiTxBuf[0] = CMD_RSTEN;  // 复位使能命令（如0x66）
    g_spiLen = 1;
    _spi_transfer();
    CS_1();
    HAL_Delay(10);  // 等待复位生效

    CS_0();
    g_spiTxBuf[0] = CMD_RST;    // 复位命令（如0x99）
    g_spiLen = 1;
    _spi_transfer();
    CS_1();
    HAL_Delay(200);  // 增加稳定时间（根据芯片手册调整）

    // 验证芯片ID读取的稳定性（多次读取确认）
    while (retry--) {
        spiflash_read_info();
        if (g_tSF.ChipID == ZD25WQ32C_SHORT_ID || g_tSF.ChipID == NM25Q128 || GD25Q32EEIGR_ID == g_tSF.ChipID) {
            return 0;  // ID验证通过
        }
        HAL_Delay(100);
    }

    // ID读取失败，提示硬件错误
    //printf("SPI Flash ID read failed!\n");
    return 1;  // 初始化失败
}

#define TEST_ADDR 4096 * 30
#define TEST_SIZE 4096
static uint8_t buf[TEST_SIZE];

void spiflash_test(void)
{
    spiflash_init();

    uint16_t short_id = spiflash_read_shortid();
    uint32_t long_id = spiflash_read_longid();

    snprintf(str_buffer, sizeof(str_buffer),
        "dma read id: 0x%02x\r\n", spiflash_read_shortid());

    spiflash_read_info();

    uint16_t i;
    int32_t iTime1, iTime2;

    // write test
#if 0
    /* 填充测试缓冲区 */
    for (i = 0; i < TEST_SIZE; i++)
    {
        buf[i] = 0xFF - (uint8_t)i;
    }

    /* 写EEPROM, 起始地址 = 0，数据长度为 256 */
    iTime1 = HAL_GetTick(); /* 记下开始时间 */
    if (spiflash_write_buffer(buf, TEST_ADDR, TEST_SIZE) == 0)
    {
        printf("写串行Flash出错！\r\n");
        return;
    }
    else
    {
        iTime2 = HAL_GetTick();	/* 记下结束时间 */
        printf("写串行Flash成功！\r\n");
    }

    /* 打印读速度 */
    printf("数据长度: %d字节, 写耗时: %dms, 写速度: %dB/s\r\n",
        TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));

#endif


    // read test
#if 1

    /* 起始地址 = 0， 数据长度为 256 */
    iTime1 = HAL_GetTick();	/* 记下开始时间 */
    spiflash_read_buffer(buf, TEST_ADDR, TEST_SIZE);
    iTime2 = HAL_GetTick();	/* 记下结束时间 */

    printf("读串行Flash成功，数据如下：\r\n");

    /* 打印数据 */
    for (i = 0; i < TEST_SIZE; i++)
    {
        printf(" %02X", buf[i]);

        if ((i & 31) == 31)
        {
            printf("\r\n");         /* 每行显示16字节数据 */
        }
        else if ((i & 31) == 15)
        {
            printf(" - ");
        }
    }

    /* 打印读速度 */
    printf("数据长度: %d字节, 读耗时: %dms, 读速度: %d Bytes/s\r\n",
        TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));

#endif


    while (1)
    {
        HAL_Delay(100);
    }
}



/** \} end of group SPI_FLASH_Exported_Functions */
/** \} end of group SPI_FLASH */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/

