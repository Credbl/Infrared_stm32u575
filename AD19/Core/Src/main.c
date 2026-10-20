/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "icache.h"
#include "memorymap.h"
#include "battery.h"
#include "Lepton_Delay.h"
#include "Lepton_Stm32_Control.h"
#include "Lepton_Stm32_SPI_DMA.h"
#include "Lepton_Common_Data.h"
#include "Lepton_Stm32_I2C.h"
#include "string.h"
#include "bsp_ov2640.h"
#include "bsp_gc2145.h"
#include "spi_flash.h"
#include "sd_diskio.h"
#include "key.h"
#include "bsp_flash.h"
#include "i2c_cst816t.h"
#include "photo_album.h"
//#include "diskio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
extern volatile uint8_t g_Infrared_Working_flag;

// 相册进入前保存当前工作状态，退出相册时再恢复，避免影响正常模式。
static uint8_t s_album_saved_working_flag = 0;
static uint8_t s_album_saved_infrared_flag = 0;
// 标记相册模式是否已经暂停后台采集，防止重复进入时覆盖保存值。
static uint8_t s_album_streams_paused = 0;

static void _do_test(void)
{
    Battery_PowerOn();
    HAL_Delay(500);

    st7789_test();

    BSP_Flash_Test();



    //spiflash_test();

    //spiflash_init();

    //ff_test();

//    usbd_user_init();
//
    while (1)
    {
        HAL_Delay(1000);
    }

//    MX_USBX_Device_Init();
//    while (1)
//    {
//        ux_system_tasks_run();
//    }
}


extern uint8_t g_ir_gray[320][240];
extern uint16_t g_disp_buffer[320][240];
uint8_t win_mode = 0;

int8_t process_save(uint8_t save)
{
    int8_t save_ret = 0;

    if (!save)
    {
        win_mode = MODE_NORMAL;
        return 0;
    }

    st7789_draw_image_dma((240 - 0x78) / 2, (280 - 0x50) / 2, 0x78, 0x50, (uint16_t *)icon_info_saving);

    if (0 == g_Working_flag)
    {
        save_ret = ff_save_bmp_rgb565((const uint16_t *)&g_disp_buffer[20], 240, 280);
        // save_ret = ff_save_bmp_256((const uint8_t *)&g_ir_gray[20], 240, 280);
    }
    else if (1 == g_Working_flag)
    {
        save_ret = ff_save_bmp_rgb565((const uint16_t *)&g_disp_buffer[20], 240, 280);
    }

    if (0 == save_ret)
    {
        // 保存成功后，把新照片文件名加入相册链表，后面进入相册可以直接翻到这张图。
        PhotoAlbum_AddFile(ff_get_last_bmp_name());
        st7789_draw_image_dma((240 - 0x78) / 2, (280 - 0x50) / 2, 0x78, 0x50, (uint16_t *)icon_save_success);
    }
    else
    {
        st7789_draw_image_dma((240 - 0x78) / 2, (280 - 0x50) / 2, 0x78, 0x50, (uint16_t *)icon_save_failed);
    }

    HAL_Delay(1000);

    win_mode = MODE_NORMAL;

    return 0;
}


static int8_t _saving_mode(uint8_t key)
{
    if (KEY_PWR_PRESS == key)
    {
        //win_mode = MODE_NORMAL;
    }

    if (FrameOkay)
    {
        FrameOkay = 0;
    }

    cst816t_process(win_mode);

    return 0;
}


uint8_t g_Battery_Level = 0;

// 相册双击判定时间，两个单击间隔小于这个时间就认为是双击。
#define ALBUM_DOUBLE_CLICK_MS 600U
// 删除确认界面只响应底部选择区域，触摸Y小于该值才认为点到了按钮区域。
#define ALBUM_SELECT_TOUCH_Y 80U

