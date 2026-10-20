/*!
\file       spi_flash.h
\brief      spi_flash.h 文件概述
\details    =
\author     EmbeddedLab Of SWUST
\date       2025-01-09
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
#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif
/*!
\addtogroup SPI_FLASH
\{
*/
/* INCLUDE FILES **************************************************************/
/* #include "xx.h" */


/* EXPORTED DEFINES ***********************************************************/
/*!
\defgroup SPI_FLASH_Exported_Macros SPI_FLASH 导出的宏定义
\brief 导出的宏定义
\{
*/

/** \} end of group SPI_FLASH_Exported_Macros */


/* EXPORTED TYPE DEFINITIONS **************************************************/
/*!
\defgroup SPI_FLASH_Exported_Types SPI_FLASH 导出的数据类型
\brief 导出的数据类型
\{
*/
typedef struct
{
    uint32_t ChipID;        /* 芯片ID */
    char ChipName[16];      /* 芯片型号字符串，主要用于显示 */
    uint32_t TotalSize;     /* 总容量 */
    uint16_t SectorSize;    /* 扇区大小 */
    uint16_t SectorNum;     /* 扇区数量 */
} spi_flash_t;

/** \} end of group SPI_FLASH_Exported_Types */


/* EXPORTED PUBLIC DATA *******************************************************/
/*!
\defgroup SPI_FLASH_Exported_Datas SPI_FLASH 导出的数据定义
\brief 导出的数据定义
\{
*/


/** \} end of group SPI_FLASH_Exported_Datas */


/* EXPORTED PUBLIC FUNCTIONS **************************************************/
/*!
\addtogroup SPI_FLASH_Exported_Functions
\{
*/
uint8_t spiflash_init(void);
void spiflash_erase_sector(uint32_t _uiSectorAddr);
void spiflash_erase_chip(void);
uint8_t spiflash_write_buffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint32_t _usWriteSize);
void spiflash_read_buffer(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void spiflash_test(void);
void Init_SPI_buffer(void);

/** \} SPI_FLASH_Exported_Functions */



/** \} end of group SPI_FLASH */
#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H__ */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
