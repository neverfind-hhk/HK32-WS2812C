/**
  ******************************************************************************
  * @file    hk32f030m_it.h
  * @brief   中断服务函数声明
  ******************************************************************************
  */

#ifndef __HK32F030M_IT_H
#define __HK32F030M_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* 内核异常 -------------------------------------------------------------- */
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* 外部中断（与启动文件 startup_hk32f030m.s 的向量表一一对应） ------------ */
void WWDG_IRQHandler(void);
void EXTI11_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI5_IRQHandler(void);
void TIM1_BRK_IRQHandler(void);
void ADC1_IRQHandler(void);
void TIM1_UP_TRG_COM_IRQHandler(void);
void TIM1_CC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM6_IRQHandler(void);
void EXTI6_IRQHandler(void);
void EXTI7_IRQHandler(void);
void I2C1_IRQHandler(void);
void SPI1_IRQHandler(void);
void USART1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __HK32F030M_IT_H */
