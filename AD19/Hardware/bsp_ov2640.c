/**
 ******************************************************************************
 * @file    bsp_ov3640.c
 * @author  fire
 * @version V1.0
 * @date    2025-01-02
 * @brief   OV2640摄像头驱动
 ******************************************************************************
 **/

/* Includes ------------------------------------------------------------------*/
#include "bsp_ov2640.h"
#include "bsp_i2c.h"
#include "battery.h"

/** @addtogroup DCMI_Camera
 * @{
 */
DCMI_HandleTypeDef DCMI_Handle;

DMA_NodeTypeDef Node_GPDMA1_Channel0;
DMA_QListTypeDef List_GPDMA1_Channel0;
DMA_HandleTypeDef handle_GPDMA1_Channel0;
// DMA_HandleTypeDef DMA_Handle_dcmi;
DMA_HandleTypeDef handle_GPDMA1_Channel9; // RX
const uint8_t IMAGE_HEIGHT = 160;
const uint8_t IMAGE_WIDTH = 120;
// uint16_t g_Video_buff[160 * 120] = {0};
// 属性直接加在数组上
uint16_t g_Video_buff[160 * 120] __attribute__((aligned(4))) = {0};
uint16_t g_Video_save_buff[160 * 120] = {0};
uint16_t g_Display_buff[160 * 120] = {0};
uint8_t g_Display_Gray_buff[160 * 120] = {0};
uint8_t g_Display_OutLine_buff[160 * 120] = {0};
// uint16_t g_Display_Zoom_buff[320][240] = {0};

uint16_t (*g_Display_Zoom_buff)[320][240];

volatile uint32_t fps = 0;

const uint8_t OV2640_CONFIG_QQVGA[][2] =
{
    // ???????0
    {0xff, 0x00},
    // ??????
    {0x2c, 0xff},
    // ??PLL?????????ADCCLK
    {0x2e, 0xdf},

    // ???????1
    {0xff, 0x01},
    // ??????0?????????????
    {0x3c, 0x32},
    // ???????
    {0x11, 0x00},
    // ??????0???
    {0x09, 0x02},
    // ??????3???
    {0x03, 0xcf},
    // ??????4???
    {0x04, 0x08},
    // ??????/????
    {0x13, 0xe5},
    // ????????
    {0x14, 0x48},
    // ??VSYNC????
    {0x2c, 0x0c},
    // ??ABLC??
    {0x33, 0x78},
    // ??????0???
    {0x3a, 0x33},
    // ??????1???
    {0x3b, 0xfb},
    // ??????2???
    {0x3e, 0x00},
    // ??PCLK???
    {0x43, 0x11},
    // ???????????
    {0x16, 0x10},
    // ??AEC/AGC - ???????
    {0x39, 0x02},
    // ??AEC/AGC - ???????
    {0x35, 0x88},
    // ??PCLK??
    {0x22, 0x0a},
    // ??BLC??
    {0x37, 0x40},
    // ??AEC/AGC - ??????
    {0x23, 0x00},
    // ??ABLC????
    {0x34, 0xa0},
    // ??ABLC??
    {0x36, 0x1a},
    // ????????
    {0x06, 0x02},
    // ????????
    {0x07, 0xc0},
    // ??????????8?
    {0x0d, 0xb7},
    // ??????????8?
    {0x0e, 0x01},
    // ??PCLK??????
    {0x4c, 0x00},
    // ??50/60Hz??????
    {0x4a, 0x81},
    // ??AEC/AGC????
    {0x21, 0x99},
    // ??AEC/AGC - ??????
    {0x24, 0x3a},
    // ??AEC/AGC - ??????
    {0x25, 0x32},
    // ??AGC/AEC - ??????
    {0x26, 0x82},
    // ??VFIFO??0
    {0x5c, 0x00},
    // ??AWB??3
    {0x63, 0x00},
    // ??AWB??????
    {0x5d, 0x55},
    // ??AWB??????
    {0x5e, 0x7d},
    // ??AWB??????
    {0x5f, 0x7d},
    // ??AWB??????
    {0x60, 0x55},
    // ??AWB??????
    {0x61, 0x70},
    // ??AWB??????
    {0x62, 0x80},
    // ??AWB??1
    {0x7c, 0x05},
    // ??AEC/AGC - ??????
    {0x20, 0x80},
    // ??AEC/AGC - ??????
    {0x28, 0x30},
    // ??AWB??0
    {0x6c, 0x00},
    // ??AWB??1
    {0x6d, 0x80},
    // ??AWB??2
    {0x6e, 0x00},
    // ??HSYNC??
    {0x70, 0x02},
    // ??HSYNC??
    {0x71, 0x96},
    // ??VSYNC??
    {0x73, 0xe1},
    // ??????0???
    {0x3d, 0x34},
    // ??VFIFO??????
    {0x5a, 0x57},
    // ??DSP?????
    {0x4f, 0xbb},
    // ??DSP????
    {0x50, 0x9c},
    // ??????0???
    {0x0f, 0x43},

    // ???????0
    {0xff, 0x00},
    // ??FIFO??
    {0xe5, 0x7f},
    // ??????
    {0xf9, 0xc0},
    // ????????
    {0x41, 0x24},
    // ??JPEG??1
    {0xe0, 0x14},
    // ??DBLC??
    {0x76, 0xff},
    // ??ISP??2
    {0x33, 0xa0},
    // ??????
    {0x42, 0x20},
    // ??JPEG??1
    {0x43, 0x18},
    // ??PCLK???
    {0x4c, 0x00},
    // ??PCLK??
    {0x87, 0xd0},
    // ??PCLK??????
    {0x88, 0x3f},
    // ??FIFO??0
    {0xd7, 0x03},
    // ??FIFO??1
    {0xd9, 0x10},
    // ??DVP??
    {0xd3, 0x82},
    // ??????0
    {0xc8, 0x08},
    // ??????1
    {0xc9, 0x80},
    // ??BIST??
    {0x7c, 0x00},
    // ??BIST??
    {0x7d, 0x02},
    // ??BIST???
    {0x7c, 0x03},
    // ??BIST???
    {0x7d, 0x48},
    // ??BIST???
    {0x7d, 0x48},
    // ??????
    {0x7c, 0x08},
    {0x7d, 0x20},
    {0x7d, 0x10},
    {0x7d, 0x0e},
    // ????????
    {0x90, 0x00},
    // ????????
    {0x91, 0x0e},
    {0x91, 0x1a},
    {0x91, 0x31},
    {0x91, 0x5a},
    {0x91, 0x69},
    {0x91, 0x75},
    {0x91, 0x7e},
    {0x91, 0x88},
    {0x91, 0x8f},
    {0x91, 0x96},
    {0x91, 0xa3},
    {0x91, 0xaf},
    {0x91, 0xc4},
    {0x91, 0xd7},
    {0x91, 0xe8},
    {0x91, 0x20},
    // ??DPC??
    {0x92, 0x00},
    // ??DPC??
    {0x93, 0x06},
    {0x93, 0xe3},
    {0x93, 0x05},
    {0x93, 0x05},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    {0x93, 0x00},
    // ??LENC??
    {0x96, 0x00},
    // ??LENC??
    {0x97, 0x08},
    {0x97, 0x19},
    {0x97, 0x02},
    {0x97, 0x0c},
    {0x97, 0x24},
    {0x97, 0x30},
    {0x97, 0x28},
    {0x97, 0x26},
    {0x97, 0x02},
    {0x97, 0x98},
    {0x97, 0x80},
    {0x97, 0x00},
    {0x97, 0x00},
    // ??BDBase
    {0xc3, 0xed},
    // ??BD??
    {0xa4, 0x00},
    {0xa8, 0x00},
    // ??BD??
    {0xbf, 0x00},
    {0xba, 0xf0},
    // ??BD??
    {0xbc, 0x64},
    {0xbb, 0x02},
    // ??BD??
    {0xb6, 0x3d},
    {0xb8, 0x57},
    {0xb7, 0x38},
    {0xb9, 0x4e},
    // ??BD??
    {0xb3, 0xe8},
    {0xb4, 0xe1},
    {0xb5, 0x66},
    // ??AWB??
    {0xb0, 0x67},
    {0xb1, 0x5e},
    {0xb2, 0x04},
    // ??AWB??
    {0xc7, 0x00},
    {0xc6, 0x51},
    {0xc5, 0x11},
    {0xc4, 0x9c},
    // ??CC??
    {0xcf, 0x02},
    // ??CC??
    {0xa6, 0x00},
    {0xa7, 0xe0},
    {0xa7, 0x10},
    {0xa7, 0x1e},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    {0xa7, 0xd0},
    {0xa7, 0x10},
    {0xa7, 0x16},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    {0xa7, 0xd0},
    {0xa7, 0x10},
    {0xa7, 0x17},
    {0xa7, 0x21},
    {0xa7, 0x00},
    {0xa7, 0x28},
    // ????????
    {0xc0, 0xc8},
    // ????????
    {0xc1, 0x96},
    // ??PCLK??
    {0x86, 0x1d},
    // ??????
    {0x50, 0x00},
    // ?????? - UYVY
    {0x51, 0x90},
    // ????????
    {0x52, 0x18},
    // ??FIFO????
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    // ??VFIFO??0
    {0x57, 0x00},
    // ??VFIFO??????
    {0x5a, 0x90},
    // ??VFIFO??????
    {0x5b, 0x18},
    // ??VFIFO??0
    {0x5c, 0x05},
    // ??BDBase
    {0xc3, 0xef},
    // ??SDE??
    {0x7f, 0x00},
    // ??SDE??1
    {0xda, 0x00},
    // ??FIFO??
    {0xe5, 0x1f},
    // ??FIFO??
    {0xe1, 0x67},
    // ??JPEG??1
    {0xe0, 0x00},
    // ??DVP??{0xdd, 0xff},
    // ????????
    {0x05, 0x00},

    // ???????1
    {0xff, 0x01},
    // ?????????1?????????????
    {0xff, 0x01},
    // ???????
    {0x12, 0x00},
    // ????????
    {0x17, 0x11},
    // ????????
    {0x18, 0x75},
    // ?????????8?
    {0x19, 0x01},
    // ?????????8?
    {0x1a, 0x97},
    // ??VSYNC??
    {0x32, 0x36},
    // ??DSP?????
    {0x4f, 0xbb},
    // ??AWB??1
    {0x6d, 0x80},
    // ??????0???
    {0x3d, 0x34},
    // ??AEC/AGC - ???????
    {0x39, 0x02},
    // ??AEC/AGC - ???????
    {0x35, 0x88},
    // ??PCLK??
    {0x22, 0x0a},
    // ??BLC??
    {0x37, 0x40},
    // ??AEC/AGC - ??????
    {0x23, 0x00},
    // ??ABLC????
    {0x34, 0xa0},
    // ??ABLC??
    {0x36, 0x1a},
    // ????????
    {0x06, 0x02},
    // ????????
    {0x07, 0xc0},
    // ??????????8?
    {0x0d, 0xb7},
    // ??????????8?
    {0x0e, 0x01},
    // ??PCLK??????
    {0x4c, 0x00},

    // ???????0
    {0xff, 0x00},
    // ??JPEG??1
    {0xe0, 0x04},
    // ??QS??
    {0x8c, 0x00},
    // ??PCLK??
    {0x87, 0xd0},
    // ??JPEG??1
    {0xe0, 0x00},

    // ???????0
    {0xff, 0x00},
    // ??JPEG??1
    {0xe0, 0x14},
    // ??FIFO??
    {0xe1, 0x77},
    // ??FIFO??
    {0xe5, 0x1f},
    // ??FIFO??0
    {0xd7, 0x03},
    // ??FIFO??1
    {0xda, 0x10},
    // ??JPEG??1
    {0xe0, 0x00},

    // ???????0
    {0xff, 0x00},
    // ??JPEG??1
    {0xe0, 0x04},
    // ????????
    {0xc0, 0xc8},
    // ????????
    {0xc1, 0x96},
    // ??PCLK??
    {0x86, 0x1d},
    // ??????
    {0x50, 0x00},
    // ?????? - UYVY
    {0x51, 0x90},
    // ????????
    {0x52, 0x2c},
    // ??FIFO????
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x88},
    // ??VFIFO??0
    {0x57, 0x00},
    // ??VFIFO??????
    {0x5a, 0x90},
    // ??VFIFO??????
    {0x5b, 0x2c},
    // ??VFIFO??0
    {0x5c, 0x05},
    // ??JPEG??1
    {0xe0, 0x00},
    // ??DVP??
    {0xd3, 0x04},

    // ???????0
    {0xff, 0x00},
    // ??BDBase
    {0xc3, 0xef},
    // ??CC??
    {0xa6, 0x00},
    // ??CC??
    {0xa7, 0xdd},
    {0xa7, 0x78},
    {0xa7, 0x7e},
    {0xa7, 0x24},
    {0xa7, 0x00},
    {0xa7, 0x25},
    {0xa6, 0x06},
    {0xa7, 0x20},
    {0xa7, 0x58},
    {0xa7, 0x73},
    {0xa7, 0x34},
    {0xa7, 0x00},
    {0xa7, 0x25},
    {0xa6, 0x0c},
    {0xa7, 0x28},
    {0xa7, 0x58},
    {0xa7, 0x6d},
    {0xa7, 0x34},
    {0xa7, 0x00},
    {0xa7, 0x25},

    // ???????0
    {0xff, 0x00},
    // ??JPEG??1
    {0xe0, 0x04},
    // ??FIFO??
    {0xe1, 0x67},
    // ??FIFO??
    {0xe5, 0x1f},
    // ??FIFO??0
    {0xd7, 0x01},
    // ??FIFO??1
    {0xda, 0x08},
    // ??FIFO??1
    //{0xda, 0x09},
    // ??JPEG??1
    {0xe0, 0x00},
    // ????????
    {0x98, 0x00},
    // ????????
    {0x99, 0x00},

    // ???????1
    {0xff, 0x01},
    // ??????4???
    {0x04, 0x28},

    // ???????1
    {0xff, 0x01},
    // ??????0???
    {0x12, 0x40},
    // ????????
    {0x17, 0x11},
    // ????????
    {0x18, 0x43},
    // ?????????8?
    {0x19, 0x00},
    // ?????????8?
    {0x1a, 0x4b},
    // ??VSYNC??
    {0x32, 0x09},
    // ??DSP?????
    {0x4f, 0xca},
    // ??DSP????
    {0x50, 0xa8},
    // ??VFIFO??????
    {0x5a, 0x23},
    // ??AWB??1
    {0x6d, 0x00},
    // ??AEC/AGC - ???????
    {0x39, 0x12},
    // ??AEC/AGC - ???????
    {0x35, 0xda},
    // ??PCLK??
    {0x22, 0x1a},
    // ??BLC??
    {0x37, 0xc3},
    // ??AEC/AGC - ??????
    {0x23, 0x00},
    // ??ABLC????
    {0x34, 0xc0},
    // ??ABLC??
    {0x36, 0x1a},
    // ????????
    {0x06, 0x88},
    // ????????
    {0x07, 0xc0},
    // ??????????8?
    {0x0d, 0x87},
    // ??????????8?
    {0x0e, 0x41},
    // ??PCLK??????
    {0x4c, 0x00},

    // ???????0
    {0xff, 0x00},
    // ??JPEG??1
    {0xe0, 0x04},
    // ????????
    {0xc0, 0x64},
    // ????????
    {0xc1, 0x4b},
    // ??PCLK??
    {0x86, 0x35},
    // ??????
    {0x50, 0x92},
    // ?????? - RGB565
    {0x51, 0xc8},
    // ????????
    {0x52, 0x96},
    // ??FIFO????
    {0x53, 0x00},
    {0x54, 0x00},
    {0x55, 0x00},
    // ??VFIFO??0
    {0x57, 0x00},
    // ??VFIFO??????
    {0x5a, 0x28},
    // ??VFIFO??????
    {0x5b, 0x1e},
    // ??VFIFO??0
    {0x5c, 0x00},
    // ??JPEG??1
    {0xe0, 0x00},

    // ???????1
    {0xff, 0x01},
    // ???????
    {0x11, 0x00},
    // ??????0???
    {0x3d, 0x38},
    // ??PCLK???
    {0x2d, 0x00},
    // ??DSP????
    {0x50, 0x65},

    // ???????0
    {0xff, 0x00},
    // ??DVP??
    {0xd3, 0x04},
    // ??BIST??
    {0x7c, 0x00},
    // ??BIST??
    {0x7d, 0x04},
    // ??BIST???
    {0x7c, 0x09},
    // ??BIST???
    {0x7d, 0x28},
    // ??BIST???
    {0x7d, 0x00},

    //     // ??????
    //     0xff, 0x01,
    //     0x12, 0x02,
};

