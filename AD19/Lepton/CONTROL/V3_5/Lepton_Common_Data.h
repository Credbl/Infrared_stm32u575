/*******************************************************************************
**
**      File NAME: Lepton_Common_Data.h
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 2016-09-01
**
**      DESCRIPTION:
**
**      (C) COPYRIGHT 2015 ~ 2025 EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/
#ifndef __LEPTON_COMMON_DATA_H__
#define __LEPTON_COMMON_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/

#include  "bsp.h"

/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/

#define IMGHIGHTEMP  (5500 + 2730)
#define	IMGLOWTEMP   (-50 + 2730)
#define HistableNum  (IMGHIGHTEMP-IMGLOWTEMP)


/* 每个 packet 大小 */
#define LEPT_PACKET_SIZE                    (82)

/* 一帧图像 packet 数量 */
#define LEPT_FRAME_PACKET_NUM               (66)

/* 每次 DMA 传输数据大小 */
#define LEPT_FRAME_SIZE                     (LEPT_PACKET_SIZE * LEPT_FRAME_PACKET_NUM)

/* 图像尺寸 */
#define  IMAGE_HEIGHT  (120)
#define  IMAGE_WIDTH   (160)
#define LeptonFrameSize     (61 * 81)
/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/



/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
extern uint16_t LeptonTxBuffer[LEPT_PACKET_SIZE];
extern uint16_t RxLeptonBuff[82 * 66];
extern uint16_t g_Lepton_Rec_Buffer[IMAGE_HEIGHT + 2][IMAGE_WIDTH];
extern uint16_t g_Lepton_Buffer[IMAGE_HEIGHT + 2][IMAGE_WIDTH];
extern uint16_t g_Lepton_Process_Buffer[IMAGE_HEIGHT + 2][IMAGE_WIDTH];
// extern uint8_t g_Lepton_Process_Buffer[IMAGE_HEIGHT + 3][IMAGE_WIDTH * 2];
extern uint16_t g_Lepton_Buffer_ex[IMAGE_HEIGHT][IMAGE_WIDTH];
extern uint16_t LeptonMetry[82 * 4];
extern uint16_t LeptonTempBuf[2];
// extern uint16_t LeptonFrameRaw[];
extern uint16_t g_HisTable[];
extern uint32_t g_Sum_HisTable[];

extern volatile uint16_t CalFrameCnt;
extern volatile uint32_t RunTime;


/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __LEPTON_COMMON_DATA_H__ */
/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/






