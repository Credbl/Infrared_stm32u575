/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "Lepton_Stm32_Control.h"
#include "Lepton_Stm32_SPI_DMA.h"
// #include "Lepton_Stm32_TempMeasure.h"
#include "Lepton_Common_Data.h"
#include "bsp.h"
#include "Lepton_Delay.h"
#include "battery.h"

#define LEPTON_EXIT_PIN GPIO_PIN_2
#define LEPTON_EXIT_PORT GPIOD
#define LEPTON_EXIT_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define LEPTON_EXIT_IRQ EXTI2_IRQn
#define LEPTON_EXIT_IRQHandler EXTI2_IRQHandler

#define LEPTON_PWD_PORT GPIOC
#define LEPTON_PWD_PIN GPIO_PIN_2
#define LEPTON_PWD_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define LEPTON_RES_PORT GPIOC
#define LEPTON_RES_PIN GPIO_PIN_3
#define LEPTON_RES_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#if defined(LEPTON_25)
    #define LEPTON_TLINEAR_RESOLUTION_POS (82 * 2 + 49)
    #define LEPTON_TLINEAR_GAINMODE_POS (82 * 2 + 5)
#else
    #define LEPTON_TLINEAR_RESOLUTION_POS (82 * 2 + 49 + 2)
    #define LEPTON_TLINEAR_GAINMODE_POS (82 * 2 + 5 + 2)
#endif

//超过150摄氏度切换为低增益模式
const uint16_t gs_HIGH_TEMP_LOW_GAIN = (130 + 273) * 10;
const uint16_t gs_HIGH_TEMP_HIGH_GAIN = (130 + 273) * 100;

uint16_t g_Number_Hightemp = 0;


// 温度单位，0：摄氏度，1：华氏度
uint8_t g_TempUnit = TEMP_UNIT_F;
uint8_t g_TempChangeFlag = 0;

// 定义结构体
typedef struct
{
    uint16_t max_Raw; // 对应 g_LeptonMaxRaw
    uint16_t max_X;   // 对应 g_LeptonMax_Org_X
    uint16_t max_Y;   // 对应 g_LeptonMax_Org_Y
    uint16_t min_Raw; // 对应 g_LeptonMaxRaw
    uint16_t min_X;   // 对应 g_LeptonMax_Org_X
    uint16_t min_Y;   // 对应 g_LeptonMax_Org_Y
} LeptonMaxMinData;

#define DAFAULT_WINDOW_TRANSIMIT_RATE (8192)
/******************************************************************************/
/** PRIVATE DATA DECLARATIONS                                                **/
/******************************************************************************/
LEP_CAMERA_PORT_DESC_T LepPortDesc;
LEP_RBFO_T g_RBFO_InternalHighGain;
uint8_t g_Lepton_Version = 0;
static uint8_t g_LeptonPause = 0;
uint16_t g_LeptonCenterRaw = 0;
LeptonMaxMinData g_LeptonMaxMinRaw;
#ifdef LEPTON_25
uint16_t g_WindowTransimitRate = 6470;//DAFAULT_WINDOW_TRANSIMIT_RATE;
#else
uint16_t g_WindowTransimitRate = 6798;//DAFAULT_WINDOW_TRANSIMIT_RATE;
#endif

uint32_t g_ReadWindowTransimitRate = 0;
/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
volatile uint8_t LeptonFrameReady = 0; // Lepton是否准备好一帧数据的标志

/******************************************************************************/
/** PRIVATE FUNCTION DECLARATIONS                                            **/
/******************************************************************************/
void (*LepntonDprocess)(void);
void Lepton_Reset(void);
void Lepton_PowerOn(void);
LEP_RESULT RunLeptonInitCmd(void);
void Lepton_PwrRstPin_Init(void);

LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParamsPtr;

static void Lepton_ChangeGainMode(LEP_SYS_GAIN_MODE_E gainMode)
{
    if (LEP_OK != LEP_SetSysGainMode(&LepPortDesc, gainMode))
    {
        Error_Handler();
    }
}


uint8_t Lepton_SetFluxParams(uint32_t val)
{
    uint8_t result = 0;

    if (val >= 82 && val <= 8192)
    {
        fluxParamsPtr.tauWindow = val;
        fluxParamsPtr.reflWindow = 8192 - fluxParamsPtr.tauWindow;
        result = LEP_SetRadFluxLinearParams(&LepPortDesc, fluxParamsPtr);
        if (LEP_OK == result)
        {
            g_WindowTransimitRate = val;
        }
    }

    return result;
}

