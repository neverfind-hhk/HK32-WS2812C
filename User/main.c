/**
  ******************************************************************************
  * @file    main.c
  * @brief   HK32F030MF4P6 + 2 颗 WS2812C-2020 最小示例
  *
  *          时钟：HSI 32MHz，SYSCLK = HCLK = PCLK，由 SystemInit() 在
  *          Reset_Handler 里配置完成，进入 main() 时已经就绪，无需手工配置。
  *
  *          程序流程：
  *            1. WS2812_Init()  —— 内部完成 SPI1 初始化（8MHz、10 位帧、只发送）
  *            2. 先全灭一次，清掉上电可能残留的旧数据
  *            3. 进入循环，两颗灯每 500ms 换一个颜色
  *
  *          ★ 注意第 0 颗灯目前故意把 R / B 通道对调后再发送，第 1 颗灯按
  *            正常 RGB 参数发送，用来对比确认 WS2812C 到底是 GRB 还是 RGB
  *            顺序。颜色顺序定下来之后，把第 0 颗那三行改成和第 1 颗一样
  *            的写法即可。
  ******************************************************************************
  */

#include "main.h"
#include "ws2812.h"

/* 换色用的色表 */
static const uint8_t s_color[14][3] =
{
    { 255U,   0U,   0U },       /* 0 */
		{ 255U,   128U,   0U },			/* 30 */
		{ 255U,   255U,   0U },			/* 60 */
		{ 128U,   255U,   0U },			/* 90 */
		{ 0U,   255U,   0U },				/* 120 */
		{ 0U,   255U,   128U },			/* 150 */
		{ 0U,   255U,   255U },			/* 180 */
		{ 0U,   128U,   255U },			/* 210 */
    {   0U, 0U,   255U },       /* 240 */
    {   128U,   0U, 255U },     /* 270 */
    {   255U,   0U, 255U },     /* 300 */		
    {   255U,   0U, 128U },     /* 330 */		
    { 255U, 255U, 255U },       /* 白 */
    {   0U,   0U,   0U }        /* 灭 */
};

/* 当前色表索引 */
static uint8_t s_color_step = 0U;

/**
  * @brief  主函数
  * @param  无
  * @retval 正常情况不会返回
  */
int main(void)
{
    /* WS2812_Init() 内部会调用 SPI1_Init()，不需要再单独初始化 SPI */
    WS2812_Init();

    /* 上电先全灭一次，确保两颗灯都不会残留旧数据 */
    WS2812_Fill(0, 0, 0);
    WS2812_Show();
    Delay_ms(200);

    while (1)
    {
        /* 第 0 颗：R / B 对调后发送（颜色顺序对比用，定下来后改成正常写法） */
        WS2812_SetPixel(0, s_color[s_color_step][2],
                           s_color[s_color_step][1],
                           s_color[s_color_step][0]);

        /* 第 1 颗：按正常 RGB 参数发送 */
        WS2812_SetPixel(1, s_color[s_color_step][0],
                           s_color[s_color_step][1],
                           s_color[s_color_step][2]);

        WS2812_Show();                  /* 发送整帧 + 自动产生复位脉冲 */

        s_color_step++;
        if (s_color_step >= 14U)
        {
            s_color_step = 0U;
        }

        Delay_ms(500);
    }
}

/**
  * @brief  SysTick 轮询式毫秒延时（阻塞，不产生中断）
  * @param  ms  延时毫秒数
  * @note   用轮询 COUNTFLAG 的方式，不开启 SysTick 中断，因此与
  *         hk32f030m_it.c 里的 SysTick_Handler 互不干扰；但两者共用
  *         SysTick 定时器，若改用中断式滴答就不要同时调用本函数。
  */
void Delay_ms(uint32_t ms)
{
    uint32_t reload;

    if ((ms == 0U) || (SystemCoreClock == 0U))
    {
        return;
    }

    reload = (SystemCoreClock / 1000U) - 1U;

    SysTick->LOAD = reload;
    SysTick->VAL  = 0U;
    /* 使用内核时钟、开启计数、不开中断 */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms != 0U)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
            /* 等待一次 1ms 计数溢出 */
        }
        ms--;
    }

    SysTick->CTRL = 0U;
    SysTick->VAL  = 0U;
}
