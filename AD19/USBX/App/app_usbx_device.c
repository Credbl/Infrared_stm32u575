/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx_device.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "bsp.h"
#include "app_usbx_device.h"
#include "ux_api.h"
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_device_stack.h"
#include "ux_dcd_stm32.h"
#include "ux_device_descriptors.h"
#include "ux_device_class_storage.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define USBX_DEVICE_MEMORY_STACK_SIZE       (1024 * 10)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN UX_Device_Memory_Buffer */

/* USER CODE END UX_Device_Memory_Buffer */
#if defined ( __ICCARM__ )
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN static UCHAR ux_device_byte_pool_buffer[UX_DEVICE_APP_MEM_POOL_SIZE] __ALIGN_END;

static ULONG storage_interface_number;
static ULONG storage_configuration_number;
static UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static UINT USBD_ChangeFunction(ULONG Device_State);
/* USER CODE BEGIN PFP */

PCD_HandleTypeDef hpcd_USB_OTG_FS;

static void MX_USB_OTG_FS_PCD_Init(void)
{
    hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
    //hpcd_USB_OTG_FS.Init.ep0_mps = 0x40;
    hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.battery_charging_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
    hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
    {
        Error_Handler();
    }
#if 1

//     HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
//     HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, USBD_MAX_EP0_SIZE / 4);
//     HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, USBD_MSC_EPIN_FS_MPS / 2);
//     HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, USBD_MSC_EPOUT_FS_MPS / 2);

    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);

    if (ux_dcd_stm32_initialize(
            (ULONG)USB_OTG_FS,
            (ULONG)&hpcd_USB_OTG_FS) != UX_SUCCESS)
    {
        Error_Handler();
    }

    HAL_PCD_Start(&hpcd_USB_OTG_FS);
#endif
}


void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (hpcd->Instance == USB_OTG_FS)
    {
#if 0
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
        PeriphClkInit.IclkClockSelection = RCC_CLK48CLKSOURCE_PLL2;
        PeriphClkInit.PLL2.PLL2Source = RCC_PLLSOURCE_HSE;
        PeriphClkInit.PLL2.PLL2M = 2;
        PeriphClkInit.PLL2.PLL2N = 96;
        PeriphClkInit.PLL2.PLL2P = 2;
        PeriphClkInit.PLL2.PLL2Q = 8;
        PeriphClkInit.PLL2.PLL2R = 2;
        PeriphClkInit.PLL2.PLL2RGE = RCC_PLLVCIRANGE_0;
        PeriphClkInit.PLL2.PLL2FRACN = 0.0;
        PeriphClkInit.PLL2.PLL2ClockOut = RCC_PLL2_DIVQ;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
#else
        /* 使用 HIS48 之前，必须确保时钟配置时已经打开了 HIS48 时钟 */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
        PeriphClkInit.IclkClockSelection = RCC_CLK48CLKSOURCE_HSI48;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }
#endif

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /*
            USB_OTG_FS GPIO Configuration
            PA12     ------> USB_OTG_FS_DP
            PA11     ------> USB_OTG_FS_DM
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_USB;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        __HAL_RCC_USB_CLK_ENABLE();

        if (__HAL_RCC_PWR_IS_CLK_DISABLED())
        {
            __HAL_RCC_PWR_CLK_ENABLE();
            HAL_PWREx_EnableVddUSB();
            __HAL_RCC_PWR_CLK_DISABLE();
        }
        else
        {
            HAL_PWREx_EnableVddUSB();
        }
        HAL_NVIC_SetPriority(OTG_FS_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
    if (hpcd->Instance == USB_OTG_FS)
    {
        /* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

        /* USER CODE END USB_OTG_FS_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USB_CLK_DISABLE();

        /**USB_OTG_FS GPIO Configuration
        PA12     ------> USB_OTG_FS_DP
        PA11     ------> USB_OTG_FS_DM
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_11);

        /* USB_OTG_FS interrupt DeInit */
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
        /* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

        /* USER CODE END USB_OTG_FS_MspDeInit 1 */
    }

}