// OV2640 UXGA初始化寄存器序列表
// 此模式下帧率为15帧
// UXGA(1600*1200)
const u8 ov2640_uxga_init_reg_tbl[][2] =
{
    0xff,
    0x00,
    0x2c,
    0xff,
    0x2e,
    0xdf,
    0xff,
    0x01,
    0x3c,
    0x32,
    //
    0x11,
    0x00,
    0x09,
    0x02,
    // 0x04, 0xD8,//水平镜像,垂直翻转
    0x04,
    0x88, // 水平镜像,垂直翻转
    0x13,
    0xe5,
    0x14,
    0x48,
    0x2c,
    0x0c,
    0x33,
    0x78,
    0x3a,
    0x33,
    0x3b,
    0xfB,
    //
    0x3e,
    0x00,
    0x43,
    0x11,
    0x16,
    0x10,
    //
    0x39,
    0x92,
    //
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,

    0x48,
    0x00,
    0x5B,
    0x00,
    0x42,
    0x03,
    //
    0x4a,
    0x81,
    0x21,
    0x99,
    //
    0x24,
    0x40,
    0x25,
    0x38,
    0x26,
    0x82,
    0x5c,
    0x00,
    0x63,
    0x00,
    0x46,
    0x00,
    0x0c,
    0x3c,
    //
    0x61,
    0x70,
    0x62,
    0x80,
    0x7c,
    0x05,
    //
    0x20,
    0x80,
    0x28,
    0x30,
    0x6c,
    0x00,
    0x6d,
    0x80,
    0x6e,
    0x00,
    0x70,
    0x02,
    0x71,
    0x94,
    0x73,
    0xc1,
    0x3d,
    0x34,
    0x5a,
    0x57,
    //
    0x12,
    0x00, // UXGA 1600*1200

    0x17,
    0x11,
    0x18,
    0x75,
    0x19,
    0x01,
    0x1a,
    0x97,
    0x32,
    0x36,
    0x03,
    0x0f,
    0x37,
    0x40,
    //
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x6d,
    0x38,
    //
    0xff,
    0x00,
    0xe5,
    0x7f,
    0xf9,
    0xc0,
    0x41,
    0x24,
    0xe0,
    0x14,
    0x76,
    0xff,
    0x33,
    0xa0,
    0x42,
    0x20,
    0x43,
    0x18,
    0x4c,
    0x00,
    0x87,
    0xd5,
    0x88,
    0x3f,
    0xd7,
    0x03,
    0xd9,
    0x10,
    0xd3,
    0x82,
    //
    0xc8,
    0x08,
    0xc9,
    0x80,
    //
    0x7c,
    0x00,
    0x7d,
    0x00,
    0x7c,
    0x03,
    0x7d,
    0x48,
    0x7d,
    0x48,
    0x7c,
    0x08,
    0x7d,
    0x20,
    0x7d,
    0x10,
    0x7d,
    0x0e,
    //
    0x90,
    0x00,
    0x91,
    0x0e,
    0x91,
    0x1a,
    0x91,
    0x31,
    0x91,
    0x5a,
    0x91,
    0x69,
    0x91,
    0x75,
    0x91,
    0x7e,
    0x91,
    0x88,
    0x91,
    0x8f,
    0x91,
    0x96,
    0x91,
    0xa3,
    0x91,
    0xaf,
    0x91,
    0xc4,
    0x91,
    0xd7,
    0x91,
    0xe8,
    0x91,
    0x20,
    //
    0x92,
    0x00,
    0x93,
    0x06,
    0x93,
    0xe3,
    0x93,
    0x05,
    0x93,
    0x05,
    0x93,
    0x00,
    0x93,
    0x04,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    //
    0x96,
    0x00,
    0x97,
    0x08,
    0x97,
    0x19,
    0x97,
    0x02,
    0x97,
    0x0c,
    0x97,
    0x24,
    0x97,
    0x30,
    0x97,
    0x28,
    0x97,
    0x26,
    0x97,
    0x02,
    0x97,
    0x98,
    0x97,
    0x80,
    0x97,
    0x00,
    0x97,
    0x00,
    //
    0xc3,
    0xef,

    0xa4,
    0x00,
    0xa8,
    0x00,
    0xc5,
    0x11,
    0xc6,
    0x51,
    0xbf,
    0x80,
    0xc7,
    0x10,
    0xb6,
    0x66,
    0xb8,
    0xA5,
    0xb7,
    0x64,
    0xb9,
    0x7C,
    0xb3,
    0xaf,
    0xb4,
    0x97,
    0xb5,
    0xFF,
    0xb0,
    0xC5,
    0xb1,
    0x94,
    0xb2,
    0x0f,
    0xc4,
    0x5c,
    //
    0xc0,
    0xc8,
    0xc1,
    0x96,
    0x8c,
    0x00,
    0x86,
    0x3d,
    0x50,
    0x92,
    // 0x50, 0x00,
    0x51,
    0x90,
    0x52,
    0x2c,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x88,

    0x5a,
    0x90,
    0x5b,
    0x2C,
    0x5c,
    0x05,

    0xd3,
    0x03, // auto设置要小心
    //
    0xc3,
    0xed,
    0x7f,
    0x00,

    0xda,
    0x09,

    0xe5,
    0x1f,
    0xe1,
    0x67,
    0xe0,
    0x00,
    0xdd,
    0x7f,
    0x05,
    0x00,
};

const u8 ov2640_rgb565_reg_tbl[][2] =
{
    0xFF,
    0x00,
    0xDA,
    0x09,
    0xD7,
    0x03,
    0xDF,
    0x02,
    0x33,
    0xa0,
    0x3C,
    0x00,
    0xe1,
    0x67,

    0xff,
    0x01,
    0xe0,
    0x00,
    0xe1,
    0x00,
    0xe5,
    0x00,
    0xd7,
    0x00,
    0xda,
    0x00,
    0xe0,
    0x00,
};

