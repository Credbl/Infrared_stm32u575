#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__
/****************************** Includes *****************************/
#include "bsp.h"

/****************************** Defines *******************************/

#define I2C_OWN_ADDRESS           0x00
#define OV2640_DEVICE_ADDRESS     0x60

//毫秒级延时(需要定时器支持)，或者重写Delay宏
#define Delay 		HAL_Delay

 
#define I2Cx_FLAG_TIMEOUT             ((uint32_t) 1000) //0x1100
#define I2Cx_LONG_TIMEOUT             ((uint32_t) (300 * I2Cx_FLAG_TIMEOUT)) //was300
 
 
/*引脚定义*/ 

#define SENSORS_IIC_SCL_PORT         		  GPIOB
#define SENSORS_IIC_SCL_CLK_ENABLE()    	__GPIOB_CLK_ENABLE()
#define SENSORS_IIC_SCL_PIN         		  GPIO_PIN_10
 
#define SENSORS_IIC_SDA_PORT         		  GPIOB
#define SENSORS_IIC_SDA_CLK_ENABLE()   	  __GPIOB_CLK_ENABLE()
#define SENSORS_IIC_SDA_PIN          		  GPIO_PIN_2

#define SENSORS_IIC_SCL_HIGH()           HAL_GPIO_WritePin(SENSORS_IIC_SCL_PORT, SENSORS_IIC_SCL_PIN, GPIO_PIN_SET)
#define SENSORS_IIC_SCL_LOW()            HAL_GPIO_WritePin(SENSORS_IIC_SCL_PORT, SENSORS_IIC_SCL_PIN, GPIO_PIN_RESET)
#define SENSORS_IIC_SDA_HIGH()           HAL_GPIO_WritePin(SENSORS_IIC_SDA_PORT, SENSORS_IIC_SDA_PIN, GPIO_PIN_SET)
#define SENSORS_IIC_SDA_LOW()            HAL_GPIO_WritePin(SENSORS_IIC_SDA_PORT, SENSORS_IIC_SDA_PIN, GPIO_PIN_RESET)

#define SENSORS_IIC_SDA_IN()    {GPIOB->MODER &= ~(3<<(2*2)); GPIOB->MODER |= 0<<(2*2);GPIOB->PUPDR&=~(3<<(2*2));GPIOB->PUPDR|=1<<(2*2);}
#define SENSORS_IIC_SDA_OUT()   {GPIOB->MODER &= ~(3<<(2*2)); GPIOB->MODER |= 1<<(2*2);}


#define SENSORS_IIC_READ_SDA    HAL_GPIO_ReadPin(SENSORS_IIC_SDA_PORT, SENSORS_IIC_SDA_PIN)


#define SENSORS_IIC_DELAY_TIME      10

// #define SENSORS_I2C_AF                    		GPIO_AF4_I2C1

// #define SENSORS_I2C              		  		  I2C1
// #define SENSORS_I2C_RCC_CLK_ENABLE()     		__HAL_RCC_I2C1_CLK_ENABLE()

// #define SENSORS_I2C_FORCE_RESET()      			__HAL_RCC_I2C1_FORCE_RESET()
// #define SENSORS_I2C_RELEASE_RESET()    			__HAL_RCC_I2C1_RELEASE_RESET()

/*信息输出*/
#define I2C_DEBUG_ON         1
#define I2C_DEBUG_FUNC_ON    0

#define I2C_INFO(fmt,arg...)           printf("<<-I2C-INFO->> "fmt"\n",##arg)
#define I2C_ERROR(fmt,arg...)          printf("<<-I2C-ERROR->> "fmt"\n",##arg)
#define I2C_DEBUG(fmt,arg...)          do{\
                                          if(I2C_DEBUG_ON)\
                                          printf("<<-I2C-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)

#define I2C_DEBUG_FUNC()               do{\
                                         if(I2C_DEBUG_FUNC_ON)\
                                         printf("<<-I2C-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)


																			 
void I2CMaster_Init(void);
unsigned short Get_I2C_Retry(void); 
																			 
HAL_StatusTypeDef Sensors_I2C_ReadRegister(unsigned char slave_addr,
                                       unsigned char reg_addr,
                                       unsigned short len, 
                                       unsigned char *data_ptr);
HAL_StatusTypeDef Sensors_I2C_WriteRegister(unsigned char slave_addr,
                                        unsigned char reg_addr,
                                        unsigned short len, 
                                        unsigned char *data_ptr);

uint8_t OV2640_WriteReg(uint8_t Addr, uint8_t Data);
uint8_t OV2640_ReadReg(uint8_t Addr);
uint8_t GC2145_ReadReg(uint8_t slv_addr, uint8_t regAddress);
uint8_t GC2145_WriteReg(uint8_t slv_addr, uint8_t regAddress, uint8_t regVal);

#endif // __BSP_I2C_H__


