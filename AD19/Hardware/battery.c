/*!
\file       battery.c
\brief      battery.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2024-12-24
\see        =
\version    =
\pre        =
\bug        =
\attention  系统电源控制 。
\warning


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
#include "battery.h"
#include "Lepton_Stm32_Control.h"
#include "bsp_flash.h"
#include "sd_diskio.h"

const uint8_t* ICON_BATTERY[] = {&icon_battery0[0],&icon_battery30[0],&icon_battery60[0],&icon_battery100[0],&icon_charging[0]};

ADC_HandleTypeDef hadc1;
static void _bat_adc_init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_14B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_391CYCLES_5; // ADC_SAMPLETIME_5CYCLE;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if (hadc->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
    PeriphClkInit.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(ADC1_IRQn);
    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */
  }
}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA4     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    /* ADC1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(ADC1_IRQn);
    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  }
}

void Battery_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  SYSTEM_POWER_CLK_ENABLE();

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = SYSTEM_POWER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SYSTEM_POWER_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SYSTEM_POWER_KEY_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(SYSTEM_POWER_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}
void ADC1_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1);
}

// #define VDDA_APPLI                       (3300UL)

/* Init variable out of expected ADC conversion data range */
// #define VAR_CONVERTED_DATA_INIT_VALUE    (__LL_ADC_DIGITAL_SCALE(ADC1, LL_ADC_RESOLUTION_12B) + 1)

/* Variables for ADC conversion data */
__IO uint16_t uhADCxConvertedData = 0; // VAR_CONVERTED_DATA_INIT_VALUE; /* ADC group regular conversion data */

/* Variables for ADC conversion data computation to physical values */
// uint16_t uhADCxConvertedData_Voltage_mVolt = 0;  /* Value of voltage calculated from ADC conversion data (unit: mV) */

/* Variable to report status of ADC group regular unitary conversion          */
/*  0: ADC group regular unitary conversion is not completed                  */
/*  1: ADC group regular unitary conversion is completed                      */
/*  2: ADC group regular unitary conversion has not been started yet          */
/*     (initial state)                                                        */
__IO uint8_t ubAdcGrpRegularUnitaryConvStatus = 2; /* Variable set into ADC interruption callback */

static uint16_t _adc_buffer[256];
static uint8_t _adc_buffer_count = 0;
__IO static uint8_t _adc_buffer_full = 1;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  /* Retrieve ADC conversion data */
  uhADCxConvertedData = HAL_ADC_GetValue(hadc);
  _adc_buffer[_adc_buffer_count] = uhADCxConvertedData;

  // if (_adc_buffer_count >= 32)
  // {
  //   // 表示电量缓冲区已经采集满
  //   _adc_buffer_full = 1;
  // }

  _adc_buffer_count = (_adc_buffer_count + 1) % 32;

  /* Computation of ADC conversions raw data to physical values           */
  /* using helper macro.                                                  */
  // uhADCxConvertedData_Voltage_mVolt = __LL_ADC_CALC_DATA_TO_VOLTAGE(ADC1, VDDA_APPLI, uhADCxConvertedData, LL_ADC_RESOLUTION_12B);

  /* Update status variable of ADC unitary conversion                     */
  ubAdcGrpRegularUnitaryConvStatus = 1;
}

void Battery_ADC_Init(void)
{
  _bat_adc_init();

  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
  {
    /* Calibration Error */
    Error_Handler();
  }

  // //开启采集一次
  // if (ubAdcGrpRegularUnitaryConvStatus)
  // {
  //   if (HAL_ADC_Start_IT(&hadc1) != HAL_OK)
  //   {
  //       /* Error: ADC conversion start could not be performed */
  //       Error_Handler();
  //   }
  //   /* Reset status variable of ADC group regular unitary conversion */
  //   ubAdcGrpRegularUnitaryConvStatus = 0;
  // }

  // while (1)
  // {
  //     /* Start ADC group regular conversion */
  //     if (HAL_ADC_Start_IT(&hadc1) != HAL_OK)
  //     {
  //         /* Error: ADC conversion start could not be performed */
  //         Error_Handler();
  //     }

  //     /* For this example purpose, wait until conversion is done */
  //     while (ubAdcGrpRegularUnitaryConvStatus != 1);

  //     /* Reset status variable of ADC group regular unitary conversion */
  //     ubAdcGrpRegularUnitaryConvStatus = 0;

  //     // _adc_buffer[cnt] = uhADCxConvertedData;
  //     // cnt = (cnt + 1) % 128;

  //     HAL_Delay(100);
  // }
}

#define POWER_ADC_AVG_NUMBER (256)

// // 3.9v 3.7v 3.5v 3.0v
#define POWER_ADC_CHARGE (16383)
// #define POWER_ADC_100 (15150)
// #define POWER_ADC_75 (13900)
// #define POWER_ADC_50 (12500)
// #define POWER_ADC_10 (11820)

#define POWER_ADC_100 (15200)
#define POWER_ADC_75 (13980)
#define POWER_ADC_50 (12580)
#define POWER_ADC_10 (11900)
#define POWER_ADC_OFFSET (75 * 2)