/* QQVGA 160x120 */
const char OV2640_QQVGA[][2] =
{
    0xff,
    0x00,
    0x2c,
    0xff,
    0x2e,
    0xdf,
    0xff,
    0x01,
    0x3c,
    0x32,
    0x11,
    0x00,
    0x09,
    0x02,
    0x03,
    0xcf,
    0x04,
    0x08,
    0x13,
    0xe5,
    0x14,
    0x48,
    0x2c,
    0x0c,
    0x33,
    0x78,
    0x3a,
    0x33,
    0x3b,
    0xfb,
    0x3e,
    0x00,
    0x43,
    0x11,
    0x16,
    0x10,
    0x39,
    0x02,
    0x35,
    0x88,
    0x22,
    0x0a,
    0x37,
    0x40,
    0x23,
    0x00,
    0x34,
    0xa0,
    0x36,
    0x1a,
    0x06,
    0x02,
    0x07,
    0xc0,
    0x0d,
    0xb7,
    0x0e,
    0x01,
    0x4c,
    0x00,
    0x4a,
    0x81,
    0x21,
    0x99,
    0x24,
    0x3a,
    0x25,
    0x32,
    0x26,
    0x82,
    0x5c,
    0x00,
    0x63,
    0x00,
    0x5d,
    0x55,
    0x5e,
    0x7d,
    0x5f,
    0x7d,
    0x60,
    0x55,
    0x61,
    0x70,
    0x62,
    0x80,
    0x7c,
    0x05,
    0x20,
    0x80,
    0x28,
    0x30,
    0x6c,
    0x00,
    0x6d,
    0x80,
    0x6e,
    0x00,
    0x70,
    0x02,
    0x71,
    0x96,
    0x73,
    0xe1,
    0x3d,
    0x34,
    0x5a,
    0x57,
    0x4f,
    0xbb,
    0x50,
    0x9c,
    0x0f,
    0x43,
    0xff,
    0x00,
    0xe5,
    0x7f,
    0xf9,
    0xc0,
    0x41,
    0x24,
    0xe0,
    0x14,
    0x76,
    0xff,
    0x33,
    0xa0,
    0x42,
    0x20,
    0x43,
    0x18,
    0x4c,
    0x00,
    0x87,
    0xd0,
    0x88,
    0x3f,
    0xd7,
    0x03,
    0xd9,
    0x10,
    0xd3,
    0x82,
    0xc8,
    0x08,
    0xc9,
    0x80,
    0x7c,
    0x00,
    0x7d,
    0x02,
    0x7c,
    0x03,
    0x7d,
    0x48,
    0x7d,
    0x48,
    0x7c,
    0x08,
    0x7d,
    0x20,
    0x7d,
    0x10,
    0x7d,
    0x0e,
    0x90,
    0x00,
    0x91,
    0x0e,
    0x91,
    0x1a,
    0x91,
    0x31,
    0x91,
    0x5a,
    0x91,
    0x69,
    0x91,
    0x75,
    0x91,
    0x7e,
    0x91,
    0x88,
    0x91,
    0x8f,
    0x91,
    0x96,
    0x91,
    0xa3,
    0x91,
    0xaf,
    0x91,
    0xc4,
    0x91,
    0xd7,
    0x91,
    0xe8,
    0x91,
    0x20,
    0x92,
    0x00,
    0x93,
    0x06,
    0x93,
    0xe3,
    0x93,
    0x05,
    0x93,
    0x05,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x96,
    0x00,
    0x97,
    0x08,
    0x97,
    0x19,
    0x97,
    0x02,
    0x97,
    0x0c,
    0x97,
    0x24,
    0x97,
    0x30,
    0x97,
    0x28,
    0x97,
    0x26,
    0x97,
    0x02,
    0x97,
    0x98,
    0x97,
    0x80,
    0x97,
    0x00,
    0x97,
    0x00,
    0xc3,
    0xed,
    0xa4,
    0x00,
    0xa8,
    0x00,
    0xbf,
    0x00,
    0xba,
    0xf0,
    0xbc,
    0x64,
    0xbb,
    0x02,
    0xb6,
    0x3d,
    0xb8,
    0x57,
    0xb7,
    0x38,
    0xb9,
    0x4e,
    0xb3,
    0xe8,
    0xb4,
    0xe1,
    0xb5,
    0x66,
    0xb0,
    0x67,
    0xb1,
    0x5e,
    0xb2,
    0x04,
    0xc7,
    0x00,
    0xc6,
    0x51,
    0xc5,
    0x11,
    0xc4,
    0x9c,
    0xcf,
    0x02,
    0xa6,
    0x00,
    0xa7,
    0xe0,
    0xa7,
    0x10,
    0xa7,
    0x1e,
    0xa7,
    0x21,
    0xa7,
    0x00,
    0xa7,
    0x28,
    0xa7,
    0xd0,
    0xa7,
    0x10,
    0xa7,
    0x16,
    0xa7,
    0x21,
    0xa7,
    0x00,
    0xa7,
    0x28,
    0xa7,
    0xd0,
    0xa7,
    0x10,
    0xa7,
    0x17,
    0xa7,
    0x21,
    0xa7,
    0x00,
    0xa7,
    0x28,
    0xc0,
    0xc8,
    0xc1,
    0x96,
    0x86,
    0x1d,
    0x50,
    0x00,
    0x51,
    0x90,
    0x52,
    0x18,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x88,
    0x57,
    0x00,
    0x5a,
    0x90,
    0x5b,
    0x18,
    0x5c,
    0x05,
    0xc3,
    0xef,
    0x7f,
    0x00,
    0xda,
    0x00,
    0xe5,
    0x1f,
    0xe1,
    0x67,
    0xe0,
    0x00,
    0xdd,
    0xff,
    0x05,
    0x00,
    0xff,
    0x01,
    0xff,
    0x01,
    0x12,
    0x00,
    0x17,
    0x11,
    0x18,
    0x75,
    0x19,
    0x01,
    0x1a,
    0x97,
    0x32,
    0x36,
    0x4f,
    0xbb,
    0x6d,
    0x80,
    0x3d,
    0x34,
    0x39,
    0x02,
    0x35,
    0x88,
    0x22,
    0x0a,
    0x37,
    0x40,
    0x23,
    0x00,
    0x34,
    0xa0,
    0x36,
    0x1a,
    0x06,
    0x02,
    0x07,
    0xc0,
    0x0d,
    0xb7,
    0x0e,
    0x01,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0x8c,
    0x00,
    0x87,
    0xd0,
    0xe0,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x14,
    0xe1,
    0x77,
    0xe5,
    0x1f,
    0xd7,
    0x03,
    0xda,
    0x10,
    0xe0,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0xc8,
    0xc1,
    0x96,
    0x86,
    0x1d,
    0x50,
    0x00,
    0x51,
    0x90,
    0x52,
    0x2c,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x88,
    0x57,
    0x00,
    0x5a,
    0x90,
    0x5b,
    0x2c,
    0x5c,
    0x05,
    0xe0,
    0x00,
    0xd3,
    0x04,
    0xff,
    0x00,
    0xc3,
    0xef,
    0xa6,
    0x00,
    0xa7,
    0xdd,
    0xa7,
    0x78,
    0xa7,
    0x7e,
    0xa7,
    0x24,
    0xa7,
    0x00,
    0xa7,
    0x25,
    0xa6,
    0x06,
    0xa7,
    0x20,
    0xa7,
    0x58,
    0xa7,
    0x73,
    0xa7,
    0x34,
    0xa7,
    0x00,
    0xa7,
    0x25,
    0xa6,
    0x0c,
    0xa7,
    0x28,
    0xa7,
    0x58,
    0xa7,
    0x6d,
    0xa7,
    0x34,
    0xa7,
    0x00,
    0xa7,
    0x25,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xe1,
    0x67,
    0xe5,
    0x1f,
    0xd7,
    0x01,
    0xda,
    0x08,
    0xda,
    0x09,
    0xe0,
    0x00,
    0x98,
    0x00,
    0x99,
    0x00,
    0xff,
    0x01,
    0x04,
    0x28,
    0xff,
    0x01,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x39,
    0x12,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0x64,
    0xc1,
    0x4b,
    0x86,
    0x35,
    0x50,
    0x92,
    0x51,
    0xc8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x57,
    0x00,
    0x5a,
    0x28,
    0x5b,
    0x1e,
    0x5c,
    0x00,
    0xe0,
    0x00,
    0xff,
    0x01,
    0x11,
    0x00,
    0x3d,
    0x38,
    0x2d,
    0x00,
    0x50,
    0x65,
    0xff,
    0x00,
    0xd3,
    0x04,
    0x7c,
    0x00,
    0x7d,
    0x04,
    0x7c,
    0x09,
    0x7d,
    0x28,
    0x7d,
    0x00,
};

/* OV2640的 SVGA是 600列*800行的，在800列*480行的液晶屏上不能全屏*/
/* 所以直接用 UXGA 模式，再根据所需的图像窗口裁剪 */
const unsigned char OV2640_UXGA[][2] =
{
    0xff, 0x00,
    0x2c, 0xff,
    0x2e, 0xdf,
    0xff, 0x01,
    0x3c, 0x32,
    0x11, 0x00,
    0x09, 0x02,
    0x04, 0xd0, // 水平翻转
    0x13, 0xe5,
    0x14, 0x48,
    0x2c, 0x0c,
    0x33, 0x78,
    0x3a, 0x33,
    0x3b, 0xfB,
    0x3e, 0x00,
    0x43, 0x11,
    0x16, 0x10,
    0x4a, 0x81,
    0x21, 0x99,
    0x24, 0x40,
    0x25, 0x38,
    0x26, 0x82,
    0x5c, 0x00,
    0x63, 0x00,
    0x46, 0x3f,
    0x0c, 0x3c,
    0x61, 0x70,
    0x62, 0x80,
    0x7c, 0x05,
    0x20, 0x80,
    0x28, 0x30,
    0x6c, 0x00,
    0x6d, 0x80,
    0x6e, 0x00,
    0x70, 0x02,
    0x71, 0x94,
    0x73, 0xc1,
    0x3d, 0x34,
    0x5a, 0x57,
    0x12, 0x00,
    0x11, 0x00,
    0x17, 0x11,
    0x18, 0x75,
    0x19, 0x01,
    0x1a, 0x97,
    0x32, 0x36,
    0x03, 0x0f,
    0x37, 0x40,
    0x4f, 0xbb,
    0x50, 0x9c,
    0x5a, 0x57,
    0x6d, 0x80,
    0x6d, 0x38,
    0x39, 0x02,
    0x35, 0x88,
    0x22, 0x0a,
    0x37, 0x40,
    0x23, 0x00,
    0x34, 0xa0,
    0x36, 0x1a,
    0x06, 0x02,
    0x07, 0xc0,
    0x0d, 0xb7,
    0x0e, 0x01,
    0x4c, 0x00,
    0xff, 0x00,
    0xe5, 0x7f,
    0xf9, 0xc0,
    0x41, 0x24,
    0xe0, 0x14,
    0x76, 0xff,
    0x33, 0xa0,
    0x42, 0x20,
    0x43, 0x18,
    0x4c, 0x00,
    0x87, 0xd0,
    0x88, 0x3f,
    0xd7, 0x03,
    0xd9, 0x10,
    0xd3, 0x82,
    0xc8, 0x08,
    0xc9, 0x80,
    0x7d, 0x00,
    0x7c, 0x03,
    0x7d, 0x48,
    0x7c, 0x08,
    0x7d, 0x20,
    0x7d, 0x10,
    0x7d, 0x0e,
    0x90, 0x00,
    0x91, 0x0e,
    0x91, 0x1a,
    0x91, 0x31,
    0x91, 0x5a,
    0x91, 0x69,
    0x91, 0x75,
    0x91, 0x7e,
    0x91, 0x88,
    0x91, 0x8f,
    0x91, 0x96,
    0x91, 0xa3,
    0x91, 0xaf,
    0x91, 0xc4,
    0x91, 0xd7,
    0x91, 0xe8,
    0x91, 0x20,
    0x92, 0x00,
    0x93, 0x06,
    0x93, 0xe3,
    0x93, 0x02,
    0x93, 0x02,
    0x93, 0x00,
    0x93, 0x04,
    0x93, 0x00,
    0x93, 0x03,
    0x93, 0x00,
    0x93, 0x00,
    0x93, 0x00,
    0x93, 0x00,
    0x93, 0x00,
    0x93, 0x00,
    0x93, 0x00,
    0x96, 0x00,
    0x97, 0x08,
    0x97, 0x19,
    0x97, 0x02,
    0x97, 0x0c,
    0x97, 0x24,
    0x97, 0x30,
    0x97, 0x28,
    0x97, 0x26,
    0x97, 0x02,
    0x97, 0x98,
    0x97, 0x80,
    0x97, 0x00,
    0x97, 0x00,
    0xc3, 0xef,
    0xff, 0x00,
    0xba, 0xdc,
    0xbb, 0x08,
    0xb6, 0x24,
    0xb8, 0x33,
    0xb7, 0x20,
    0xb9, 0x30,
    0xb3, 0xb4,
    0xb4, 0xca,
    0xb5, 0x43,
    0xb0, 0x5c,
    0xb1, 0x4f,
    0xb2, 0x06,
    0xc7, 0x00,
    0xc6, 0x51,
    0xc5, 0x11,
    0xc4, 0x9c,
    0xbf, 0x00,
    0xbc, 0x64,
    0xa6, 0x00,
    0xa7, 0x1e,
    0xa7, 0x6b,
    0xa7, 0x47,
    0xa7, 0x33,
    0xa7, 0x00,
    0xa7, 0x23,
    0xa7, 0x2e,
    0xa7, 0x85,
    0xa7, 0x42,
    0xa7, 0x33,
    0xa7, 0x00,
    0xa7, 0x23,
    0xa7, 0x1b,
    0xa7, 0x74,
    0xa7, 0x42,
    0xa7, 0x33,
    0xa7, 0x00,
    0xa7, 0x23,
    0xc0, 0xc8,
    0xc1, 0x96,
    0x8c, 0x00,
    0x86, 0x3d,
    0x50, 0x92,
    0x51, 0x90,
    0x52, 0x2c,
    0x53, 0x00,
    0x54, 0x00,
    0x55, 0x88,
    0x5a, 0x50,
    0x5b, 0x3c,
    0x5c, 0x00,
    0xd3, 0x04,
    0x7f, 0x00,
    0xda, 0x00,
    0xe5, 0x1f,
    0xe1, 0x67,
    0xe0, 0x00,
    0xdd, 0x7f,
    0x05, 0x00,
    0xff, 0x00,
    0xe0, 0x04,
    0xc0, 0xc8,
    0xc1, 0x96,
    0x86, 0x3d,
    0x50, 0x92,
    0x51, 0x90,
    0x52, 0x2c,
    0x53, 0x00,
    0x54, 0x00,
    0x55, 0x88,
    0x57, 0x00,
    0x5a, 0x50,
    0x5b, 0x3c,
    0x5c, 0x00,
    0xd3, 0x04,
    0xe0, 0x00,
    0xFF, 0x00,
    0x05, 0x00,
    0xDA, 0x08,
    0xda, 0x09,
    0x98, 0x00,
    0x99, 0x00,
    0x00, 0x00,

    0xff, 0x01,
    0x11, 0x00,
    //  0x12, 0x10,
    //  0x2a, 0x00,
    //  0x2b, 0x00,
    //  0x46, 0x87,
    //  0x47, 0x00,
    //  0x3d, 0x33,//帧率相关，跟0x11组合
    //  0xff, 0x00,
    //  0xe0, 0x04,
    //  0xc0, 0xc8,
    //  0xc1, 0x96,
    //  0x86, 0x35,
    //  0x50, 0x80,//必须配置这个寄存器控制时钟
    //  0x51, 0x90,
    //  0x52, 0x2c,
    //  0x53, 0x00,
    //  0x54, 0x00,
    //  0x55, 0x88,
    //  0x57, 0x00,
    //  0x5a, 0x78,//480
    //  0x5b, 0x44,//272
    //  0x5c, 0x00,
    //  0xd3, 0x04,
    //  0xe0, 0x00,
};