LEP_RESULT RunLeptonInitCmd(void)
{
    LEP_RESULT result = LEP_OK;
    LEP_RBFO_T temp;
    // LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParamsPtr;
    LEP_SYS_GAIN_MODE_OBJ_T obj;

    // Run Telemetry Disable Command
    result = LEP_SetSysTelemetryEnableState(&LepPortDesc, LEP_TELEMETRY_ENABLED);
    if (LEP_OK != result)
    {
        return result;
    }

#if defined(LEPTON_25)
    result = LEP_SetSysTelemetryLocation(&LepPortDesc, LEP_TELEMETRY_LOCATION_HEADER);
    if (LEP_OK != result)
    {
        return result;
    }
#else
    result = LEP_SetSysTelemetryLocation(&LepPortDesc, LEP_TELEMETRY_LOCATION_FOOTER);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    /* 开启 lepton 数据中断 */
    result = LEP_SetCfgGpioMode(&LepPortDesc, LEP_CFG_GPIO_MODE_VSYNC);
    if (LEP_OK != result)
    {
        return result;
    }

    // Run ACG Enable Command
#if RAW14BIT // 14位原始数据
    result = LEP_SetAgcEnableState(&LepPortDesc, LEP_AGC_DISABLE);
    if (LEP_OK != result)
    {
        return result;
    }
    result = LEP_SetRadEnableState(&LepPortDesc, LEP_RAD_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadTShutterMode(&LepPortDesc, LEP_RAD_TS_CAL_MODE); // LEP_RAD_TS_USER_MODE);
    if (LEP_OK != result)
    {
        return result;
    }
#else // 8位原始数据
    result = LEP_SetAgcEnableState(&LepPortDesc, LEP_AGC_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }
    result = LEP_SetAgcHeqClipLimitHigh(&LepPortDesc, 512);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetAgcHeqClipLimitLow(&LepPortDesc, 512);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetAgcHeqBinExtension(&LepPortDesc, 16);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    // Disable Vid Focus cal
    result = LEP_SetVidFocusCalcEnableState(&LepPortDesc, (LEP_VID_FOCUS_CALC_ENABLE_E)0);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetSysGainMode(&LepPortDesc, LEP_SYS_GAIN_MODE_HIGH);
    if (LEP_OK != result)
    {
        return result;
    }

#ifndef LEPTON_30
    result = LEP_GetRadFluxLinearParams(&LepPortDesc, &fluxParamsPtr);
    if (LEP_OK != result)
    {
        return result;
    }
    // fluxParamsPtr.sceneEmissivity = 7782;
    fluxParamsPtr.tauWindow = g_WindowTransimitRate;//6470;//DAFAULT_WINDOW_TRANSIMIT_RATE;                            // 7373;
    fluxParamsPtr.reflWindow = DAFAULT_WINDOW_TRANSIMIT_RATE - fluxParamsPtr.tauWindow; // 819;

    result = LEP_SetRadFluxLinearParams(&LepPortDesc, fluxParamsPtr);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadTLinearAutoResolution(&LepPortDesc, LEP_RAD_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    // result = LEP_GetRadExternalRBFOHighGain(&LepPortDesc, &temp);
    // if (LEP_OK != result)
    // {
    //     return result;
    // }
    // result = LEP_GetRadRBFOExternal0(&LepPortDesc, &g_RBFO_InternalHighGain);
    // if (LEP_OK != result)
    // {
    //      return result;
    // }
    g_RBFO_InternalHighGain.RBFO_R = 358929;
    g_RBFO_InternalHighGain.RBFO_B = 1435;
    g_RBFO_InternalHighGain.RBFO_F = 1;
    g_RBFO_InternalHighGain.RBFO_O = 405;

#if 0
    result = LEP_GetSysGainModeObj(&LepPortDesc, &obj);
    if (LEP_OK != result)
    {
        return result;
    }
//         obj.sysGainModeFluxThresholdHigh = 9876;
//         obj.sysGainModeFluxThresholdLow = 8765;
//         obj.sysGainModeROI.startRow = 0;
//         obj.sysGainModeROI.startCol = 0;
//         obj.sysGainModeROI.endRow = 79;
//         obj.sysGainModeROI.endCol = 59;
//         obj.sysGainModeTempEnabled = 1;
//         obj.sysGainModeThresholds.sys_C_high_to_low = 115;
//         obj.sysGainModeThresholds.sys_C_low_to_high = 85;
    obj.sysGainModeThresholds.sys_P_high_to_low = 20;//25;
//         obj.sysGainModeThresholds.sys_P_low_to_high = 90;
//         obj.sysGainModeThresholds.sys_T_high_to_low = 388;
//         obj.sysGainModeThresholds.sys_T_low_to_high = 358;
//         obj.sysGainRoiPopulation = 4800;
//         obj.sysGainModeTempEnabled = 1;
    result = LEP_SetSysGainModeObj(&LepPortDesc, obj);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    // result = Lepton_RunFFCCMD();
    // if (LEP_OK != result)
    // {
    //     return result;
    // }

    return (result);
}

/**
 * @brief  发送Lepton初始化时必要的命令
 *
 * @param  None
 * 
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT RunLeptonInitCmd_1(void)
{

    LEP_RESULT result = LEP_OK;
    LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParamsPtr;
    //     LEP_SYS_GAIN_MODE_OBJ_T obj;
    //     LEP_RBFO_T temp;
    LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shuter_mode;
    LEP_OEM_SW_VERSION_T software_version;
    //    LEP_RAD_TS_MODE_E value;

    // Run Telemetry Disable Command
    result = LEP_SetSysTelemetryEnableState(&LepPortDesc, LEP_TELEMETRY_ENABLED);
    if (LEP_OK != result)
    {
        return result;
    }

#if defined(LEPTON_25)
    result = LEP_SetSysTelemetryLocation(&LepPortDesc, LEP_TELEMETRY_LOCATION_HEADER);
    if (LEP_OK != result)
    {
        return result;
    }
#else
    result = LEP_SetSysTelemetryLocation(&LepPortDesc, LEP_TELEMETRY_LOCATION_FOOTER);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    /* 开启 lepton 数据中断 */
    result = LEP_SetCfgGpioMode(&LepPortDesc, LEP_CFG_GPIO_MODE_VSYNC);
    if (LEP_OK != result)
    {
        return result;
    }

    // Run ACG Enable Command
#if RAW14BIT // 14位原始数据
    result = LEP_SetAgcEnableState(&LepPortDesc, LEP_AGC_DISABLE);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadEnableState(&LepPortDesc, LEP_RAD_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadTShutterMode(&LepPortDesc, LEP_RAD_TS_CAL_MODE); // LEP_RAD_TS_USER_MODE);
    if (LEP_OK != result)
    {
        return result;
    }

#else // 8位原始数据
    result = LEP_SetAgcEnableState(&LepPortDesc, LEP_AGC_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }
    result = LEP_SetAgcHeqClipLimitHigh(&LepPortDesc, 512);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetAgcHeqClipLimitLow(&LepPortDesc, 512);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetAgcHeqBinExtension(&LepPortDesc, 16);
    if (LEP_OK != result)
    {
        return result;
    }
#endif

    // Disable Vid Focus cal
    result = LEP_SetVidFocusCalcEnableState(&LepPortDesc, (LEP_VID_FOCUS_CALC_ENABLE_E)0);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadTLinearEnableState(&LepPortDesc, LEP_RAD_DISABLE);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetSysGainMode(&LepPortDesc, LEP_SYS_GAIN_MODE_AUTO); // LEP_SYS_GAIN_MODE_AUTO);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_GetSysFfcShutterModeObj(&LepPortDesc, &shuter_mode);
    if (LEP_OK != result)
    {
        return result;
    }

    if (180000 != shuter_mode.desiredFfcPeriod)
    {
        shuter_mode.desiredFfcPeriod = 180000;
        result = LEP_SetSysFfcShutterModeObj(&LepPortDesc, shuter_mode);
        if (LEP_OK != result)
        {
            return result;
        }
    }

    result = LEP_GetOemSoftwareVersion(&LepPortDesc, &software_version);
    if (LEP_OK != result)
    {
        return result;
    }
    if ((software_version.gpp_major >= 3) && (software_version.gpp_minor >= 3))
    {
        g_Lepton_Version = 1;
    }
    else
    {
        g_Lepton_Version = 0;
    }

    result = LEP_SetVidFocusCalcEnableState(&LepPortDesc, (LEP_VID_FOCUS_CALC_ENABLE_E)0);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_GetRadFluxLinearParams(&LepPortDesc, &fluxParamsPtr);
    if (LEP_OK != result)
    {
        return result;
    }
    fluxParamsPtr.sceneEmissivity = 7782;
    fluxParamsPtr.tauWindow = 7373;
    fluxParamsPtr.reflWindow = 819;

    result = LEP_SetRadFluxLinearParams(&LepPortDesc, fluxParamsPtr);
    if (LEP_OK != result)
    {
        return result;
    }

    result = LEP_SetRadTLinearAutoResolution(&LepPortDesc, LEP_RAD_ENABLE);
    if (LEP_OK != result)
    {
        return result;
    }

    // result = LEP_GetRadExternalRBFOHighGain(&LepPortDesc, &g_RBFO_InternalHighGain);
    // if (LEP_OK != result)
    // {
    //      return result;
    // }
    
    // result = LEP_GetRadRBFOExternal0(&LepPortDesc, &g_RBFO_InternalHighGain);
    // if (LEP_OK != result)
    // {
    //      return result;
    // }
    
    

#if 0
    result = LEP_GetSysGainModeObj(&LepPortDesc, &obj);
    if (LEP_OK != result)
    {
        return result;
    }
    //         obj.sysGainModeFluxThresholdHigh = 9876;
    //         obj.sysGainModeFluxThresholdLow = 8765;
    //         obj.sysGainModeROI.startRow = 0;
    //         obj.sysGainModeROI.startCol = 0;
    //         obj.sysGainModeROI.endRow = 79;
    //         obj.sysGainModeROI.endCol = 59;
    //         obj.sysGainModeTempEnabled = 1;
    //         obj.sysGainModeThresholds.sys_C_high_to_low = 110;
    //         obj.sysGainModeThresholds.sys_C_low_to_high = 90;
    //         obj.sysGainModeThresholds.sys_P_high_to_low = 5;
    //         obj.sysGainModeThresholds.sys_P_low_to_high = 95;
    //         obj.sysGainModeThresholds.sys_T_high_to_low = 383;
    //         obj.sysGainModeThresholds.sys_T_low_to_high = 363;
    //         obj.sysGainRoiPopulation = 4800;
    //         obj.sysGainModeThresholds.sys_P_high_to_low = 5;
    //         obj.sysGainModeTempEnabled = 1;
    /*    obj.sysGainModeThresholds.sys_P_low_to_high = 95;*/
    result = LEP_SetSysGainModeObj(&LepPortDesc, obj);
    if (LEP_OK != result)
    {
        return result;
    }
#endif
    //     result = Lepton_RunFFCCMD();
    //     if (LEP_OK != result)
    //     {
    //         return result;
    //     }

    return (result);
}
/**
 * @brief  Init the port of Lepton PWD
 *
 * @param  None
 *
 * @return None
 */
void Lepton_PwrRstPin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    LEPTON_RES_CLK_ENABLE();
    LEPTON_PWD_CLK_ENABLE();

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin = LEPTON_RES_PIN;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LEPTON_RES_PORT, &GPIO_InitStructure);
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_RESET);

    GPIO_InitStructure.Pin = LEPTON_PWD_PIN;
    HAL_GPIO_Init(LEPTON_PWD_PORT, &GPIO_InitStructure);

    HAL_Delay(100);

    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LEPTON_PWD_PORT, LEPTON_PWD_PIN, GPIO_PIN_SET);
}

/**
 * @brief  Reset Lepton
 *
 * @param  None
 *
 * @return None
 */
void Lepton_Reset(void)
{
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_RESET);
    Delay_Ms(10);
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_SET);
    Delay_Ms(10);
}

/**
 * @brief  Open the Lepton power
 *
 * @param  None
 *
 * @return None
 */
void Lepton_PowerOn(void)
{
    HAL_GPIO_WritePin(LEPTON_PWD_PORT, LEPTON_PWD_PIN, GPIO_PIN_SET);
}

extern SPI_HandleTypeDef SpiHandle;
void Lepton_PowerOff(void)
{
    // HAL_SPI_DMAStop(&SpiHandle);
    // __HAL_SPI_DISABLE_IT(&SpiHandle,SPI_IT_EOT);
    // HAL_SPI_DeInit(&SpiHandle);
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_RESET);
    Delay_Ms(10);
    HAL_GPIO_WritePin(LEPTON_PWD_PORT, LEPTON_PWD_PIN, GPIO_PIN_RESET);
    Delay_Ms(10);
}

