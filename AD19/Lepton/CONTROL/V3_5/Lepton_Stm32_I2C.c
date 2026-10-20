/*******************************************************************************
**
**      File NAME: Lepton_Stm32_I2C.c
**
**      AUTHOR:  EmbeddedLab Of SWUST
**
**      CREATED: 2016-09-02
**
**      DESCRIPTION: 
**
**      HISTORY:  
**
**      Copyright 2015 ~ 2025 EmbeddedLab Of SWUST
**      All rights reserved.
**
*******************************************************************************/
/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/
#include "main.h"
#include "Lepton_Stm32_I2C.h"
#include "Lepton_Delay.h"
/******************************************************************************/
/** LOCAL DEFINES                                                            **/
/******************************************************************************/
#define LEPTON_IIC_SDA_PIN              GPIO_PIN_1
#define LEPTON_IIC_SDA_PORT             GPIOC
#define LEPTON_IIC_SDA_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()

#define LEPTON_IIC_SCL_PIN              GPIO_PIN_0
#define LEPTON_IIC_SCL_PORT             GPIOC
#define LEPTON_IIC_SCL_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()

#define LEPTON_IIC_SCL_HIGH()           HAL_GPIO_WritePin(LEPTON_IIC_SCL_PORT, LEPTON_IIC_SCL_PIN, GPIO_PIN_SET)
#define LEPTON_IIC_SCL_LOW()            HAL_GPIO_WritePin(LEPTON_IIC_SCL_PORT, LEPTON_IIC_SCL_PIN, GPIO_PIN_RESET)
#define LEPTON_IIC_SDA_HIGH()           HAL_GPIO_WritePin(LEPTON_IIC_SDA_PORT, LEPTON_IIC_SDA_PIN, GPIO_PIN_SET)
#define LEPTON_IIC_SDA_LOW()            HAL_GPIO_WritePin(LEPTON_IIC_SDA_PORT, LEPTON_IIC_SDA_PIN, GPIO_PIN_RESET)

#define READ_SDA    HAL_GPIO_ReadPin(LEPTON_IIC_SDA_PORT, LEPTON_IIC_SDA_PIN)

#define SDA_IN()    {GPIOC->MODER &= ~(3<<(1*2)); GPIOC->MODER |= 0<<(1*2);GPIOC->PUPDR&=~(3<<(1*2));GPIOC->PUPDR|=1<<(1*2);}
#define SDA_OUT()   {GPIOC->MODER &= ~(3<<(1*2)); GPIOC->MODER |= 1<<(1*2);}

/******************************************************************************/
/** LOCAL TYPE DEFINITIONS                                                   **/
/******************************************************************************/



/******************************************************************************/
/** PRIVATE DATA DECLARATIONS                                                **/
/******************************************************************************/



/******************************************************************************/
/** PRIVATE FUNCTION DECLARATIONS                                            **/
/******************************************************************************/
void Lepton_IIC_Init(void);                //初始化IIC的IO�?
void Lepton_IIC_Start(void);               //发送IIC开始信�?
void Lepton_IIC_Stop(void);                //发送IIC停止信号
void Lepton_IIC_Ack(void);                 //IIC发送ACK信号
void Lepton_IIC_NAck(void);                //IIC不发送ACK信号
void Lepton_IIC_Send_Byte(uint8_t txd);    //IIC发送一个字�?
uint8_t Lepton_IIC_Read_Byte(uint8_t ack); //IIC读取一个字�?
uint8_t Lepton_IIC_Wait_Ack(void);         //IIC等待ACK信号
LEP_RESULT Lepton_I2C_SingleRead( LEP_UINT16 portID, /* User-defined port ID */
    LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */
    LEP_UINT16 regAddress, /* Lepton Register Address */
    LEP_UINT16 *readDataPtr /* Read DATA buffer pointer */
    );
