/**
  ******************************************************************************
  * @file    hk32f030m_conf.h
  * @brief   工程级标准外设库配置 —— 目标器件 HK32F030MF4P6
  *
  *          本文件由 CMSIS 设备头 hk32f030m.h 在末尾 #include，是整份标准库
  *          的总开关：振荡器频率、断言宏、各外设模块头都在这里汇聚。
  *
  * @note    ★★ HSI 频率必须与芯片一致，否则一切都是错的 ★★
  *
  *          HK32F030MF4P6 ：HSI = 32MHz ← 本工程
  *          HK32F0301MF4P6：HSI = 48MHz
  *
  *          原厂 Pack 自带的 system_hk32f030m.c 是按 32MHz 写的，其中的
  *          SetSysClockToHSI_32M() 只负责使能 HSI、设置 Flash 等待周期与
  *          分频，并不改写 HSI 基频 —— HSI 基频由芯片本身决定。所以真正
  *          决定 SystemCoreClock 数值、进而决定 Delay、USART 波特率、
  *          ADC 采样时间、I2C 时序换算的，就是下面这个 HSI_VALUE。
  *
  *          本工程配套的原厂配置（FLASH_Latency_2 + FLITFCLK_PRE=0x07）
  *          正是为 32MHz/2MHz 的 FLITFCLK 设计的，与芯片手册
  *          "HK32F030M 数据手册 V2.4 修订记录 1.7：FLITFCLK 为 2MHz" 一致，
  *          无需改动。
  *
  *          若日后换回 HK32F0301MF4P6（48MHz、SRAM 4KB），除把下面
  *          HSI_VALUE 改为 48000000U 外，还要在 Keil 里把器件改回去。
  ******************************************************************************
  */

#ifndef __HK32F030M_CONF_H
#define __HK32F030M_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ########################## HSE/HSI/LSI Values ########################### */

/*!< 外部时钟频率（航顺命名为 EXTCLK，对应通常说的 HSE），单位 Hz。
     没有外接晶振时该值仅作占位，不参与默认时钟配置。 */
#define EXTCLK_VALUE   ((uint32_t)32000000U)

/* ########################## 芯片选择与时钟 ############################### */

/* 本工程用一个宏切换两种芯片，**改芯片时不需要动这个文件**。
 * 器件宏由 Keil 工程里的 Target 决定（Options for Target → C/C++ → Define）：
 *
 *    Target  "HK32F030M_F4P6"    →  Define: HK32F030MF4P6
 *    Target  "HK32F0301M_F4P7C"  →  Define: HK32F0301MF4P7C
 *
 * 下面按器件宏自动选振荡器频率，因此换 Target 后时钟值会自己跟着变。
 *
 *   HK32F030MF4P6   ：HSI 32MHz，LSI 114kHz，SRAM 2KB
 *   HK32F0301MF4P7C ：HSI 48MHz，LSI  60kHz，SRAM 4KB
 *
 * 其余外设差异（BEEP / EEPROM / I2S / UART 数量等）由 CMSIS 的 umbrella
 * 头 hk32f030m.h 按同一个器件宏自动分发到 hk32f030m_2k.h 或
 * hk32f0301mxxc.h，同样无需手工处理。
 */
#if defined(HK32F0301MF4P7C) || defined(HK32F0301MxxC)
  #define HK32_CHIP_NAME    "HK32F0301MF4P7C"
  /*!< 内部高速振荡器 HSI 频率，Hz（C 系列 = 48MHz） */
  #define HSI_VALUE         ((uint32_t)48000000U)
  /*!< 内部低速振荡器 LSI 频率，Hz（C 系列 = 60kHz） */
  #define LSI_VALUE         ((uint32_t)60000U)
#elif defined(HK32F030MF4P6) || defined(HK32F030M) || defined(HK32F030MD4P6) || defined(HK32F030MJ4M6) || defined(HK32F030MF4U6)
  #define HK32_CHIP_NAME    "HK32F030MF4P6"
  /*!< 内部高速振荡器 HSI 频率，Hz（F030M 系列 = 32MHz） */
  #define HSI_VALUE         ((uint32_t)32000000U)
  /*!< 内部低速振荡器 LSI 频率，Hz（F030M 系列 = 114kHz） */
  #define LSI_VALUE         ((uint32_t)114000U)
#else
  #error "未选择芯片！请在 Keil 的 Options->C/C++->Define 里定义 HK32F030MF4P6 或 HK32F0301MF4P7C"
#endif

/*!< HSI 启动等待超时计数。 */
#define STARTUP_TIMEOUT ((uint32_t)0xFFFFU)


/* ########################## 外设模块头文件 ############################### */

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
#include "hk32f030m_dbgmcu.h"

/* ########################## 断言 ######################################### */

/* 需要参数合法性检查时，取消下面一行的注释，并在工程里实现 assert_failed() */
/* #define USE_FULL_ASSERT   (1U) */

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((char *)__FILE__, __LINE__))
  void assert_failed(char *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __HK32F030M_CONF_H */
