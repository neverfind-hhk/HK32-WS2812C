/**
  ******************************************************************************
  * @file    system_hk32f030m.c
  * @author  laura.C (modified for self-contained CMSIS / HAL usage)
  * @brief   CMSIS Cortex-M0 Device Peripheral Access Layer System Source File
  *
  *          Defaults to HSI 32 MHz, AHB=HSI/1, APB=AHB/1, FLASH latency=2.
  *          Does NOT depend on any external library (StdPeriph or HAL).
  *          Oscillator values are defined locally (can be overridden by
  *          user code that defines HSI_VALUE etc. before including this file
  *          or on the command line).
  *
  *          For other clock configurations (EXTCLK, PLL, different dividers)
  *          use the HAL_RCC_OscConfig / HAL_RCC_ClockConfig API, or write
  *          your own low-level init code modelled after the HSI_32M example
  *          kept below as a comment.
  ******************************************************************************
  */

#include "hk32f030m.h"

/* [工程修改] 改为"条件包裹"：本文件与 system_hk32f030xm.c 一起挂在工程里，
   器件宏不匹配时整个文件编译成空单元，避免 SystemInit / AHBPrescTable 重复定义 */
#if defined(HK32F030M_2K_VERSION)

/*---------------------------------------------------------------------------*/
/*  Oscillator values — defaults for HK32F030M (HSI = 32 MHz).               */
/*  User code may override these by defining the macro before including      */
/*  this file or via -D on the command line.                                 */
/*---------------------------------------------------------------------------*/
#if !defined(HSI_VALUE)
#define HSI_VALUE          ((uint32_t)32000000U)
#endif

#if !defined(LSI_VALUE)
#define LSI_VALUE ((uint32_t)114000)   // 114kHz
#endif

#if !defined(STARTUP_TIMEOUT)
#define STARTUP_TIMEOUT    ((uint32_t)0xFFFFU)
#endif

/*---------------------------------------------------------------------------*/
/*  Clock prescaler tables                                                   */
/*  HK32-specific: AHBPrescTable[1]=6  (/6 divisor, not present on STM32).   */
/*---------------------------------------------------------------------------*/
const uint16_t AHBPrescTable[16] = {
   1U, 6U,   1U, 1U, 1U, 1U, 1U, 1U,           /* 0x0..0x7 */
   2U, 4U,   8U, 16U, 64U, 128U, 256U, 512U     /* 0x8..0xF */
};

/* APBPrescTable: stored as shift values.  PCLK = HCLK >> APBPrescTable[idx].  */
const uint8_t APBPrescTable[8] = {
   0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U
};

/*---------------------------------------------------------------------------*/
/*  Macro for optional Vector Table relocation to SRAM                       */
/*---------------------------------------------------------------------------*/
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x0U
/*!< Vector Table base offset field.
     This value must be a multiple of 0x200. */


/*---------------------------------------------------------------------------*/
/*  SystemCoreClock variable (updated by SystemCoreClockUpdate)              */
/*---------------------------------------------------------------------------*/
uint32_t SystemCoreClock = HSI_VALUE;


/*---------------------------------------------------------------------------*/
/*  Clock configuration (default: HSI 32 MHz, AHB/1, APB/1)                 */
/*---------------------------------------------------------------------------*/

#define FLASH_Latency_2 0x2U


/**
  * @brief  Setup the microcontroller system.
  *         Initialize the HSI 32 MHz clock, FLASH wait states and VTOR.
  */