LEP_RESULT Lepton_I2C_SingleWrite(LEP_UINT16 portID,
    LEP_UINT8  deviceAddress, 
    LEP_UINT16 regAddress,
    LEP_UINT16 regValue     // Number of 16-bit words actually written
    );
LEP_RESULT Lepton_I2C_SequenRead( LEP_UINT16 portID, /* User-defined port ID */ 
    LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */
    LEP_UINT16 regAddress, /* Lepton Register Address */ 
    LEP_UINT16 *readDataPtr, /* Read DATA buffer pointer */ 
    LEP_UINT16 wordsToRead, /* Number of 16-bit words to Read */
    LEP_UINT16 *numWordsRead /* Number of 16-bit words actually Read */ 
    );
LEP_RESULT Lepton_I2C_SequenWrite( LEP_UINT16 portID, /* User-defined port ID */
    LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */ 
    LEP_UINT16 regAddress, /* Lepton Register Address */
    LEP_UINT16 *writeDataPtr, /* Write DATA buffer pointer */
    LEP_UINT16 wordsToWrite, /* Number of 16-bit words to Write */
    LEP_UINT16 *numWordsWritten /* Number of 16-bit words actually written */ 
    );


/******************************************************************************/
/** EXPORTED PUBLIC DATA                                                     **/
/******************************************************************************/



/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/



/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/






/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/


/******************************************************************************/
/** INCLUDE FILES                                                            **/
/******************************************************************************/








/******************************************************************************/
/** PRIVATE FUNCTION DECLARATIONS                                            **/
/******************************************************************************/



/******************************************************************************/
/** EXPORTED PUBLIC FUNCTIONS                                                **/
/******************************************************************************/

/**
 * @brief  完成利用I/O端口模拟I2C的端口配�?
 *
 * @param  None
 * 
 * @return None
 */
void Lepton_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    LEPTON_IIC_SDA_CLK_ENABLE();
    LEPTON_IIC_SCL_CLK_ENABLE();

    GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull   = GPIO_PULLUP;
    GPIO_InitStructure.Speed  = GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin = LEPTON_IIC_SCL_PIN;
    HAL_GPIO_Init(LEPTON_IIC_SCL_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.Mode   = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pin = LEPTON_IIC_SDA_PIN;
    HAL_GPIO_Init(LEPTON_IIC_SDA_PORT, &GPIO_InitStructure);

    LEPTON_IIC_SDA_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_HIGH();
     Delay_Us(DELAY_TIME);
//     IIC_SCL = 1;
//     IIC_SDA = 1;
}

/**
 * @brief  I/O口模拟开始信�?
 *
 * @param  None
 * 
 * @return None
 */
void Lepton_IIC_Start(void)
{
    SDA_OUT();              // SDA线输�?
    LEPTON_IIC_SDA_HIGH();
    LEPTON_IIC_SCL_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SDA_LOW();            //START:when CLK is high,DATA change form high to low
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_LOW();            //钳住I2C总线，准备发送或接收数据
}

/**
 * @brief  I/O口模拟停止信�?
 *
 * @param  None
 * 
 * @return None
 */
void Lepton_IIC_Stop(void)
{
    SDA_OUT();      //sda线输�?
    LEPTON_IIC_SDA_LOW();    //STOP:when CLK is high DATA change form low to high
    LEPTON_IIC_SCL_LOW();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SDA_HIGH();    //发送I2C总线结束信号
    Delay_Us(DELAY_TIME);
}


/**
 * @brief  等待从机应答信号
 *
 * @param  None
 * 
 * @return 0-接收应答成功 1-接收应答失败
 */
uint8_t Lepton_IIC_Wait_Ack(void)
{
    uint16_t  ucErrTime   = 0;
    LEPTON_IIC_SDA_HIGH();  
    Delay_Us(DELAY_TIME);
    SDA_IN();
    LEPTON_IIC_SCL_HIGH();   
    Delay_Us(DELAY_TIME);
    while (READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            Lepton_IIC_Stop();
            return 1;
        }
        // Delay_Us(1);
    }

    LEPTON_IIC_SCL_LOW();//时钟输出0
    return 0;
}