void Lepton_ExitStandby(void)
{
    HAL_GPIO_WritePin(LEPTON_PWD_PORT, LEPTON_PWD_PIN, GPIO_PIN_SET); // power high

    // assert reset
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_RESET);
    Delay_Ms(10);
    // de-assert reset
    HAL_GPIO_WritePin(LEPTON_RES_PORT, LEPTON_RES_PIN, GPIO_PIN_SET);
    Delay_Ms(10);

    Delay_Ms(750);
    Delay_Ms(250); // 重要！！防止Lepton还没有启动起来，延时1s

    while (Lepton_Init())
    {
        Error_Handler();
    }
    if (!LEPTON_SPI_DMA_Init())
    {
        Error_Handler();
    }

    // 等待Lepton启动成功，重要
    // Delay_Ms(1000);
}
/**
 * @brief  关闭shutter进行校正
 *
 * @param  None
 *
 * @return LEP_OK if all goes well, otherwise an error code.
 */
// #define  SHUTTER_ON
LEP_RESULT Lepton_RunFFCCMD(void)
{
    LEP_RESULT result = LEP_OK;

    result = LEP_RunRadFFC(&LepPortDesc);
    if (LEP_OK != result)
    {
        return result;
    }

    return LEP_OK;
}

LEP_RESULT Lepton_SetGainMode(uint8_t mode)
{
    LEP_RESULT result = LEP_OK;

    if (GAIN_MODE_HIGH == mode)
    {
        result = LEP_SetSysGainMode(&LepPortDesc, LEP_SYS_GAIN_MODE_HIGH);
    }
    else if (GAIN_MODE_LOW == mode)
    {
        result = LEP_SetSysGainMode(&LepPortDesc, LEP_SYS_GAIN_MODE_LOW);
    }
    else if (GAIN_MODE_AUTO == mode)
    {
        result = LEP_SetSysGainMode(&LepPortDesc, LEP_SYS_GAIN_MODE_AUTO);
    }

    return (result);
}

LEP_RESULT Lepton_GetModuleType(uint8_t *type)
{
    if (NULL != type)
    {
        *type = LEPTON_3_5;

        return (LEP_OK);
    }
    else
    {
        return (LEP_ERROR);
    }
}
LEP_RESULT Lepton_GetSerialNumber(uint64_t *serialNumber)
{
    LEP_RESULT result = LEP_OK;
    LEP_SYS_FLIR_SERIAL_NUMBER_T flirSerialNumber;

    if (NULL == serialNumber)
    {
        return (LEP_ERROR);
    }

    result = LEP_GetSysFlirSerialNumber(&LepPortDesc, &flirSerialNumber);

    if (LEP_OK != result)
    {
        return (result);
    }

    *serialNumber = flirSerialNumber;

    return (LEP_OK);
}

void LEPTON_EXIT_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(LEPTON_EXIT_PIN);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    static uint16_t dcnt = 0;

    if (LEPTON_EXIT_PIN == GPIO_Pin)
    {
        // if (dcnt < 15)
        // {
        //     dcnt++;
        // }

        // if((dcnt >= 15) && (DmaBusy == 0) && (0 == g_LeptonPause))
        if (TRANSFER_COMPLETE == wTransferState && !FrameOkay)
        {
            // DmaBusy = 1;
            LEPTON_SPI_DMA_Begin();
        }
    }
    // 处理触摸屏外部中断事件
    else if (CST816T_EXTI_PIN == GPIO_Pin)
    {
        cst816t_exti_callback(GPIO_Pin);
    }
}

void Lepton_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    LEPTON_EXIT_CLK_ENABLE();

    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = LEPTON_EXIT_PIN;
    HAL_GPIO_Init(LEPTON_EXIT_PORT, &GPIO_InitStructure);

    HAL_NVIC_SetPriority(LEPTON_EXIT_IRQ, 2, 0);
    HAL_NVIC_EnableIRQ(LEPTON_EXIT_IRQ);
}

/**
 * @brief  Lepton首次运行时初始化
 *
 * @param  None
 *
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT Lepton_Init(void)
{
    LEP_RESULT result = LEP_OK;

    HAL_Delay(1000); // 重要！！防止Lepton还没有启动起来，延时1s
    Lepton_EXTI_Init();

    result = LEP_OpenPort(1, LEP_CCI_TWI, 400, &LepPortDesc);
    if (LEP_OK != result)
    {
        return result;
    }

    result = RunLeptonInitCmd();
    if (LEP_OK != result)
    {
        return result;
    }

    return LEP_OK;
}

/**
 * @brief  对获得的原始数据进行图像增强操作
 *
 * @param  None
 *
 * @return None
 */
// uint8_t GrayAvgMap[256];
// static void Lepton_Dprocess_HotMental(void)
// {
//     uint16_t i = 0,j = 0,pos = 0;
//     uint16_t max_sum = 0;
//     uint16_t min_sum = 0;
//     uint16_t max_data = 0;
//     uint16_t min_data = 0;
//     uint16_t alladdnum = 0;
//     uint8_t  addval = 30;
//     // 	uint8_t MaxGrayData = 0;
//     // 	uint8_t MinGrayData = 0;
//     // 	uint8_t MaxGrayTemp = 0;
//     // 	uint8_t GrayTotalNum = 0;
//     // 	uint8_t GrayNum = 0;
//     uint16_t addnum = 500;//512 + 256;//512;
//     uint16_t clip_high = 500;//512 + 256;//512;
//     uint32_t temp_Cum = 0;

//     for(i = HistableNum - 1; i > 0; i--)
//     {
//         max_sum = max_sum + g_HisTable[i];
//         if(max_sum > 0)
//         {
//             max_data = i;
//             break;
//         }
//     }

//     for(i = 0; i < HistableNum; i++)
//     {
//         min_sum = min_sum + g_HisTable[i];
//         if(min_sum > 0)
//         {
//             min_data = i;
//             break;
//         }
//     }

//     if((max_data - min_data) < 5)
//     {
//         addval = 100;
//         alladdnum = 200;
//     }
//     else if( (max_data - min_data) < 15)
//     {
//         addval = 105 - (max_data - min_data);
//         alladdnum = 200;
//     }
//     else if((max_data - min_data) > 105)
//     {
//         addval = 20;
//         alladdnum = 200;
//     }
//     else
//     {
//         addval = 125 - (max_data - min_data);
//         alladdnum = 200;//1025 - (max_data - min_data)*15;
//     }

//     for (i = 0; i < min_data; i++)
//     {
//         g_Sum_HisTable[i] = (addnum + alladdnum)* addval/5;//5120;
//     }
//     temp_Cum = (addnum + alladdnum) * addval/5;

//     for (i = min_data; i <= max_data; i++)
//     {
//         if (g_HisTable[i] > clip_high)
//         {
//             g_HisTable[i] = addnum + alladdnum + clip_high;//1024;
//         }
//         else if (g_HisTable[i] > 0)
//         {
//             g_HisTable[i] += addnum + alladdnum;
//         }
//         else
//         {
//             g_HisTable[i] += alladdnum;
//         }

//         temp_Cum += g_HisTable[i];

//         g_Sum_HisTable[i] += temp_Cum;
//     }

//     for (i = HistableNum - 1; i > max_data; i--)
//     {
//         g_Sum_HisTable[i] = g_Sum_HisTable[max_data];
//     }

//     for (i = 0; i < 120; i++)
//     {
//         for (j = 0; j < 160; j++)
//         {
//             g_Lepton_Buffer_ex[i][j] = (255) * (g_Sum_HisTable[g_Lepton_Buffer[i][j]]) / (g_Sum_HisTable[max_data] + addnum * addval);
//         }
//     }

//     for (i = 0; i < (120 + 1); i++)
//     {
//         g_Lepton_Buffer_ex[i][160] = g_Lepton_Buffer_ex[i][160 - 1];
//     }

//     for (j = 0; j < (160 + 1); j++)
//     {
//         g_Lepton_Buffer_ex[120][j] = g_Lepton_Buffer_ex[120][j - 1];
//     }

//     for (i = 0; i < 61; i++)
//         for (j = 0; j < 81; j++)
//         {
//             //拷贝到Wifi发送缓冲区
//             if ((RESET == HAL_GPIO_ReadPin(WIFI_BUSY_PORT, WIFI_BUSY_PIN)) && (1 == g_Trans_Finish) && (0 == g_Wifi_Trans_Flag))
//             {
//                 if (i < 60 && j < 80)
//                 {
//                     g_SPI_Send_Buffer[i * 80 + j] = ColorMap_Color_RGB565(g_Lepton_Buffer_ex[((59 - i) << 1)][((79 - j) << 1)]);
//                 }
//             }
//         }
// }