// 相册模式统一入口：enable=1进入相册，enable=0退出相册。
static void _set_album_mode(uint8_t enable)
{
    if (enable)
    {
        // 已经在相册模式时直接返回，避免重复保存状态。
        if (s_album_streams_paused)
        {
            return;
        }

        // 记录进入相册前的摄像头/红外工作状态。
        s_album_saved_working_flag = g_Working_flag;
        s_album_saved_infrared_flag = g_Infrared_Working_flag;
        s_album_streams_paused = 1;

        // 暂停后台采集和正常显示刷新，让相册翻页时屏幕覆盖更快。
        g_Working_flag = 0;
        g_Infrared_Working_flag = 0;
        display_flag = 0;
        FrameOkay = 1;
        cst816t_clear_gesture();
        win_mode = MODE_ALBUM;
        PhotoAlbum_Enter();
        return;
    }

    // 没有暂停过就不需要恢复。
    if (!s_album_streams_paused)
    {
        return;
    }

    // 退出相册，恢复进入前的工作状态，并清掉旧帧标志。
    g_Working_flag = s_album_saved_working_flag;
    g_Infrared_Working_flag = s_album_saved_infrared_flag;
    display_flag = 0;
    FrameOkay = 0;
    s_album_streams_paused = 0;
    win_mode = MODE_NORMAL;
}

// 正常模式下检测触摸双击，触发后进入相册模式。
static uint8_t _normal_touch_open_album(void)
{
    static uint32_t last_click_tick = 0;
    uint8_t gesture = cst816t_peek_gesture();

    // 触摸芯片直接识别到双击时，立即进入相册。
    if (CST816T_GESTURE_DOUBLE == gesture)
    {
        last_click_tick = 0;
        _set_album_mode(1);
        return 1;
    }

    // 如果触摸芯片只上报单击，就在软件里用时间间隔补一个双击判断。
    if (CST816T_GESTURE_CLICK == gesture)
    {
        uint32_t now = HAL_GetTick();

        cst816t_clear_gesture();
        if ((0U != last_click_tick) && ((now - last_click_tick) <= ALBUM_DOUBLE_CLICK_MS))
        {
            last_click_tick = 0;
            _set_album_mode(1);
            return 1;
        }

        last_click_tick = now;
    }
    else if (CST816T_GESTURE_NONE != gesture)
    {
        last_click_tick = 0;
    }

    return 0;
}

static int8_t _normal_mode(uint8_t key)
{
    if (_normal_touch_open_album())
    {
        return 0;
    }

    if (KEY_PWR_RELEASE == key && MODE_NORMAL == win_mode)
    {
        win_mode = MODE_SAVING;
        if (0 == g_Working_flag)
        {
            #ifndef LEPTON_25
            memcpy((uint8_t *)&g_Lepton_Buffer[0], (uint8_t *)&g_Lepton_Process_Buffer[0], (120 + 2) * (160) * sizeof(uint16_t));
            #endif
            Lepton_Display(0);
        }
        else if (1 == g_Working_flag)
        {
            while (!display_flag);

            OV2640_Display(0);
        }

        st7789_draw_image_dma(0, 285 - 0x3c, 240, 60, (uint16_t *)icon_save_select);

        return 0;
    }


    if (FrameOkay)
    {
        FrameOkay = 0;
        if (g_Infrared_Working_flag)
        {
#if defined(LEPTON_25)
            memcpy((uint8_t *)g_Lepton_Process_Buffer, (uint8_t *)g_Lepton_Rec_Buffer, 60 * 80 * sizeof(uint16_t));
#else
            memcpy((uint8_t *)&g_Lepton_Buffer[0], (uint8_t *)&g_Lepton_Process_Buffer[0], (120 + 2) * (160) * sizeof(uint16_t));
#endif
            Lepton_Display(g_Battery_Level);
        }
    }

    OV2640_Display(g_Battery_Level);

    cst816t_process(win_mode);

    return 0;
}


