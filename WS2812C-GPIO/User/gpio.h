/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   gpio配置
  *
  *
  * @note    
  ******************************************************************************
  */

#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hk32f030m.h"
#include "hk32f030m_conf.h"

#define GPIO_OUT  GPIO_Pin_4
#define GPIO_PORT GPIOD
#define GPIO_POS GPIO_PinSource4

void IO_Init(void);

#ifdef __cplusplus
}
#endif

#endif