static uint16_t total_max = 0, total_min = 0xFFFF;
uint16_t g_disp_buffer[320][240];
uint8_t g_ir_gray[320][240];
void Lepton_ImageProcess(void)
{
    static uint8_t frame_cnt = 0;

    uint16_t i, j;

    total_max = 0, total_min = 0xFFFF;
    for (i = 0; i < 120; i++)
    {
        for (j = 0; j < 160; j++)
        {
            if (LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS] == 1)
            {
                // g_Lepton_Buffer[i][j] = g_Lepton_Buffer[i][j] / 10;
            }

            if (g_Lepton_Buffer[i][j] > total_max)
            {
                total_max = g_Lepton_Buffer[i][j];
            }

            if (g_Lepton_Buffer[i][j] < total_min)
            {
                total_min = g_Lepton_Buffer[i][j];
            }
        }
    }

    ++frame_cnt;
    if (10 < frame_cnt)
    {
        frame_cnt = 0;
    }

    int16_t value = 0;
    // 执行旋转和放大
    for (int i = 0; i < IMAGE_HEIGHT; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH; j++)
        {
            value = (int16_t)(255.0f * (float)(g_Lepton_Buffer[i][j] - total_min) / (float)(total_max - total_min));

            if (value < 0)
            {
                value = 0;
            }
            else if (255 < value)
            {
                value = 255;
            }

            uint16_t new_i = j * 2;
            uint16_t new_j = (IMAGE_HEIGHT * 2) - 1 - (i * 2);

            g_disp_buffer[new_i][new_j] = COLORMAP_RGB565_MSB[2][value];
            g_disp_buffer[new_i][new_j - 1] = COLORMAP_RGB565_MSB[2][value];
            g_disp_buffer[new_i + 1][new_j] = COLORMAP_RGB565_MSB[2][value];
            g_disp_buffer[new_i + 1][new_j - 1] = COLORMAP_RGB565_MSB[2][value];
        }
    }

    // st7789_draw_image(0, 0, 240, 320, g_disp_buffer);
    st7789_draw_image(0, 0, 240, 280, (const uint16_t *)&g_disp_buffer[20]);
}

// 函数用于逆时针旋转90度并放大2倍
void rotateAndScale(uint16_t ir_raw[IMAGE_HEIGHT][IMAGE_WIDTH], uint16_t disp_buff[320][240])
{
    // 执行旋转和放大
    for (int i = 0; i < IMAGE_HEIGHT; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH; j++)
        {
            uint16_t value = ir_raw[i][j];
            uint16_t new_i = j * 2;
            uint16_t new_j = (IMAGE_HEIGHT * 2) - 1 - (i * 2);

            disp_buff[new_i][new_j] = value;
            disp_buff[new_i][new_j - 1] = value;
            disp_buff[new_i + 1][new_j] = value;
            disp_buff[new_i + 1][new_j - 1] = value;
        }
    }
}

#define SRC_WIDTH 80
#define SRC_HEIGHT 60
#define DST_WIDTH (SRC_WIDTH * 2)
#define DST_HEIGHT (SRC_HEIGHT * 2)

void Lepton_Scale_up_2x_near(const uint16_t *src, uint16_t *dst, uint16_t src_height, uint16_t src_width)
{
    for (uint16_t row = 0; row < DST_HEIGHT; row++)
    {
        for (uint16_t col = 0; col < DST_WIDTH; col++)
        {
            uint8_t new_row = (uint8_t)(row >> 1);
            uint8_t new_col = (uint8_t)(col >> 1);

            dst[(row * DST_WIDTH) + col] = src[new_row * SRC_WIDTH + new_col];

            // dst[(y * 240) + x] = 0xf800;
        }
    }
}

void Lepton_Scale_up_2x(const uint16_t *src, uint16_t *dst, uint16_t src_height, uint16_t src_width)
{
    uint16_t dst_height = src_height << 1;
    uint16_t dst_width = src_width << 1;

    g_LeptonMaxMinRaw.max_Raw = 0;
    g_LeptonMaxMinRaw.max_X = 0;
    g_LeptonMaxMinRaw.max_Y = 0;

    g_LeptonMaxMinRaw.min_Raw = 65535;
    g_LeptonMaxMinRaw.min_X = 0;
    g_LeptonMaxMinRaw.min_Y = 0;
    // 清零
    g_Number_Hightemp = 0;

    for (int y = 0; y < src_height; y++)
    {
        for (int x = 0; x < src_width; x++)
        {
            // 计算源图像中的索引
            int src_idx = y * src_width + x;

            // 获取当前像素及其右侧和下方的像素值
            uint16_t current = src[src_idx];

            //高温点统计
            if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
            {
                if (current > gs_HIGH_TEMP_HIGH_GAIN)
                {
                    g_Number_Hightemp++;
                }
            }
            else if (0 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS]) //lowGain
            {
                if (current > gs_HIGH_TEMP_LOW_GAIN)
                {
                    g_Number_Hightemp++;
                }
            }
            if ((current > g_LeptonMaxMinRaw.max_Raw)) //&& (x >= 5) && (x < 75))
            {
                g_LeptonMaxMinRaw.max_Raw = current;
                g_LeptonMaxMinRaw.max_X = x;
                g_LeptonMaxMinRaw.max_Y = y;
            }
            if (current < g_LeptonMaxMinRaw.min_Raw)
            {
                g_LeptonMaxMinRaw.min_Raw = current;
                g_LeptonMaxMinRaw.min_X = x;
                g_LeptonMaxMinRaw.min_Y = y;
            }

            uint16_t right = (x < src_width - 1) ? src[src_idx + 1] : current;
            uint16_t bottom = (y < src_height - 1) ? src[(y + 1) * src_width + x] : current;
            uint16_t bottom_right = ((y < src_height - 1) && (x < src_width - 1)) ? src[(y + 1) * src_width + x + 1] : current;

            // 计算中间值
            uint16_t top_mid = (current + right + 1) >> 1; // 加1是为了四舍五入
            uint16_t bottom_mid = (bottom + bottom_right + 1) >> 1;
            uint16_t left_mid = (current + bottom + 1) >> 1;
            uint16_t right_mid = (right + bottom_right + 1) >> 1;

#if 1
            // 目标缓冲区中的起始位置
            int dst_start = (y * 2) * dst_width + (x * 2);

            // 将计算结果放置到目标缓冲区中
            dst[dst_start] = current;                                         // 左上角
            dst[dst_start + 1] = top_mid;                                     // 右上角
            dst[dst_start + dst_width] = left_mid;                            // 左下角
            dst[dst_start + dst_width + 1] = (top_mid + bottom_mid + 1) >> 1; // 右下角
#else
            // 将计算结果放置到目标缓冲区中，注意这里是转置后的坐标
            int dst_x_start = y * 2; // 注意这里的转换，因为是转置
            int dst_y_start = x * 2;

            // 填充放大并转置后的图像
            dst[dst_y_start * dst_height + dst_x_start] = COLORMAP_RGB565_MSB[COLOUR_IDX][current];                                     // current; // 左上角
            dst[dst_y_start * dst_height + dst_x_start + 1] = COLORMAP_RGB565_MSB[COLOUR_IDX][top_mid];                                 // top_mid; // 右上角
            dst[(dst_y_start + 1) * dst_height + dst_x_start] = COLORMAP_RGB565_MSB[COLOUR_IDX][left_mid];                              // left_mid; // 左下角
            dst[(dst_y_start + 1) * dst_height + dst_x_start + 1] = COLORMAP_RGB565_MSB[COLOUR_IDX][((top_mid + bottom_mid + 1) >> 1)]; //(top_mid + bottom_mid + 1) >> 1; // 右下角

#endif
        }
    }
}