void OTG_FS_IRQHandler(void)
{
    /* USER CODE BEGIN OTG_FS_IRQn 0 */

    /* USER CODE END OTG_FS_IRQn 0 */
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    /* USER CODE BEGIN OTG_FS_IRQn 1 */

    /* USER CODE END OTG_FS_IRQn 1 */
}


/* USER CODE END PFP */

/**
  * @brief  Application USBX Device Initialization.
  * @param  none
  * @retval status
  */
UINT MX_USBX_Device_Init(VOID)
{
    UINT ret = UX_SUCCESS;
    UCHAR *device_framework_high_speed;
    UCHAR *device_framework_full_speed;
    ULONG device_framework_hs_length;
    ULONG device_framework_fs_length;
    ULONG string_framework_length;
    ULONG language_id_framework_length;
    UCHAR *string_framework;
    UCHAR *language_id_framework;

    UCHAR *pointer;

    /* USER CODE BEGIN MX_USBX_Device_Init0 */

    /* USER CODE END MX_USBX_Device_Init0 */
    pointer = ux_device_byte_pool_buffer;

    /* Initialize USBX Memory */
    if (ux_system_initialize(
            pointer,
            USBX_DEVICE_MEMORY_STACK_SIZE,
            UX_NULL, 0) != UX_SUCCESS)
    {
        /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
        return UX_ERROR;
        /* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
    }

    /* Get Device Framework High Speed and get the length */
    device_framework_high_speed = USBD_Get_Device_Framework_Speed(
            USBD_HIGH_SPEED,
            &device_framework_hs_length);

    /* Get Device Framework Full Speed and get the length */
    device_framework_full_speed = USBD_Get_Device_Framework_Speed(
            USBD_FULL_SPEED,
            &device_framework_fs_length);

    /* Get String Framework and get the length */
    string_framework = USBD_Get_String_Framework(&string_framework_length);

    /* Get Language Id Framework and get the length */
    language_id_framework = USBD_Get_Language_Id_Framework(&language_id_framework_length);

    /* Install the device portion of USBX */
    if (ux_device_stack_initialize(NULL,//device_framework_high_speed,
            0,//device_framework_hs_length,
            device_framework_full_speed,
            device_framework_fs_length,
            string_framework,
            string_framework_length,
            language_id_framework,
            language_id_framework_length,
            USBD_ChangeFunction) != UX_SUCCESS)
    {
        /* USER CODE BEGIN USBX_DEVICE_INITIALIZE_ERROR */
        return UX_ERROR;
        /* USER CODE END USBX_DEVICE_INITIALIZE_ERROR */
    }

    /* Initialize the storage class parameters for the device */
    storage_parameter.ux_slave_class_storage_instance_activate   = USBD_STORAGE_Activate;
    storage_parameter.ux_slave_class_storage_instance_deactivate = USBD_STORAGE_Deactivate;

    /* Store the number of LUN in this device storage instance */
    storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;//STORAGE_NUMBER_LUN;

    /* Initialize the storage class parameters for reading/writing to the Flash Disk */
    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_last_lba = USBD_STORAGE_GetMediaLastLba();

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_block_length = USBD_STORAGE_GetMediaBlocklength();

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_type = 0;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_removable_flag = 0x80;//STORAGE_REMOVABLE_FLAG;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_read_only_flag = STORAGE_READ_ONLY;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_read = USBD_STORAGE_Read;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_write = USBD_STORAGE_Write;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_flush = USBD_STORAGE_Flush;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_status = USBD_STORAGE_Status;

    storage_parameter.ux_slave_class_storage_parameter_lun[0].
    ux_slave_class_storage_media_notification = USBD_STORAGE_Notification;

    /* USER CODE BEGIN STORAGE_PARAMETER */

    /* USER CODE END STORAGE_PARAMETER */

    /* Get storage configuration number */
    storage_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_MSC, 0);

    /* Find storage interface number */
    storage_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_MSC, 0);

    /* Initialize the device storage class */
    if (ux_device_stack_class_register(_ux_system_slave_class_storage_name,
            ux_device_class_storage_entry,
            1,//storage_configuration_number,
            0,//storage_interface_number,
            &storage_parameter) != UX_SUCCESS)
    {
        /* USER CODE BEGIN USBX_DEVICE_STORAGE_REGISTER_ERROR */
        return UX_ERROR;
        /* USER CODE END USBX_DEVICE_STORAGE_REGISTER_ERROR */
    }

    /* USER CODE BEGIN MX_USBX_Device_Init1 */

    MX_USB_OTG_FS_PCD_Init();

    /* USER CODE END MX_USBX_Device_Init1 */

    return ret;
}