uint16_t g_History_val = 0;
const uint16_t max_offset = 200;
extern uint8_t g_Battery_Level;
 uint8_t Battery_Battery_ADC_Task(void)
{
  uint32_t battery_val = 0;
  uint8_t ret_val = 0;
  static uint16_t idx = 0, cnt = 0, refresh_count = 0;

  HAL_ADC_Start(&hadc1);

  HAL_ADC_PollForConversion(&hadc1, 10);

  battery_val = HAL_ADC_GetValue(&hadc1);

  uint8_t strTemp[10] = {0};
  // sprintf(strTemp,"%d",HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13));
  // st7789_draw_string(20, 100, strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);

  if (!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13))
  {
    // 重新开始计数，避免充电拔出后不及时更新状态
    cnt = 0;
    idx = 0;
    return BATTERY_LEVEL_CHARGING;
  }

  // if (battery_val >= POWER_ADC_CHARGE)
  // {
  //   return BATTERY_LEVEL_CHARGING;
  // }

  if (cnt < POWER_ADC_AVG_NUMBER)
  {
    _adc_buffer[idx] = battery_val;
    idx = (idx + 1) % POWER_ADC_AVG_NUMBER;
    ++cnt;
  }
  else
  {
    // 当读出来的值和平均值误差太大，则直接丢弃
    //if (abs(battery_val - g_History_val) < max_offset)
    uint16_t diff = (battery_val >= g_History_val) ? (battery_val - g_History_val) : (g_History_val - battery_val);
    if (diff < max_offset)
    {
      _adc_buffer[idx] = battery_val;
      idx = (idx + 1) % POWER_ADC_AVG_NUMBER;
    }
  }

  uint32_t sum = 0;
  for (int i = 0; i < cnt; i++)
  {
    sum += _adc_buffer[i];
  }

  g_History_val = (uint16_t)(sum / cnt);

  // sprintf(strTemp,"%d",g_History_val);
  // st7789_draw_string(20, 180, strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);

  // 开机给一个初始电量状态
  if (cnt < POWER_ADC_AVG_NUMBER)
  {
    if (g_History_val >= (POWER_ADC_100 + POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_100;
    }
    else if (g_History_val >= (POWER_ADC_75 + POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_60;
    }
    else if (g_History_val >= (POWER_ADC_50 + POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_30;
    }
    else if (g_History_val >= (POWER_ADC_10 + POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_0;
    }
    else
    {
      // ret_val = 0x11;
      st7789_backlight_off();
      Battery_PowerOff();
    }
  }
  else 
  {
    // 电量刷新频率变慢，避免波动
    if ((++refresh_count) <= 10000)
    {
      return g_Battery_Level; 
    }
    refresh_count = 0; 
    // 避免电量上下波动，每个段之间留下一个小的间隙不处理，保留上次的值
    // sprintf(strTemp,"%d",g_History_val);
    // st7789_draw_string(20, 180, strTemp, Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
  
    if (g_History_val >= (POWER_ADC_100 + POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_100;
    }
    else if (g_History_val >= (POWER_ADC_75 + POWER_ADC_OFFSET) && g_History_val <= (POWER_ADC_100 - POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_60;
    }
    else if (g_History_val >= (POWER_ADC_50 + POWER_ADC_OFFSET) && g_History_val <= (POWER_ADC_75 - POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_30;
    }
    else if (g_History_val >= (POWER_ADC_10) && g_History_val <= (POWER_ADC_50 - POWER_ADC_OFFSET))
    {
      ret_val = BATTERY_LEVEL_0;
    }
    else if (g_History_val < (POWER_ADC_10))
    {
      // ret_val = 0x11;
      st7789_backlight_off();
      Battery_PowerOff();
    }
    else
    {
      ret_val = g_Battery_Level;
      //BATTERY_LEVEL_INVALID;
    }
  }

  return ret_val;
}

/* USER CODE BEGIN 2 */
void Battery_PowerOn(void)
{
  HAL_GPIO_WritePin(SYSTEM_POWER_PORT, SYSTEM_POWER_PIN, GPIO_PIN_SET);
}

void Battery_PowerOff(void)
{
  if (g_TempChangeFlag)
  {
    BSP_Flash_Write((g_TempUnit << 16) | g_WindowTransimitRate);
  }
  
  HAL_GPIO_WritePin(SYSTEM_POWER_PORT, SYSTEM_POWER_PIN, GPIO_PIN_RESET);
}

#define CLOSE_MACHINE_TIMEOUT (9 * 5)
void Battery_KeyScan_Task(void)
{
  // 关机计数器
  static uint32_t power_count = 0;
  if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(SYSTEM_POWER_KEY_PORT, SYSTEM_POWER_KEY_PIN))
  {
    power_count++;
    if (power_count >= CLOSE_MACHINE_TIMEOUT)
    {
      st7789_backlight_off();
      Battery_PowerOff();
    }
  }
  else
  {
    power_count = 0;
  }
}

/** \} end of group ST7789_Exported_Functions */
/** \} end of group ST7789 */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
