/**
  ******************************************************************************
  * @file    bsp_gc2145.c
  * @author  fire
  * @version V1.0
  * @date    2025-03-15
  * @brief   GC2145ÉãÏñÍ·Çý¶¯
  ******************************************************************************
**/
  

/* Includes ------------------------------------------------------------------*/
#include "bsp_gc2145.h"
#include "bsp_i2c.h"
#include  "bsp_ov2640.h"
#include "gc2145_regs.h"
#include "gc2145_settings.h"


#define write_reg GC2145_WriteReg
#define read_reg  GC2145_ReadReg

//???????????
volatile uint8_t g_GC2145_state = 0;

uint8_t gc2145_detect(uint8_t slv_addr)
{
    uint8_t MIDL = GC2145_ReadReg(slv_addr, CHIP_ID_LOW);
    uint8_t MIDH = GC2145_ReadReg(slv_addr, CHIP_ID_HIGH);
    uint16_t PID = MIDH << 8 | MIDL;
    if (GC2145_PID == PID)
      return 0;

    return 1;
}


static int set_reg_bits(uint8_t slv_addr, uint16_t reg, uint8_t offset, uint8_t mask, uint8_t value)
{
    int ret = 0;
    uint8_t c_value, new_value;
    ret = read_reg(slv_addr, reg);
    if (ret < 0) {
        return ret;
    }
    c_value = ret;
    new_value = (c_value & ~(mask << offset)) | ((value & mask) << offset);
    ret = write_reg(slv_addr, reg, new_value);
    return ret;
}

