/**
  ******************************************************************************
  * @file    spi1.h
  * @brief   SPI1 主机驱动（**只发送模式**）—— HK32F030MF4P6（TSSOP20）
  *
  *          引脚分配（依据数据手册表 6-5，SPI 复用编号 AF2）：
  *
  *            信号   引脚        配置
  *            SCK    PC5 (脚15)  AF2 → SPI_SCK
  *            MOSI   PC6 (脚16)  AF2 → SPI_MOSI   ← WS2812 数据线
  *            CS     PC4 (脚14)  普通 GPIO 推挽输出（软件片选）
  *
  * @note    ★ 本工程使用「只发送模式」(SPI_Direction_1Line_Tx)。
  *
  *          WS2812 是单向器件，不返回任何数据，因此：
  *            · 不需要 MISO —— PC7 不再配置为 SPI，可释放作其他用途
  *            · 不需要等 RXNE —— 这一点非常关键：
  *              MISO 悬空时 RXNE 永远不会置位，如果还去等它，
  *              每次发送都要跑满超时循环（实测每字节约 6ms），
  *              120 字节累计 700ms 以上，远超 WS2812 的 280µs 复位门限，
  *              整个位流会被切成一个个碎帧，灯完全点不亮。
  *
  *          发送完最后一个字节后必须等 BSY 清零（SPI1_WaitDone），
  *          否则移位寄存器里最后几位还没出去就切了引脚，末位数据会丢。
  ******************************************************************************
  */

#ifndef __SPI1_H
#define __SPI1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hk32f030m.h"
/* 标准外设库配置头 —— 本工程的 hk32f030m.h 是 "umbrella" 头，不像原厂头
   那样在末尾 include conf.h，所以凡是只 include 设备头的地方都要再显式
   引入它，否则拿不到 GPIO_InitTypeDef / SPI_InitTypeDef 等标准库类型。 */
#include "hk32f030m_conf.h"

/* ==========================================================================
 *                          引脚定义（按需修改）
 * ========================================================================== */

#define SPI1_GPIO_PORT      GPIOC
#define SPI1_SCK_PIN        GPIO_Pin_5      /* 脚15 */
#define SPI1_MOSI_PIN       GPIO_Pin_6      /* 脚16 —— WS2812 数据线 */

/* 软件片选：普通 GPIO 输出，低电平有效（驱动 WS2812 时用不到，留着给普通 SPI 从设备） */
#define SPI1_CS_PORT        GPIOC
#define SPI1_CS_PIN         GPIO_Pin_4      /* 脚14 */

#define SPI1_CS_LOW()       GPIO_ResetBits(SPI1_CS_PORT, SPI1_CS_PIN)
#define SPI1_CS_HIGH()      GPIO_SetBits(SPI1_CS_PORT, SPI1_CS_PIN)

/* 波特率分频（本芯片 32MHz）：
 *   _2 → 16MHz（超出 GPIO 能力，勿用）  _4 → 8MHz（本工程）  _8 → 4MHz  _16 → 2MHz
 *   _32 → 1MHz  _64 → 500kHz  _128 → 250kHz  _256 → 125kHz
 *
 * ★ 本工程固定 8MHz —— WS2812C 每 bit 占 10 个 SPI bit，
 *   10 / 8MHz = 1.25µs，正好等于 WS2812C 的位周期。
 *   不能更快：本芯片 GPIO 最高只有 10MHz 翻转能力（GPIO_Speed_Level_2），
 *   配 16MHz 会导致 MOSI 边沿严重变缓，WS2812 会把 0 码误判成 1 码（全白）。
 *   改这个分频会直接破坏 WS2812 时序。 */
#define SPI1_PRESCALER      SPI_BaudRatePrescaler_4

/* 超时计数（防止标志位卡死导致死循环） */
#define SPI1_TIMEOUT        0x10000U

/* ==========================================================================
 *                              对外接口
 * ========================================================================== */

/**
  * @brief  初始化 SPI1（只发送模式）与相关 GPIO
  * @note   CS 引脚上电默认拉高
  */
void SPI1_Init(void);

/**
  * @brief  发送一个字节（只等发送缓冲空，不等待接收）
  * @param  tx  要发送的字节
  * @note   返回时数据已写入数据寄存器，但可能仍在移位，需要时用
  *         SPI1_WaitDone() 等它真正发完。
  */
void SPI1_WriteByte(uint8_t tx);

/**
  * @brief  等待移位寄存器发送完毕（BSY 清零）
  * @note   连续发送的最后一步必须调用，否则最后几位会被截断
  */
void SPI1_WaitDone(void);

/**
  * @brief  连续发送若干字节（阻塞，发完自动等 BSY）
  * @param  buf  数据缓冲区
  * @param  len  字节数
  */
void SPI1_Transmit(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SPI1_H */