// --- 确保所有外部依赖正确 ---
#define SRC_ROWS 120
#define SRC_COLS 160
#define DST_ROWS 320
#define DST_COLS 240
#define INTERP_PREC 256  // 8位小数精度
void bilinear_interpolation(uint32_t div)
{
    uint16_t dst_row, dst_col;
    float i, j;                  // 源图像坐标（行i，列j）
    float u, v;                  // 小数偏移（0.0~1.0）
    float val[4];                // 4个邻域像素的浮点灰度值
    float mid_top, mid_bottom;   // 分阶段插值中间值
    float val_interp;            // 最终插值结果
    uint8_t val_final;           // 最终灰度值
    uint16_t rgb_dst;            // 目标RGB值

    // 遍历目标图像（320行×240列）
    for (dst_row = 0; dst_row < 320; dst_row++)
    {
        uint16_t *dst_row_ptr = &g_disp_buffer[dst_row][0];
        
        for (dst_col = 0; dst_col < 240; dst_col++)
        {
            // --- 1. 修正：逆时针旋转90度+2倍缩放的坐标映射 ---
            float rot_row = dst_row / 2.0f;  // 旋转后行（对应源列反向）
            float rot_col = dst_col / 2.0f;  // 旋转后列（对应源行）
            i = rot_col;                     // 源行 = 旋转后列（核心旋转映射）
            j = (SRC_COLS - 1) - rot_row;    // 源列 = 源列最大值 - 旋转后行（核心旋转映射）

            // 提取整数部分和小数偏移
            uint16_t i_int = (uint16_t)i;
            uint16_t j_int = (uint16_t)j;
            u = j - j_int;  // 源列方向小数偏移（左右）
            v = i - i_int;  // 源行方向小数偏移（上下）

            // --- 2. 边界处理：镜像邻域 ---
            uint16_t i1 = (i_int + 1 < SRC_ROWS) ? (i_int + 1) : (i_int > 0 ? i_int - 1 : i_int);
            uint16_t j1 = (j_int + 1 < SRC_COLS) ? (j_int + 1) : (j_int > 0 ? j_int - 1 : j_int);
            if (i_int >= SRC_ROWS) i_int = SRC_ROWS - 1;
            if (j_int >= SRC_COLS) j_int = SRC_COLS - 1;

            // --- 3. 获取源图像4个邻域像素 ---
            val[0] = 255.0f * g_Sum_HisTable[g_Lepton_Buffer[i_int][j_int]] / (float)div;  // 当前
            val[1] = 255.0f * g_Sum_HisTable[g_Lepton_Buffer[i_int][j1]] / (float)div;     // 右
            val[2] = 255.0f * g_Sum_HisTable[g_Lepton_Buffer[i1][j_int]] / (float)div;     // 下
            val[3] = 255.0f * g_Sum_HisTable[g_Lepton_Buffer[i1][j1]] / (float)div;        // 下右

            // --- 4. 分阶段插值（水平→垂直） ---
            mid_top = val[0] * (1.0f - u) + val[1] * u;        // 水平上
            mid_bottom = val[2] * (1.0f - u) + val[3] * u;     // 水平下
            val_interp = mid_top * (1.0f - v) + mid_bottom * v; // 垂直组合

            // val_interp = (0.6 + (0.5f / 256.0f) * val_interp) * val_interp;
            // uint8_t value = (uint8_t)temp_value;
            // // --- 5. 转换为最终灰度值 ---
            val_final = (uint8_t)roundf(val_interp);
            val_final = (val_final > 255) ? 255 : (val_final < 0 ? 0 : val_final);

            // --- 6. 写入目标缓冲区 ---
            rgb_dst = REVERSE_RGB565(COLORMAP_RGB565_MSB[COLOUR_IDX][val_final]);
            dst_row_ptr[dst_col] = rgb_dst;
        }
    }
}

/**
 * @brief  对获得的原始数据进行图像增强操作
 *
 * @param  None
 *
 * @return None
 */
uint8_t GrayAvgMap[256];
// 宏条件编译：区分函数实现
#if defined(LEPTON_25) || defined(LEPTON_FS)
void Lepton_Dprocess_HotMental(void)
{
    uint16_t i = 0, j = 0, pos = 0;
    uint16_t max_sum = 0;
    uint16_t min_sum = 0;
    uint16_t max_data = 0;
    uint16_t min_data = 0;
    uint16_t alladdnum = 0;
    uint8_t addval = 30;
    uint16_t addnum = 500;
    uint16_t clip_high = 500;
    uint32_t temp_Cum = 0;

    for (i = HistableNum - 1; i > 0; i--)
    {
        // max_sum = max_sum + g_HisTable[i];
        // if (max_sum > 0)
        if (g_HisTable[i] > 0)
        {
            max_data = i;
            break;
        }
    }

    for (i = 0; i < HistableNum; i++)
    {
        // min_sum = min_sum + g_HisTable[i];
        // if (min_sum > 0)
        if (g_HisTable[i] > 0)
        {
            min_data = i;
            break;
        }
    }

    uint16_t max_offset = max_data - min_data;

    if (max_offset < 5)
    {
        addval = 100;
        alladdnum = 200;
    }
    else if (max_offset < 15)
    {
        addval = 105 - max_offset;
        alladdnum = 200;
    }
    else if (max_offset > 105)
    {
        addval = 20;
        alladdnum = 200;
    }
    else
    {
        addval = 125 - max_offset;
        alladdnum = 200; // 1025 - (max_data - min_data)*15;
    }

    for (i = 0; i < min_data; i++)
    {
        g_Sum_HisTable[i] = (addnum + alladdnum) * addval / 5; // 5120;
    }
    temp_Cum = (addnum + alladdnum) * addval / 5;

    for (i = min_data; i <= max_data; i++)
    {
        if (g_HisTable[i] > clip_high)
        {
            g_HisTable[i] = addnum + alladdnum + clip_high; // 1024;
        }
        else if (g_HisTable[i] > 0)
        {
            g_HisTable[i] += addnum + alladdnum;
        }
        else
        {
            g_HisTable[i] += alladdnum;
        }

        temp_Cum += g_HisTable[i];

        g_Sum_HisTable[i] += temp_Cum;
    }

    for (i = HistableNum - 1; i > max_data; i--)
    {
        g_Sum_HisTable[i] = g_Sum_HisTable[max_data];
    }

    uint32_t divider = (g_Sum_HisTable[max_data] + addnum * addval);
    for (i = 0; i < 120; i++)
    {
        for (j = 0; j < 160; j++)
        {
            uint8_t value = 255 * (g_Sum_HisTable[g_Lepton_Buffer[i][j]]) / divider;
 
            uint16_t new_i = (IMAGE_WIDTH << 1) - 1 - (j << 1); // j * 2;
            uint16_t new_j = (i << 1);                          //(IMAGE_HEIGHT * 2) - 1 - (i * 2);

            // 假设 COLOUR_IDX 和 value 在此代码块之前已经被定义和计算
            uint16_t rgb565_value = REVERSE_RGB565(COLORMAP_RGB565_MSB[COLOUR_IDX][value]); // 缓存查表结果

            // 使用临时变量来减少 g_disp_buffer 的索引计算次数
            uint16_t *row = &g_disp_buffer[new_i][0];          // 获取当前行的起始地址
            uint16_t *prev_row = &g_disp_buffer[new_i - 1][0]; // 获取前一行的起始地址（注意边界检查）
            // 设置像素值
            row[new_j] = rgb565_value;
            row[new_j + 1] = rgb565_value;
            prev_row[new_j] = rgb565_value;
            prev_row[new_j + 1] = rgb565_value;

        }
    }
}
#else
void Lepton_Dprocess_HotMental(void)
{
    uint16_t i = 0, j = 0;
    uint16_t max_data = 0;
    uint16_t min_data = 0;
    uint16_t alladdnum = 0;
    uint8_t addval = 30;
    uint16_t addnum = 500;
    uint16_t clip_high = 500;
    uint32_t temp_Cum = 0;

    // 寻找有效灰度范围的最大值（亮部边界）
    for (i = HistableNum - 1; i > 0; i--)
    {
        if (g_HisTable[i] > 0)
        {
            max_data = i;
            break;
        }
    }

    // 寻找有效灰度范围的最小值（暗部边界）
    for (i = 0; i < HistableNum; i++)
    {
        if (g_HisTable[i] > 0)
        {
            min_data = i;
            break;
        }
    }

    uint16_t max_offset = max_data - min_data;

    // 动态调整增强参数，针对窄范围场景强化暗部压缩
    if (max_offset < 5)
    {
        addval = 120;  // 提高addval增强暗部压缩
        alladdnum = 200;
    }
    else if (max_offset < 15)
    {
        addval = 115 - max_offset;  // 提高基准值，增强暗部压制
        alladdnum = 200;
    }
    else if (max_offset > 105)
    {
        addval = 20;
        alladdnum = 200;
    }
    else
    {
        addval = 125 - max_offset;
        alladdnum = 200;
    }

    // 处理min_data之前的无效暗部区域，降低初始累积值
    for (i = 0; i < min_data; i++)
    {
        g_Sum_HisTable[i] = (addnum + alladdnum) * addval / 10;  // 除以10压低暗部起点
    }
    temp_Cum = (addnum + alladdnum) * addval / 10;  // 同步降低累积值起点

    // 动态调整clip_high，避免亮部像素过度集中
    if (max_offset < 15) {
        clip_high = 300;
    } else if (max_offset > 105) {
        clip_high = 800;
    } else {
        clip_high = 500 + (max_offset - 15) * 3;
    }

    // 处理有效灰度范围，同时增强亮部和压制暗部
    for (i = min_data; i <= max_data; i++) {
        // 亮部额外增量（越亮增量越多）
        uint16_t bright_offset = max_data - i;
        uint16_t extra_add = (bright_offset < 20) ? (20 - bright_offset) * 20 : 0;
        
        // 暗部压制（越暗削减越多）
        uint16_t dark_offset = i - min_data;
        uint16_t dark_reduce = (dark_offset < 20) ? (20 - dark_offset) * 10 : 0;
        
        // 计算最终增量（确保不小于基础值）
        uint16_t final_add = (addnum + alladdnum + extra_add) - dark_reduce;
        if (final_add < alladdnum) final_add = alladdnum;
        
        // 更新直方图并累积
        if (g_HisTable[i] > clip_high) {
            g_HisTable[i] = final_add + clip_high;
        } else if (g_HisTable[i] > 0) {
            g_HisTable[i] += final_add;
        } else {
            g_HisTable[i] += alladdnum;
        }

        temp_Cum += g_HisTable[i];
        g_Sum_HisTable[i] += temp_Cum;
    }

    // 处理max_data之后的无效亮部区域
    for (i = HistableNum - 1; i > max_data; i--)
    {
        g_Sum_HisTable[i] = g_Sum_HisTable[max_data];
    }

    // 计算映射除数（确保亮部能达到255）
    uint32_t divider = g_Sum_HisTable[max_data];
    if (divider == 0) divider = 1;

    // 14位原始数据映射到8位灰度图
// #if defined (LEPTON_FS)
//     for (i = 0; i < 120; i++)
//     {
//         for (j = 0; j < 160; j++)
//         {
//             uint8_t value = 255 * (g_Sum_HisTable[g_Lepton_Buffer[i][j]]) / divider;
//             // 请补充将value赋值到输出数组的代码
//             // 例如：output_buffer[i][j] = value;
//         }
//     }
// #endif

    bilinear_interpolation(divider);
}
#endif

