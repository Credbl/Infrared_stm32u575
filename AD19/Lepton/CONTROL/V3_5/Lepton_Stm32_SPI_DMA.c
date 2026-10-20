/*******************************************************************************
**
**      File NAME: Lepton_Stm32_SPI_DMA.c
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 2016-09-01
**
**      DESCRIPTION:
**
**      HISTORY:
**
**      Copyright 2015 ~ 2025 EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "Lepton_Stm32_SPI_DMA.h"
#include "Lepton_Stm32_Control.h"
#include "Lepton_Common_Data.h"
#include "string.h"
#include "Lepton_Delay.h"
/******************************************************************************/
/** LOCAL DEFINES                                                            **/
/******************************************************************************/

#define LEPTON_SPI                          SPI2
#define LEPTON_SPI_CLK_ENABLE()             __HAL_RCC_SPI2_CLK_ENABLE()
#define LEPTON_SPI_DMA_CLK_ENABLE()         __HAL_RCC_GPDMA1_CLK_ENABLE()
#define LEPTON_SPI_SCK_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define LEPTON_SPI_MISO_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define LEPTON_SPI_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define LEPTON_SPI_NSS_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()



#define LEPTON_SPI_FORCE_RESET()            __HAL_RCC_SPI2_FORCE_RESET()
#define LEPTON_SPI_RELEASE_RESET()          __HAL_RCC_SPI2_RELEASE_RESET()


// NSS PA4
#define LEPTON_SPI_NSS_PIN                  GPIO_PIN_12
#define LEPTON_SPI_NSS_GPIO_PORT            GPIOB


// SCK PA5
#define LEPTON_SPI_SCK_PIN                  GPIO_PIN_13
#define LEPTON_SPI_SCK_GPIO_PORT            GPIOB
#define LEPTON_SPI_SCK_AF                   GPIO_AF5_SPI2

// MISO PA6
#define LEPTON_SPI_MISO_PIN                 GPIO_PIN_14
#define LEPTON_SPI_MISO_GPIO_PORT           GPIOB
#define LEPTON_SPI_MISO_AF                  GPIO_AF5_SPI2

// MOSI PA7
#define LEPTON_SPI_MOSI_PIN                 GPIO_PIN_15
#define LEPTON_SPI_MOSI_GPIO_PORT           GPIOB
#define LEPTON_SPI_MOSI_AF                  GPIO_AF5_SPI2



/* Definition for SPIx's NVIC */
#define LEPTON_SPI_IRQn                     SPI2_IRQn
#define LEPTON_SPI_IRQHandler               SPI2_IRQHandler

#define LEPTON_SPI_DMA_RX_IRQn              GPDMA1_Channel11_IRQn
#define LEPTON_SPI_DMA_RX_IRQHandler        GPDMA1_Channel11_IRQHandler

/* 使用 16bit 数据传输 */
#define DATA_SIZE_BYTE                      (0)


#define LEPT_SPI_SCK_PIN  (GPIO_PIN_13)
#define LEPT_SPI_MOSI_PIN (GPIO_PIN_15)
#define LEPT_SPI_MISO_PIN (GPIO_PIN_14)
#define LEPT_SPI_NSS_PIN  (GPIO_PIN_12)
#define LEPT_SPI_NSS_PORT (GPIOB)



/******************************************************************************/
/** LOCAL TYPE DEFINITIONS                                                   **/
/******************************************************************************/



/******************************************************************************/
/** PRIVATE DATA DECLARATIONS                                                **/
/******************************************************************************/

#define  IMAGE_HEIGHT  (120)
#define  IMAGE_WIDTH   (160)
/******************************************************************************/
/** PRIVATE FUNCTION DECLARATIONS                                            **/
/******************************************************************************/
static void LEPTON_SPI_MspInit(SPI_HandleTypeDef *hspi);


/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/

DMA_HandleTypeDef handle_GPDMA1_Channel11; //RX
// DMA_HandleTypeDef handle_GPDMA1_Channel10; //TX
SPI_HandleTypeDef SpiHandle;



