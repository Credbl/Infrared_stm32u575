/*!
\file       bsp_flash.c
\brief      bsp_flash.c
\details    =
\author     EmbeddedLab Of SWUST
\date       2025-04-02
\see        =
\version    =
\pre        =
\bug        =
\attention  片内flash读写驱动。
\warning


\copyright
<h2><center>Copyright &copy; \f$\text{2023}\sim +\infty\f$
EmbeddedLab Of SWUST<br>All rights reserved</center></h2>
*/

/* INCLUDE FILES **************************************************************/
/* 相关头文件的.h */

/* C库的.h */

/* C++库的.h */

/* 其他库的.h */

/* 本项目内的.h */
#include "bsp.h"
#include "bsp_flash.h"

#define FLASH_USER_START_ADDR ADDR_FLASH_PAGE_26                      /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR (ADDR_FLASH_PAGE_26 + FLASH_PAGE_SIZE - 1) /* End @ of user Flash area */

uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = 0, PageError = 0;
__IO uint32_t MemoryProgramStatus = 0;
uint32_t Index = 0;
__IO uint32_t data32 = 0;

uint32_t QuadWord_Write[4] = {0x12345678,
                              0x87654321,
                              0x12344321,
                              0x56788765};

uint32_t QuadWord_Read[4] = {0};

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;

  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
 * @brief  Gets the bank of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The bank of a given address
 */
static uint32_t GetBank(uint32_t Addr)
{
  return FLASH_BANK_1;
}

static void BSP_Flash_Erase(void)
{

  /* Disable instruction cache prior to internal cacheable memory update */
  if (HAL_ICACHE_Disable() != HAL_OK)
  {
    Error_Handler();
  }

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Get the 1st page to erase */
  FirstPage = GetPage(FLASH_USER_START_ADDR);

  /* Get the number of pages to erase from 1st page */
  NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;

  /* Get the bank */
  BankNumber = GetBank(FLASH_USER_START_ADDR);

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks = BankNumber;
  EraseInitStruct.Page = FirstPage;
  EraseInitStruct.NbPages = NbOfPages;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PageError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
    /* Infinite loop */
    // Error_Handler();
    while (1)
    {
      st7789_clear_dma(COLOR_RGB565_RED);
    }
  }
}

void BSP_Flash_Write(uint32_t data)
{
  BSP_Flash_Erase();

  QuadWord_Write[0] = data;
  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  Address = FLASH_USER_START_ADDR;

  //while (Address < FLASH_USER_END_ADDR)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Address, ((uint32_t)QuadWord_Write)) == HAL_OK)
    {
      //Address = Address + 16; /* increment to next quad word*/
    }
    else
    {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error */
      while (1)
      {
        st7789_clear_dma(COLOR_RGB565_BLUE);
      }
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  /* Re-enable instruction cache */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
}

uint32_t BSP_Flash_Read(void)
{
  /* Check if the programmed data is OK
  MemoryProgramStatus = 0: data programmed correctly
  MemoryProgramStatus != 0: number of words not programmed correctly ******/
  Address = FLASH_USER_START_ADDR;
  MemoryProgramStatus = 0x0;

  //while (Address < FLASH_USER_END_ADDR)
  {
    for (Index = 0; Index < 4; Index++)
    {
      data32 = *(uint32_t *)Address;
      QuadWord_Read[Index] = data32;
      // if (data32 != QuadWord[Index])
      // {
      //   MemoryProgramStatus++;
      // }
      Address += 4;
    }
  }
  return QuadWord_Read[0]; /* Return the first word of the quadword */

  /*Check if there is an issue to program data*/
  // if (MemoryProgramStatus == 0)
  // {
  //   st7789_clear_dma(COLOR_RGB565_GREEN);
  //   ;
  // }
  // else
  // {
  //   st7789_clear_dma(COLOR_RGB565_YELLOW);
  // }
}

void BSP_Flash_Test(void)

{
  uint32_t temp = 0;
  uint8_t strTemp[100] = {0};

  BSP_Flash_Write(0x55aaabab);
  temp = BSP_Flash_Read();
  if (0x55aaabab == temp)
  {
    st7789_clear_dma(COLOR_RGB565_GREEN);
  }
  else
  {
    st7789_clear_dma(COLOR_RGB565_YELLOW);
  }

  sprintf((char *)strTemp, "V:%x", temp);
  st7789_draw_string(20, 180, (const char *)strTemp, Font_11x18, COLOR_RGB565_WHITE, COLOR_RGB565_BLACK);
}

/** \} end of group BSP_FLASH_Exported_Functions */
/** \} end of group BSP_FLASH */

/***************** (C) COPYRIGHT EmbeddedLab Of SWUST *********END OF FILE*****/