// 相册模式主逻辑：退出、长按删除、点击确认、上下翻页都在这里处理。
static int8_t _album_mode(uint8_t key)
{
    static uint8_t delete_confirm = 0;
    static uint32_t last_click_tick = 0;
    uint8_t gesture = cst816t_peek_gesture();

    // 电源键短按或触摸双击：退出相册，回到正常模式。
    if (KEY_PWR_RELEASE == key || CST816T_GESTURE_DOUBLE == gesture)
    {
        cst816t_clear_gesture();
        delete_confirm = 0;
        last_click_tick = 0;
        _set_album_mode(0);
        return 0;
    }

    // 长按当前照片：进入删除确认界面。
    if (CST816T_GESTURE_LONG == gesture)
    {
        cst816t_clear_gesture();
        last_click_tick = 0;
        if (PhotoAlbum_Count() > 0U)
        {
            delete_confirm = 1;
            PhotoAlbum_ShowDeleteSelect();
        }
        return 0;
    }

    if (CST816T_GESTURE_CLICK == gesture)
    {
        uint32_t now = HAL_GetTick();
        uint16_t touch_x = 0;
        uint16_t touch_y = 0;

        if (delete_confirm)
        {
            // 删除确认状态下，根据点击坐标判断选择删除还是返回。
            cst816t_peek_touch(&touch_x, &touch_y);
            cst816t_clear_gesture();
            delete_confirm = 0;
            last_click_tick = 0;
            if (touch_y < ALBUM_SELECT_TOUCH_Y)
            {
                if (touch_x < 120U)
                {
                    // 左侧区域：删除当前照片，并显示成功/失败提示。
                    int8_t delete_ret = PhotoAlbum_DeleteCurrent();
                    const uint16_t *icon =
                        (0 == delete_ret) ? (uint16_t *)icon_save_success : (uint16_t *)icon_save_failed;

                    st7789_draw_image_dma((240 - 0x78) / 2, (280 - 0x50) / 2,
                                          0x78, 0x50, icon);
                    HAL_Delay(1000);
                    PhotoAlbum_ShowCurrent();
                }
                else
                {
                    // 右侧区域：取消删除，重新显示当前照片。
                    PhotoAlbum_ShowCurrent();
                }
            }
            else
            {
                // 点到按钮区域外时，保持删除确认界面不退出。
                delete_confirm = 1;
                PhotoAlbum_ShowDeleteSelect();
            }
            return 0;
        }

        cst816t_clear_gesture();
        // 普通点击也做一次软件双击判断，用来退出相册。
        if ((0U != last_click_tick) && ((now - last_click_tick) <= ALBUM_DOUBLE_CLICK_MS))
        {
            last_click_tick = 0;
            _set_album_mode(0);
            return 0;
        }

        last_click_tick = now;
        return 0;
    }

    // 上滑：显示下一张照片。
    if (CST816T_GESTURE_UP == gesture)
    {
        cst816t_clear_gesture();
        delete_confirm = 0;
        last_click_tick = 0;
        PhotoAlbum_ShowNext();
        return 0;
    }

    // 下滑：显示上一张照片。
    if (CST816T_GESTURE_DOWN == gesture)
    {
        cst816t_clear_gesture();
        delete_confirm = 0;
        last_click_tick = 0;
        PhotoAlbum_ShowPrevious();
        return 0;
    }

    if (gesture != CST816T_GESTURE_NONE)
    {
        cst816t_clear_gesture();
        delete_confirm = 0;
        last_click_tick = 0;
    }

    return 0;
}
#if 1
RTC_HandleTypeDef hrtc;
// 定义一个独特的标记值（避免与默认值冲突）
#define RTC_INIT_MAGIC 0x12345678
/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  uint32_t magic = 0;
  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  // 读取后备寄存器中的标记
  magic = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
  // 若标记已存在，说明已初始化过，直接返回
  if (magic == RTC_INIT_MAGIC)
  {
    return; // 跳过初始化
  }

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  // 初始化完成后，写入标记到后备寄存器
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_INIT_MAGIC);

#if 0
  // 立即读取验证
  uint32_t test = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
  if (test != RTC_INIT_MAGIC)
  {
      // 写入失败，说明后备域配置有问题
     Error_Handler();
   }
#endif
  /* USER CODE END RTC_Init 2 */

}