const unsigned char OV2640_JPEG_INIT[][2] =
{
    0xff,
    0x00,
    0x2c,
    0xff,
    0x2e,
    0xdf,
    0xff,
    0x01,
    0x3c,
    0x32,
    0x11,
    0x30,
    0x09,
    0x02,
    0x04,
    0x28,
    0x13,
    0xe5,
    0x14,
    0x48,
    0x2c,
    0x0c,
    0x33,
    0x78,
    0x3a,
    0x33,
    0x3b,
    0xfB,
    0x3e,
    0x00,
    0x43,
    0x11,
    0x16,
    0x10,
    0x39,
    0x92,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0x48,
    0x00,
    0x5B,
    0x00,
    0x42,
    0x03,
    0x4a,
    0x81,
    0x21,
    0x99,
    0x24,
    0x40,
    0x25,
    0x38,
    0x26,
    0x82,
    0x5c,
    0x00,
    0x63,
    0x00,
    0x61,
    0x70,
    0x62,
    0x80,
    0x7c,
    0x05,
    0x20,
    0x80,
    0x28,
    0x30,
    0x6c,
    0x00,
    0x6d,
    0x80,
    0x6e,
    0x00,
    0x70,
    0x02,
    0x71,
    0x94,
    0x73,
    0xc1,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x37,
    0xc0,
    0x4f,
    0x60,
    0x50,
    0xa8,
    0x6d,
    0x00,
    0x3d,
    0x38,
    0x46,
    0x3f,
    0x4f,
    0x60,
    0x0c,
    0x3c,
    0xff,
    0x00,
    0xe5,
    0x7f,
    0xf9,
    0xc0,
    0x41,
    0x24,
    0xe0,
    0x14,
    0x76,
    0xff,
    0x33,
    0xa0,
    0x42,
    0x20,
    0x43,
    0x18,
    0x4c,
    0x00,
    0x87,
    0xd5,
    0x88,
    0x3f,
    0xd7,
    0x03,
    0xd9,
    0x10,
    0xd3,
    0x82,
    0xc8,
    0x08,
    0xc9,
    0x80,
    0x7c,
    0x00,
    0x7d,
    0x00,
    0x7c,
    0x03,
    0x7d,
    0x48,
    0x7d,
    0x48,
    0x7c,
    0x08,
    0x7d,
    0x20,
    0x7d,
    0x10,
    0x7d,
    0x0e,
    0x90,
    0x00,
    0x91,
    0x0e,
    0x91,
    0x1a,
    0x91,
    0x31,
    0x91,
    0x5a,
    0x91,
    0x69,
    0x91,
    0x75,
    0x91,
    0x7e,
    0x91,
    0x88,
    0x91,
    0x8f,
    0x91,
    0x96,
    0x91,
    0xa3,
    0x91,
    0xaf,
    0x91,
    0xc4,
    0x91,
    0xd7,
    0x91,
    0xe8,
    0x91,
    0x20,
    0x92,
    0x00,
    0x93,
    0x06,
    0x93,
    0xe3,
    0x93,
    0x05,
    0x93,
    0x05,
    0x93,
    0x00,
    0x93,
    0x04,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x93,
    0x00,
    0x96,
    0x00,
    0x97,
    0x08,
    0x97,
    0x19,
    0x97,
    0x02,
    0x97,
    0x0c,
    0x97,
    0x24,
    0x97,
    0x30,
    0x97,
    0x28,
    0x97,
    0x26,
    0x97,
    0x02,
    0x97,
    0x98,
    0x97,
    0x80,
    0x97,
    0x00,
    0x97,
    0x00,
    0xc3,
    0xed,
    0xa4,
    0x00,
    0xa8,
    0x00,
    0xc5,
    0x11,
    0xc6,
    0x51,
    0xbf,
    0x80,
    0xc7,
    0x10,
    0xb6,
    0x66,
    0xb8,
    0xA5,
    0xb7,
    0x64,
    0xb9,
    0x7C,
    0xb3,
    0xaf,
    0xb4,
    0x97,
    0xb5,
    0xFF,
    0xb0,
    0xC5,
    0xb1,
    0x94,
    0xb2,
    0x0f,
    0xc4,
    0x5c,
    0xc0,
    0x64,
    0xc1,
    0x4B,
    0x8c,
    0x00,
    0x86,
    0x3D,
    0x50,
    0x00,
    0x51,
    0xC8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x5a,
    0xC8,
    0x5b,
    0x96,
    0x5c,
    0x00,
    0xd3,
    0x7f,
    0xc3,
    0xed,
    0x7f,
    0x00,
    0xda,
    0x00,
    0xe5,
    0x1f,
    0xe1,
    0x67,
    0xe0,
    0x00,
    0xdd,
    0x7f,
    0x05,
    0x00,

    0x12,
    0x40,
    0xd3,
    0x7f,
    0xc0,
    0x16,
    0xC1,
    0x12,
    0x8c,
    0x00,
    0x86,
    0x3d,
    0x50,
    0x00,
    0x51,
    0x2C,
    0x52,
    0x24,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x5A,
    0x2c,
    0x5b,
    0x24,
    0x5c,
    0x00,
};

const unsigned char OV2640_YUV422[][2] =
{
    0xFF,
    0x00,
    0x05,
    0x00,
    0xDA,
    0x10,
    0xD7,
    0x03,
    0xDF,
    0x00,
    0x33,
    0x80,
    0x3C,
    0x40,
    0xe1,
    0x77,
    0x00,
    0x00,
};

const unsigned char OV2640_JPEG[][2] =
{
    0xe0,
    0x14,
    0xe1,
    0x77,
    0xe5,
    0x1f,
    0xd7,
    0x03,
    0xda,
    0x10,
    0xe0,
    0x00,
    0xFF,
    0x01,
    0x04,
    0x08,
};

/* JPG 160x120 */
const unsigned char OV2640_160x120_JPEG[][2] =
{
    0xff,
    0x01,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x39,
    0x12,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0x64,
    0xc1,
    0x4b,
    0x86,
    0x35,
    0x50,
    0x92,
    0x51,
    0xc8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x57,
    0x00,
    0x5a,
    0x28,
    0x5b,
    0x1e,
    0x5c,
    0x00,
    0xe0,
    0x00,
};

/* JPG, 0x176x144 */
const unsigned char OV2640_176x144_JPEG[][2] =
{
    0xff,
    0x01,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x39,
    0x12,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0x64,
    0xc1,
    0x4b,
    0x86,
    0x35,
    0x50,
    0x92,
    0x51,
    0xc8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x57,
    0x00,
    0x5a,
    0x2c,
    0x5b,
    0x24,
    0x5c,
    0x00,
    0xe0,
    0x00,
};

/* JPG 320x240 */
const unsigned char OV2640_320x240_JPEG[][2] =
{
    0xff,
    0x01,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x39,
    0x12,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0x64,
    0xc1,
    0x4b,
    0x86,
    0x35,
    0x50,
    0x89,
    0x51,
    0xc8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x57,
    0x00,
    0x5a,
    0x50,
    0x5b,
    0x3c,
    0x5c,
    0x00,
    0xe0,
    0x00,
};

/* JPG 352x288 */
const unsigned char OV2640_352x288_JPEG[][2] =
{
    0xff,
    0x01,
    0x12,
    0x40,
    0x17,
    0x11,
    0x18,
    0x43,
    0x19,
    0x00,
    0x1a,
    0x4b,
    0x32,
    0x09,
    0x4f,
    0xca,
    0x50,
    0xa8,
    0x5a,
    0x23,
    0x6d,
    0x00,
    0x39,
    0x12,
    0x35,
    0xda,
    0x22,
    0x1a,
    0x37,
    0xc3,
    0x23,
    0x00,
    0x34,
    0xc0,
    0x36,
    0x1a,
    0x06,
    0x88,
    0x07,
    0xc0,
    0x0d,
    0x87,
    0x0e,
    0x41,
    0x4c,
    0x00,
    0xff,
    0x00,
    0xe0,
    0x04,
    0xc0,
    0x64,
    0xc1,
    0x4b,
    0x86,
    0x35,
    0x50,
    0x89,
    0x51,
    0xc8,
    0x52,
    0x96,
    0x53,
    0x00,
    0x54,
    0x00,
    0x55,
    0x00,
    0x57,
    0x00,
    0x5a,
    0x58,
    0x5b,
    0x48,
    0x5c,
    0x00,
    0xe0,
    0x00,
};

// 设置图像输出大小
// OV2640输出图像的大小(分辨率),完全由该函数确定
// width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
// 返回值:0,设置成功
//     其他,设置失败
u8 OV2640_OutSize_Set(u16 width, u16 height)
{
    u16 outh;
    u16 outw;
    u8 temp;
    if (width % 4)
    { return 1; }
    if (height % 4)
    { return 2; }
    outw = width / 4;
    outh = height / 4;
    OV2640_WriteReg(0XFF, 0X00);
    OV2640_WriteReg(0XE0, 0X04);
    OV2640_WriteReg(0X5A, outw & 0XFF); // 设置OUTW的低八位
    OV2640_WriteReg(0X5B, outh & 0XFF); // 设置OUTH的低八位
    temp = (outw >> 8) & 0X03;
    temp |= (outh >> 6) & 0X04;
    OV2640_WriteReg(0X5C, temp); // 设置OUTH/OUTW的高位
    OV2640_WriteReg(0XE0, 0X00);
    return 0;
}

/**
 * @brief  初始化控制摄像头使用的GPIO(I2C/DCMI)
 * @param  None
 * @retval None
 */