uint16_t *g_LeptonFrameRaw[] =
{
    g_Lepton_Rec_Buffer[0], g_Lepton_Rec_Buffer[31], g_Lepton_Rec_Buffer[61], g_Lepton_Rec_Buffer[92],
    //g_Lepton_Buffer[0], g_Lepton_Buffer[31], g_Lepton_Buffer[61], g_Lepton_Buffer[92],
};

uint8_t FrameMask = 0;
volatile uint16_t TlineCnt = 0;
volatile uint8_t DmaBusy = 0;
volatile uint8_t FrameOkay = 0;



/* transfer state */
volatile uint8_t wTransferState = TRANSFER_COMPLETE;



/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
bool LEPTON_SPI_DMA_Init(void)
{
    SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};
    SpiHandle.Instance = LEPTON_SPI;
    SpiHandle.Init.Mode = SPI_MODE_MASTER;
    SpiHandle.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;//SPI_DIRECTION_2LINES;
    SpiHandle.Init.DataSize = SPI_DATASIZE_16BIT;
    SpiHandle.Init.CLKPolarity = SPI_POLARITY_HIGH;
    SpiHandle.Init.CLKPhase = SPI_PHASE_2EDGE;
    SpiHandle.Init.NSS = SPI_NSS_SOFT;
    #if defined(LEPTON_25)
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    #else
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    #endif
    SpiHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial = 0x7;
    SpiHandle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    SpiHandle.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    SpiHandle.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    SpiHandle.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    SpiHandle.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    SpiHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    SpiHandle.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    SpiHandle.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
    SpiHandle.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;


    LEPTON_SPI_MspInit(&SpiHandle);

    if (HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        return (false);
    }

    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
    HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
    if (HAL_SPIEx_SetConfigAutonomousMode(&SpiHandle, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
    {
        Error_Handler();
    }

    return (true);
}




bool LEPTON_SPI_DMA_Begin(void)
{
    memset((char *)RxLeptonBuff, 0, LEPT_FRAME_SIZE * sizeof(uint16_t));
    wTransferState = TRANSFER_WAIT;
    if (HAL_SPI_Receive_DMA(&SpiHandle, (uint8_t *)RxLeptonBuff, LEPT_FRAME_SIZE) != HAL_OK)
    {
        return (false);
    }
    LEPTON_SPI_Enable();

    return (true);
}



void LEPTON_SPI_Enable(void)
{
    HAL_GPIO_WritePin(LEPT_SPI_NSS_PORT, LEPT_SPI_NSS_PIN, GPIO_PIN_RESET);
    Delay_Us(10);
}


/**
* @brief  Lepton不选中为从设备(片�?
*
* @param  None
*
* @return None
*/
void LEPTON_SPI_Disable(void)
{
    HAL_GPIO_WritePin(LEPT_SPI_NSS_PORT, LEPT_SPI_NSS_PIN, GPIO_PIN_SET);
    Delay_Us(20);
}

#if defined(LEPTON_25)
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint16_t CrcSum = 0;
    uint16_t i = 0,j = 0;
    uint8_t SegNum = 0;
    uint8_t tempSegNum = 0;
    uint8_t numOffset = 0;
    uint16_t PacketNum = 0;

    uint16_t* dest_Buff = (uint16_t*)g_Lepton_Rec_Buffer;



    CalFrameCnt++;
    RunTime++;

    for (i = 0; i < 66; i++)
    {
        if((RxLeptonBuff[82*i] & 0x0fff) == 0)
        {
            j = i;
            break;
        }
    }

    if(j + 63 < 66)
    {
        for(i = j; i < j + 63; i++)   //?????????
        {
            CrcSum = CrcSum + (RxLeptonBuff[82*i] & 0x0fff);
        }
    }

    // if (j + 61 < 66)
    // {
    //     for (i = j; i < j + 60; i++) //??????????
    //     {
    //         CrcSum = CrcSum + (RxLeptonBuff[82 * i] & 0x0fff);
    //     }
    // }

    if((CrcSum == 1953))// && (FrameOkay == 0)) 
    {
        for (i = j; i < j + 63; i++)
        { 
            PacketNum = (RxLeptonBuff[82*i] & 0x0fff);
            //Telemetry Line Header
            if (PacketNum < 3)
            {
                memcpy(LeptonMetry + PacketNum * 82, RxLeptonBuff + i * 82 + 2, 80 * 2);
            }
            else
            {
                // memcpy(LeptonFrameRaw + (PacketNum - 3 + 1) * 81, RxLeptonBuff + i * 82 + 2, 80 * 2);
                memcpy(dest_Buff, &RxLeptonBuff[i * 82 + 2], 80 * 2);
                dest_Buff += 80;
                // memcpy(LeptonFrameRaw + (PacketNum - 3) * 80, RxLeptonBuff + i * 82 + 2, 80 * 2);
            }
        }

        // memcpy(LeptonFrameRaw, LeptonFrameRaw + 80, 80 * 2);
        FrameOkay = 1;
        TlineCnt = 0;
    }
    else
    {
        TlineCnt++;
    }
    
     wTransferState = TRANSFER_COMPLETE;

    if(TlineCnt > 208)
        DmaBusy = 1;
    else
        DmaBusy = 0;
}
#else
// SPI DMA传输完毕
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{

    uint16_t CrcSum = 0;
    uint16_t i = 0, j = 0;
    uint8_t SegNum = 0;
    uint8_t tempSegNum = 0;
    uint8_t numOffset = 0;
    uint16_t PacketNum = 0;

    for (i = 0; i < 66; i++)
    {
        if ((RxLeptonBuff[82 * i] & 0x0fff) == 0)
        {
            j = i;
            break;
        }
    }

    if (j + 61 < 66)
    {
        for (i = j; i < j + 60; i++) //??????????
        {
            CrcSum = CrcSum + (RxLeptonBuff[82 * i] & 0x0fff);
        }
    }

    if (CrcSum == 1770)
    {
        SegNum = (RxLeptonBuff[82 * (20 + j)] >> 12) & 0x0007;
        TlineCnt = 0;
    }
    else
    {
        SegNum = 5;
        TlineCnt = TlineCnt + 1;
    }

    if (SegNum == 4) //????????LeptonMetry?
    {
        memcpy(LeptonMetry, RxLeptonBuff + 57 * 82, 82 * 4 * sizeof(uint16_t));
    }

    CalFrameCnt++;
    RunTime++;

    if ((SegNum > 0) && (SegNum < 5))  //&& (FrameOkay == 0))
    {
        if (1 == SegNum)
        {
            for (i = 0; i < 30; i++)
            {
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH + 80, RxLeptonBuff + (i * 2 + 1) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
            }
            i = 30;
            memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
        }

        else if (2 == SegNum)
        {
            i = 30;
            memcpy(g_LeptonFrameRaw[SegNum - 2] + i * IMAGE_WIDTH + 80, RxLeptonBuff + (0 * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
            for (i = 0; i < 30; i++)
            {
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2 + 1) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH + 80, RxLeptonBuff + ((i + 1) * 2) * 82 + 2, 80 * sizeof(uint16_t)); //将数据拷贝到有效数据区域
            }
        }
        else if (3 == SegNum)
        {
            for (i = 0; i < 30; i++)
            {
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH + 80, RxLeptonBuff + (i * 2 + 1) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
            }
            i = 30;
            memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
        }
        else if (4 == SegNum)
        {
            i = 30;
            memcpy(g_LeptonFrameRaw[SegNum - 2] + i * IMAGE_WIDTH + 80, RxLeptonBuff + (0 * 2) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
            for (i = 0; i < 28; i++)
            {
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH, RxLeptonBuff + (i * 2 + 1) * 82 + 2, 80 * sizeof(uint16_t));  //将数据拷贝到有效数据区域
                memcpy(g_LeptonFrameRaw[SegNum - 1] + i * IMAGE_WIDTH + 80, RxLeptonBuff + ((i + 1) * 2) * 82 + 2, 80 * sizeof(uint16_t)); //将数据拷贝到有效数据区域
            }
        }

        FrameMask = FrameMask | (1 << ((SegNum - 1) & 0x0F));

        if ((SegNum == 4) && (FrameMask == 15))
        {
//            if(1 == g_IrPlugState)
            {
                memset(g_Lepton_Process_Buffer[0], 0, 160 * 122 * 2);
                memcpy(g_Lepton_Process_Buffer[0], g_Lepton_Rec_Buffer[0], (IMAGE_HEIGHT + 2) * (IMAGE_WIDTH)* sizeof(uint16_t));
                memcpy(g_Lepton_Process_Buffer[120], LeptonMetry, 80 * 8);

                // //����RBFOֵ
                // memcpy(g_Lepton_Process_Buffer[122], &g_RBFO_InternalHighGain, 16);
                // //����Lepton�汾��
                // g_Lepton_Process_Buffer[122][16] = g_Lepton_Version;
                // //����֡��־byte316-319Ϊ֡��־���̶�Ϊ0x00574152
                // g_Lepton_Process_Buffer[122][316] = 0x00;
                // g_Lepton_Process_Buffer[122][317] = 0x57;
                // g_Lepton_Process_Buffer[122][318] = 0x41;
                // g_Lepton_Process_Buffer[122][319] = 0x52;
            }
            FrameOkay = 1;
            FrameMask = 0;
        }
    }

    wTransferState = TRANSFER_COMPLETE;

    if (TlineCnt > 208)
    { DmaBusy = 1; }
    else
    { DmaBusy = 0; }
}
#endif

