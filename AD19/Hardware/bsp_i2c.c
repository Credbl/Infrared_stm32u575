/**
 ******************************************************************************
 * @file    i2c.c
 * @author  fire
 * @version V1.0
 * @date    2024-01-02
 * @brief   i2c 总线驱动，方便与各种I2C传感器通讯。
 *******************************************************************************/
#include "bsp_i2c.h"
#include "Lepton_Delay.h"
// I2C_HandleTypeDef I2C_Handle;
/*******************************  Function  ************************************/

/**
 * @brief  初始化I2C总线，使用I2C前需要调用
 * @param  无
 * @retval 无
 */
void I2CMaster_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能I2C GPIO 时钟 */
  SENSORS_IIC_SCL_CLK_ENABLE();
  SENSORS_IIC_SDA_CLK_ENABLE();

  /* 配置I2Cx引脚: SCL ----------------------------------------*/
  GPIO_InitStructure.Pin = SENSORS_IIC_SCL_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SENSORS_IIC_SCL_PORT, &GPIO_InitStructure);

  /* 配置I2Cx引脚: SDA ----------------------------------------*/
  GPIO_InitStructure.Pin = SENSORS_IIC_SDA_PIN;
  HAL_GPIO_Init(SENSORS_IIC_SDA_PORT, &GPIO_InitStructure);

  SENSORS_IIC_SDA_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
}
/**
 * @brief  Manages error callback by re-initializing I2C.
 * @param  Addr: I2C Address
 * @retval None
 */
static void I2Cx_Error(void)
{
  /* 恢复I2C寄存器为默认值 */
  // HAL_I2C_DeInit(&I2C_Handle);
  /* 重新初始化I2C外设 */
  I2CMaster_Init();
}

static void SENSORS_IIC_Start(void)
{
  SENSORS_IIC_SDA_OUT();
  SENSORS_IIC_SDA_HIGH();
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SDA_LOW(); // START:when CLK is high,DATA change form high to low
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_LOW();
}

/**
 * @brief  I/O????¨????????????????
 *
 * @param  None
 *
 * @return None
 */
static void SENSORS_IIC_Stop(void)
{
  SENSORS_IIC_SDA_OUT(); // sda?????????
  SENSORS_IIC_SDA_LOW(); // STOP:when CLK is high DATA change form low to high
  SENSORS_IIC_SCL_LOW();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SDA_HIGH(); //???é??I2C?????????????????ˇ
  Delay_Us(SENSORS_IIC_DELAY_TIME);
}

/**
 * @brief  ???????????????????????ˇ
 *
 * @param  None
 *
 * @return 0-?????????????????? 1-?????????????¤????
 */
static uint8_t SENSORS_IIC_Wait_Ack(void)
{
  uint16_t ucErrTime = 0;
  SENSORS_IIC_SDA_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SDA_IN();
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  while (SENSORS_IIC_READ_SDA)
  {
    ucErrTime++;
    if (ucErrTime > 250)
    {
      SENSORS_IIC_Stop();
      return 1;
    }
    // Delay_Us(1);
  }

  SENSORS_IIC_SCL_LOW(); //???é????????0
  return 0;
}

/**
 * @brief  I/O?¨??????§???ACK???????????ˇ
 *
 * @param  None
 *
 * @return None
 */
static void SENSORS_IIC_Ack(void)
{
  SENSORS_IIC_SCL_LOW();
  SENSORS_IIC_SDA_OUT();
  SENSORS_IIC_SDA_LOW();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_LOW();
}

/**
 * @brief  I/O?¨?????????§???ACK???????????ˇ
 *
 * @param  None
 *
 * @return None
 */
static void SENSORS_IIC_NAck(void)
{
  SENSORS_IIC_SCL_LOW();
  SENSORS_IIC_SDA_OUT();
  SENSORS_IIC_SDA_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_HIGH();
  Delay_Us(SENSORS_IIC_DELAY_TIME);
  SENSORS_IIC_SCL_LOW();
}

/**
 * @brief  IIC???é??????????????
 *
 * @param  txd-??????é???????°???
 *
 * @return None
 */
