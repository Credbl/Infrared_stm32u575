/*!
\file       colormap.h
\brief      colormap.h 文件概述
\details    =
\author     EmbeddedLab Of SWUST
\date       2024-12-22
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
#ifndef __COLORMAP_H__
#define __COLORMAP_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup COLORMAP
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */


/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup COLORMAP_Exported_Macros COLORMAP 导出的宏定义
\brief 导出的宏定义
\{
*/

/** \} end of group COLORMAP_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup COLORMAP_Exported_Types COLORMAP 导出的数据类型
\brief 导出的数据类型
\{
*/

enum
{
    COLOUR_WHITEHOT = 0x0,
    COLOUR_BLACKHOT,
    COLOUR_FUSION,
    COLOUR_RAINBOW,
    COLOUR_GLOBOW,
    COLOUR_IRONBOW1,
    COLOUR_IRONBOW2,
    COLOUR_SEPIA,
    COLOUR_COLOUR1,
    COLOUR_COLOUR2,
    COLOUR_ICEFIRE,
    COLOUR_RAIN,
    COLOUR_REDHOT,
    COLOUR_GREENHOT,
    COLOUR_GREYRED,
    COLOUR_LAVA,
    COLOUR_INSTALERT,
    COLOUR_ARCTIC,
    COLOUR_NUM,
};

/** \} end of group COLORMAP_Exported_Types */


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup COLORMAP_Exported_Datas COLORMAP 导出的数据定义
\brief 导出的数据定义
\{
*/
extern const unsigned short COLORMAP_RGB565_MSB[COLOUR_NUM][256];
extern uint8_t COLOUR_IDX;

/** \} end of group COLORMAP_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup COLORMAP_Exported_Functions
\{
*/

void colour_change(uint8_t val);

/** \} COLORMAP_Exported_Functions */



/** \} end of group COLORMAP */
#ifdef __cplusplus
}
#endif

#endif /* __COLORMAP_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
