/**
  ******************************************************************************
  * @file    spi1.h
  * @brief   SPI1 主机驱动（**只发送模式 + 10 位帧**）—— HK32F030MF4P6（TSSOP20）
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
  *          发送完最后一个帧后必须等 BSY 清零（SPI1_WaitDone 或
  *          SPI1_TransmitFrames 内部），否则移位寄存器里最后几位还没出去
  *          就切了引脚，末位数据会丢。
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
 * ★ 本工程固定 8MHz —— WS2812C 每个码占 10 个 SPI 位，
 *   10 / 8MHz = 1.25μs，正好等于 WS2812C 的位周期。
 *   不能更快：本芯片 GPIO 最高只有 10MHz 翻转能力（GPIO_Speed_Level_2），
 *   配 16MHz 会导致 MOSI 边沿严重变缓，WS2812 会把 0 码误判成 1 码（全白）。
 *   改这个分频会直接破坏 WS2812 时序。 */
#define SPI1_PRESCALER      SPI_BaudRatePrescaler_4

/* ==========================================================================
 *                      帧宽定义 —— 本驱动最关键的一处
 * ========================================================================== */

/* 本芯片 SPI 的 CR2.DS[3:0] 是「数据位宽」字段，支持 4~16 位，
 * **不是**常见的那种只有 8 / 16 两档的 DFF 位。标准外设库也如实提供了
 * SPI_DataSize_4b ~ SPI_DataSize_16b 全部 13 个选项。
 *
 * 这里设成 10 位：一个 SPI 帧 = 一个 WS2812 码。
 *
 *     10 bit / 8MHz = 1.25μs = WS2812C 的位周期   （0 码、1 码都是一帧）
 *
 * ★ 为什么必须按「码」发，而不是按「字节」发：
 *
 *   按字节发时，一个 10 位的 WS2812 码必然跨在字节边界上。以 0 码
 *   1100000000 为例：前 8 位落在 byte N，后 2 位落在 byte N+1。
 *   一旦 CPU 在字节边界换填 FIFO 慢了一拍，MOSI 就会掉下来，
 *   把正在输出的高电平劈成两段。
 *
 *   实测证据：某帧里唯一一个 1 码的高电平位于 SPI 位 [230..234]，
 *   正好跨在 byte28(0x03)/byte29(0xE0) 的边界(bit232)上，示波器上
 *   看得到 625ns 被切成 250ns + 375ns，整帧高脉冲从 48 个变成 49 个，
 *   WS2812 由此错位，第二颗灯收到错误的字节。
 *
 *   按码发之后，每个码的高电平都完整落在自己那一帧的开头，帧与帧之间
 *   的空隙只会把「低电平」拉长。而低电平拉长对 WS2812 无害（只要远小于
 *   280μs 复位门限），所以即使 CPU 偶尔慢一拍、甚至被中断打断，
 *   也**不可能**再破坏高电平宽度。这是结构性保证，不是靠调优凑出来的。
 */
#define SPI1_DATA_BITS      10U
#define SPI1_DATA_SIZE      SPI_DataSize_10b

/* 帧宽不足 9 位时不能按 uint16_t 写 DR，直接编译期拦下 */
#if (SPI1_DATA_BITS < 9U) || (SPI1_DATA_BITS > 16U)
#error "SPI1_DATA_BITS 必须在 9~16 之间，才能用 uint16_t 写入 DR"
#endif

/* 超时计数（防止标志位卡死导致死循环） */
#define SPI1_TIMEOUT        0x10000U

/* ==========================================================================
 *                              对外接口
 * ========================================================================== */

/**
  * @brief  初始化 SPI1（只发送模式 + 10 位帧）与相关 GPIO
  * @note   CS 引脚上电默认拉高
  * @note   ★ 初始化后 SPI1 是 10 位帧，不能再当普通 8 位 SPI 用。
  *         要接普通 SPI 从设备，把 SPI1_DATA_BITS 改回 8 即可。
  */
void SPI1_Init(void);

/**
  * @brief  写入一帧（只等发送缓冲空，不等待接收）
  * @param  frame  帧数据，只有低 SPI1_DATA_BITS 位有效
  * @note   返回时数据已进 FIFO，但可能仍在移位；连续发送请用
  *         SPI1_TransmitFrames()，最后用 SPI1_WaitDone() 收尾。
  */
void SPI1_WriteFrame(uint16_t frame);

/**
  * @brief  等待移位寄存器发送完毕（BSY 清零）
  * @note   连续发送的最后一步必须调用，否则最后几位会被截断
  */
void SPI1_WaitDone(void);

/**
  * @brief  连续发送若干帧（阻塞，发完自动等 BSY）
  * @param  frames  帧数组，每个元素一帧（低 SPI1_DATA_BITS 位有效）
  * @param  num     帧数
  * @note   WS2812 专用：每帧 10 位 = 一个码，所以帧边界一定落在码边界上，
  *         不会出现高电平跨帧被劈开的情况。
  */
void SPI1_TransmitFrames(const uint16_t *frames, uint16_t num);

#ifdef __cplusplus
}
#endif

#endif /* __SPI1_H */