/**
 * @brief  I/O模拟产生ACK应答信号
 *
 * @param  None
 * 
 * @return None
 */
void Lepton_IIC_Ack(void)
{
    LEPTON_IIC_SCL_LOW();
    SDA_OUT();
    LEPTON_IIC_SDA_LOW();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_LOW();
}

/**
 * @brief  I/O模拟不产生ACK应答信号
 *
 * @param  None
 * 
 * @return None
 */
void Lepton_IIC_NAck(void)
{
    LEPTON_IIC_SCL_LOW();
    SDA_OUT();
    LEPTON_IIC_SDA_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_HIGH();
    Delay_Us(DELAY_TIME);
    LEPTON_IIC_SCL_LOW();
}

/**
 * @brief  IIC发送一个字�?
 *
 * @param  txd-待发送的数据
 * 
 * @return None
 */
void Lepton_IIC_Send_Byte(uint8_t txd)
{
    uint8_t  t;

    SDA_OUT();
    LEPTON_IIC_SDA_LOW();  
    LEPTON_IIC_SCL_LOW();                      //拉低时钟开始数据传�?
    for (t = 0; t < 8; t++)
    {
/*        IIC_SDA = (txd & 0x80) >> 7;*/

        if ((txd & 0x80) >> 7)
        {
            LEPTON_IIC_SDA_HIGH();
        }
        else
        {
            LEPTON_IIC_SDA_LOW();
        }
        txd <<= 1;
        Delay_Us(DELAY_TIME);
        LEPTON_IIC_SCL_HIGH();
        Delay_Us(DELAY_TIME);
        LEPTON_IIC_SCL_LOW();
        // Delay_Us(DELAY_TIME);
    }
}

/**
 * @brief  IIC读取一个字�?
 *
 * @param  ack-是否给从机应答信�?1:ACK,0:nACK
 * 
 * @return None
 */
uint8_t Lepton_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive = 0;
    SDA_IN();//SDA设置为输�?
    for (i = 0; i < 8; i++)
    {
        LEPTON_IIC_SCL_LOW();
        Delay_Us(DELAY_TIME);
        LEPTON_IIC_SCL_HIGH();
        receive <<= 1;
        if (READ_SDA)
            receive++;
        Delay_Us(DELAY_TIME);
    }

    if (!ack)
    {
        Lepton_IIC_NAck();
    }//发送nACK
    else
    {
        Lepton_IIC_Ack();
    } //发送ACK
    return receive;
}


/**
 * @brief  Performs I2C Master Initialization
 *
 * @param  portID    LEP_UINT16  User specified port ID tag.  Can be used to
 *                   select between multiple cameras
 * @param BaudRate   Clock speed in kHz. Typically this is 400.
 *                   The Device Specific Driver will try to match the desired
 *                   speed.  This parameter is updated to the actual speed the
 *                   driver can use.
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise
 */
LEP_RESULT DEV_I2C_MasterInit(LEP_UINT16 portID,
                              LEP_UINT16 *BaudRate)
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */
    // Lepton_IIC_Init();

    return (result);
}

/**
 * @brief  Closes the I2C driver connection.
 *
 * @param  None
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterClose()
{
    LEP_RESULT result = LEP_OK;

    return (result);
}

/**
 * @brief  Resets the I2C driver back to the READY state.
 *
 * @param  None
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterReset(void )
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */

    return (result);
}

