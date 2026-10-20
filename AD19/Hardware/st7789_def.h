/*!
\file       st7789.h
\brief      st7789.h 文件概述
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
<h2><center>Copyright &copy; \f$\text{2021}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/
#ifndef __ST7789_DEF_H__
#define __ST7789_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup ST7789
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */


/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup ST7789_Exported_Macros ST7789 导出的宏定义
\brief 导出的宏定义
\{
*/

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


#define ST7789_COLOR_MODE_16BIT 0x55  //  RGB565 (16bit)
#define ST7789_COLOR_MODE_18BIT 0x66  //  RGB666 (18bit)



#define COLOR_RGB565_WHITE      0xFFFF
#define COLOR_RGB565_BLACK      0x0000
#define COLOR_RGB565_BLUE       0x001F
#define COLOR_RGB565_RED        0xF800
#define COLOR_RGB565_MAGENTA    0xF81F
#define COLOR_RGB565_GREEN      0x07E0
#define COLOR_RGB565_CYAN       0x7FFF
#define COLOR_RGB565_YELLOW     0xFFE0
#define COLOR_RGB565_BROWN      0XBC40
#define COLOR_RGB565_BRRED      0XFC07
#define COLOR_RGB565_DARKBLUE   0X01CF
#define COLOR_RGB565_LIGHTBLUE  0X7D7C
#define COLOR_RGB565_GRAYBLUE   0X5458
#define COLOR_RGB565_LIGHTGREEN 0X841F
#define COLOR_RGB565_LGRAY      0XC618
#define COLOR_RGB565_LGRAYBLUE  0XA651
#define COLOR_RGB565_LBBLUE     0X2B12

#define COLOR_RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

/*!翻转 16bit rgb565 的高低字节。
因为，LCD控制器首先接收 rgb565 高字节。
见控制器手册 8.8.42 Write data for 16-bit/pixel */
#define REVERSE_RGB565(rgb565)      ((((rgb565) & 0x00FF) << 8) | ((rgb565) >> 8))


/** \} end of group ST7789_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup ST7789_Exported_Types ST7789 导出的数据类型
\brief 导出的数据类型
\{
*/

/** \} end of group ST7789_Exported_Types */


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup ST7789_Exported_Datas ST7789 导出的数据定义
\brief 导出的数据定义
\{
*/


/** \} end of group ST7789_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup ST7789_Exported_Functions
\{
*/


/** \} ST7789_Exported_Functions */



/** \} end of group ST7789 */
#ifdef __cplusplus
}
#endif

#endif /* __ST7789_DEF_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/