ALIGN_TYPE _ux_utility_interrupt_disable(VOID)
{
    UINT interrupt_save;
    /* USER CODE BEGIN _ux_utility_interrupt_disable */
    interrupt_save = __get_PRIMASK();
    __disable_irq();
    /* USER CODE END _ux_utility_interrupt_disable */

    return interrupt_save;
}

VOID _ux_utility_interrupt_restore(ALIGN_TYPE flags)
{
    /* USER CODE BEGIN _ux_utility_interrupt_restore */
    __set_PRIMASK(flags);
    /* USER CODE END _ux_utility_interrupt_restore */
}

/**
  * @brief  _ux_utility_time_get
  *         Get Time Tick for host timing.
  * @param  none
  * @retval time tick
  */
ULONG _ux_utility_time_get(VOID)
{
    ULONG time_tick = 0U;

    /* USER CODE BEGIN _ux_utility_time_get */
    time_tick = HAL_GetTick();

    /* USER CODE END _ux_utility_time_get */

    return time_tick;
}

/**
  * @brief  USBD_ChangeFunction
  *         This function is called when the device state changes.
  * @param  Device_State: USB Device State
  * @retval status
  */
static UINT USBD_ChangeFunction(ULONG Device_State)
{
    UINT status = UX_SUCCESS;

    /* USER CODE BEGIN USBD_ChangeFunction0 */

    /* USER CODE END USBD_ChangeFunction0 */

    switch (Device_State)
    {
        case UX_DEVICE_ATTACHED:

            /* USER CODE BEGIN UX_DEVICE_ATTACHED */

            /* USER CODE END UX_DEVICE_ATTACHED */

            break;

        case UX_DEVICE_REMOVED:

            /* USER CODE BEGIN UX_DEVICE_REMOVED */

            /* USER CODE END UX_DEVICE_REMOVED */

            break;

        case UX_DCD_STM32_DEVICE_CONNECTED:

            /* USER CODE BEGIN UX_DCD_STM32_DEVICE_CONNECTED */

            /* USER CODE END UX_DCD_STM32_DEVICE_CONNECTED */

            break;

        case UX_DCD_STM32_DEVICE_DISCONNECTED:

            /* USER CODE BEGIN UX_DCD_STM32_DEVICE_DISCONNECTED */

            /* USER CODE END UX_DCD_STM32_DEVICE_DISCONNECTED */

            break;

        case UX_DCD_STM32_DEVICE_SUSPENDED:

            /* USER CODE BEGIN UX_DCD_STM32_DEVICE_SUSPENDED */

            /* USER CODE END UX_DCD_STM32_DEVICE_SUSPENDED */

            break;

        case UX_DCD_STM32_DEVICE_RESUMED:

            /* USER CODE BEGIN UX_DCD_STM32_DEVICE_RESUMED */

            /* USER CODE END UX_DCD_STM32_DEVICE_RESUMED */

            break;

        case UX_DCD_STM32_SOF_RECEIVED:

            /* USER CODE BEGIN UX_DCD_STM32_SOF_RECEIVED */

            /* USER CODE END UX_DCD_STM32_SOF_RECEIVED */

            break;

        default:

            /* USER CODE BEGIN DEFAULT */

            /* USER CODE END DEFAULT */

            break;

    }

    /* USER CODE BEGIN USBD_ChangeFunction1 */

    /* USER CODE END USBD_ChangeFunction1 */

    return status;
}
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