void RTC_Test(void)
{
     // 定义时间和日期结构体
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
#if 0
    // 设置时间（23:59:50）
    sTime.Hours = 0x23;    // 小时（BCD格式：23 → 0x23）
    sTime.Minutes = 0x59;  // 分钟
    sTime.Seconds = 0x50;  // 秒
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler(); // 处理错误
    }

    // 设置日期（2024年6月30日，星期日）
    sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
    sDate.Month = RTC_MONTH_JUNE;  // 月份（BCD格式：6 → 0x06）
    sDate.Date = 0x30;             // 日期
    sDate.Year = 0x24;             // 年份（2024 → 0x24）
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
#endif
    RTC_TimeTypeDef gTime;
    RTC_DateTypeDef gDate;

    // 读取时间（注意：先读时间，再读日期，HAL库要求）
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

    // 打印时间（需实现串口打印，此处为伪代码）
    printf("Time: %02X:%02X:%02X\n", gTime.Hours, gTime.Minutes, gTime.Seconds);
    printf("Date: 20%02X-%02X-%02X, %d\n", gDate.Year, gDate.Month, gDate.Date, gDate.WeekDay);

    // 延时10秒（使用定时器或阻塞延时，注意RTC精度为秒级）
    HAL_Delay(10000);

    // 再次读取，验证秒数是否增加10
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);
    printf("After 10s: %02X:%02X:%02X\n", gTime.Hours, gTime.Minutes, gTime.Seconds);

}
#endif
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    uint8_t TouchCheckCount = 0;
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    // 系统时钟配置
    SystemClock_Config();

    SYSTEM_DelayInit(160);

    Battery_GPIO_Init();
    KEY_Init();
    // _do_test();

    /* Initialize all configured peripherals */
    Lepton_IIC_Init();
    // 初始化Lepton
    Lepton_PwrRstPin_Init();

    MX_ICACHE_Init();

    //cst816t_test();

    //cst816t_init();

    st7789_init();

    st7789_clear_dma(COLOR_RGB565_BLACK);
    // st7789_fill_rect_dma(0, 0, 240, 280, COLOR_RGB565_RED);

#ifdef LEPTON_25
    // 若定义了LEPTON_25，使用"80_"作为前缀
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_dark_anx[0], (uint16_t *)&g_disp_buffer[0]);
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_dark_default[0], (uint16_t *)&g_disp_buffer[0]);
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_dark_laureii[0], (uint16_t *)&g_disp_buffer[0]);
#elif defined(LEPTON_FS) 
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_dark_laureii[0], (uint16_t *)&g_disp_buffer[0]);
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_anx_ad19fs[0], (uint16_t *)&g_disp_buffer[0]);
    st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_dark_default[0], (uint16_t *)&g_disp_buffer[0]);
#else
    //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_anx_ad19u[0], (uint16_t *)&g_disp_buffer[0]);
     //st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_laureii_ad19u[0], (uint16_t *)&g_disp_buffer[0]);
      st7789_append_image(34, 100, 172, 78, (uint16_t *)&icon_logo_default_ad19u[0], (uint16_t *)&g_disp_buffer[0]);
#endif


    st7789_draw_image_dma(0, 0, 240, 285, (const uint16_t *)&g_disp_buffer[0]);
    HAL_Delay(100);
    st7789_backlight_on();

    //spiflash_init();

    // 亮屏显示logo，再开电
    Battery_PowerOn();
