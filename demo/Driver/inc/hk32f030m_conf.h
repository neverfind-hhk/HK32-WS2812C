/**
  ******************************************************************************
  * @file    hk32f030m_conf.h
  * @brief   HK32F030M 配置文件 — 同时服务于标准库与 HAL 库.
  *
  *         hk32f030m.h 在末尾 #include 本文件。当定义了 HK32_HAL_DRIVER
  *         (在 platformio.ini build_flags 中) 时，本文件只提供振荡器值，
  *         不引入任何标准库头，从而避免标准库的 GPIO_InitTypeDef 等类型
  *         与 HAL 库的同名类型冲突。HAL 的模块开关与 assert_param 由
  *         hk32f030m_hal_conf.h 独立管理。
  *
  *         未定义 HK32_HAL_DRIVER 时，本文件维持标准库行为（引入标准库
  *         头 + assert_param），与改造前完全兼容。
  ******************************************************************************
  */

#ifndef __HK32F030M_CONF_H
#define __HK32F030M_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ########################## HSE/HSI/LSI Values ########################### */
/* 这些值在两种模式下都被 system_hk32f030m.c 和 RCC 驱动使用，必须始终可用。 */

/*!< Value of the External clock in Hz (HK32 names it EXTCLK, not HSE). */
#define EXTCLK_VALUE   ((uint32_t)32000000U)

/*!< Internal High Speed oscillator (HSI) value, in Hz. */
#define HSI_VALUE      ((uint32_t)32000000U)

/*!< HSI startup timeout. */
#define STARTUP_TIMEOUT ((uint32_t)0xFFFFU)

/*!< Internal Low Speed oscillator (LSI) value, in Hz (~114 kHz). */
#define LSI_VALUE      ((uint32_t)114000U)

/* ======================================================================
 * 标准库模式：原有行为，引入标准库外设头并定义 assert_param。
 * ====================================================================== */

 #include "hk32f030m_rcc.h"
 #include "hk32f030m_crc.h"
 #include "hk32f030m_exti.h"
 #include "hk32f030m_flash.h"
 #include "hk32f030m_gpio.h"
 #include "hk32f030m_misc.h"
 #include "hk32f030m_adc.h"
 #include "hk32f030m_syscfg.h"
 #include "hk32f030m_def.h"
 #include "hk32f030m_i2c.h"
 #include "hk32f030m_iwdg.h"
 #include "hk32f030m_pwr.h"
 #include "hk32f030m_spi.h"
 #include "hk32f030m_tim.h"
 #include "hk32f030m_usart.h"
 #include "hk32f030m_wwdg.h"
 #include "hk32f030m_awu.h"
 #include "hk32f030m_beep.h"

 /* #define USE_FULL_ASSERT   (1U) */
 #ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((char *)__FILE__, __LINE__))
  void assert_failed(char* file, uint32_t line);
 #else
  #define assert_param(expr) ((void)0U)
 #endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __HK32F030M_CONF_H */
