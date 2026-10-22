/**
  ******************************************************************************
  * @file    ws2812.h
  * @brief   WS2812 / WS2812B 驱动（SPI 编码，阻塞发送）
  *
  *          为什么用 SPI：WS2812 要的是 0.4µs / 0.8µs 级别的高电平宽度，
  *          SPI 可以把它"编码"成固定位宽的串行波形，精度由时钟分频决定，
  *          比软件 GPIO 翻转 + 循环延时稳得多。
  *
  *          编码方案（5 bit 表示 1 bit）：
  *
  *            SPI 时钟 = 32MHz / 8 = 4MHz，1 个 SPI bit = 0.25µs
  *            每个 WS2812 bit 用 5 个 SPI bit，合计 1.25µs（= WS2812 周期）
  *
  *              数据 0 → 11000 → T0H = 0.50µs，T0L = 0.75µs
  *              数据 1 → 11100 → T1H = 0.75µs，T1L = 0.50µs
  *
  *            对照 WS2812B 手册（T0H 0.25~0.55 / T0L 0.70~1.00 /
  *            T1H 0.65~0.95 / T1L 0.30~0.60µs），四个值都落在容差中段。
  *
  * @note    ★ 本工程用阻塞方式发送，不是 DMA 也不是中断。
  *            HK32F030 全系列都没有 DMA 外设（SVD 里 24 个外设无 DMA），
  *          而 SPI 的 FIFO 只有 4 层（约 8µs 缓冲），中断一旦延迟超时就会
  *          造成数据流中断 —— WS2812 对连续性极敏感，断流会直接花屏。
  *          阻塞发送能保证波形绝对连续，代价是发送期间 CPU 被占用：
  *          每个 LED 约 30µs，30 个 LED 约 450µs。
  ******************************************************************************
  */

#ifndef __WS2812_H
#define __WS2812_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hk32f030m.h"
#include "hk32f030m_conf.h"
#include "spi1.h"

/* 支持的最大 LED 数 —— 每个 LED 占 3 字节缓冲（GRB），
 * 32 个 = 96 字节。SRAM 只有 2KB，调大前先看看余量。 */
#define WS2812_LED_MAX      2

/**
  * @brief  初始化：清零缓冲区 + 初始化 SPI1（4MHz）
  * @note   会调用 SPI1_Init()，无需另外初始化 SPI
  */
void WS2812_Init(void);

/**
  * @brief  设置单个像素颜色（只改缓冲，不发送）
  * @param  idx  像素索引，0 ~ WS2812_LED_MAX-1
  * @param  r    红色 0~255
  * @param  g    绿色 0~255
  * @param  b    蓝色 0~255
  * @note   注意 WS2812 的物理顺序是 GRB，本函数内部已处理，按 RGB 传即可
  */
void WS2812_SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);

/**
  * @brief  把所有像素填成同一颜色（只改缓冲，不发送）
  */
void WS2812_Fill(uint8_t r, uint8_t g, uint8_t b);

/**
  * @brief  把缓冲区数据编码并通过 SPI 发送出去
  * @note   阻塞执行，发送完会额外产生 >300µs 的复位低电平
  */
void WS2812_Show(void);

#ifdef __cplusplus
}
#endif

#endif /* __WS2812_H */