/**
 * @brief  Read data from Lepton
 *
 * @param  
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterReadData(LEP_UINT16  portID,               // User-defined port ID
                                  LEP_UINT8   deviceAddress,        // Lepton Camera I2C Device Address
                                  LEP_UINT16  regAddress,           // Lepton Register Address
                                  LEP_UINT16 *readDataPtr,          // Read DATA buffer pointer
                                  LEP_UINT16  wordsToRead,          // Number of 16-bit words to Read
                                  LEP_UINT16 *numWordsRead,         // Number of 16-bit words actually Read
                                  LEP_UINT16 *status                // Transaction Status
                                 )
{
    LEP_RESULT result = LEP_OK;
    /* Place Device-Specific Interface here
    */
    if (1 == wordsToRead)
    {
        result = Lepton_I2C_SingleRead(portID, deviceAddress, regAddress, readDataPtr);
        *numWordsRead = 1;
//         if (0 != result)
//         {
//             LCD_Clear(Yellow);
//             Delay_Ms(1000);
//         }
        return result;
    }

    result = Lepton_I2C_SequenRead(portID, deviceAddress, regAddress, readDataPtr, wordsToRead, numWordsRead);
    if (result != LEP_OK || wordsToRead != *numWordsRead)
    {
        //UART_DbgLog("result=%d, wordsToRead=%d, numWordsRead=%d \r\n", result, wordsToRead, *numWordsRead);
        return LEP_ERROR;
    }

    return LEP_OK;
}

/**
 * @brief  Write data to Lepton
 *
 * @param  
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterWriteData(LEP_UINT16  portID,              // User-defined port ID
                                   LEP_UINT8   deviceAddress,       // Lepton Camera I2C Device Address
                                   LEP_UINT16  regAddress,          // Lepton Register Address
                                   LEP_UINT16 *writeDataPtr,        // Write DATA buffer pointer
                                   LEP_UINT16  wordsToWrite,        // Number of 16-bit words to Write
                                   LEP_UINT16 *numWordsWritten,     // Number of 16-bit words actually written
                                   LEP_UINT16 *status)              // Transaction Status
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */
    if (1 == wordsToWrite)
    {
        result = Lepton_I2C_SingleWrite(portID, deviceAddress, regAddress, *writeDataPtr);
        *numWordsWritten = 1;
        return result;
    }

    result = Lepton_I2C_SequenWrite(portID, deviceAddress, regAddress, writeDataPtr, wordsToWrite, numWordsWritten); 

    if (result != LEP_OK || *numWordsWritten != wordsToWrite)
    {
        //UART_DbgLog("result=%d, wordsToWrite=%d ,numWordsWritten=%d, \r\n", result, wordsToWrite, *numWordsWritten);
        result = LEP_ERROR;
			  return result;
    }

    return LEP_OK;
}

/**
 * @brief  Read data from Lepton register
 *
 * @param  
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterReadRegister( LEP_UINT16 portID,
                                       LEP_UINT8  deviceAddress,
                                       LEP_UINT16 regAddress,
                                       LEP_UINT16 *regValue,     // Number of 16-bit words actually written
                                       LEP_UINT16 *status
                                     )
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */
     result = Lepton_I2C_SingleRead(portID, deviceAddress, regAddress, regValue);
    //UART_DbgLog("DEV_I2C_MasterReadRegister %d. \r\n", result);

    return (result);
}

/**
 * @brief  Write data to Lepton register
 *
 * @param  
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterWriteRegister( LEP_UINT16 portID,
                                        LEP_UINT8  deviceAddress,
                                        LEP_UINT16 regAddress,
                                        LEP_UINT16 regValue,     // Number of 16-bit words actually written
                                        LEP_UINT16 *status
                                      )
{
    LEP_RESULT result = LEP_OK;
    
    /* Place Device-Specific Interface here
    */
    result = Lepton_I2C_SingleWrite(regAddress, deviceAddress, regAddress, regValue);

    return (result);
}

/**
 * @brief  Read lepton status
 *
 * @param  
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT DEV_I2C_MasterStatus(void )
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */

    return (result);
}


