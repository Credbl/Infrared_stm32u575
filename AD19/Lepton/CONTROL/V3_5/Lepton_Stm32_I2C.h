/*******************************************************************************
**
**      File NAME: Lepton_Stm32_I2C.h
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 3/28/2014
**  
**      DESCRIPTION: Control the Lepton by I2C  
**
**      Copyright 2011,2012,2013- EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/
#ifndef _Lepton_Stm32_I2C_H_
#define _Lepton_Stm32_I2C_H_

#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_Types.h"
#include "LEPTON_ErrorCodes.h"
#include "LEPTON_Types.h"
#include "LEPTON_ErrorCodes.h"
#include "LEPTON_MACROS.h"
#include "LEPTON_I2C_Reg.h"
#include "main.h"
/******************************************************************************/
/** PRIVATE DEFINES                                                         **/
/******************************************************************************/
//延时时间
#define DELAY_TIME      10

// IO方坑设置  Lepton I2C端坣�?PB6(SCL) PB7(SDA)
//#define SDA_IN()    {GPIOC->MODER &= ~(3<<(1*2)); GPIOC->MODER |= 0<<(1*2);GPIOC->PUPDR&=~(3<<(1*2));GPIOC->PUPDR|=1<<(1*2);}
//#define SDA_OUT()   {GPIOC->MODER &= ~(3<<(1*2)); GPIOC->MODER |= 1<<(1*2);}

    
// IO擝作函数
// #define IIC_SCL     PAout(8) // SCL
// #define IIC_SDA     PCout(9) // SDA
// #define READ_SDA    PCin(9)  // 输入SDA

/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
extern void Lepton_IIC_Init(void);
extern LEP_RESULT DEV_I2C_MasterInit(LEP_UINT16 portID,
    LEP_UINT16 *BaudRate);
extern LEP_RESULT DEV_I2C_MasterClose(void);
extern LEP_RESULT DEV_I2C_MasterReset(void);
extern LEP_RESULT DEV_I2C_MasterReadData(LEP_UINT16 portID,
    LEP_UINT8   deviceAddress,
    LEP_UINT16  regAddress,            // Lepton Register Address
    LEP_UINT16 *readDataPtr,
    LEP_UINT16  wordsToRead,          // Number of 16-bit words to Read
    LEP_UINT16 *numWordsRead,         // Number of 16-bit words actually Read
    LEP_UINT16 *status
    );
extern LEP_RESULT DEV_I2C_MasterWriteData(LEP_UINT16 portID,
    LEP_UINT8   deviceAddress,
    LEP_UINT16  regAddress,            // Lepton Register Address
    LEP_UINT16 *writeDataPtr,
    LEP_UINT16  wordsToWrite,        // Number of 16-bit words to Write
    LEP_UINT16 *numWordsWritten,     // Number of 16-bit words actually written
    LEP_UINT16 *status
    );
extern LEP_RESULT DEV_I2C_MasterReadRegister( LEP_UINT16 portID,
    LEP_UINT8  deviceAddress, 
    LEP_UINT16 regAddress,
    LEP_UINT16 *regValue,     // Number of 16-bit words actually written
    LEP_UINT16 *status
    );
extern LEP_RESULT DEV_I2C_MasterWriteRegister( LEP_UINT16 portID,
    LEP_UINT8  deviceAddress, 
    LEP_UINT16 regAddress,
    LEP_UINT16 regValue,     // Number of 16-bit words actually written
    LEP_UINT16 *status
    );
extern LEP_RESULT DEV_I2C_MasterStatus(void);
extern uint8_t Lepton_IIC_Wait_Ack(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif  /* _Lepton_Stm32_I2C_H_ */