/**
  * @brief This function handles SPI2 global interrupt.
  */
void LEPTON_SPI_IRQHandler(void)
{
    /* USER CODE BEGIN SPI2_IRQn 0 */

    /* USER CODE END SPI2_IRQn 0 */
    HAL_SPI_IRQHandler(&SpiHandle);
    /* USER CODE BEGIN SPI2_IRQn 1 */

    /* USER CODE END SPI2_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 10 global interrupt.
  */
// void GPDMA1_Channel10_IRQHandler(void)
// {
//   /* USER CODE BEGIN GPDMA1_Channel10_IRQn 0 */

//   /* USER CODE END GPDMA1_Channel10_IRQn 0 */
//   HAL_DMA_IRQHandler(&handle_GPDMA1_Channel10);
//   /* USER CODE BEGIN GPDMA1_Channel10_IRQn 1 */

//   /* USER CODE END GPDMA1_Channel10_IRQn 1 */
// }

/**
  * @brief This function handles GPDMA1 Channel 11 global interrupt.
  */
void LEPTON_SPI_DMA_RX_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel11_IRQn 0 */

    /* USER CODE END GPDMA1_Channel11_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel11);
    /* USER CODE BEGIN GPDMA1_Channel11_IRQn 1 */

    /* USER CODE END GPDMA1_Channel11_IRQn 1 */
}