#if 1
    while(TouchCheckCount < 5)
    {
        if(cst816t_init())
        {
            TouchCheckCount++;
            if(TouchCheckCount >= 5)
            {  
                st7789_draw_string(20, 220, "TOUCH ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
                Error_Handler();
            }     
        }
        else
        {
              break;
        }    
    }
#endif
    //RTC初始化
    MX_RTC_Init();

   // RTC_Test();

    // 手动调用GET_FATTIME()，观察是否进入你的函数
    //DWORD test_time = GET_FATTIME();
    //printf("测试时间戳: 0x%08X\n", test_time); // 若输出0x2C0808C0，则说明调用成功

    if(GC2145_Init())
    {
        g_GC2145_state = 1;
        //st7789_draw_string(20, 220, "VIS ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        //Error_Handler();
    }

    Battery_ADC_Init();

    //KEY_Init();

    if(0 != spiflash_init())
    {
        st7789_draw_string(20, 220, "Flash ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
    }

    if(0 != ff_init())
    {
        st7789_draw_string(20, 220, "File ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
    }
    // 初始化相册链表，并扫描SD卡里已有的BMP照片。
    PhotoAlbum_Init();
    PhotoAlbum_Rebuild();

    // 读取校准参数
    g_ReadWindowTransimitRate = BSP_Flash_Read();
    if ((TEMP_UNIT_C == (g_ReadWindowTransimitRate >> 16)) || (TEMP_UNIT_F == (g_ReadWindowTransimitRate >> 16)))
    {
        g_TempUnit = (g_ReadWindowTransimitRate >> 16);
    }

    // OV2640_Test();

    while (Lepton_Init())
    {
        st7789_draw_string(20, 220, "IR ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
    }
    if (!LEPTON_SPI_DMA_Init())
    {
        st7789_draw_string(20, 220, "IR ERROR!", Font_16x26, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
        Error_Handler();
    }


    // 设置保存的校准参数
    Lepton_SetFluxParams(g_ReadWindowTransimitRate & 0xFFFF);

    // 等待Lepton启动成功，重要
    Delay_Ms(1000);
#if 1
    // 校准，避免开机错误图像
    //Lepton_RunFFCCMD();

    Delay_Ms(1000);
    // Lepton_RunFFCCMD();
    // Delay_Ms(1000);
#endif
    // 开始读取Lepton的数据
    wTransferState = TRANSFER_COMPLETE;

    /* usbx 初始化 */
    //MX_USBX_Device_Init();


    // GC2145_PowerOff();

    win_mode = MODE_NORMAL;

    g_Display_Zoom_buff = &g_disp_buffer;

    while (1)
    {
        uint8_t key = KEY_NONE;

        key = KEY_GetKey();

        if (KEY_PWR_LONG_PRESS == key)
        {
            st7789_backlight_off();
            Battery_PowerOff();
            return 0;
        }
        if (MODE_SAVING == win_mode)
        {
            _saving_mode(key);
        }
        else if (MODE_ALBUM == win_mode)
        {
            // 当前处于相册模式，按相册手势和按键规则处理。
            _album_mode(key);
        }
        else
        {
            _normal_mode(key);
        }

        // 电池电量检测
        g_Battery_Level = Battery_Battery_ADC_Task();
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
    RCC_OscInitStruct.PLL.PLLM = 2;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 1;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_0;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
        | RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }

    if (160000000 != HAL_RCC_GetSysClockFreq())
    {
        while (1);
    }

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    uint8_t key = KEY_NONE;
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
   // __disable_irq();
    while (1)
    {
        key = KEY_GetKey();

        if (KEY_PWR_LONG_PRESS == key)
        {
            st7789_backlight_off();
            Battery_PowerOff();
            //return 0;
        }
        Delay_Ms(100);
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


// ITM_SendChar说明
//      这个函数位于 core_cm3.h（或cm4 cm7）中，直接调用即可。
//  一般都使用 port0 无需修改
int fputc(int ch, FILE *f)
{
    ITM_SendChar(ch);
    return ch;
}


// 将RTC时间转换为FatFs时间戳
#define BCD_TO_DEC(x) ((x >> 4) * 10 + (x & 0x0F))
DWORD rtc_to_fattime(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  DWORD y, m, d, h, mi, s;
  DWORD fattime;

  // 读取RTC（严格顺序：先时间后日期）
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    return 0; // RTC读取失败，返回默认时间
  if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    return 0;

  // BCD转十进制
  y = BCD_TO_DEC(sDate.Year);    // 如0x24 → 24（表示2024）
  m = BCD_TO_DEC(sDate.Month);   // 如0x06 → 6
  d = BCD_TO_DEC(sDate.Date);    // 如0x30 → 30
  h = BCD_TO_DEC(sTime.Hours);   // 如0x23 → 23
  mi = BCD_TO_DEC(sTime.Minutes);// 如0x59 → 59
  s = BCD_TO_DEC(sTime.Seconds); // 如0x50 → 50

  // 年份转换为1980年偏移（2024 → 44）
  y = 2000 + y - 1980;
  if (y > 127) y = 127; // 超出FatFs范围（最大2107年）

  // 组装FatFs时间戳（高16位日期，低16位时间）
  fattime = (y << 25) | (m << 21) | (d << 16) |
            (h << 11) | (mi << 5) | (s / 2);

  // 打印转换后的时间戳（调试用）
//  printf("转换后时间戳: 0x%08X\n", fattime);
  return fattime;
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime (void)
{

  return  rtc_to_fattime();
}
