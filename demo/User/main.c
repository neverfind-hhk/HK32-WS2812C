/**
  ******************************************************************************
  * @file    main.c
  * @brief   HK32F030MF4P6
  *
  *          时钟：HSI 32MHz，SYSCLK = HCLK = PCLK，由 SystemInit() 在
  *          Reset_Handler 里配置完成，进入 main() 时已经就绪，无需手工配置。
  *
  ******************************************************************************
  */

#include "main.h"

int main(void)
{
	
	LED_init();

    while (1)
    {
		GPIO_Toggle(GPIOD, GPIO_Pin_4);
		Delay_us(500000);
    }
}

/**
  * @brief  SysTick 轮询式毫秒延时（阻塞，不产生中断）
  * @param  ms  延时毫秒数
  * @note   用轮询 COUNTFLAG 的方式，不开启 SysTick 中断，因此与
  *         hk32f030m_it.c 里的 SysTick_Handler 互不干扰；但两者共用
  *         SysTick 定时器，若改用中断式滴答就不要同时调用本函数。
  */
void Delay_us(uint32_t us)
{
    uint32_t reload;

    if ((us == 0U) || (SystemCoreClock == 0U))
    {
        return;
    }

    reload = (SystemCoreClock / 1000000U) - 1U;

    SysTick->LOAD = reload;
    SysTick->VAL  = 0U;
    /* 使用内核时钟、开启计数、不开中断 */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (us != 0U)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
            /* 等待一次 1us 计数溢出 */
        }
        us--;
    }

    SysTick->CTRL = 0U;
    SysTick->VAL  = 0U;
}

void LED_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD,ENABLE);
	
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Schmit = GPIO_Schmit_Enable;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
	
	GPIO_Init(GPIOD , &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOD, GPIO_Pin_4);
}