void Lepton_ImageProcess_Hist(void)
{
    memset(g_HisTable, 0, HistableNum * sizeof(uint16_t));
    memset(g_Sum_HisTable, 0, HistableNum * sizeof(uint32_t));

    const uint16_t high_temp_range = IMGHIGHTEMP - IMGLOWTEMP;
    const uint16_t low_temp = IMGLOWTEMP;

    for (uint8_t i = 0; i < 120; i++)
    {
        for (uint8_t j = 0; j < 160; j++)
        {
            if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
            {
                g_Lepton_Buffer[i][j] = g_Lepton_Buffer[i][j] / 10;
            }

            uint16_t temp = g_Lepton_Buffer[i][j];
            uint16_t new_value;

            if (temp > IMGHIGHTEMP)
            {
                new_value = high_temp_range;
            }
            else if (temp < IMGLOWTEMP)
            {
                new_value = 0;
            }
            else
            {
                new_value = temp - low_temp;
            }

            g_Lepton_Buffer[i][j] = new_value;
            g_HisTable[new_value]++;
        }
    }

    Lepton_Dprocess_HotMental();
}


// lepton_v: 0->25  1->35
uint16_t Lepton_Get_CenterRaw(uint16_t *buffer, uint8_t lepton_v)
{
    int center_y = 30;
    int center_x = 40;

    if (1 == lepton_v)
    {
        center_y = 60;
        center_x = 80;
    }

    // 获取四个点的坐标
    int y1 = center_y - 1; // 左上角和右上角的y坐标
    int y2 = center_y;     // 左下角和右下角的y坐标
    int x1 = center_x - 1; // 左上角和左下角的x坐标
    int x2 = center_x;     // 右上角和右下角的x坐标

    // 计算四个点在一维数组中的索引
    uint16_t val1 = buffer[y1 * center_x * 2 + x1]; // 左上角
    uint16_t val2 = buffer[y1 * center_x * 2 + x2]; // 右上角
    uint16_t val3 = buffer[y2 * center_x * 2 + x1]; // 左下角
    uint16_t val4 = buffer[y2 * center_x * 2 + x2]; // 右下角

    // 计算平均值
    uint32_t sum = (uint32_t)(val1 + val2 + val3 + val4);
    uint16_t average = (uint16_t)(sum / 4);

    return average;
}

void Lepton_Fill_MaxMinRaw(uint16_t buffer[][160])
{
    // 清零
    g_Number_Hightemp = 0;
    
    g_LeptonMaxMinRaw.max_Raw = 0;
    g_LeptonMaxMinRaw.max_X = 0;
    g_LeptonMaxMinRaw.max_Y = 0;

    g_LeptonMaxMinRaw.min_Raw = 65535;
    g_LeptonMaxMinRaw.min_X = 0;
    g_LeptonMaxMinRaw.min_Y = 0;

    for (uint8_t i = 0; i < 120; i++)
    {
        for (uint8_t j = 0; j < 160; j++)
        {
            // 获取当前像素及其右侧和下方的像素值
            uint16_t current = buffer[i][j];
            if ((current > g_LeptonMaxMinRaw.max_Raw)) //&& (x >= 5) && (x < 75))
            {
                g_LeptonMaxMinRaw.max_Raw = current;
                g_LeptonMaxMinRaw.max_X = j;
                g_LeptonMaxMinRaw.max_Y = i;
            }
            if (current < g_LeptonMaxMinRaw.min_Raw)
            {
                g_LeptonMaxMinRaw.min_Raw = current;
                g_LeptonMaxMinRaw.min_X = j;
                g_LeptonMaxMinRaw.min_Y = i;
            }
            
           //高温点统计
            if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
            {
                if (current > gs_HIGH_TEMP_HIGH_GAIN)
                {
                    g_Number_Hightemp++;
                }
            }
            else if (0 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS]) //lowGain
            {
                if (current > gs_HIGH_TEMP_LOW_GAIN)
                {
                    g_Number_Hightemp++;
                }
            } 
        }
    }
}