static void SENSORS_IIC_Send_Byte(uint8_t txd)
{
  uint8_t t;

  SENSORS_IIC_SDA_OUT();
  SENSORS_IIC_SDA_LOW();
  SENSORS_IIC_SCL_LOW(); //?????????é??????§???°?????????
  for (t = 0; t < 8; t++)
  {
    /*        IIC_SDA = (txd & 0x80) >> 7;*/

    if ((txd & 0x80) >> 7)
    {
      SENSORS_IIC_SDA_HIGH();
    }
    else
    {
      SENSORS_IIC_SDA_LOW();
    }
    txd <<= 1;
    Delay_Us(SENSORS_IIC_DELAY_TIME);
    SENSORS_IIC_SCL_HIGH();
    Delay_Us(SENSORS_IIC_DELAY_TIME);
    SENSORS_IIC_SCL_LOW();
    // Delay_Us(SENSORS_IIC_DELAY_TIME);
  }
}

/**
 * @brief  IIC??????????????????
 *
 * @param  ack-???????????????????????????1:ACK,0:nACK
 *
 * @return None
 */
static uint8_t SENSORS_IIC_Read_Byte(unsigned char ack)
{
  unsigned char i, receive = 0;
  SENSORS_IIC_SDA_IN();
  for (i = 0; i < 8; i++)
  {
    SENSORS_IIC_SCL_LOW();
    Delay_Us(SENSORS_IIC_DELAY_TIME);
    SENSORS_IIC_SCL_HIGH();
    receive <<= 1;
    if (SENSORS_IIC_READ_SDA)
      receive++;
    Delay_Us(SENSORS_IIC_DELAY_TIME);
  }

  if (!ack)
  {
    SENSORS_IIC_NAck();
  } //???é??nACK
  else
  {
    SENSORS_IIC_Ack();
  } //???é??ACK
  return receive;
}

/**
 * @brief  写一字节数据到OV2640寄存器
 * @param  Addr: OV2640 的寄存器地址
 * @param  Data: 要写入的数据
 * @retval 返回0表示写入正常，0xFF表示错误
 */
uint8_t OV2640_WriteReg(uint8_t regAddress, uint8_t regVal)
{
  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(OV2640_DEVICE_ADDRESS);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regAddress);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regVal);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Stop();
  return 0;
}

/**
 * @brief  从OV2640寄存器中读取一个字节的数据
 * @param  Addr: 寄存器地址
 * @retval 返回读取得的数据
 */
uint8_t OV2640_ReadReg( uint8_t regAddress)
{
  uint8_t data = 0;

  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(OV2640_DEVICE_ADDRESS);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regAddress);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Stop();
  Delay_Us(100);

  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(OV2640_DEVICE_ADDRESS | (0x01));
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  data = SENSORS_IIC_Read_Byte(0);
  SENSORS_IIC_Stop();
  /* return the read data */
  return data;
}



/**
 * @brief  写一字节数据到OV2640寄存器
 * @param  Addr: OV2640 的寄存器地址
 * @param  Data: 要写入的数据
 * @retval 返回0表示写入正常，0xFF表示错误
 */
uint8_t GC2145_WriteReg(uint8_t slv_addr, uint8_t regAddress, uint8_t regVal)
{
  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(slv_addr);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regAddress);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regVal);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Stop();
  return 0;
}

/**
 * @brief  从OV2640寄存器中读取一个字节的数据
 * @param  Addr: 寄存器地址
 * @retval 返回读取得的数据
 */
uint8_t GC2145_ReadReg(uint8_t slv_addr, uint8_t regAddress)
{
  uint8_t data = 0;

  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(slv_addr);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Send_Byte(regAddress);
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  SENSORS_IIC_Stop();
  Delay_Us(100);

  SENSORS_IIC_Start();
  SENSORS_IIC_Send_Byte(slv_addr | (0x01));
  if (SENSORS_IIC_Wait_Ack())
  {
    return 1;
  }
  Delay_Us(100);
  data = SENSORS_IIC_Read_Byte(0);
  SENSORS_IIC_Stop();
  /* return the read data */
  return data;
}