void OV2640_HW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /***DCMI引脚配置***/
    /* 使能DCMI时钟 */
    DCMI_PWDN_GPIO_CLK_ENABLE();
    DCMI_RST_GPIO_CLK_ENABLE();
    // DCMI_VSYNC_GPIO_CLK_ENABLE();
    // DCMI_HSYNC_GPIO_CLK_ENABLE();
    // DCMI_PIXCLK_GPIO_CLK_ENABLE();
    // DCMI_D0_GPIO_CLK_ENABLE();
    // DCMI_D1_GPIO_CLK_ENABLE();
    // DCMI_D2_GPIO_CLK_ENABLE();
    // DCMI_D3_GPIO_CLK_ENABLE();
    // DCMI_D4_GPIO_CLK_ENABLE();
    // DCMI_D5_GPIO_CLK_ENABLE();
    // DCMI_D6_GPIO_CLK_ENABLE();
    // DCMI_D7_GPIO_CLK_ENABLE();

    /*控制/同步信号线*/
    // GPIO_InitStructure.Pin = DCMI_VSYNC_GPIO_PIN;
    // GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    // GPIO_InitStructure.Alternate = DCMI_VSYNC_AF;
    // HAL_GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_HSYNC_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_HSYNC_AF;
    // HAL_GPIO_Init(DCMI_HSYNC_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_PIXCLK_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_PIXCLK_AF;
    // HAL_GPIO_Init(DCMI_PIXCLK_GPIO_PORT, &GPIO_InitStructure);

    // /*数据信号*/
    // GPIO_InitStructure.Pin = DCMI_D0_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D0_AF;
    // HAL_GPIO_Init(DCMI_D0_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D1_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D1_AF;
    // HAL_GPIO_Init(DCMI_D1_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D2_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D2_AF;
    // HAL_GPIO_Init(DCMI_D2_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D3_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D3_AF;
    // HAL_GPIO_Init(DCMI_D3_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D4_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D4_AF;
    // HAL_GPIO_Init(DCMI_D4_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D5_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D5_AF;
    // HAL_GPIO_Init(DCMI_D5_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D6_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D6_AF;
    // HAL_GPIO_Init(DCMI_D6_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_D7_GPIO_PIN;
    // GPIO_InitStructure.Alternate = DCMI_D7_AF;
    // HAL_GPIO_Init(DCMI_D7_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PWDN_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(DCMI_PWDN_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_RST_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(DCMI_RST_GPIO_PORT, &GPIO_InitStructure);

    /*PWDN引脚，高电平关闭电源，低电平供电*/
    HAL_GPIO_WritePin(DCMI_PWDN_GPIO_PORT, DCMI_PWDN_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(DCMI_RST_GPIO_PORT, DCMI_RST_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DCMI_RST_GPIO_PORT, DCMI_RST_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(20);
}
/**
 * @brief  Resets the OV2640 camera.
 * @param  None
 * @retval None
 */
void OV2640_Reset(void)
{
    /*OV2640有两组寄存器，设置0xFF寄存器的值为0或为1时可选择使用不同组的寄存器*/
    OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);
    OV2640_WriteReg(OV2640_SENSOR_COM7, 0x80);
}

/**
 * @brief  读取摄像头的ID.
 * @param  OV2640ID: 存储ID的结构体
 * @retval None
 */
void OV2640_ReadID(OV2640_IDTypeDef *OV2640ID)
{
    /*OV2640有两组寄存器，设置0xFF寄存器的值为0或为1时可选择使用不同组的寄存器*/
    OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);

    /*读取寄存芯片ID*/
    OV2640ID->Manufacturer_ID1 = OV2640_ReadReg(OV2640_SENSOR_MIDH);
    OV2640ID->Manufacturer_ID2 = OV2640_ReadReg(OV2640_SENSOR_MIDL);
    OV2640ID->PIDH = OV2640_ReadReg(OV2640_SENSOR_PIDH);
    OV2640ID->PIDL = OV2640_ReadReg(OV2640_SENSOR_PIDL);
}

/**
 * @brief  配置 DCMI/DMA 以捕获摄像头数据
 * @param  None
 * @retval None
 */
void OV2640_DCMI_Init(void)
{
    /*** 配置DCMI接口 ***/
    /* 使能DCMI时钟 */
    __HAL_RCC_DCMI_PSSI_CLK_ENABLE();

    /* DCMI 配置*/
    // DCMI外设寄存器基地址
    DCMI_Handle.Instance = DCMI;
    // 硬件同步
    DCMI_Handle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    // 连续采集模式
    //  DCMI_Handle.Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
    // 像素时钟上升沿有效
    DCMI_Handle.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    // VSP低电平有效
    DCMI_Handle.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
    // HSP低电平有效
    DCMI_Handle.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    // 全采集
    DCMI_Handle.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    // 8位数据宽度
    DCMI_Handle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; // DCMI_EXTEND_DATA_10B;

    DCMI_Handle.Init.JPEGMode = DCMI_JPEG_DISABLE;
    DCMI_Handle.Init.ByteSelectMode = DCMI_BSM_ALL;
    DCMI_Handle.Init.ByteSelectStart = DCMI_OEBS_ODD;
    DCMI_Handle.Init.LineSelectMode = DCMI_LSM_ALL;
    DCMI_Handle.Init.LineSelectStart = DCMI_OELS_ODD;

    HAL_DCMI_Init(&DCMI_Handle);

    DCMI->IER = 0x0;

    __HAL_DCMI_ENABLE_IT(&DCMI_Handle, DCMI_IT_FRAME); // 开启帧中断
    //__HAL_DCMI_ENABLE(&DCMI_Handle);

    // 开始传输，数据大小以32位数据为单位(即像素个数/4，LCD_GetXSize()*LCD_GetYSize()*2/4)
    //  OV2640_DMA_Config(LCD_FB_START_ADDRESS, LCD_GetXSize()*LCD_GetYSize()/2);
    //  OV2640_DMA_Config((uint32_t)&g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT / 2);
}

/**
 * @brief  配置 DCMI/DMA 以捕获摄像头数据
 * @param  DMA_Memory0BaseAddr:本次传输的目的首地址
 * @param DMA_BufferSize：本次传输的数据量(单位为字,即4字节)
 */
void OV2640_DMA_Config(uint32_t DMA_Memory0BaseAddr, uint32_t DMA_BufferSize)
{
    /* DCMI DMA Init */
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    DMA_NodeConfTypeDef NodeConfig;
    /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);

    handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
    handle_GPDMA1_Channel0.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel0.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel0.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel0.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel0) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }

    /* GPDMA1_REQUEST_DCMI_PSSI Init */
    NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request = GPDMA1_REQUEST_DCMI_PSSI;
    NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
    NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
    NodeConfig.Init.SrcBurstLength = 1;
    NodeConfig.Init.DestBurstLength = 1;
    NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel0) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel0, NULL, &Node_GPDMA1_Channel0) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel0) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel0, &List_GPDMA1_Channel0) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&DCMI_Handle, DMA_Handle, handle_GPDMA1_Channel0);

#if 0
    /* 配置DMA从DCMI中获取数据*/
    /* 使能DMA*/
    // __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_GPDMA1_CLK_ENABLE();
    __HAL_LINKDMA(&DCMI_Handle, DMA_Handle, handle_GPDMA1_Channel9);

    handle_GPDMA1_Channel9.Instance = GPDMA1_Channel9;
    handle_GPDMA1_Channel9.Init.Request = GPDMA1_REQUEST_DCMI;
    handle_GPDMA1_Channel9.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel9.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel9.Init.DestInc = DMA_DINC_INCREMENTED;
    handle_GPDMA1_Channel9.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
    handle_GPDMA1_Channel9.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;

    // handle_GPDMA1_Channel9.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    // handle_GPDMA1_Channel9.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    handle_GPDMA1_Channel9.Init.Mode = DMA_NORMAL;//DMA_CIRCULAR;								//循环模式
    handle_GPDMA1_Channel9.Init.Priority = DMA_HIGH_PRIORITY;//DMA_PRIORITY_VERY_HIGH;
    // handle_GPDMA1_Channel9.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    // handle_GPDMA1_Channel9.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    handle_GPDMA1_Channel9.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel9.Init.DestBurstLength = 1;
    HAL_DMA_DeInit(&handle_GPDMA1_Channel9);                               //先清除以前的设置
    if (HAL_DMA_Init(&handle_GPDMA1_Channel9) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_UNLOCK(&handle_GPDMA1_Channel9);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel9, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(GPDMA1_Channel9_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel9_IRQn);

    // HAL_DMA_Start(&handle_GPDMA1_Channel9,(u32)&DCMI->DR,DMA_Memory0BaseAddr,DMA_BufferSize);
    //使能DCMI采集数据
    HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, (uint32_t)DMA_Memory0BaseAddr, DMA_BufferSize);
#endif
}

// DCMI,启动传输
void DCMI_Start(void)
{
    //__HAL_DMA_ENABLE(&handle_GPDMA1_Channel9); 	//使能DMA
    DCMI->CR |= DCMI_CR_CAPTURE; // DCMI捕获使能
}

// DCMI,关闭传输
void DCMI_Stop(void)
{
    DCMI->CR &= ~(DCMI_CR_CAPTURE); // 关闭捕获
    while (DCMI->CR & 0X01)
    {
    };                                         // 等待传输结束
    HAL_DMA_Abort_IT(&handle_GPDMA1_Channel9); // 关闭DMA，使用Abort函数！
}

/**
 * @brief  Configures the OV2640 camera in QQVGA mode.
 * @param  None
 * @retval None
 */
void OV2640_QQVGAConfig(void)
{
    uint32_t i;

    OV2640_Reset();
    Delay(200);

    /* Initialize OV2640 */
    for (i = 0; i < (sizeof(OV2640_QQVGA) / 2); i++)
    {
        //  OV2640_WriteReg(OV2640_QQVGA[i][0], OV2640_QQVGA[i][1]);
        OV2640_WriteReg(OV2640_CONFIG_QQVGA[i][0], OV2640_CONFIG_QQVGA[i][1]);

        Delay(2);
    }
}

// OV2640切换为RGB565模式
void OV2640_RGB565_Mode(void)
{
    u16 i = 0;
    // 设置:RGB565输出
    for (i = 0; i < (sizeof(ov2640_rgb565_reg_tbl) / 2); i++)
    {
        OV2640_WriteReg(ov2640_rgb565_reg_tbl[i][0], ov2640_rgb565_reg_tbl[i][1]);
    }
}

//????
// sw:0,????
//   1,????(??OV2640????????????)
void OV2640_Color_Bar(u8 sw)
{
    u8 reg;
    OV2640_WriteReg(0XFF, 0X01);
    reg = OV2640_ReadReg(0X12);
    reg &= ~(1 << 1);
    if (sw)
    { reg |= 1 << 1; }
    OV2640_WriteReg(0X12, reg);
}

/**
 * @brief  Configures the OV2640 camera in QQVGA mode.
 * @param  None
 * @retval None
 */
void OV2640_VXGAConfig(void)
{
    uint32_t i;

    /* Initialize OV2640 */
    // 初始化 OV2640,采用SXGA分辨率(1600*1200)
    for (i = 0; i < sizeof(ov2640_uxga_init_reg_tbl) / 2; i++)
    {
        OV2640_WriteReg(ov2640_uxga_init_reg_tbl[i][0], ov2640_uxga_init_reg_tbl[i][1]);
    }
}

/**
 * @brief  设置图像输出大小，OV2640输出图像的大小(分辨率),完全由该函数确定
 * @param  width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
 * @retval 0,设置成功，其他,设置失败
 */
// u8 OV2640_OutSize_Set(u16 width,u16 height)
// {
// 	u16 outh;
// 	u16 outw;
// 	u8 temp;
// 	if(width%4)return 1;
// 	if(height%4)return 2;
// 	outw=width/4;
// 	outh=height/4;
// 	OV2640_WriteReg(0XFF,0X00);
// 	OV2640_WriteReg(0XE0,0X04);
//     OV2640_WriteReg(0X50,outw&0X00);		//配置v_divider hdivider
// 	OV2640_WriteReg(0X5A,outw&0XFF);		//设置OUTW的低八位
// 	OV2640_WriteReg(0X5B,outh&0XFF);		//设置OUTH的低八位
// 	temp=(outw>>8)&0X03;
// 	temp|=(outh>>6)&0X04;
// 	OV2640_WriteReg(0X5C,temp);				//设置OUTH/OUTW的高位
// 	OV2640_WriteReg(0XE0,0X00);
// 	return 0;
// }