void SystemInit(void)
{
  __IO uint32_t StartUpCounter;
  __IO uint32_t HSIStatus;
  __IO uint32_t ACRreg;
  __IO uint32_t RCCHCLKReg;
  __IO uint32_t RCCPCLKReg;

  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0] and MCOSEL[2:0] bits */
  RCC->CFGR &= (uint32_t)0xF8FFB81C;

  /* Reset USARTSW[1:0], I2CSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFFFFFEC;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

  /*------------------------------------------------------------------*/
  /*  Reload the factory HSI trimming values                          */
  /*  The per-chip HSITRIM[4:0] / HSICAL[5:0] values are stored in the */
  /*  system memory area at 0x1FFFF820 (low half = data, high half =  */
  /*  its complement). Ported from the official HK32F030M SDK.        */
  /*------------------------------------------------------------------*/
  {
    uint32_t u32HSIFLASH = *(uint32_t *)0x1FFFF820U;
    uint16_t u16HSITempH = (uint16_t)(u32HSIFLASH >> 16U);
    uint16_t u16HSITempL = (uint16_t)(u32HSIFLASH);

    if (!(u16HSITempH & u16HSITempL))
    {
      /* Valid factory data: HSITRIM = bits [4:0], HSICAL = bits [10:5]
       * of the low half, mapped to RCC_CR bits [7:3] and [13:8].      */
      uint32_t u32RCC_CR = RCC->CR & (uint32_t)~(RCC_CR_HSITRIM | RCC_CR_HSICAL);
      u32RCC_CR |= (uint32_t)(((u16HSITempL & 0x001FU) << 3U) |
                              (((u16HSITempL >> 5U) & 0x003FU) << 8U));
      RCC->CR = u32RCC_CR;
    }
  }

  /*------------------------------------------------------------------*/
  /*  HSI 32 MHz setup                                               */
  /*------------------------------------------------------------------*/

  /* Enable HSI */
  RCC->CR |= RCC_CR_HSION;

  /* Set FLITF clock prescaler to /8 for safe FLASH access at boot.
   * Without this the flash controller may not keep up with HSI.   */
  RCC->CFGR4 &= ~RCC_CFGR4_FLITFCLK_PRE;
  RCC->CFGR4 |= (((uint32_t)0x07) << RCC_CFGR4_FLITFCLK_PRE_Pos);

  /* Wait till HSI is ready */
  StartUpCounter = 0U;
  do {
    HSIStatus = RCC->CR & RCC_CR_HSIRDY;
    StartUpCounter++;
  } while ((HSIStatus == 0U) && (StartUpCounter != STARTUP_TIMEOUT));


  if (HSIStatus != 0)
  {
    /* Flash wait state — 32 MHz requires 2 wait states. */
    ACRreg  = FLASH->ACR;
    ACRreg &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
    FLASH->ACR = (uint32_t)(FLASH_Latency_2 | ACRreg);   /* FLASH_Latency_2 */

    /* HCLK = SYSCLK / 1 */
    RCCHCLKReg  = RCC->CFGR;
    RCCHCLKReg &= (uint32_t)((uint32_t)~RCC_CFGR_HPRE_Msk);
    RCC->CFGR   = (uint32_t)(RCC_CFGR_HPRE_DIV1 | RCCHCLKReg);

    /* PCLK = HCLK / 1 */
    RCCPCLKReg  = RCC->CFGR;
    RCCPCLKReg &= (uint32_t)((uint32_t)~RCC_CFGR_PPRE_Msk);
    RCC->CFGR   = (uint32_t)(RCC_CFGR_PPRE_DIV1 | RCCPCLKReg);

    /* Select HSI as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;

    /* Wait till HSI is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
  }
  else
  {
    /* HSI start-up failed — application has wrong clock configuration.
     * Add error handling here if needed. */
  }

  /* Configure the Vector Table location */
#ifdef VECT_TAB_SRAM
  SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE; /* Vector Table in SRAM */
#else
  FLASH->INT_VEC_OFFSET = VECT_TAB_OFFSET;/* Vector Table offset in FLASH */
#endif
}


/*---------------------------------------------------------------------------*/
/*  SystemCoreClockUpdate() — CMSIS standard                                 */
/*---------------------------------------------------------------------------*/

/**
  * @brief  Update SystemCoreClock to reflect the current RCC configuration.
  * @note   Must be called after any RCC reconfiguration.
  */
void SystemCoreClockUpdate(void)
{
  uint32_t tmp;
  uint32_t presc;

  /* Get SYSCLK source */
  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp)
  {
    case RCC_CFGR_SWS_HSI:    /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;

    case RCC_CFGR_SWS_EXTCLK: /* EXTCLK (external clock) used as system clock */
      SystemCoreClock = 32000000U;   /* default EXTCLK frequency */
      break;

    case RCC_CFGR_SWS_LSI:    /* LSI used as system clock */
      SystemCoreClock = 114000U;     /* ~114 kHz */
      break;

    default:                  /* HSI used as system clock (fallback) */
      SystemCoreClock = HSI_VALUE;
      break;
  }

  /* Apply AHB prescaler */
  tmp   = (RCC->CFGR & RCC_CFGR_HPRE) >> 4U;
  presc = AHBPrescTable[tmp];
  SystemCoreClock = SystemCoreClock / presc;
}


/*---------------------------------------------------------------------------*/
/*  Optional: Vector Table migration to SRAM (ARM Compiler / GCC / IAR)      */
/*---------------------------------------------------------------------------*/
/* #ifndef FORBID_VECT_TAB_MIGRATION ... (same as original, omitted for brevity) */

#endif /* HK32F030M_2K_VERSION */
