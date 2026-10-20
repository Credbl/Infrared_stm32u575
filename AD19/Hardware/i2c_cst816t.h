/*!
\file       i2c_cst816t.h
\brief      i2c_cst816t.h 文件概述
\details    =
\author     EmbeddedLab Of SWUST
\date       2024-12-18
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
#ifndef __I2C_CST816T_H__
#define __I2C_CST816T_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup I2C_CST816T
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */


/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup I2C_CST816T_Exported_Macros I2C_CST816T 导出的宏定义
\brief 导出的宏定义
\{
*/
#define CST816T_EXTI_PIN        GPIO_PIN_1
#define CST816T_GESTURE_NONE    0x00
#define CST816T_GESTURE_UP      0x01
#define CST816T_GESTURE_DOWN    0x02
#define CST816T_GESTURE_LEFT    0x03
#define CST816T_GESTURE_RIGHT   0x04
#define CST816T_GESTURE_CLICK   0x05
#define CST816T_GESTURE_DOUBLE  0x0B
#define CST816T_GESTURE_LONG    0x0C
/** \} end of group I2C_CST816T_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup I2C_CST816T_Exported_Types I2C_CST816T 导出的数据类型
\brief 导出的数据类型
\{
*/

/** \} end of group I2C_CST816T_Exported_Types */


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup I2C_CST816T_Exported_Datas I2C_CST816T 导出的数据定义
\brief 导出的数据定义
\{
*/


/** \} end of group I2C_CST816T_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup I2C_CST816T_Exported_Functions
\{
*/



uint8_t cst816t_init(void);
void cst816t_get_axis(void);
void cst816t_test(void);
void cst816t_exti_callback(uint16_t exti_pin);
void cst816t_process(uint8_t mode);
uint8_t cst816t_peek_gesture(void);
void cst816t_peek_touch(uint16_t *x, uint16_t *y);
void cst816t_clear_gesture(void);



/** \} I2C_CST816T_Exported_Functions */



/** \} end of group I2C_CST816T */
#ifdef __cplusplus
}
#endif

#endif /* __I2C_CST816T_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/


