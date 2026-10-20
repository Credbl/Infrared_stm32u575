/*******************************************************************************
**
**      File NAME: Lepton_Stm32_SPI.h
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 3/28/2014
**  
**      DESCRIPTION: Read data from Lepton using SPI  
**
**      Copyright 2011,2012,2013- EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/

#ifndef _Lepton_Stm32_SPI_DMA_H_
#define _Lepton_Stm32_SPI_DMA_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "LEPTON_Types.h"
#include "LEPTON_ErrorCodes.h"
#include "bsp.h"

/******************************************************************************/
/** PRIVATE DEFINES                                                         **/
/******************************************************************************/
enum {
	TRANSFER_WAIT = 0x11,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/
extern volatile uint8_t DmaBusy;
extern volatile uint8_t FrameOkay;
extern volatile uint16_t TlineCnt;

/* transfer state */
extern volatile uint8_t wTransferState;
/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/
bool LEPTON_SPI_DMA_Init(void);
bool LEPTON_SPI_DMA_Begin(void);
void LEPTON_SPI_Enable(void);
void LEPTON_SPI_Disable(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif  /* _Lepton_Stm32_SPI_DMA_H_ */


