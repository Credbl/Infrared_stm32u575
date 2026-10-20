/**
  ******************************************************************************
  * @file    sd_diskio.h
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    08-May-2015
  * @brief   Header for sd_diskio.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SD_DISKIO_H
#define __SD_DISKIO_H
#include "ff.h"
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//extern Diskio_drvTypeDef  SD_Driver;
void ff_test(void);
int8_t ff_init(void);
int8_t ff_save_bmp_rgb565(const uint16_t *pbuff, uint16_t w, uint16_t h);
int8_t ff_save_bmp_raw(const uint16_t *pbuff, uint16_t w, uint16_t h);
int8_t ff_save_bmp_256(const uint8_t *pbuff, uint16_t w, uint16_t h);
const char *ff_get_last_bmp_name(void);
int8_t ff_format(void);

#endif /* __SD_DISKIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

