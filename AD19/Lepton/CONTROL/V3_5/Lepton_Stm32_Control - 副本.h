/*******************************************************************************
**
**      File NAME: Lepton_Stm32_Control.H
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 2016-09-02
**
**      DESCRIPTION:
**
**      (C) COPYRIGHT 2015 ~ 2025 EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/
#ifndef __LEPTON_STM32_CONTROL_H__
#define __LEPTON_STM32_CONTROL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_SDK.h"
#include "LEPTON_OEM.h"
#include "LEPTON_AGC.h"
#include "LEPTON_VID.h"
#include "LEPTON_SYS.h"
#include "LEPTON_RAD.h"
#include "LEPTON_CFG.h"
#include "Lepton_Stm32_I2C.h"
#include "LEPTON_MACROS.h"
#include "LEPTON_CFG.h"
#include "CRC16.H"
//#include "system.h"
#include "main.h"


/******************************************************************************/
/** EXPORTED DEFINES                                                         **/
/******************************************************************************/
typedef enum enmLeptonType{
    LEPTON_2_0 = 20,
    LEPTON_2_5 = 25,
    LEPTON_3_0 = 30,
    LEPTON_3_5 = 35,
    LEPTON_UNKNOW = 0xFF,
}LeptonType;

typedef enum enmLeptonGainMode{
    GAIN_MODE_HIGH = 0,
    GAIN_MODE_LOW,
    GAIN_MODE_AUTO
}LeptonGainMode;


/******************************************************************************/
/** EXPORTED TYPE DEFINITIONS                                                **/
/******************************************************************************/
extern volatile uint8_t LeptonFrameReady;//Lepton是否准备好一帧数据的标志
extern uint32_t g_FrameSum;//计算图像的平均值
extern uint16_t g_CrossX;
extern uint16_t g_CrossY;
extern uint8_t g_Lepton_Row;
extern uint8_t g_Lepton_Col;
extern LEP_RBFO_T g_RBFO_InternalHighGain;
extern uint8_t g_Lepton_Version;
extern LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParamsPtr;
/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
#define RAW14BIT            1 //原始数据的位数
extern LEP_CAMERA_PORT_DESC_T LepPortDesc;
extern uint8_t  g_AutoFFCFlag;
extern uint16_t g_AutoFFCTimeCount;
extern void (*LepntonDprocess) (void);

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
extern void Lepton_PwrRstPin_Init(void);
/* 完成Lepton相关的初始化
*/
extern LEP_RESULT Lepton_Init(void);

/* 读取一帧数据
*/
extern uint8_t Lepton_ReadFrame(void);

/* 16位CRC校验函数
*/
extern CRC16 CalcCRC16Words(unsigned int count, short *buffer);

/* 将Lepton读取的数据插值写入LCD缓存
*/
/* Lepton进行校正
*/
extern LEP_RESULT Lepton_RunFFCCMD(void);
extern LEP_RESULT Lepton_SetGainMode(uint8_t);
extern LEP_RESULT Lepton_GetModuleType(uint8_t*);
extern LEP_RESULT Lepton_GetSerialNumber(uint64_t*);
extern void Lepton_PowerOff(void);
extern void Lepton_Display(void);
extern void Lepton_Scale_up_2x(const uint16_t* src, uint16_t* dst,uint16_t src_height, uint16_t src_width);
extern void Lepton_ExitStandby(void);
extern void Lepton_Display(void);
extern void Lepton_Scale_up_2x(const uint16_t* src, uint16_t* dst,uint16_t src_height, uint16_t src_width);

extern uint8_t Lepton_SetFluxParams(float factor);
#ifdef __cplusplus
}
#endif

#endif /* __LEPTON_STM32_CONTROL_H__ */



/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/

