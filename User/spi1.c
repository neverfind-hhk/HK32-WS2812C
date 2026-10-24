/**
  ******************************************************************************
  * @file    spi1.c
  * @brief   SPI1 主机驱动实现（只发送模式 + 10 位帧）—— HK32F030MF4P6
  ******************************************************************************
  */

#include "spi1.h"

/**
  * @brief  初始化 SPI1（只发送模式 + 10 位帧）与相关 GPIO
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
    GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_Level_2;   /* 最高档也只有 10MHz，故 SPI 取 8MHz */
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
     * 4. SPI1 参数 —— 只发送模式 + 10 位帧
     * ------------------------------------------------------------------ */
    SPI_InitStructure.SPI_Direction         = SPI_Direction_1Line_Tx;  /* 只输出，不接收 */
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize          = SPI1_DATA_SIZE;   /* 10 位 = 一个 WS2812 码 */
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
  * @brief  写入一帧（只等发送缓冲空，不等待接收）
  * @param  frame  帧数据，只有低 SPI1_DATA_BITS 位有效
  * @note   只写模式下 RXNE 不会有效，所以这里绝不去等它 —— 否则每帧
  *         都会跑满超时循环，把 WS2812 的位流切成碎帧。
  */
void SPI1_WriteFrame(uint16_t frame)
{
    uint32_t timeout;

    /* ★ 直接读寄存器，不走 SPI_GetFlagStatus() / SPI_SendData8() 函数调用 ——
     *   那两个函数每次调用约 15 个 CPU 周期，加上外层函数调用开销，
     *   每帧会多出约 1μs 空隙，把 WS2812 的低电平拉长。 */
    timeout = SPI1_TIMEOUT;
    while ((SPI1->SR & SPI_SR_TXE) == 0U)
    {
        if (--timeout == 0U)
        {
            return;                         /* 超时退出，别死等 */
        }
    }

    SPI1->DR = (uint32_t)frame;
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
    while ((SPI1->SR & SPI_SR_BSY) != 0U)
    {
        if (--timeout == 0U)
        {
            return;
        }
    }
}

/**
  * @brief  连续发送若干帧（阻塞，发完自动等 BSY）
  * @param  frames  帧数组，每个元素一帧（低 SPI1_DATA_BITS 位有效）
  * @param  num     帧数
  *
  * @note   ★ 每帧 10 位 = 一个 WS2812 码（8MHz 下 1.25μs）。因为帧边界
  *         一定落在码边界上，高电平永远不会跨帧被劈开；帧与帧之间的
  *         任何空隙只会把低电平拉长，而低电平拉长对 WS2812 无害
  *         （只要远小于 280μs 复位门限）。这是结构性保证。
  *
  * @note   这里刻意没有加 __disable_irq()：本方案下即使被中断打断，
  *         最坏也只是把低电平拉长几十 μs，仍远低于 280μs 复位门限，
  *         不会破坏数据。若日后需要绝对确定，可在前后自行加
  *         __get_PRIMASK() / __disable_irq() / __set_PRIMASK() 关中断。
  *
  * @note   本函数是热点：每帧只有 40 个 CPU 周期（1.25μs @ 32MHz），
  *         建议用 -O1/-O2 以上优化等级编译。即便 -O0 跟不上，
  *         也只是低电平变长，不会破坏数据。
  */
void SPI1_TransmitFrames(const uint16_t *frames, uint16_t num)
{
    uint16_t i = 0U;

    if ((frames == 0) || (num == 0U))
    {
        return;
    }

    while (i < num)
    {
        while ((SPI1->SR & SPI_SR_TXE) == 0U)
        {
        }
        SPI1->DR = (uint32_t)frames[i];
        i++;
    }

    while ((SPI1->SR & SPI_SR_BSY) != 0U)
    {
    }
}