#if 1
// lepton_v: 0->25  1->35
void Lepton_Display_TempInfo(uint8_t lepton_v)
{
    uint8_t str_temp[10] = {0};
    uint8_t lepton_width_max = 79;
    uint8_t scale_factor = 2;
    if (1 == lepton_v)
    {
        lepton_width_max = 159;
        scale_factor = 1;
    }

    uint8_t base_row = 20;

    // 添加中心点温度及光标
    double org_temp = g_LeptonCenterRaw / 10.0f - 273.0f;

    #ifdef LEPTON_30
        org_temp = g_RBFO_InternalHighGain.RBFO_B / log(g_RBFO_InternalHighGain.RBFO_R / (g_LeptonCenterRaw - g_RBFO_InternalHighGain.RBFO_O) + g_RBFO_InternalHighGain.RBFO_F);
        org_temp = org_temp - 273.0f;
    #endif

    double target_temp = 0;
    // double a =  -0.000551, b = 1.198073, c = -11.889881; //60cm校准的高增益参数
    double a = 0, b = 0, c = 0;
    if (0 == LeptonMetry[LEPTON_TLINEAR_GAINMODE_POS]) //highGain
    {
        // a = -0.000768;
        // b = 1.287136;
        // c = -15.071710; //150cm校准的高增益参数

        a = -0.000620;  // 原：-0.000768（二次项绝对值减小，抛物线变平缓）
        b =  1.220000;  // 原：1.287136（一次项稍减，配合二次项）
        c = -12.500000; // 原：-15.071710（常数项增大，整体上移）
    }
    else if (1 == LeptonMetry[LEPTON_TLINEAR_GAINMODE_POS]) //lowGain
    {
        a = -0.000153;
        b = 1.175507;
        c = -10.121193; //150cm校准的低增益参数
    }

    target_temp = a * org_temp * org_temp + b * org_temp + c;
    if (TEMP_UNIT_F == g_TempUnit)
    {
        sprintf((char *)str_temp, "Cen:%.1fF", target_temp * 1.8 + 32);
    }
    else if (TEMP_UNIT_C == g_TempUnit)
    {
        sprintf((char *)str_temp, "Cen:%.1fC", target_temp);
    }
    
    st7789_append_string(15, 10 + base_row, (const char *)str_temp, Font_11x18, 0xFFFF, 0, (uint16_t *)&g_disp_buffer[0]);
    st7789_append_image(100, 120 + base_row, 39, 39, (uint16_t *)&icon_spot_center[0], (uint16_t *)&g_disp_buffer[0]);

    // 添加最大值温度及光标
    /* 温度最值点 x y 坐标 */
    uint16_t spotx = (g_LeptonMaxMinRaw.max_Y << scale_factor), spoty = ((lepton_width_max - g_LeptonMaxMinRaw.max_X) << scale_factor);
    if (240 - 13 < spotx)
    {
        /* 右越界 */
        spotx = 240 - 26;
    }
    else if (spotx < 13)
    {
        /* 左越界 */
        spotx = 0;
    }
    else
    {
        spotx -= 13;
    }

    if ((base_row + 280 - 12) < spoty)
    {
        /* 下越界 */
        spoty = base_row + 280 - 24;
    }
    else if (spoty < (12 + base_row))
    {
        /* 上越界 */
        spoty = base_row;
    }
    else
    {
        spoty -= 12;  
    }

    st7789_append_image(spotx, spoty, 26, 24,
        (uint16_t *)&icon_spot_hot[0], (uint16_t *)&g_disp_buffer[0]);

    if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
    {
        g_LeptonMaxMinRaw.max_Raw /= 10;
    }
    org_temp = g_LeptonMaxMinRaw.max_Raw / 10.0f - 273.0f;

    #ifdef LEPTON_30
        org_temp = g_RBFO_InternalHighGain.RBFO_B / log(g_RBFO_InternalHighGain.RBFO_R / (g_LeptonMaxMinRaw.max_Raw - g_RBFO_InternalHighGain.RBFO_O) + g_RBFO_InternalHighGain.RBFO_F);
        org_temp = org_temp - 273.0f;
    #endif

    target_temp = a * org_temp * org_temp + b * org_temp + c;
    // sprintf((char *)str_temp, "Max:%.1fC", target_temp);
    if (TEMP_UNIT_F == g_TempUnit)
    {
        sprintf((char *)str_temp, "Max:%.1fF", target_temp * 1.8 + 32);
    }
    else if (TEMP_UNIT_C == g_TempUnit)
    {
        sprintf((char *)str_temp, "Max:%.1fC", target_temp);
    }
    st7789_append_string(15, 10 + base_row + 15, (const char *)str_temp, Font_11x18, 0xFFFF, 0, (uint16_t *)&g_disp_buffer[0]);

    // 添加最小值温度及光标
    spotx = (g_LeptonMaxMinRaw.min_Y << scale_factor), spoty = ((lepton_width_max - g_LeptonMaxMinRaw.min_X) << scale_factor);
    if (240 - 13 < spotx)
    {
        /* 右越界 */
        spotx = 240 - 26;
    }
    else if (spotx < 13)
    {
        /* 左越界 */
        spotx = 0;
    }
    else
    {
        spotx -= 13;
    }
    if ((base_row + 280 - 12) < spoty)
    {
        /* 下越界 */
        spoty = base_row + 280 - 24;
    }
    else if (spoty < (12 + base_row))
    {
        /* 上越界 */
        spoty = base_row;
    }
    else
    {
        spoty -= 12;
    }

    st7789_append_image(spotx, spoty, 26, 24,
        (uint16_t *)&icon_spot_cold[0], (uint16_t *)&g_disp_buffer[0]);

    if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
    {
        g_LeptonMaxMinRaw.min_Raw /= 10;
    }
    org_temp = g_LeptonMaxMinRaw.min_Raw / 10.0f - 273.0f;

#ifdef LEPTON_30
    org_temp = g_RBFO_InternalHighGain.RBFO_B / log(g_RBFO_InternalHighGain.RBFO_R / (g_LeptonMaxMinRaw.min_Raw - g_RBFO_InternalHighGain.RBFO_O) + g_RBFO_InternalHighGain.RBFO_F);
    org_temp = org_temp - 273.0f;
#endif
    target_temp = a * org_temp * org_temp + b * org_temp + c;
    // sprintf((char *)str_temp, "Min:%.1fC", target_temp);
    if (TEMP_UNIT_F == g_TempUnit)
    {
        sprintf((char *)str_temp, "Min:%.1fF", target_temp * 1.8 + 32);
    }
    else if (TEMP_UNIT_C == g_TempUnit)
    {
        sprintf((char *)str_temp, "Min:%.1fC", target_temp);
    }
    st7789_append_string(15, 10 + base_row + 30, (const char *)str_temp, Font_11x18, 0xFFFF, 0, (uint16_t *)&g_disp_buffer[0]);

    // st7789_append_image(100, 120 + base_row, 39, 39, (uint16_t *)&icon_spot_center[0], (uint16_t *)&g_disp_buffer[0]);
}
#endif




void Lepton_Auto_Gain(void)
{
    //判断是否切换增益模式
    if (g_Number_Hightemp > 50)
    {
        if (0 == LeptonMetry[LEPTON_TLINEAR_GAINMODE_POS]) //highGain
        {
            //切换到低增益模式
            Lepton_ChangeGainMode(LEP_SYS_GAIN_MODE_LOW);
        }
    }
    else if (g_Number_Hightemp < 30)
    {
        if (1 == LeptonMetry[LEPTON_TLINEAR_GAINMODE_POS]) //lowGain
        {
            //切换到高增益模式
            Lepton_ChangeGainMode(LEP_SYS_GAIN_MODE_HIGH);
        }
    }
}

#define BAD_POINT_THRESHOLD        (300)