static uint8_t GC2145_WriteRegs(uint8_t slv_addr, const uint16_t (*regs)[2])
{
    int i = 0, ret = 0;
    while (!ret && regs[i][0] != REGLIST_TAIL) {
        if (regs[i][0] == REG_DLY) {
          HAL_Delay(regs[i][1]);
        } else {
            ret = write_reg(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }
    return ret;
}

static uint8_t GC2145_Set_pixformat_RGB565(uint8_t slv_addr)
{
  uint8_t ret = 0;
  
  write_reg(slv_addr, 0xfe, 0x00);
  ret = set_reg_bits(slv_addr, P0_OUTPUT_FORMAT, 0, 0x1f, 6);  //RGB565
  
  return ret;
}

static uint8_t GC2145_Set_framesize(uint8_t slv_addr)
{
    uint8_t ret = 0;

    
    uint16_t w = 160;//resolution[framesize].width;
    uint16_t h = 120;//resolution[framesize].height;
    uint16_t row_s = 0;// = (resolution[FRAMESIZE_UXGA].height - h) / 2;
    uint16_t col_s = 0;// = (resolution[FRAMESIZE_UXGA].width - w) / 2;

#if CONFIG_GC_SENSOR_SUBSAMPLE_MODE
    struct subsample_cfg {
        uint16_t ratio_numerator;
        uint16_t ratio_denominator;
        uint8_t reg0x99;
        uint8_t reg0x9b;
        uint8_t reg0x9c;
        uint8_t reg0x9d;
        uint8_t reg0x9e;
        uint8_t reg0x9f;
        uint8_t reg0xa0;
        uint8_t reg0xa1;
        uint8_t reg0xa2;
    };
    const struct subsample_cfg subsample_cfgs[] = { // define some subsample ratio
        {42, 420, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //1/10
        {60, 420, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //1/7 // A smaller ratio brings a larger view, but it reduces the frame rate
        {84, 420, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //1/5
        {105, 420, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//1/4
        {140, 420, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//1/3
        {210, 420, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//1/2
        {240, 420, 0x77, 0x02, 0x46, 0x02, 0x46, 0x02, 0x46, 0x02, 0x46},//4/7
        {252, 420, 0x55, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04},//3/5
        {280, 420, 0x33, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},//2/3
        {420, 420, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},//1/1
    };
    uint16_t win_w = 1600;//resolution[FRAMESIZE_UXGA].width;
    uint16_t win_h = 1200;//resolution[FRAMESIZE_UXGA].height;
    const struct subsample_cfg *cfg = NULL;
    /**
     * Strategy: try to keep the maximum perspective
     */
    uint8_t i = 0;
    // if (framesize >= FRAMESIZE_QVGA) {
    //     i = 1;
    // }
    for (; i < sizeof(subsample_cfgs) / sizeof(struct subsample_cfg); i++) {
        cfg = &subsample_cfgs[i];
        if ((win_w * cfg->ratio_numerator / cfg->ratio_denominator >= w) && (win_h * cfg->ratio_numerator / cfg->ratio_denominator >= h)) {
            win_w = w * cfg->ratio_denominator / cfg->ratio_numerator;
            win_h = h * cfg->ratio_denominator / cfg->ratio_numerator;
            row_s = (1200 - win_h) / 2;
            col_s = (1600 - win_w) / 2;
            // ESP_LOGI(TAG, "subsample win:%dx%d,%dx%d, ratio:%f", win_w, win_h,row_s,col_s, (float)cfg->ratio_numerator / (float)cfg->ratio_denominator);
            break;
        }
    }

    write_reg(slv_addr, 0xfe, 0x00);
    write_reg(slv_addr, P0_CROP_ENABLE, 0x01);
    write_reg(slv_addr, 0x09, H8(row_s));
    write_reg(slv_addr, 0x0a, L8(row_s));
    write_reg(slv_addr, 0x0b, H8(col_s));
    write_reg(slv_addr, 0x0c, L8(col_s));
    write_reg(slv_addr, 0x0d, H8(win_h + 8));
    write_reg(slv_addr, 0x0e, L8(win_h + 8));
    write_reg(slv_addr, 0x0f, H8(win_w + 16));
    write_reg(slv_addr, 0x10, L8(win_w + 16));

    write_reg(slv_addr, 0x99, cfg->reg0x99);
    write_reg(slv_addr, 0x9b, cfg->reg0x9b);
    write_reg(slv_addr, 0x9c, cfg->reg0x9c);
    write_reg(slv_addr, 0x9d, cfg->reg0x9d);
    write_reg(slv_addr, 0x9e, cfg->reg0x9e);
    write_reg(slv_addr, 0x9f, cfg->reg0x9f);
    write_reg(slv_addr, 0xa0, cfg->reg0xa0);
    write_reg(slv_addr, 0xa1, cfg->reg0xa1);
    write_reg(slv_addr, 0xa2, cfg->reg0xa2);

    write_reg(slv_addr, 0x95, H8(h));
    write_reg(slv_addr, 0x96, L8(h));
    write_reg(slv_addr, 0x97, H8(w));
    write_reg(slv_addr, 0x98, L8(w));


#elif CONFIG_GC_SENSOR_WINDOWING_MODE
    write_reg(sensor->slv_addr, 0xfe, 0x00);

    write_reg(sensor->slv_addr, P0_CROP_ENABLE, 0x01);
    // write_reg(sensor->slv_addr, 0xec, col_s / 8); //measure window
    // write_reg(sensor->slv_addr, 0xed, row_s / 8);
    // write_reg(sensor->slv_addr, 0xee, (col_s + h) / 8);
    // write_reg(sensor->slv_addr, 0xef, (row_s + w) / 8);

    write_reg(sensor->slv_addr, 0x09, H8(row_s));
    write_reg(sensor->slv_addr, 0x0a, L8(row_s));
    write_reg(sensor->slv_addr, 0x0b, H8(col_s));
    write_reg(sensor->slv_addr, 0x0c, L8(col_s));
    write_reg(sensor->slv_addr, 0x0d, H8(h + 8));
    write_reg(sensor->slv_addr, 0x0e, L8(h + 8));
    write_reg(sensor->slv_addr, 0x0f, H8(w + 8));
    write_reg(sensor->slv_addr, 0x10, L8(w + 8));

    write_reg(sensor->slv_addr, 0x95, H8(h));
    write_reg(sensor->slv_addr, 0x96, L8(h));
    write_reg(sensor->slv_addr, 0x97, H8(w));
    write_reg(sensor->slv_addr, 0x98, L8(w));

#endif

    // if (ret == 0) {
    //     ESP_LOGD(TAG, "Set framesize to: %ux%u", w, h);
    // }

    return ret;

}

static uint8_t GC2145_Reset(uint8_t slv_addr)
{
  uint8_t ret = 0;
  // Software Reset: clear all registers and reset them to their default values
  ret = write_reg(slv_addr, RESET_RELATED, 0xe0);
  if (ret)
  {
      return ret;
  }
  HAL_Delay(100);
  ret = GC2145_WriteRegs(slv_addr, gc2145_default_init_regs);
  if (ret == 0)
  {
      // ESP_LOGD(TAG, "Camera defaults loaded");
      HAL_Delay(100);
      // vTaskDelay(100 / portTICK_PERIOD_MS);
#ifdef CONFIG_IDF_TARGET_ESP32
      write_reg(slv_addr, 0xfe, 0x00);
      //ensure pclk <= 15MHz for esp32
      set_reg_bits(slv_addr, 0xf8, 0, 0x3f, 2); // divx4
      set_reg_bits(slv_addr, 0xfa, 4, 0x0f, 2); // divide_by
#endif

  }
  return ret;
}


static const uint8_t IMAGE_HEIGHT = 160;
static const uint8_t IMAGE_WIDTH = 120;
 
uint8_t GC2145_Init(void)
{
  OV2640_HW_Init();
  I2CMaster_Init();
  if(gc2145_detect(GC2145_DEVICE_ADDRESS))
  {
    return 1;
    //Error_Handler();
  }

  if (GC2145_Reset(GC2145_DEVICE_ADDRESS))
  {
      return 1;
    //Error_Handler();
  }

  if (GC2145_Set_framesize(GC2145_DEVICE_ADDRESS))
  {
      return 1;
    //Error_Handler();
  }


  if (GC2145_Set_pixformat_RGB565(GC2145_DEVICE_ADDRESS))
  {
      return 1;
    //Error_Handler();
  }

  OV2640_DCMI_Init(); 

  OV2640_DMA_Config((uint32_t)&g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT / 2);
  // OV2640_OutSize_Set(IMAGE_WIDTH, IMAGE_HEIGHT);
  
  HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, (uint32_t)&g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT / 2);
  
  return 0;

}

void GC2145_PowerOff(void)
{

  HAL_DCMI_Stop(&DCMI_Handle);
  __HAL_DCMI_DISABLE_IT(&DCMI_Handle, DCMI_IT_FRAME); 
  HAL_DCMI_DeInit(&DCMI_Handle);  

  HAL_GPIO_WritePin(DCMI_PWDN_GPIO_PORT, DCMI_PWDN_GPIO_PIN, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(DCMI_RST_GPIO_PORT, DCMI_RST_GPIO_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(DCMI_PWDN_GPIO_PORT, DCMI_PWDN_GPIO_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
}



void GC2145_Test(void)
{
  GC2145_Init();

  #if 0
    while(1)
    {
      if (display_flag)
      {
        for (uint16_t i = 0; i < 320; i++)
        {  
          for (uint16_t j = 0; j < 240; j++)
          {
            uint16_t new_i,new_j;
            new_i = (uint16_t)(i / 2.0f);
            new_j = (uint16_t)(j / 2.0f);
            g_Display_Zoom_buff[i][j] = g_Display_buff[ new_j * 160 + new_i];
          }
        }
        
        // st7789_draw_image(0, 0, 120, 160, (const uint16_t *)&g_Display_buff[0]);
        st7789_draw_image_dma(0, 0, 240, 280, (const uint16_t *)&g_Display_Zoom_buff[20]);
        display_flag = 0;
      }
      // HAL_Delay(10);
    }
#endif


}


