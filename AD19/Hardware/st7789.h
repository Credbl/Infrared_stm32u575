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
#ifndef __ST7789_H__
#define __ST7789_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup ST7789
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */
#include "st7789_def.h"

/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup ST7789_Exported_Macros ST7789 导出的宏定义
\brief 导出的宏定义
\{
*/


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


void st7789_init(void);
void st7789_invert(uint8_t enable);

void st7789_backlight_on(void);
void st7789_backlight_off(void);

void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7789_draw_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void st7789_draw_string(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void st7789_clear(uint16_t color);
void st7789_test(void);

void st7789_fill_rect_dma(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_draw_image_dma(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void st7789_show_image(IMAGE_ELEMENT *pimg);
void st7789_show_num_str(uint16_t x, uint16_t y, char *str);
void st7789_show_num_str_s(uint16_t x, uint16_t y, char *str);
void st7789_clear_dma(uint16_t color);
void st7789_append_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *src, uint16_t *dst);
void st7789_append_string(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor, uint16_t *dst);

/** \} ST7789_Exported_Functions */



/** \} end of group ST7789 */
#ifdef __cplusplus
}
#endif

#endif /* __ST7789_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/