void detectBadPixels(uint16_t image [][IMAGE_WIDTH]) {
    uint8_t  x, y;
    uint16_t bad_counter = 0;
    for (y = 0; y < IMAGE_HEIGHT; y++) {
        for (x = 0; x < IMAGE_WIDTH; x++) {
            uint32_t sum = 0;
            uint16_t count = 0;

            // 检查上方像素
            
            if (y > 0) {
                sum += image[y-1][x];
                count++;
            }

            // 检查下方像素
            if (y < IMAGE_HEIGHT - 1) {
                sum += image[y+1][x];
                count++;
            }

            // 检查左侧像素
            if (x > 0) {
                sum += image[y][x-1];
                count++;
            }

            // 检查右侧像素
            if (x < IMAGE_WIDTH - 1) {
                sum += image[y][x+1];
                count++;
            }

            // 计算邻域像素的平均值（整数运算）
            int average = sum / count;

            // 判断是否为坏点
            if (abs(image[y][x] - average) > BAD_POINT_THRESHOLD) {
                bad_counter++;
               // badPoints[y][x] = 1; // 标记为坏点
            } 
            else {
                //badPoints[y][x] = 0; // 不是坏点
            }
        }
    }
    
        uint8_t strTemp[10] = {0};
        sprintf((char *)strTemp, "Bad:%d", bad_counter);
        st7789_draw_string(20, 220, (const char *)strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
    
}

#if defined (LEPTON_FS)
static uint16_t downscaled_buffer[96][128]; // 用于存储缩小后的图像
/**
 * @brief 将 g_Lepton_Buffer 缩小到 96x128，再放大回 120x160
 * 使用两次最近邻插值，以达到最简洁和高效。
 */
void rescale_image_simple(void) {
    // --- 步骤 1: 缩小 (120x160 -> 96x128) ---
    // 缩放因子: 120/96 = 1.25, 160/128 = 1.25
    const float scale_down_row = 120.0f / 60.0f; // 1.25
    const float scale_down_col = 160.0f / 80.0f; // 1.25

    for (int i_dst = 0; i_dst < 60; i_dst++) {
        for (int j_dst = 0; j_dst < 80; j_dst++) {
            // 计算在原图中的浮点坐标
            float i_src_f = i_dst * scale_down_row;
            float j_src_f = j_dst * scale_down_col;

            // 四舍五入到最近的整数像素
            int i_src = (int)(i_src_f + 0.5f);
            int j_src = (int)(j_src_f + 0.5f);

            // 边界检查 (理论上不会越界，因为 scale > 1)
            if (i_src >= 120) i_src = 119;
            if (j_src >= 160) j_src = 159;

            // 采样并存入缩小后的缓冲区
            downscaled_buffer[i_dst][j_dst] = g_Lepton_Buffer[i_src][j_src];
        }
    }

    // 计算源缓冲区 g_Lepton_Buffer 的中心区域起始坐标
    // g_Lepton_Buffer 假设尺寸为 120x160
    // int src_rows = 120;
    // int src_cols = 160;
    // int dst_rows = 96;
    // int dst_cols = 128;

    // // 计算需要截取的区域在源缓冲区中的起始位置 (i_start, j_start)
    // // 目标是截取中心部分
    // int i_start = (src_rows - dst_rows) / 2; // (120 - 96) / 2 = 12
    // int j_start = (src_cols - dst_cols) / 2; // (160 - 128) / 2 = 16

    // // 遍历目标缓冲区的每个像素
    // for (int i_dst = 0; i_dst < dst_rows; i_dst++) {
    //     for (int j_dst = 0; j_dst < dst_cols; j_dst++) {
    //         // 计算对应源缓冲区的实际坐标
    //         int i_src = i_start + i_dst; // 从 i_start 开始，逐行对应
    //         int j_src = j_start + j_dst; // 从 j_start 开始，逐列对应

    //         // 边界检查（虽然理论上不会越界，但加上更安全）
    //         if (i_src >= 0 && i_src < src_rows && j_src >= 0 && j_src < src_cols) {
    //             // 采样并存入缩小后的缓冲区
    //             downscaled_buffer[i_dst][j_dst] = g_Lepton_Buffer[i_src][j_src];
    //         } else {
    //             // 理论上不应到达这里，但可以设置默认值或错误处理
    //             downscaled_buffer[i_dst][j_dst] = 0; // 或其他默认值
    //         }
    //     }
    // }


    // --- 步骤 2: 放大 (96x128 -> 120x160) ---
    // 缩放因子: 96/120 = 0.8, 128/160 = 0.8
    // const float scale_up_row = 96.0f / 120.0f; // 0.8
    // const float scale_up_col = 128.0f / 160.0f; // 0.8

    // for (int i_dst = 0; i_dst < 120; i_dst++) {
    //     for (int j_dst = 0; j_dst < 160; j_dst++) {
    //         // 计算在缩小图中的浮点坐标
    //         float i_src_f = i_dst * scale_up_row;
    //         float j_src_f = j_dst * scale_up_col;

    //         // 四舍五入到最近的整数像素
    //         int i_src = (int)(i_src_f + 0.5f);
    //         int j_src = (int)(j_src_f + 0.5f);

    //         // 边界检查 (理论上不会越界，因为 scale < 1)
    //         if (i_src >= 96) i_src = 95;
    //         if (j_src >= 128) j_src = 127;

    //         // 插值并写回原缓冲区
    //         g_Lepton_Buffer[i_dst][j_dst] = downscaled_buffer[i_src][j_src];
    //     }
    // }

    // --- 步骤 2: 放大 (96x128 -> 120x160) ---
    // 缩放因子: 120/96=1.25，160/128=1.25（目标/源）
    const float scale_factor = 2.0f;//1.25f;  // 统一缩放因子（行和列相同）

    for (int i_dst = 0; i_dst < 120; i_dst++) {
        for (int j_dst = 0; j_dst < 160; j_dst++) {
            // 1. 计算目标像素在缩小图中的浮点坐标（逆映射）
            float i_src_f = i_dst / scale_factor;  // 等价于 i_dst * 0.8f
            float j_src_f = j_dst / scale_factor;  // 等价于 j_dst * 0.8f

            // 2. 定位周围4个源像素（整数坐标）
            int i0 = (int)i_src_f;                // 上一行
            int i1 = (i0 + 1 < 60) ? (i0 + 1) : i0;  // 下一行（边界保护）
            int j0 = (int)j_src_f;                // 左一列
            int j1 = (j0 + 1 < 80) ? (j0 + 1) : j0;  // 右一列（边界保护）

            // 3. 提取4个源像素值
            uint16_t top_left = downscaled_buffer[i0][j0];
            uint16_t top_right = downscaled_buffer[i0][j1];
            uint16_t bottom_left = downscaled_buffer[i1][j0];
            uint16_t bottom_right = downscaled_buffer[i1][j1];

            // 4. 计算小数偏移（0~1，用于插值权重）
            float u = j_src_f - j0;  // 水平方向偏移（右偏程度）
            float v = i_src_f - i0;  // 垂直方向偏移（下偏程度）

            // 5. 分阶段线性插值（类似简化双线性）
            // 第一步：水平方向插值（左→右）
            uint16_t top_mid = (uint16_t)(top_left * (1 - u) + top_right * u + 0.5f);  // 上边缘中间值
            uint16_t bottom_mid = (uint16_t)(bottom_left * (1 - u) + bottom_right * u + 0.5f);  // 下边缘中间值

            // 第二步：垂直方向插值（上→下）
            uint16_t final_val = (uint16_t)(top_mid * (1 - v) + bottom_mid * v + 0.5f);  // 最终插值结果

            // 6. 写入目标缓冲区
            g_Lepton_Buffer[i_dst][j_dst] = final_val;
        }
    }

}
#endif

#define COLOR_RED  0xF800  // ST7789 RGB565 红色

/*! 在 g_disp_buffer 中心绘制双层空心红色矩形 */
void draw_double_border_red_rectangle(void)
{
    uint16_t x, y;
    
    // ================== 1. 尺寸与坐标计算 ==================
    // 外框参数 (宽120, 高160, 居中)
    const uint16_t outer_w = 120;
    const uint16_t outer_h = 160;
    const uint16_t outer_x = (240 - outer_w) / 2; // 外框起始X: 60
    const uint16_t outer_y = (320 - outer_h) / 2; // 外框起始Y: 80

    // 内框参数
    // offset: 两层线之间的间隔
    // offset=1: 两层线紧挨着; offset=2: 中间隔1个像素(更清晰)
    const uint16_t offset = 2; 
    const uint16_t inner_w = outer_w - 2 * offset;
    const uint16_t inner_h = outer_h - 2 * offset;
    const uint16_t inner_x = outer_x + offset;
    const uint16_t inner_y = outer_y + offset;

    // ================== 2. 绘制外框 ==================
    // 2.1 画外框顶部横线
    for (x = outer_x; x < outer_x + outer_w; x++)
    {
        g_disp_buffer[outer_y][x] = COLOR_RED;
    }
    // 2.2 画外框底部横线
    for (x = outer_x; x < outer_x + outer_w; x++)
    {
        g_disp_buffer[outer_y + outer_h - 1][x] = COLOR_RED;
    }
    // 2.3 画外框左边竖线
    for (y = outer_y; y < outer_y + outer_h; y++)
    {
        g_disp_buffer[y][outer_x] = COLOR_RED;
    }
    // 2.4 画外框右边竖线
    for (y = outer_y; y < outer_y + outer_h; y++)
    {
        g_disp_buffer[y][outer_x + outer_w - 1] = COLOR_RED;
    }

    // ================== 3. 绘制内框 ==================
    // 3.1 画内框顶部横线
    for (x = inner_x; x < inner_x + inner_w; x++)
    {
        g_disp_buffer[inner_y][x] = COLOR_RED;
    }
    // 3.2 画内框底部横线
    for (x = inner_x; x < inner_x + inner_w; x++)
    {
        g_disp_buffer[inner_y + inner_h - 1][x] = COLOR_RED;
    }
    // 3.3 画内框左边竖线
    for (y = inner_y; y < inner_y + inner_h; y++)
    {
        g_disp_buffer[y][inner_x] = COLOR_RED;
    }
    // 3.4 画内框右边竖线
    for (y = inner_y; y < inner_y + inner_h; y++)
    {
        g_disp_buffer[y][inner_x + inner_w - 1] = COLOR_RED;
    }
}


void Lepton_Display(uint8_t show_battery)
{
    uint8_t str_temp[10] = {0};
    uint8_t base_row = 18;
#if defined(LEPTON_25)
    uint16_t *pBuff = (uint16_t *)g_Lepton_Process_Buffer;
    g_LeptonCenterRaw = Lepton_Get_CenterRaw((uint16_t *)g_Lepton_Process_Buffer, 0);
    // if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
    // {
    //     g_LeptonCenterRaw = g_LeptonCenterRaw / 10;
    // }
    // 将LeptonRAW里面的数据放大到g_Lepton_Buffer中，
    Lepton_Scale_up_2x((uint16_t *)g_Lepton_Process_Buffer, (uint16_t *)g_Lepton_Buffer, 60, 80);
#else
//    detectBadPixels(g_Lepton_Buffer);
// #if defined (LEPTON_FS)
//     rescale_image_simple();
// #endif
    g_LeptonCenterRaw = Lepton_Get_CenterRaw((uint16_t *)g_Lepton_Buffer, 1);
    Lepton_Fill_MaxMinRaw(g_Lepton_Buffer);
#endif
    if (1 == LeptonMetry[LEPTON_TLINEAR_RESOLUTION_POS])
    {
        g_LeptonCenterRaw = g_LeptonCenterRaw / 10;
    }
    Lepton_ImageProcess_Hist();

    Lepton_Auto_Gain();

#if defined(LEPTON_25)
    Lepton_Display_TempInfo(0);
#else
    Lepton_Display_TempInfo(1);
#endif


    if (show_battery && (show_battery < BATTERY_LEVEL_NUM))
    {
        // 显示电量
        st7789_append_image(240 - 22 - 20, 10 + base_row, 22, 13, (uint16_t *)ICON_BATTERY[show_battery - 1], (uint16_t *)&g_disp_buffer[0]);
    }
    
    // //增加中心红色矩形
    // draw_double_border_red_rectangle();

    // 显示图像
    st7789_draw_image_dma(0, 0, 240, 285, (const uint16_t *)&g_disp_buffer[base_row]);
}
