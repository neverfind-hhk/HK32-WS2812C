/**
  ******************************************************************************
  * @file    hk32f030m_it.c
  * @brief   中断服务函数
  *
  * @note    启动文件 startup_hk32f030m.s 里的所有中断入口都带 [WEAK] 标记，
  *          指向 Default_Handler（死循环）。本文件重新定义了其中一部分，
  *          链接器会用这里的实现覆盖弱定义。
  *
  *          用法：
  *            · 用不到的中断，把对应函数整段删掉即可 —— 删掉后自动回落到
  *              启动文件的 Default_Handler，未预期的中断会停在死循环里，
  *              配合调试器能直接看出是哪个中断没处理，便于排查。
  *            · 保留空函数则中断会被安静地吞掉，不适合用来查问题。
  ******************************************************************************
  */

#include "main.h"
#include "hk32f030m_it.h"

/* ==========================================================================
 *                              内核异常
 * ========================================================================== */

/**
  * @brief  不可屏蔽中断
  */
void NMI_Handler(void)
{
}

/**
  * @brief  硬件错误
  * @note   停在死循环里便于挂上调试器查看 LR/PC/PSR 定位出错指令。
  */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief  SVCall
  */
void SVC_Handler(void)
{
}

/**
  * @brief  PendSV
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  系统滴答定时器
  * @note   本工程的 Delay_ms() 采用轮询 COUNTFLAG 的方式，不开启 SysTick
  *         中断，所以这个函数默认不会被触发。
  *         若要改用中断式 1ms 滴答，可在这里累加计数，并在初始化时调用
  *         SysTick_Config(SystemCoreClock / 1000U)。
  */
void SysTick_Handler(void)
{
}

/* ==========================================================================
 *                            外部中断服务函数
 * ========================================================================== */

/**
  * @brief  窗口看门狗
  */
void WWDG_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 11（AWU 自动唤醒）
  */
void EXTI11_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  FLASH 全局中断
  */
void FLASH_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  RCC 全局中断
  */
void RCC_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 0
  */
void EXTI0_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 1
  */
void EXTI1_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 2
  */
void EXTI2_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 3
  */
void EXTI3_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 4
  */
void EXTI4_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 5
  */
void EXTI5_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  TIM1 刹车中断
  */
void TIM1_BRK_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  ADC1 中断（与 EXTI Line 8 共用）
  */
void ADC1_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  TIM1 更新 / 触发 / 换相中断
  */
void TIM1_UP_TRG_COM_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  TIM1 捕获比较中断
  */
void TIM1_CC_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  TIM2 全局中断
  */
void TIM2_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  TIM6 全局中断
  */
void TIM6_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 6
  */
void EXTI6_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  EXTI Line 7
  */
void EXTI7_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  I2C1 事件中断（与 EXTI Line 10 共用）
  */
void I2C1_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  SPI1 全局中断
  */
void SPI1_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}

/**
  * @brief  USART1 全局中断（与 EXTI Line 9 共用）
  */
void USART1_IRQHandler(void)
{
    /* TODO: 在此添加处理代码 */
}