/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/
static void LEPTON_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    if (hspi->Instance != LEPTON_SPI)
    {
        return;
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
    PeriphClkInit.Spi1ClockSelection = RCC_SPI2CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }


    /* 时钟 */
    LEPTON_SPI_SCK_GPIO_CLK_ENABLE();
    LEPTON_SPI_MISO_GPIO_CLK_ENABLE();
    LEPTON_SPI_MOSI_GPIO_CLK_ENABLE();
    LEPTON_SPI_CLK_ENABLE();
    LEPTON_SPI_DMA_CLK_ENABLE();
    LEPTON_SPI_NSS_GPIO_CLK_ENABLE();

    /* GPDMA1 interrupt Init */
    // HAL_NVIC_SetPriority(GPDMA1_Channel10_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(GPDMA1_Channel10_IRQn);
    HAL_NVIC_SetPriority(LEPTON_SPI_DMA_RX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LEPTON_SPI_DMA_RX_IRQn);

    /* SPI NSS 引脚 */
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pin       = LEPTON_SPI_NSS_PIN;
    HAL_GPIO_Init(LEPTON_SPI_NSS_GPIO_PORT, &GPIO_InitStruct);

    /* SPI 引脚 */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;

    // SCK
    GPIO_InitStruct.Pin       = LEPTON_SPI_SCK_PIN;
    GPIO_InitStruct.Alternate = LEPTON_SPI_SCK_AF;
    HAL_GPIO_Init(LEPTON_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);

    // MISO
    GPIO_InitStruct.Pin = LEPTON_SPI_MISO_PIN;
    GPIO_InitStruct.Alternate = LEPTON_SPI_MISO_AF;
    HAL_GPIO_Init(LEPTON_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);

    // MOSI
    GPIO_InitStruct.Pin = LEPTON_SPI_MOSI_PIN;
    GPIO_InitStruct.Alternate = LEPTON_SPI_MOSI_AF;
    HAL_GPIO_Init(LEPTON_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);


    /* SPI2 DMA Init */
    /* GPDMA1_REQUEST_SPI2_RX Init */
    handle_GPDMA1_Channel11.Instance = GPDMA1_Channel11;
    handle_GPDMA1_Channel11.Init.Request = GPDMA1_REQUEST_SPI2_RX;
    handle_GPDMA1_Channel11.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel11.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel11.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel11.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_GPDMA1_Channel11.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
    handle_GPDMA1_Channel11.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
    handle_GPDMA1_Channel11.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT; 
    handle_GPDMA1_Channel11.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel11.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel11.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel11.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel11.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel11) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hspi, hdmarx, handle_GPDMA1_Channel11);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel11, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }
   
    // /* GPDMA1_REQUEST_SPI2_TX Init */
    // handle_GPDMA1_Channel10.Instance = GPDMA1_Channel10;
    // handle_GPDMA1_Channel10.Init.Request = GPDMA1_REQUEST_SPI2_TX;
    // handle_GPDMA1_Channel10.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    // handle_GPDMA1_Channel10.Init.Direction = DMA_MEMORY_TO_PERIPH;
    // handle_GPDMA1_Channel10.Init.SrcInc = DMA_SINC_INCREMENTED;
    // handle_GPDMA1_Channel10.Init.DestInc = DMA_DINC_FIXED;
    // handle_GPDMA1_Channel10.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
    // handle_GPDMA1_Channel10.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
    // handle_GPDMA1_Channel10.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    // handle_GPDMA1_Channel10.Init.SrcBurstLength = 1;
    // handle_GPDMA1_Channel10.Init.DestBurstLength = 1;
    // handle_GPDMA1_Channel10.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    // handle_GPDMA1_Channel10.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    // handle_GPDMA1_Channel10.Init.Mode = DMA_NORMAL;
    // if (HAL_DMA_Init(&handle_GPDMA1_Channel10) != HAL_OK)
    // {
    //   Error_Handler();
    // }

    // __HAL_LINKDMA(hspi, hdmatx, handle_GPDMA1_Channel10);

    // if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel10, DMA_CHANNEL_NPRIV) != HAL_OK)
    // {
    //   Error_Handler();
    // }

    /* SPI2 interrupt Init */
    HAL_NVIC_SetPriority(LEPTON_SPI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LEPTON_SPI_IRQn);
}


void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI2)
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();

    /**SPI2 GPIO Configuration
    PB12     ------> SPI2_NSS
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    /* SPI2 DMA DeInit */
    HAL_DMA_DeInit(hspi->hdmarx);
    HAL_DMA_DeInit(hspi->hdmatx);

    /* SPI2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }

}








