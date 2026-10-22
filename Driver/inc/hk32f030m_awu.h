/**
  ******************************************************************************
  * @file    hk32f030m_awu.h
  * @author  Rakan.zhang
  * @version V1.0  
  * @brief   This file contains all functions prototype and macros for the AWU peripheral.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F030M_AWU_H
#define __HK32F030M_AWU_H

/* Includes ------------------------------------------------------------------*/
#include "hk32f030m.h"
#include "hk32f030m_conf.h"


/* Exported macros ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/** @addtogroup AWU_Private_Macros
  * @{
  */


#define AWU_CR_RESET_VALUE 0x00000000U
#define AWU_SR_RESET_VALUE 0x00000000U
/* AWU_SR_BUSY 由设备头提供 (2k: hk32f030m_2k.h / C: hk32f0301mxxc.h)，此处不再重复定义 */

typedef enum
{
  AWU_CLK_LSI,
  AWU_CLK_HSE,
}AWU_CLK_TYPE;

#define IS_AWU_CLK(AWU_CLK) \
  (((AWU_CLK) == AWU_CLK_LSI) || \
   ((AWU_CLK) == AWU_CLK_HSE))
/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */

/** @addtogroup AWU_Exported_Functions
  * @{
  */
void AWU_DeInit(void);
void AWU_CLKConfig(AWU_CLK_TYPE eAWU_CLK);
ErrorStatus AWU_TimerCounterAndStart(uint32_t TimerCounter);
FlagStatus AWU_GetFlagStatus(void);
/**
  * @}
  */

#endif /* __HK32F030M_AWU_H */