/**未测试*/
/**
 * @brief  设置图像尺寸大小,也就是所选格式的输出分辨率
 * @param  width,height:图像宽度和图像高度
 * @retval 0,设置成功，其他,设置失败
 */
u8 OV2640_ImageSize_Set(u16 width, u16 height)
{
    u8 temp;
    OV2640_WriteReg(0XFF, 0X00);
    OV2640_WriteReg(0XE0, 0X04);
    OV2640_WriteReg(0XC0, (width) >> 3 & 0XFF);  // 设置HSIZE的10:3位
    OV2640_WriteReg(0XC1, (height) >> 3 & 0XFF); // 设置VSIZE的10:3位
    temp = (width & 0X07) << 3;
    temp |= height & 0X07;
    temp |= (width >> 4) & 0X80;
    OV2640_WriteReg(0X8C, temp);
    OV2640_WriteReg(0XE0, 0X00);
    return 0;
}

/**未测试*/
/**
  * @brief  设置图像输出窗口
  * @param  sx,sy,起始地址
            width,height:宽度(对应:horizontal)和高度(对应:vertical)
  * @retval 0,设置成功， 其他,设置失败
  */
void OV2640_Window_Set(u16 sx, u16 sy, u16 width, u16 height)
{
    u16 endx;
    u16 endy;
    u8 temp;
    endx = sx + width / 2; // V*2
    endy = sy + height / 2;

    OV2640_WriteReg(0XFF, 0X01);
    temp = OV2640_ReadReg(0X03); // 读取Vref之前的值
    temp &= 0XF0;
    temp |= ((endy & 0X03) << 2) | (sy & 0X03);
    OV2640_WriteReg(0X03, temp);      // 设置Vref的start和end的最低2位
    OV2640_WriteReg(0X19, sy >> 2);   // 设置Vref的start高8位
    OV2640_WriteReg(0X1A, endy >> 2); // 设置Vref的end的高8位

    temp = OV2640_ReadReg(0X32); // 读取Href之前的值
    temp &= 0XC0;
    temp |= ((endx & 0X07) << 3) | (sx & 0X07);
    OV2640_WriteReg(0X32, temp);      // 设置Href的start和end的最低3位
    OV2640_WriteReg(0X17, sx >> 3);   // 设置Href的start高8位
    OV2640_WriteReg(0X18, endx >> 3); // 设置Href的end的高8位
}

// 未测试
/**
  * @brief  设置图像开窗大小
            由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
            该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
            注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
                 OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
                 自动计算缩放比例,输出给外部设备.
  * @param  width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
  * @retval 0,设置成功， 其他,设置失败
  */
u8 OV2640_ImageWin_Set(u16 offx, u16 offy, u16 width, u16 height)
{
    u16 hsize;
    u16 vsize;
    u8 temp;
    if (width % 4)
    { return 1; }
    if (height % 4)
    { return 2; }
    hsize = width / 4;
    vsize = height / 4;
    OV2640_WriteReg(0XFF, 0X00);
    OV2640_WriteReg(0XE0, 0X04);
    OV2640_WriteReg(0X51, hsize & 0XFF); // 设置H_SIZE的低八位
    OV2640_WriteReg(0X52, vsize & 0XFF); // 设置V_SIZE的低八位
    OV2640_WriteReg(0X53, offx & 0XFF);  // 设置offx的低八位
    OV2640_WriteReg(0X54, offy & 0XFF);  // 设置offy的低八位
    temp = (vsize >> 1) & 0X80;
    temp |= (offy >> 4) & 0X70;
    temp |= (hsize >> 5) & 0X08;
    temp |= (offx >> 8) & 0X07;
    OV2640_WriteReg(0X55, temp);                // 设置H_SIZE/V_SIZE/OFFX,OFFY的高位
    OV2640_WriteReg(0X57, (hsize >> 2) & 0X80); // 设置H_SIZE/V_SIZE/OFFX,OFFY的高位
    OV2640_WriteReg(0XE0, 0X00);
    return 0;
}

/**
 * @brief  配置OV2640为UXGA模式，并设置输出图像大小
 * @param  None
 * @retval None
 */
void OV2640_UXGAConfig(void)
{
    uint32_t i;

    /*摄像头复位*/
    OV2640_Reset();

    /*进行三次寄存器写入，确保配置写入正常
    (在使用摄像头长排线时，IIC数据线干扰较大，必须多次写入来保证正常)*/
    /* 写入寄存器配置 */
    for (i = 0; i < (sizeof(OV2640_UXGA) / 2); i++)
    {
        OV2640_WriteReg(OV2640_UXGA[i][0], OV2640_UXGA[i][1]);
    }
    /* Initialize OV2640 */
    for (i = 0; i < (sizeof(OV2640_UXGA) / 2); i++)
    {
        OV2640_WriteReg(OV2640_UXGA[i][0], OV2640_UXGA[i][1]);
    }
    /* Initialize OV2640 */
    for (i = 0; i < (sizeof(OV2640_UXGA) / 2); i++)
    {
        OV2640_WriteReg(OV2640_UXGA[i][0], OV2640_UXGA[i][1]);
    }
    /*设置输出的图像大小*/
    //	OV2640_OutSize_Set(img_width,img_height);
}

/**
 * @brief  Configures the OV2640 camera in JPEG mode.
 * @param  JPEGImageSize: JPEG image size
 * @retval None
 */
void OV2640_JPEGConfig(ImageFormat_TypeDef ImageFormat)
{
    uint32_t i;

    OV2640_Reset();
    Delay(200);

    /* Initialize OV2640 */
    for (i = 0; i < (sizeof(OV2640_JPEG_INIT) / 2); i++)
    {
        OV2640_WriteReg(OV2640_JPEG_INIT[i][0], OV2640_JPEG_INIT[i][1]);
    }

    /* Set to output YUV422 */
    for (i = 0; i < (sizeof(OV2640_YUV422) / 2); i++)
    {
        OV2640_WriteReg(OV2640_YUV422[i][0], OV2640_YUV422[i][1]);
    }

    OV2640_WriteReg(0xff, 0x01);
    OV2640_WriteReg(0x15, 0x00);

    /* Set to output JPEG */
    for (i = 0; i < (sizeof(OV2640_JPEG) / 2); i++)
    {
        OV2640_WriteReg(OV2640_JPEG[i][0], OV2640_JPEG[i][1]);
    }

    Delay(100);

    switch (ImageFormat)
    {
        case JPEG_160x120:
        {
            for (i = 0; i < (sizeof(OV2640_160x120_JPEG) / 2); i++)
            {
                OV2640_WriteReg(OV2640_160x120_JPEG[i][0], OV2640_160x120_JPEG[i][1]);
            }
            break;
        }
        case JPEG_176x144:
        {
            for (i = 0; i < (sizeof(OV2640_176x144_JPEG) / 2); i++)
            {
                OV2640_WriteReg(OV2640_176x144_JPEG[i][0], OV2640_176x144_JPEG[i][1]);
            }
            break;
        }
        case JPEG_320x240:
        {
            for (i = 0; i < (sizeof(OV2640_320x240_JPEG) / 2); i++)
            {
                OV2640_WriteReg(OV2640_320x240_JPEG[i][0], OV2640_320x240_JPEG[i][1]);
            }
            break;
        }
        case JPEG_352x288:
        {
            for (i = 0; i < (sizeof(OV2640_352x288_JPEG) / 2); i++)
            {
                OV2640_WriteReg(OV2640_352x288_JPEG[i][0], OV2640_352x288_JPEG[i][1]);
            }
            break;
        }
        default:
        {
            for (i = 0; i < (sizeof(OV2640_160x120_JPEG) / 2); i++)
            {
                OV2640_WriteReg(OV2640_160x120_JPEG[i][0], OV2640_160x120_JPEG[i][1]);
            }
            break;
        }
    }
}

/**
  * @brief  配置光线模式
  * @param  参数用于选择光线模式
  *         0x00 Auto     自动
  *         0x01 Sunny    光照
  *         0x02 Cloudy   阴天
  *         0x03 Office   办公室
  *         0x04 Home     家里

  * @retval None
  */
void OV2640_LightMode(uint8_t mode)
{
    switch (mode)
    {

        case 0: // Auto
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0xc7, 0x00); // AWB on
            break;

        case 1: // Sunny
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0xc7, 0x40); // AWB off
            OV2640_WriteReg(0xcc, 0x5e);
            OV2640_WriteReg(0xcd, 0x41);
            OV2640_WriteReg(0xce, 0x54);

            break;

        case 2: // Cloudy
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0xc7, 0x40); // AWB off
            OV2640_WriteReg(0xcc, 0x65);
            OV2640_WriteReg(0xcd, 0x41);
            OV2640_WriteReg(0xce, 0x4f);
            break;

        case 3: // Office
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0xc7, 0x40); // AWB off
            OV2640_WriteReg(0xcc, 0x52);
            OV2640_WriteReg(0xcd, 0x41);
            OV2640_WriteReg(0xce, 0x66);
            break;

        case 4: // Home
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0xc7, 0x40); // AWB off
            OV2640_WriteReg(0xcc, 0x42);
            OV2640_WriteReg(0xcd, 0x3f);
            OV2640_WriteReg(0xce, 0x71);
            break;
    }
}

/**
  * @brief  特殊效果
  * @param  参数用于选择光线模式
  *         0x00 Antique   复古
  *         0x01 Bluish    偏蓝
  *         0x02 Greenish  偏绿
  *         0x03 Reddish   偏红
  *         0x04 B&W       黑白
  *         0x05 Negative  反相
  *         0x06 B&W negative  黑白反相
  *         0x07 Normal     正常

  * @retval None
  */
void OV2640_SpecialEffects(uint8_t mode)
{
    switch (mode)
    {
        case 0:
            // Antique
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x18);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x40);
            OV2640_WriteReg(0x7d, 0xa6);
            break;

        case 1:
            // Bluish
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x18);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0xa0);
            OV2640_WriteReg(0x7d, 0x40);

            break;

        case 2:
            // Greenish
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x18);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x40);
            OV2640_WriteReg(0x7d, 0x40);
            break;

        case 3:
            // Reddish
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x18);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x40);
            OV2640_WriteReg(0x7d, 0xc0);
            break;

        case 4:
            // B&W
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x18);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x80);
            OV2640_WriteReg(0x7d, 0x80);
            break;

        case 5:
            // Negative
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x40);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x80);
            OV2640_WriteReg(0x7d, 0x80);

            break;

        case 6:
            // B&W negative
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x58);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x80);
            OV2640_WriteReg(0x7d, 0x80);

            break;

        case 7:
            // Normal
            OV2640_WriteReg(0xff, 0x00);
            OV2640_WriteReg(0x7c, 0x00);
            OV2640_WriteReg(0x7d, 0x00);
            OV2640_WriteReg(0x7c, 0x05);
            OV2640_WriteReg(0x7d, 0x80);
            OV2640_WriteReg(0x7d, 0x80);

            break;
    }
}

/**
 * @brief  Configures the OV2640 camera brightness.
 * @param  Brightness: Brightness value, where Brightness can be:
 *         0x40 for Brightness +2,
 *         0x30 for Brightness +1,
 *         0x20 for Brightness 0,
 *         0x10 for Brightness -1,
 *         0x00 for Brightness -2,
 * @retval None
 */
void OV2640_BrightnessConfig(uint8_t Brightness)
{
    OV2640_WriteReg(0xff, 0x00);
    OV2640_WriteReg(0x7c, 0x00);
    OV2640_WriteReg(0x7d, 0x04);
    OV2640_WriteReg(0x7c, 0x09);
    OV2640_WriteReg(0x7d, Brightness);
    OV2640_WriteReg(0x7d, 0x00);
}

