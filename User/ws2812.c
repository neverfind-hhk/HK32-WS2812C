/**
  ******************************************************************************
  * @file    ws2812.c
  * @brief   WS2812 / WS2812B 驱动实现（SPI 5bit 编码，阻塞发送）
  ******************************************************************************
  */

#include "ws2812.h"

/* ==========================================================================
 *                              内部数据
 * ========================================================================== */

/* 10bit 波形常量（WS2812C-2020 专用，SPI 8MHz → 1 bit = 125ns）
 *   0 码 = 1100000000 → 高 250ns / 低 1000ns
 *   1 码 = 1111100000 → 高 625ns / 低 625ns
 *
 * 为什么 0 码取「2 高 8 低」而不是「3 高 7 低」：
 *   边沿变缓时，因 VIH(2.7V=54%)/VIL(0.7V=14%) 不对称，高电平会被拉长约
 *   0.16×(tr+tf)。若取 375ns，加边沿后约 381ns 会突破 380ns 上限，
 *   0 码被误判成 1 码（表现为设 000 却亮白）。取 250ns 加边沿约 256ns，
 *   离上限还有 124ns 余量。
 *
 * 对照手册：T0H 220~380 / T0L 580~1000 / T1H 580~1000 / T1L 580~1000 ns */
#define WS2812_PAT_0        0x0300UL
#define WS2812_PAT_1        0x03E0UL

/* 像素缓冲区，按 WS2812 的物理顺序 GRB 存放（每个 LED 3 字节） */
static uint8_t s_grb[WS2812_LED_MAX * 3U];

/* 单个 LED 编码后的 SPI 数据：24 bit x 10 bit = 240 bit = 30 字节 */
static uint8_t s_spi_buf[30];

/* ==========================================================================
 *                              内部函数
 * ========================================================================== */

/**
  * @brief  微秒级延时（SysTick 轮询，不产生中断）
  * @param  us  延时微秒数
  * @note   与 main.c 的 Delay_ms 共用 SysTick 定时器，顺序调用没问题
  */
static void WS2812_DelayUs(uint32_t us)
{
    uint32_t reload;

    if ((us == 0U) || (SystemCoreClock == 0U))
    {
        return;
    }

    reload = (SystemCoreClock / 1000000U) - 1U;

    SysTick->LOAD = reload;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (us != 0U)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
            /* 等一次 1µs 计数溢出 */
        }
        us--;
    }

    SysTick->CTRL = 0U;
    SysTick->VAL  = 0U;
}

/**
  * @brief  把 24bit 颜色编码成 15 字节 SPI 数据
  * @param  g    绿色
  * @param  r    红色
  * @param  b    蓝色
  * @param  out  输出缓冲（15 字节）
  *
  * @note   每个 WS2812 位 → 5 个 SPI 位：
  *           数据 0 → 11000（0x18），高 0.50µs / 低 0.75µs
  *           数据 1 → 11100（0x1C），高 0.75µs / 低 0.50µs
  *         按 WS2812 规范，颜色按 G→R→B、每字节 MSB 先行。
  */
static void WS2812_Encode(uint8_t g, uint8_t r, uint8_t b, uint8_t *out)
{
    uint32_t data;
    uint32_t pattern;
    uint8_t  byte   = 0U;
    uint8_t  bitpos = 0U;
    int8_t   i;
    int8_t   k;

    data = ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;

    for (i = 23; i >= 0; i--)
    {
        /* 取当前数据位，映射成 20bit 波形（WS2812C 专用） */
        pattern = ((data & (1UL << (uint8_t)i)) != 0UL) ? WS2812_PAT_1 : WS2812_PAT_0;

        /* 高位先出 */
        for (k = 9; k >= 0; k--)
        {
            byte = (uint8_t)((byte << 1) | ((pattern >> (uint8_t)k) & 0x01UL));

            if (++bitpos == 8U)
            {
                *out++  = byte;
                byte    = 0U;
                bitpos  = 0U;
            }
        }
    }
}

/**
  * @brief  产生 WS2812 复位信号（数据线保持低电平）
  * @note   WS2812B 要求复位低电平 >280µs。先把 MOSI 脚临时切成普通
  *         GPIO 输出低，保证这段时间绝对是低电平（SPI 关闭后引脚状态
  *         不确定，直接靠 SPI 空闲电平不可靠），延时完再切回复用。
  */
static void WS2812_Reset(void)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin    = SPI1_MOSI_PIN;
    gpio.GPIO_Mode   = GPIO_Mode_OUT;
    gpio.GPIO_Speed  = GPIO_Speed_Level_2;
    gpio.GPIO_OType  = GPIO_OType_PP;
    gpio.GPIO_PuPd   = GPIO_PuPd_NOPULL;
    gpio.GPIO_Schmit = GPIO_Schmit_Enable;

    GPIO_Init(SPI1_GPIO_PORT, &gpio);
    GPIO_ResetBits(SPI1_GPIO_PORT, SPI1_MOSI_PIN);

    WS2812_DelayUs(300U);               /* >280µs 即可，留点余量 */

    /* 切回 SPI 复用功能 */
    gpio.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(SPI1_GPIO_PORT, &gpio);
    GPIO_PinAFConfig(SPI1_GPIO_PORT, GPIO_PinSource6, GPIO_AF_2);
}

/* ==========================================================================
 *                              对外接口
 * ========================================================================== */

/**
  * @brief  初始化：清零缓冲区 + 初始化 SPI1（4MHz）
  */
void WS2812_Init(void)
{
    uint16_t i;

    for (i = 0U; i < (uint16_t)sizeof(s_grb); i++)
    {
        s_grb[i] = 0U;
    }

    SPI1_Init();                        /* 16MHz 只发送模式，见 spi1.h */
}

/**
  * @brief  设置单个像素颜色（只改缓冲，不发送）
  */
void WS2812_SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx >= WS2812_LED_MAX)
    {
        return;
    }

    s_grb[idx * 3U + 0U] = g;           /* WS2812 物理顺序是 G → R → B */
    s_grb[idx * 3U + 1U] = r;
    s_grb[idx * 3U + 2U] = b;
}

/**
  * @brief  把所有像素填成同一颜色（只改缓冲，不发送）
  */
void WS2812_Fill(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t i;

    for (i = 0U; i < WS2812_LED_MAX; i++)
    {
        WS2812_SetPixel(i, r, g, b);
    }
}

/**
  * @brief  编码并发送全部像素数据
  */
void WS2812_Show(void)
{
    uint16_t i;

    for (i = 0U; i < WS2812_LED_MAX; i++)
    {
        WS2812_Encode(s_grb[i * 3U + 0U],
                      s_grb[i * 3U + 1U],
                      s_grb[i * 3U + 2U],
                      s_spi_buf);

        /* 阻塞发送，保证波形连续不被打断 */
        SPI1_Transmit(s_spi_buf, 30U);
    }

    /* 整帧发完，产生复位低电平让灯珠锁存 */
    WS2812_Reset();
}
