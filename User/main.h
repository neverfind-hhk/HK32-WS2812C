/**
  ******************************************************************************
  * @file    main.h
  * @brief   HK32F030MF4P6 工程主头文件
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 引入设备头（按器件宏分发到 hk32f030m_2k.h） */
#include "hk32f030m.h"

/* 标准外设库配置头：引入全部外设模块（GPIO_InitTypeDef 等 API）。
   本工程的 hk32f030m.h 是按器件宏分发的 "umbrella" 头，不像原厂头那样
   在末尾 include conf.h，所以这里要显式引入。 */
#include "hk32f030m_conf.h"

/* ==========================================================================
 *                    板级参考 —— TSSOP20 引脚分配
 * ========================================================================== */

/* HK32F030MF4P6 为 TSSOP20 封装，可用 GPIO 共 16 个：
 *
 *     脚1   PD4                脚14  PC4 / AIN2 / WKUP0
 *     脚2   PD5 / SWDIO  ★     脚15  PC5 / EXTCLK4
 *     脚3   PD6 / AIN1         脚16  PC6
 *     脚5   PA1 / EXTCLK1      脚17  PC7
 *     脚6   PA2                脚18  PD1
 *     脚10  PA3                脚19  PD2 / AIN4
 *     脚11  PB5 / SWCLK  ★     脚20  PD3 / AIN3
 *     脚12  PB4                脚13  PC3
 *
 *   脚4 = NRST，脚7 = VSS/VSSA，脚8 = VCAP（接 1uF 到地），
 *   脚9 = VDD/VDDA，这四个不占 GPIO。
 *
 *   ★ 本芯片有 VCAP 引脚（脚8），需要接电容 —— 这一点和 HK32F0301MxxC
 *     系列不同（C 系列的脚8 是 PD7/EXTCLK2）。
 *
 *   ★ 脚2 PD5(SWDIO) 与 脚11 PB5(SWCLK) 是调试口，别拿去驱动外部器件，
 *     否则下次烧录会连不上。这对引脚【不是】STM32 常见的 PA13/PA14。
 *
 *   ★ 没有 PC13 ！GPIOC 只有 PC3 / PC4 / PC5 / PC6 / PC7。
 *
 *   本工程占用：PC5 = SPI1_SCK，PC6 = SPI1_MOSI（WS2812 数据线），
 *   PC4 = SPI1 软件片选（预留），PC7 未使用。
 */

/* ==========================================================================
 *                              对外接口
 * ========================================================================== */

/**
  * @brief  SysTick 轮询式毫秒延时（阻塞，不产生中断）
  * @param  ms  延时毫秒数
  * @note   依赖 SystemCoreClock（本工程 = 32MHz，见 hk32f030m_conf.h 中的
  *         HSI_VALUE），这个值错一位延时就不准。
  */
void Delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
