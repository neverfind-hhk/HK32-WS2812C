/*
  ******************************************************************************
  *  @file    hk32f030m_pwr.c
  * @author  Rakan.z
  * @version V1.0
  * @brief   API file of PWR module
  * @changelist
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "hk32f030m_pwr.h"
#include "hk32f030m_rcc.h"
#include "hk32f030m.h"
/** @defgroup PWR_Private_Defines
  * @{
  */



/**
  * @brief  Deinitializes the PWR peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void PWR_DeInit(void)
{
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, DISABLE);
}



/**
  * @brief  Enters Sleep mode.
  * @param  PWR_Entry: specifies if Sleep mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_Entry_WFI: enter Sleep mode with WFI instruction
  *     @arg PWR_Entry_WFE: enter Sleep mode with WFE instruction
  * @retval None
  */
void PWR_EnterSleepMode(uint8_t PWR_Entry)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_PWR_ENTRY(PWR_Entry));

  /* Select the regulator state in Sleep mode ---------------------------------*/
  tmpreg = PWR->CR;
  /* Clear LPDS bits */
  tmpreg &= CR_DS_MASK;
  /* Store the new value */
  PWR->CR = tmpreg;

  /* Select STOP mode entry --------------------------------------------------*/
  if(PWR_Entry == PWR_Entry_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
#if defined(HK32F030M_C_VERSION)
  else if(PWR_Entry == PWR_Entry_WFE)
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
  else
  {
    /* Set SLEEP on exit bit of Cortex-M0 System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
  }
#else
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
#endif
}


#if defined(HK32F030M_2K_VERSION)
/**
  * @brief  it will config LSI 128K as sysclk
  * @retval None
  * @note this fuction only used in fuction  PWR_EnterDeepSleepMode(uint8_t PWR_Entry)
  */
static void Sysclk_SwitchToLSI(void)
{
  RCC_LSICmd(ENABLE);

  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

  /* Flash wait state */
  FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
  FLASH->ACR |= (uint32_t)FLASH_Latency_0;

  /* Select LSI as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_LSI;

  /* Wait till LSI is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_LSI);

  /* HCLK = SYSCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

  /* PCLK = HCLK */
  RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

  // config the Flash Erase and program time
  RCC->CFGR4 |= RCC_CFGR4_FLITFCLK_PRE;
  RCC->CFGR4 &= ~(((uint32_t)0x0F) << RCC_CFGR4_FLITFCLK_PRE_Pos);

  /* Close HSI */
  RCC_HSICmd(DISABLE);

}

/**
  * @brief  Enters DeepSleep mode. it will config LSI 128K as sysclk
  * @param  PWR_Entry: specifies if Sleep mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_Entry_WFI: enter Sleep mode with WFI instruction
  *     @arg PWR_Entry_WFE: enter Sleep mode with WFE instruction
  * @retval None
  */
void PWR_EnterDeepSleepMode(uint8_t PWR_Entry)
{
  /* Check the parameters */
  assert_param(IS_PWR_ENTRY(PWR_Entry));
  /* set sysclk  to LSI */
  Sysclk_SwitchToLSI();
  /* enter sleep mode  */
  PWR_EnterSleepMode(PWR_Entry);
}
#endif /* HK32F030M_2K_VERSION */


/**
  * @brief  Enter Stop mode.
  * @param  PWR_Regulator: specifies the regulator state in STOP mode.
  *   This parameter can be one of the following values:
  *     @arg PWR_Regulator_LowPower: STOP mode with regulator in low power mode
  * @param  PWR_Entry: specifies if STOP mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_Entry_WFI: enter STOP mode with WFI instruction
  *     @arg PWR_Entry_WFE: enter STOP mode with WFE instruction
  * @retval None
  */
void PWR_EnterStopMode(uint32_t PWR_Regulator, uint8_t PWR_Entry)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_PWR_REGULATOR(PWR_Regulator));
  assert_param(IS_PWR_ENTRY(PWR_Entry));
// enable rcc of pwr
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Select the regulator state in Stop mode ---------------------------------*/
  tmpreg = PWR->CR;
  /* Clear  LPDS bits */
  tmpreg &= CR_DS_MASK;
  /* Set LPDS bit according to PWR_Regulator value */
  tmpreg |= PWR_Regulator;
  /* Store the new value */
  PWR->CR = tmpreg;
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;

  /* Select Stop mode entry --------------------------------------------------*/
  if(PWR_Entry == PWR_Entry_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
#if defined(HK32F030M_2K_VERSION)
  else
  {
    // wait the AWU is IDE and AWU_BUSY is 0
    while(AWU->SR & 0x00000001) {};

    //  detect and clear the AWU_EXTILINE11
    if(EXTI_GetFlagStatus(EXTI_Line11) == SET)
    {
      EXTI_ClearFlag(EXTI_Line11);
    }

    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
#else
  else if(PWR_Entry == PWR_Entry_WFE)
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
  else
  {
    /* Set SLEEP on exit bit of Cortex-M0 System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
  }
#endif

  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP);
}



#if defined(HK32F030M_2K_VERSION)
/**
  * @brief  Set PMU LDO Refernce  voltage to adc.
  * @param  Vref_Set: internal Refernce out voltage  ,
  *       This parameter can be: ADC_Vref_0d8 or ADC_Vref_LDO
          ADC_Vref_0d8: 0.8V Vref to adc.
          ADC_Vref_LDO: LDO out Voltage to adc .(1.2V)
  * @retval None
  */

void PWR_SetLDO_RefVolToADC(uint16_t Vref_Set)
{
  uint16_t temp = 0;
  /* Check the parameters */
  assert_param(IS_PWR_VTEST_SET(Vref_Set));

  /* select the LDO Voltage reference register */
  temp = PWR->VREF_SEL;

  /* Clear LPDS bits */
  temp &= VTEST_SET_MASK;

  /* set the VREF*/
  temp |= Vref_Set;

  /* set the Register*/
  PWR->VREF_SEL |= (uint32_t)temp;

}
#endif /* HK32F030M_2K_VERSION */

#if defined(HK32F030M_C_VERSION)
/**
  * @brief  Checks whether the specified PWR flag is set or not. (仅 C 版芯片有 CSR 寄存器)
  * @param  PWR_FLAG: specifies the flag to check.
  *          This parameter can be: PWR_FLAG_LDORDY
  * @retval The new state of PWR_FLAG (SET or RESET).
  */
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_PWR_GET_FLAG(PWR_FLAG));

  if ((PWR->CSR & PWR_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  /* Return the flag status */
  return bitstatus;
}
#endif /* HK32F030M_C_VERSION */