/**
 * @brief   Read 16 bit from the lepton.
 * @param   portID: pointer to the points array.
 *          deviceAddress: Number of points.
 *          regAddress: Register address.
 *          readDataPtr: Read DATA buffer pointer.
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT Lepton_I2C_SingleRead( LEP_UINT16 portID, /* User-defined port ID */
                                 LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */
                                 LEP_UINT16 regAddress, /* Lepton Register Address */
                                 LEP_UINT16 *readDataPtr /* Read DATA buffer pointer */
                                 )
{
    LEP_UINT8* dataPtr = (LEP_UINT8*)readDataPtr;

    // 1. IIC 启动
    Lepton_IIC_Start();

    // 2. 发送设备地址（注意：读取16bits时，地址需要向左移动一位）
    Lepton_IIC_Send_Byte(deviceAddress << 1);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 3 发送读数据寄存器地址(�?�?
    Lepton_IIC_Send_Byte((regAddress >> 8) & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // �?位地址
    Lepton_IIC_Send_Byte(regAddress & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 4 再次发送起始信�?
    Lepton_IIC_Start();

    // 5 发送设备地址及读信息（注意地址最低位应为1�?
    Lepton_IIC_Send_Byte((deviceAddress << 1) | (0x01));
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }
    else
    {
        dataPtr[1] = Lepton_IIC_Read_Byte(1);
        dataPtr[0] = Lepton_IIC_Read_Byte(0);
    }
    Lepton_IIC_Stop();

    // while(1);

//     UART_DbgLog("Lepton_I2C_SingleRead : deviceAddress(0x%02X), regAddress(%X,%X), regValue(%X, %X)\r\n",
//         (deviceAddress << 1), ((regAddress >> 8) & 0xFF), (regAddress & 0xFF), dataPtr[1], dataPtr[0]);

    return LEP_OK;
}

/**
 * @brief   Write 16 bit to the lepton.
 * @param   portID: pointer to the points array.
 *          deviceAddress: Number of points.
 *          regAddress: Register address.
 *          readDataPtr: Read DATA buffer pointer.
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT Lepton_I2C_SingleWrite(LEP_UINT16 portID,
                                  LEP_UINT8  deviceAddress, 
                                  LEP_UINT16 regAddress,
                                  LEP_UINT16 regValue     // Number of 16-bit words actually written
                                  )
{
//     UART_DbgLog("Lepton_I2C_SingleWrite : deviceAddress(0x%02X), regAddress(%X,%X), regValue(%X, %X)\r\n",
//         (deviceAddress << 1), ((regAddress >> 8) & 0xFF), (regAddress & 0xFF), ((regValue >>8) & 0xFF), (regValue & 0xFF));

    // 1. IIC 启动
    Lepton_IIC_Start();

    // 2. 发送设备地址（注意：读取16bits时，地址需要向左移动一位）
    Lepton_IIC_Send_Byte(deviceAddress << 1);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 3 发送读数据寄存器地址(�?�?
    Lepton_IIC_Send_Byte((regAddress >> 8) & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // �?位地址
    Lepton_IIC_Send_Byte(regAddress & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 4. 发送数据高8�?
    Lepton_IIC_Send_Byte((regValue >>8) & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // �?�?
    Lepton_IIC_Send_Byte(regValue & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    Lepton_IIC_Stop();

    return LEP_OK;
}

/**
 * @brief   Read sequential Bytes from lepton.
 * @param   portID: pointer to the points array.
 *          deviceAddress: Number of points.
 *          regAddress: Register address.
 *          readDataPtr:  Read DATA buffer pointer
 *          wordsToRead:  Number of 16-bit words to Read
 *          numWordsRead  Number of 16-bit words actually Read
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT Lepton_I2C_SequenRead( LEP_UINT16 portID, /* User-defined port ID */ 
                              LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */
                              LEP_UINT16 regAddress, /* Lepton Register Address */ 
                              LEP_UINT16 *readDataPtr, /* Read DATA buffer pointer */ 
                              LEP_UINT16 wordsToRead, /* Number of 16-bit words to Read */
                              LEP_UINT16 *numWordsRead /* Number of 16-bit words actually Read */ )
{
    LEP_UINT8* dataPtr = (LEP_UINT8*)readDataPtr;
    *numWordsRead = 0;

    // 1 发送起始信�?
    Lepton_IIC_Start();

    // 2 发送设备地址及写信息(设置起始地址)
    Lepton_IIC_Send_Byte(deviceAddress << 1);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 3. 设置起始地址(�?�?
    Lepton_IIC_Send_Byte((regAddress >> 8) & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // �?位地址
    Lepton_IIC_Send_Byte(regAddress & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 完成设置起始地址
    Lepton_IIC_Stop();

    // 重新启动IIC
    Lepton_IIC_Start();

    Lepton_IIC_Send_Byte(deviceAddress << 1 | 0x01);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    while (wordsToRead != 1)
    {
        //UART_DbgLog("wordsToRead = %d  numWordsRead=%d \r\n", wordsToRead, *numWordsRead);
        dataPtr[1] = Lepton_IIC_Read_Byte(1);
        dataPtr[0] = Lepton_IIC_Read_Byte(1);

        dataPtr += 2;
        wordsToRead--;
        (*numWordsRead)++;
    }

    // Read the last word, there is no ack signal
    dataPtr[1] = Lepton_IIC_Read_Byte(1);
    dataPtr[0] = Lepton_IIC_Read_Byte(0);
    (*numWordsRead)++;

    Lepton_IIC_Stop();
    return LEP_OK;
}

/**
 * @brief   Write sequential Bytes to lepton.
 * @param   portID: pointer to the points array.
 *          deviceAddress: Number of points.
 *          regAddress: Register address.
 *          writeDataPtr: Write DATA buffer pointer 
 *          wordsToWrite:  Number of 16-bit words to Read
 *          numWordsWritten  Number of 16-bit words actually Read
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT Lepton_I2C_SequenWrite( LEP_UINT16 portID, /* User-defined port ID */
                               LEP_UINT8 deviceAddress, /* Lepton Camera I2C Device Address */ 
                               LEP_UINT16 regAddress, /* Lepton Register Address */
                               LEP_UINT16 *writeDataPtr, /* Write DATA buffer pointer */
                               LEP_UINT16 wordsToWrite, /* Number of 16-bit words to Write */
                               LEP_UINT16 *numWordsWritten /* Number of 16-bit words actually written */ )
{
    LEP_UINT16 writeData;
    *numWordsWritten = 0;

    // 1 发送起始信�?
    Lepton_IIC_Start();

    // 2 发送设备地址及写信息(设置起始地址)
    Lepton_IIC_Send_Byte(deviceAddress << 1);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // 3 发送读数据寄存器地址(�?�?
    Lepton_IIC_Send_Byte((regAddress >> 8) & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    // �?位地址
    Lepton_IIC_Send_Byte(regAddress & 0xFF);
    if (Lepton_IIC_Wait_Ack())
    {
        return LEP_ERROR_I2C_NACK_RECEIVED;
    }

    while (wordsToWrite--)
    {
        writeData = writeDataPtr[*numWordsWritten];
        // 发送高8位数�?
        Lepton_IIC_Send_Byte((writeData >> 8) & 0xFF);
        if (Lepton_IIC_Wait_Ack())
        {
            return LEP_ERROR_I2C_NACK_RECEIVED;
        }

        // 发送低8位数�?
        Lepton_IIC_Send_Byte(writeData & 0xFF);
        if (Lepton_IIC_Wait_Ack())
        {
            return LEP_ERROR_I2C_NACK_RECEIVED;
        }

        (*numWordsWritten)++;
    }

    // 停止IIC
    Lepton_IIC_Stop();

    // while(1);

    return LEP_OK;
}

