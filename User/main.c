/**
  ******************************************************************************
  * @file    main.c
  * @brief   HK32F030MF4P6 主程序
  *
  *          时钟：HSI 32MHz 直接作为 SYSCLK，HCLK = SYSCLK，PCLK = HCLK。
  *          该配置由启动文件的 Reset_Handler 自动调用 SystemInit() 完成，
  *          进入 main() 时时钟已经就绪，不需要再手工配置。
  *
  * @note    上电流程：
  *            Reset_Handler  ->  SystemInit()  ->  __main  ->  main()
  *          其中 __main 负责散列加载（.data 搬运、.bss 清零），
  *          所以 main() 里可以直接使用已初始化的全局变量。
  ******************************************************************************
  */

#include "main.h"
#include "spi1.h"
#include "ws2812.h"

/* 演示开关 ---------------------------------------------------------------- */
/* 单独使用 SPI1（接普通 SPI 器件时置 1；接了 WS2812 就置 0，两者别同时开） */
#define SPI_DEMO_ENABLE     0
/* 驱动 WS2812 灯带（WS2812_Init 内部会初始化 SPI1） */
#define WS2812_DEMO_ENABLE  1

/* 私有函数声明 --------------------------------------------------------------*/
#if (LED_DEMO_ENABLE == 1)
static void LED_Init(void);
#endif

/**
  * @brief  主函数
  * @param  无
  * @retval 正常情况不会返回
  */
int main(void)
{
    /* ------------------------------------------------------------------
     * 需要确认实际时钟时可打开这段，把配置读回来核对：
     *
     *   RCC_ClocksTypeDef RCC_Clocks;
     *   RCC_GetClocksFreq(&RCC_Clocks);
     *   RCC_Clocks.SYSCLK_Frequency 应为 32000000
     *   RCC_Clocks.HCLK_Frequency   应为 32000000
     *   RCC_Clocks.PCLK_Frequency   应为 32000000
     * ------------------------------------------------------------------ */

#if (LED_DEMO_ENABLE == 1)
    LED_Init();
#endif

    /* 用户初始化区 ------------------------------------------------ */

#if (WS2812_DEMO_ENABLE == 1)
    /* WS2812C-2020：数据线走 PC6（脚16 = SPI1_MOSI）
     * 编码 20bit @ 16MHz，每颗 LED 约 30us，阻塞发送。详见 ws2812.h */
    WS2812_Init();

    /* 两颗灯分别设成不同颜色 —— 这样一眼就能看出两颗是否都收到数据：
     *   第 0 颗 = 红，第 1 颗 = 绿
     * 若只亮第 0 颗，说明数据没发到第二颗（先查 DIN/DO 级联接线）。 */
    WS2812_SetPixel(0, 0, 0, 0);          /* 红 */
    WS2812_SetPixel(1, 0, 0, 0);          /* 绿 */
    WS2812_Show();
#elif (SPI_DEMO_ENABLE == 1)
    SPI1_Init();
#endif

    /* ------------------------------------------------------------ */

    while (1)
    {
#if (LED_DEMO_ENABLE == 1)
        GPIO_Toggle(LED_GPIO_PORT, LED_GPIO_PIN);
#endif

#if (WS2812_DEMO_ENABLE == 1)
        /* 两颗灯已在上面的初始化区点亮并保持常亮。
         * 需要动效时，把下面几行取消注释（红绿交替 / 全亮全灭循环）：
         *
         *   WS2812_SetPixel(0, 128, 0, 0); WS2812_SetPixel(1, 0, 128, 0);
         *   WS2812_Show(); Delay_ms(500);
         *   WS2812_SetPixel(0, 0, 128, 0); WS2812_SetPixel(1, 128, 0, 0);
         *   WS2812_Show(); Delay_ms(500);
         */
        Delay_ms(500);		
#else
        Delay_ms(10000);
#endif
    }
}

#if (LED_DEMO_ENABLE == 1)
/**
  * @brief  LED 引脚初始化
  * @note   引脚定义在 main.h 顶部，请按原理图修改。
  */
static void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIO 端口时钟 */
    RCC_AHBPeriphClockCmd(LED_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin    = LED_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_OUT;      /* 推挽输出 */
    GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_Level_1; /* 低速 2MHz 够用 */
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd   = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Schmit = GPIO_Schmit_Enable;

    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

    /* 默认熄灭（按需改成 GPIO_ResetBits 以适配低电平点灯的板子） */
    GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN);
}
#endif

/**
  * @brief  SysTick 轮询式毫秒延时（阻塞）
  * @param  ms  延时毫秒数
  * @note   这里用轮询 COUNTFLAG 的方式，不开启 SysTick 中断，
  *         因此与 hk32f030m_it.c 中的 SysTick_Handler 互不干扰；
  *         但两者共用 SysTick 定时器，若改用中断式滴答请勿同时调用本函数。
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