/**
 * @brief  Configures the OV2640 camera Black and white mode.
 * @param  BlackWhite: BlackWhite value, where BlackWhite can be:
 *         0x18 for B&W,
 *         0x40 for Negative,
 *         0x58 for B&W negative,
 *         0x00 for Normal,
 * @retval None
 */
void OV2640_BandWConfig(uint8_t BlackWhite)
{
    OV2640_WriteReg(0xff, 0x00);
    OV2640_WriteReg(0x7c, 0x00);
    OV2640_WriteReg(0x7d, BlackWhite);
    OV2640_WriteReg(0x7c, 0x05);
    OV2640_WriteReg(0x7d, 0x80);
    OV2640_WriteReg(0x7d, 0x80);
}

/**
 * @brief  Configures the OV2640 camera color effects.
 * @param  value1: Color effects value1
 * @param  value2: Color effects value2
 *         where value1 and value2 can be:
 *         value1 = 0x40, value2 = 0xa6 for Antique,
 *         value1 = 0xa0, value2 = 0x40 for Bluish,
 *         value1 = 0x40, value2 = 0x40 for Greenish,
 *         value1 = 0x40, value2 = 0xc0 for Reddish,
 * @retval None
 */
void OV2640_ColorEffectsConfig(uint8_t value1, uint8_t value2)
{
    OV2640_WriteReg(0xff, 0x00);
    OV2640_WriteReg(0x7c, 0x00);
    OV2640_WriteReg(0x7d, 0x18);
    OV2640_WriteReg(0x7c, 0x05);
    OV2640_WriteReg(0x7d, value1);
    OV2640_WriteReg(0x7d, value2);
}

