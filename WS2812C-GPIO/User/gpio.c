/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   反转GPIO实现数据输出
  ******************************************************************************
  */

#include "gpio.h"

void IO_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD,ENABLE);
	
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Schmit = GPIO_Schmit_Enable;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
	
	GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIO_PORT, GPIO_OUT);
}


