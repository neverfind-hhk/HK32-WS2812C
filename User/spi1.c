/**
  ******************************************************************************
  * @file    spi1.c
  * @brief   SPI1 主机驱动实现（只发送模式）—— HK32F030MF4P6
  ******************************************************************************
  */

#include "spi1.h"

/**
  * @brief  初始化 SPI1（只发送模式）与相关 GPIO
  */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    /* ------------------------------------------------------------------
     * 1. 开时钟：GPIO 走 AHB，SPI1 挂在 APB2
     * ------------------------------------------------------------------ */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    /* ------------------------------------------------------------------
     * 2. SCK / MOSI 配成复用推挽（只发送模式不占 MISO，PC7 保持未配置）
     * ------------------------------------------------------------------ */
    GPIO_InitStructure.GPIO_Pin    = SPI1_SCK_PIN | SPI1_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_Level_2;   /* 16MHz 必须用高速档 */
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd   = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Schmit = GPIO_Schmit_Enable;
    GPIO_Init(SPI1_GPIO_PORT, &GPIO_InitStructure);

    /* 复用功能号：SPI = AF2（见数据手册表 6-5） */
    GPIO_PinAFConfig(SPI1_GPIO_PORT, GPIO_PinSource5, GPIO_AF_2);   /* SCK  */
    GPIO_PinAFConfig(SPI1_GPIO_PORT, GPIO_PinSource6, GPIO_AF_2);   /* MOSI */

    /* ------------------------------------------------------------------
     * 3. 片选脚：普通推挽输出，默认拉高（驱动 WS2812 时用不到，留给普通从设备）
     * ------------------------------------------------------------------ */
    GPIO_InitStructure.GPIO_Pin    = SPI1_CS_PIN;
    GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd   = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Schmit = GPIO_Schmit_Enable;
    GPIO_Init(SPI1_CS_PORT, &GPIO_InitStructure);
    SPI1_CS_HIGH();

    /* ------------------------------------------------------------------
     * 4. SPI1 参数 —— 只发送模式
     * ------------------------------------------------------------------ */
    SPI_InitStructure.SPI_Direction         = SPI_Direction_1Line_Tx;  /* 只输出，不接收 */
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;      /* 模式 0 */
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;    /* 模式 0 */
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;      /* 片选软件管理 */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI1_PRESCALER;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial     = 7;                 /* 不用 CRC，给个合法值 */
    SPI_Init(SPI1, &SPI_InitStructure);

    /* ------------------------------------------------------------------
     * 5. 使能 SPI1
     * ------------------------------------------------------------------ */
    SPI_Cmd(SPI1, ENABLE);
}

/**
  * @brief  发送一个字节（只等发送缓冲空，不等待接收）
  * @param  tx  要发送的字节
  * @note   只写模式下 RXNE 不会有效，所以这里绝不去等它 —— 否则每字节
  *         都会跑满超时循环，把 WS2812 的位流切成碎帧。
  */
void SPI1_WriteByte(uint8_t tx)
{
    uint32_t timeout;

    timeout = SPI1_TIMEOUT;
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET)
    {
        if (--timeout == 0U)
        {
            return;                         /* 超时退出，别死等 */
        }
    }

    SPI_SendData8(SPI1, tx);
}

/**
  * @brief  等待移位寄存器发送完毕（BSY 清零）
  * @note   连续发送的最后一步必须调用，否则移位寄存器里最后几位还没
  *         移出去就切换了引脚，末位数据会被截断。
  */
void SPI1_WaitDone(void)
{
    uint32_t timeout;

    timeout = SPI1_TIMEOUT;
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_BSY) == SET)
    {
        if (--timeout == 0U)
        {
            return;
        }
    }
}

/**
  * @brief  连续发送若干字节（阻塞，发完自动等 BSY）
  * @param  buf  数据缓冲区
  * @param  len  字节数
  * @note   字节之间只有 SPI 移位时间（16MHz 下 0.5µs），位流天然连续，
  *         不会触发 WS2812 的 280µs 复位判断。
  */
void SPI1_Transmit(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if ((buf == 0) || (len == 0U))
    {
        return;
    }

    for (i = 0U; i < len; i++)
    {
        SPI1_WriteByte(buf[i]);
    }

    SPI1_WaitDone();                        /* 等最后一位真正移出 */
}