void HAL_DCMI_MspInit(DCMI_HandleTypeDef *hdcmi)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /***DCMI引脚配置***/
    /* 使能DCMI时钟 */
    __HAL_RCC_DCMI_PSSI_CLK_ENABLE();
    // DCMI_PWDN_GPIO_CLK_ENABLE();
    // DCMI_VSYNC_GPIO_CLK_ENABLE();
    // DCMI_HSYNC_GPIO_CLK_ENABLE();
    DCMI_PIXCLK_GPIO_CLK_ENABLE();
    DCMI_D0_GPIO_CLK_ENABLE();
    DCMI_D1_GPIO_CLK_ENABLE();
    DCMI_D2_GPIO_CLK_ENABLE();
    DCMI_D3_GPIO_CLK_ENABLE();
    DCMI_D4_GPIO_CLK_ENABLE();
    DCMI_D5_GPIO_CLK_ENABLE();
    DCMI_D6_GPIO_CLK_ENABLE();
    DCMI_D7_GPIO_CLK_ENABLE();

    /*控制/同步信号线*/
    GPIO_InitStructure.Pin = DCMI_VSYNC_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = DCMI_VSYNC_AF;
    HAL_GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_HSYNC_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_HSYNC_AF;
    HAL_GPIO_Init(DCMI_HSYNC_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PIXCLK_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_PIXCLK_AF;
    HAL_GPIO_Init(DCMI_PIXCLK_GPIO_PORT, &GPIO_InitStructure);

    /*数据信号*/
    GPIO_InitStructure.Pin = DCMI_D0_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D0_AF;
    HAL_GPIO_Init(DCMI_D0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D1_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D1_AF;
    HAL_GPIO_Init(DCMI_D1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D2_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D2_AF;
    HAL_GPIO_Init(DCMI_D2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D3_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D3_AF;
    HAL_GPIO_Init(DCMI_D3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D4_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D4_AF;
    HAL_GPIO_Init(DCMI_D4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D5_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D5_AF;
    HAL_GPIO_Init(DCMI_D5_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D6_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D6_AF;
    HAL_GPIO_Init(DCMI_D6_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D7_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D7_AF;
    HAL_GPIO_Init(DCMI_D7_GPIO_PORT, &GPIO_InitStructure);

    /* 配置中断 */
    HAL_NVIC_SetPriority(DCMI_PSSI_IRQn, 0, 0); // 抢占优先级0，子优先级0
    HAL_NVIC_EnableIRQ(DCMI_PSSI_IRQn);         // 使能DCMI中断

    // GPIO_InitStructure.Pin = DCMI_PWDN_GPIO_PIN;
    // GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    // HAL_GPIO_Init(DCMI_PWDN_GPIO_PORT, &GPIO_InitStructure);

    // GPIO_InitStructure.Pin = DCMI_RST_GPIO_PIN;
    // GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    // HAL_GPIO_Init(DCMI_RST_GPIO_PORT, &GPIO_InitStructure);
}

// void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
// {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};
//   DMA_NodeConfTypeDef NodeConfig;
//   if(hdcmi->Instance==DCMI)
//   {
//   /* USER CODE BEGIN DCMI_MspInit 0 */

//   /* USER CODE END DCMI_MspInit 0 */
//     /* Peripheral clock enable */
//     __HAL_RCC_DCMI_PSSI_CLK_ENABLE();

//     __HAL_RCC_GPIOC_CLK_ENABLE();
//     __HAL_RCC_GPIOB_CLK_ENABLE();
//     __HAL_RCC_GPIOA_CLK_ENABLE();
//     /**DCMI GPIO Configuration
//     PC11     ------> DCMI_D4
//     PB6     ------> DCMI_D5
//     PC10     ------> DCMI_D8
//     PB9     ------> DCMI_D7
//     PB8     ------> DCMI_D6
//     PC12     ------> DCMI_D9
//     PB7     ------> DCMI_VSYNC
//     PC9     ------> DCMI_D3
//     PC8     ------> DCMI_D2
//     PC7     ------> DCMI_D1
//     PC6     ------> DCMI_D0
//     PA4     ------> DCMI_HSYNC
//     PA6     ------> DCMI_PIXCLK
//     */
//     GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_8
//                           |GPIO_PIN_7|GPIO_PIN_6;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
//     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//     GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_9|GPIO_PIN_8|GPIO_PIN_7;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//     GPIO_InitStruct.Pin = GPIO_PIN_9;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF4_DCMI;
//     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//     GPIO_InitStruct.Pin = GPIO_PIN_4;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF10_DCMI;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//     GPIO_InitStruct.Pin = GPIO_PIN_6;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF4_DCMI;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//     /* DCMI DMA Init */
//     /* GPDMA1_REQUEST_DCMI_PSSI Init */
//     NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
//     NodeConfig.Init.Request = GPDMA1_REQUEST_DCMI_PSSI;
//     NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
//     NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
//     NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
//     NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
//     NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
//     NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
//     NodeConfig.Init.SrcBurstLength = 1;
//     NodeConfig.Init.DestBurstLength = 1;
//     NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
//     NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
//     NodeConfig.Init.Mode = DMA_NORMAL;
//     NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
//     NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
//     NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
//     if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel0) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel0, NULL, &Node_GPDMA1_Channel0) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel0) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
//     handle_GPDMA1_Channel0.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
//     handle_GPDMA1_Channel0.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
//     handle_GPDMA1_Channel0.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
//     handle_GPDMA1_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
//     handle_GPDMA1_Channel0.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
//     if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel0) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel0, &List_GPDMA1_Channel0) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     __HAL_LINKDMA(hdcmi, DMA_Handle, handle_GPDMA1_Channel0);

//     if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
//     {
//       Error_Handler();
//     }

//     /* DCMI interrupt Init */
//     HAL_NVIC_SetPriority(DCMI_PSSI_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(DCMI_PSSI_IRQn);
//   /* USER CODE BEGIN DCMI_MspInit 1 */

//   /* USER CODE END DCMI_MspInit 1 */

//   }

//}

/**
 * @brief DCMI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hdcmi: DCMI handle pointer
 * @retval None
 */
void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef *hdcmi)
{
    if (hdcmi->Instance == DCMI)
    {
        /* USER CODE BEGIN DCMI_MspDeInit 0 */

        /* USER CODE END DCMI_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_DCMI_PSSI_CLK_DISABLE();

        /**DCMI GPIO Configuration
        PC11     ------> DCMI_D4
        PB6     ------> DCMI_D5
        PC10     ------> DCMI_D8
        PB9     ------> DCMI_D7
        PB8     ------> DCMI_D6
        PC12     ------> DCMI_D9
        PB7     ------> DCMI_VSYNC
        PC9     ------> DCMI_D3
        PC8     ------> DCMI_D2
        PC7     ------> DCMI_D1
        PC6     ------> DCMI_D0
        PA4     ------> DCMI_HSYNC
        PA6     ------> DCMI_PIXCLK
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11 | GPIO_PIN_9 | GPIO_PIN_8 | GPIO_PIN_7 | GPIO_PIN_6);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_8);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_9 | GPIO_PIN_8);

        /* DCMI DMA DeInit */
        HAL_DMA_DeInit(hdcmi->DMA_Handle);

        /* DCMI interrupt DeInit */
        HAL_NVIC_DisableIRQ(DCMI_PSSI_IRQn);
        /* USER CODE BEGIN DCMI_MspDeInit 1 */

        /* USER CODE END DCMI_MspDeInit 1 */
    }
}

/**
 * @brief  Configures the OV2640 camera contrast.
 * @param  value1: Contrast value1
 * @param  value2: Contrast value2
 *         where value1 and value2 can be:
 *         value1 = 0x28, value2 = 0x0c for Contrast +2,
 *         value1 = 0x24, value2 = 0x16 for Contrast +1,
 *         value1 = 0x20, value2 = 0x20 for Contrast 0,
 *         value1 = 0x1c, value2 = 0x2a for Contrast -1,
 *         value1 = 0x18, value2 = 0x34 for Contrast -2,
 * @retval None
 */
void OV2640_ContrastConfig(uint8_t value1, uint8_t value2)
{
    OV2640_WriteReg(0xff, 0x00);
    OV2640_WriteReg(0x7c, 0x00);
    OV2640_WriteReg(0x7d, 0x04);
    OV2640_WriteReg(0x7c, 0x07);
    OV2640_WriteReg(0x7d, 0x20);
    OV2640_WriteReg(0x7d, value1);
    OV2640_WriteReg(0x7d, value2);
    OV2640_WriteReg(0x7d, 0x06);
}

/**
 * @brief  帧同步回调函数.
 * @param  None
 * @retval None
 */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    fps++;                                                  // 帧率计数
    __HAL_DCMI_CLEAR_FLAG(&DCMI_Handle, DCMI_FLAG_VSYNCRI); // 清除帧中断
    // HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, (uint32_t)&g_Video_buff[0],IMAGE_WIDTH * IMAGE_HEIGHT / 2);
    //  OV2640_DMA_Config(LCD_FB_START_ADDRESS,LCD_GetXSize()*LCD_GetYSize()/2);
}

uint16_t reverseBits16(uint16_t value)
{
    uint16_t result = 0;
    for (int i = 0; i < 16; ++i)
    {
        if (value & (1 << i))
        {
            result |= (1 << (15 - i));
        }
    }
    return result;
}

// Function to convert RGB565 to grayscale
unsigned char rgb565ToGrayscale(uint16_t rgb565)
{
    uint16_t r = (rgb565 >> 11) & 0x1F;
    uint16_t g = (rgb565 >> 5) & 0x3F;
    uint16_t b = rgb565 & 0x1F;

    // Convert to grayscale using luminosity method
    return (unsigned char)(0.299 * (r << 3) + 0.587 * (g << 2) + 0.114 * (b << 3));
}

#define WIDTH 160
#define HEIGHT 120

// Laplacian kernel
const int laplacianKernel[3][3] =
{
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
};

void applyLaplacianFilter(uint8_t *input, uint8_t *output)
{
    for (int y = 1; y < HEIGHT - 1; y++)
    {
        unsigned char *inputRowPrev = &input[(y - 1) * WIDTH];
        unsigned char *inputRowCurr = &input[y * WIDTH];
        unsigned char *inputRowNext = &input[(y + 1) * WIDTH];
        unsigned char *outputRow = &output[y * WIDTH];

        for (int x = 1; x < WIDTH - 1; x++)
        {
            int sum =
                inputRowPrev[x - 1] * laplacianKernel[0][0] +
                inputRowPrev[x] * laplacianKernel[0][1] +
                inputRowPrev[x + 1] * laplacianKernel[0][2] +
                inputRowCurr[x - 1] * laplacianKernel[1][0] +
                inputRowCurr[x] * laplacianKernel[1][1] +
                inputRowCurr[x + 1] * laplacianKernel[1][2] +
                inputRowNext[x - 1] * laplacianKernel[2][0] +
                inputRowNext[x] * laplacianKernel[2][1] +
                inputRowNext[x + 1] * laplacianKernel[2][2];

            // Clamp to 0-255
            if (sum < 0)
            { sum = 0; }
            if (sum > 255)
            { sum = 255; }
            outputRow[x] = (unsigned char)sum;
        }
    }
}

#define SWAP_BYTES(value) \
    (((uint16_t)(value) >> 8) | ((uint16_t)(value) << 8))

void OV2640_Swap_Buffer(void)
{
    //  for (int i = 0; i < (IMAGE_HEIGHT * IMAGE_WIDTH / 2); ++i)
    //  {
    //       uint32_t value = g_Video_save_buff[i];
    //       g_Display_buff[2 * i + 1] = (value >> 16) & 0xFFFF; // 高16位（已经是MSB顺序，如果原始数据是小端序则无需调整）
    //       g_Display_buff[2 * i + 1] = (g_Display_buff[2 * i + 1] >> 8) | (g_Display_buff[2 * i + 1] << 8);
    //       g_Display_buff[2 * i ] = value & 0xFFFF; // 低16位（同样，已经是LSB顺序，但在这里我们当作16位数据处理，所以不需要调整）
    //       g_Display_buff[2 * i ] = (g_Display_buff[2 * i] >> 8) | (g_Display_buff[2 * i] << 8);
    //  }

    for (int i = 0; i < (IMAGE_HEIGHT * IMAGE_WIDTH); ++i)
    {

        // 10bit模式的处理
        //  uint32_t value = g_Video_save_buff[i];
        //  uint8_t low = ((value & 0x0000FFFF) >> 2) % 256;
        //  uint8_t high = (value >> 18) % 256;
        //  g_Display_buff[i] = (low << 8) | high;

        // 8bit模式的处理
        g_Display_buff[i] = SWAP_BYTES(g_Video_save_buff[i]);
        // g_Display_buff[i] = (g_Video_save_buff[i]);

#if 0
        uint16_t rgb565_val = SWAP_BYTES(g_Display_buff[i]);

        uint32_t r = (rgb565_val >> 11) & 0x1F;
        uint32_t g = (rgb565_val >> 5) & 0x3F;
        uint32_t b = rgb565_val & 0x1F;

        // Expand to 8-bit values
        r = (r << 3);            // 5-bit to 8-bit: 0x1F -> 0xFF
        g = (g << 2);            // 6-bit to 8-bit: 0x3F -> 0xFF
        b = (b << 3);            // 5-bit to 8-bit: 0x1F -> 0xFF

        //   // Expand to 8-bit values with higher precision
        //   r = (r * 259 + 32) >> 6;          // 5-bit to 8-bit: 0x1F -> 0xFF
        //   g = (g * 259 + 32) >> 6;          // 6-bit to 8-bit: 0x3F -> 0xFF
        //   b = (b * 259 + 32) >> 6;          // 5-bit to 8-bit: 0x1F -> 0xFF

        // Apply standard grayscale formula
        uint32_t gray = (r * 77) + (g * 150) + (b * 29);

        // Right shift by 8 to divide by 256 and clamp to 0-255
        uint32_t result = (uint32_t)(gray >> 8);
        if (result < 0)
        {
            result = 0;
        }
        if (result > 255)
        {
            result = 255;
        }

        g_Display_Gray_buff[i] = (uint8_t)(result);
#endif
    }

    // applyLaplacianFilter((uint8_t *)&g_Display_Gray_buff[0],(uint8_t *)&g_Display_OutLine_buff[0]);

    //  for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; ++i)
    // {
    //     uint16_t temp = g_Display_buff[i];
    //     g_Display_buff[i] = reverseBits16(temp);//(temp >> 8) | (temp << 8);
    // }
}

volatile uint8_t display_flag = 0;
// 是否处理图像的标志
volatile uint8_t g_Working_flag = 0;
// 捕获到一帧图像处理函数
// hdcmi:DCMI句柄
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{

    if (g_Working_flag) // && (0 == (fps % 2)))
    {
        OV2640_Swap_Buffer();
        display_flag = 1;
    }
    fps++;

    // st7789_draw_image(0, 0, 120, 160, (const uint16_t *)&g_Display_buff[0]);
    __HAL_DCMI_CLEAR_FLAG(&DCMI_Handle, DCMI_FLAG_FRAMERI); // 清除帧中断
    // 重新使能帧中断,因为HAL_DCMI_IRQHandler()函数会关闭帧中断
    __HAL_DCMI_ENABLE_IT(&DCMI_Handle, DCMI_IT_FRAME);
}

/**
 * @brief  DCMI中断服务函数
 * @param  None
 * @retval None
 */
void DCMI_PSSI_IRQHandler(void)
{
    HAL_DCMI_IRQHandler(&DCMI_Handle);
}

void GPDMA1_Channel0_IRQHandler(void)
{
    if (g_Working_flag)
    {
        memcpy(&g_Video_save_buff[0], &g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT * 2);
    }
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel0);
}

void EXTI7_IRQHandler(void)
{
    // 清除中断标志位
    HAL_GPIO_EXTI_IRQHandler(DCMI_VSYNC_GPIO_PIN);

    // 在此处添加你的中断处理代码
    // 例如，每次中断触发时增加计数器
    fps++;
}

// 回调函数（可选，用于更灵活的中断处理）
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     // 检查是哪个引脚触发了中断
//     if (GPIO_Pin == DCMI_VSYNC_GPIO_PIN)
//     {
//       fps++;
//     }
// }

void OV2640_VSYNC_EXIT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DCMI_VSYNC_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // 或GPIO_MODE_IT_FALLING, GPIO_MODE_IT_RISING_FALLING
    GPIO_InitStruct.Pull = GPIO_NOPULL;                 // 或GPIO_PULLUP, GPIO_PULLDOWN，根据你的需要选择
    HAL_GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStruct);

    // EXTI0中断优先级配置和使能（根据你的需要设置优先级）
    HAL_NVIC_SetPriority(EXTI7_IRQn, 0, 0); // 设置抢占优先级和子优先级
    HAL_NVIC_EnableIRQ(EXTI7_IRQn);         // 使能EXTI0中断
}
extern uint16_t g_disp_buffer[320][240];
void OV2640_Display(uint8_t show_battery)
{
    uint8_t base_row = 20;

    if (display_flag)
    {
        for (uint16_t i = 0; i < 320; i++)
        {
            for (uint16_t j = 0; j < 240; j++)
            {
                uint16_t new_i, new_j;
                new_i = 159 - (uint16_t)(i / 2.0f);
                new_j = (uint16_t)(j / 2.0f);

                // Map 8-bit grayscale to 5-bit red, 6-bit green, 5-bit blue
                // uint8_t gray = g_Display_Gray_buff[new_j * 160 + new_i];
                // uint16_t r = (gray >> 3);       // 8-bit to 5-bit: 0xFF -> 0x1F
                // uint16_t g = (gray >> 2);       // 8-bit to 6-bit: 0xFF -> 0x3F
                // uint16_t b = (gray >> 3);       // 8-bit to 5-bit: 0xFF -> 0x1F
#if 0
                uint16_t vis_rgb_val = SWAP_BYTES(g_Display_buff[ new_j * 160 + new_i]);
                uint16_t r1 = (vis_rgb_val >> 11) & 0x1F;
                uint16_t g1 = (vis_rgb_val >> 5) & 0x3F;
                uint16_t b1 = vis_rgb_val & 0x1F;

                uint16_t inf_rgb_val = SWAP_BYTES(g_disp_buffer[i][j]);
                uint16_t r2 = (inf_rgb_val >> 11) & 0x1F;
                uint16_t g2 = (inf_rgb_val >> 5) & 0x3F;
                uint16_t b2 = inf_rgb_val & 0x1F;

                // Calculate average of each color component
                uint16_t r_avg = (r1 + r2) >> 1;
                uint16_t g_avg = (g1 + g2) >> 1;
                uint16_t b_avg = (b1 + b2) >> 1;


                uint16_t fusion_rgb_val = (uint16_t)(((r_avg >> 3) << 11) | ((g_avg >> 2) << 5) | (b_avg >> 3));//COLORMAP_RGB565_MSB[10][value];

                // Combine averaged components into a single RGB565 value
                g_Display_Zoom_buff[i][j] = SWAP_BYTES(fusion_rgb_val);
#endif

                // // Combine into RGB565
                // uint8_t value = g_Display_Gray_buff[new_j * 160 + new_i];
                // g_Display_Zoom_buff[i][j] = COLORMAP_RGB565_MSB[10][g_Display_Gray_buff[new_j * 160 + new_i]];
                (*g_Display_Zoom_buff)[i][j] = g_Display_buff[new_j * 160 + new_i];
            }
        }
        // st7789_draw_image(0, 0, 120, 160, (const uint16_t *)&g_Display_buff[0]);
        if (show_battery && (show_battery < BATTERY_LEVEL_NUM))
        {
            //显示电量
            st7789_append_image(240 - 22 - 20, 10 + base_row, 22, 13, (uint16_t *)ICON_BATTERY[show_battery - 1], (uint16_t *)&g_Display_Zoom_buff[0]);
        }
        st7789_draw_image_dma(0, 0, 240, 285, (const uint16_t *) & (*g_Display_Zoom_buff)[18]);
        display_flag = 0;
    }
}

void OV2640_Test(void)
{
    OV2640_IDTypeDef id;
    OV2640_HW_Init();
    I2CMaster_Init();
    OV2640_Reset();
    HAL_Delay(100);

    OV2640_ReadID(&id);

    OV2640_VXGAConfig();
    // OV2640_QQVGAConfig();
    // OV2640_VSYNC_EXIT();

    // HAL_Delay(2000);

    OV2640_RGB565_Mode();
    // OV2640_Color_Bar(1);
    // OV2640_ImageSize_Set(IMAGE_WIDTH,IMAGE_HEIGHT);
    // OV2640_ImageWin_Set(0,0,IMAGE_WIDTH,IMAGE_HEIGHT);
    // OV2640_Window_Set(0,0,IMAGE_WIDTH,IMAGE_HEIGHT);
    OV2640_OutSize_Set(IMAGE_HEIGHT, IMAGE_WIDTH);
    // OV2640_JPEGConfig(JPEG_160x120);

    // for (uint8_t i = 0; i < IMAGE_HEIGHT; i++)
    // {
    //   for (uint8_t j = 0; j < IMAGE_WIDTH; j++)
    //   {
    //     g_Video_buff[i * IMAGE_WIDTH + j] = 0x001F;
    //   }
    // }

    OV2640_DCMI_Init();

    OV2640_DMA_Config((uint32_t)&g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT / 2);
    // OV2640_OutSize_Set(IMAGE_WIDTH, IMAGE_HEIGHT);

    HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, (uint32_t)&g_Video_buff[0], IMAGE_WIDTH * IMAGE_HEIGHT / 2);
    // DCMI_Start(); 			//启动传输

#if 0
    while (1)
    {
        if (display_flag)
        {
            for (uint16_t i = 0; i < 320; i++)
            {
                for (uint16_t j = 0; j < 240; j++)
                {
                    uint16_t new_i, new_j;
                    new_i = (uint16_t)(i / 2.0f);
                    new_j = (uint16_t)(j / 2.0f);
                    g_Display_Zoom_buff[i][j] = g_Display_buff[ new_j * 160 + new_i];
                }
            }

            // st7789_draw_image(0, 0, 120, 160, (const uint16_t *)&g_Display_buff[0]);
            st7789_draw_image_dma(0, 0, 240, 280, (const uint16_t *)&g_Display_Zoom_buff[20]);
            display_flag = 0;
        }
        HAL_Delay(10);
    }
#endif
}
