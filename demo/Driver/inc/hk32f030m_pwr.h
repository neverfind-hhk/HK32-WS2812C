/**
  ******************************************************************************
  * @file    hk32f030m_pwr.h
  * @author  Rakan.z
  * @version V1.0  
  * @brief   Header file of PWR module
  * @changelist  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F030M_PWR_H
#define __HK32F030M_PWR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f030m.h"
#include "hk32f030m_conf.h"

/* ------------------ PWR registers bit mask ------------------------ */

/* CR register bit mask */
#define CR_DS_MASK               ((uint32_t)0xFFFFFFFE)


/** @defgroup Regulator_state_is_STOP_mode 
  * @{
  */

#define PWR_Regulator_LowPower    ((uint32_t)0x00000001)
#if defined(HK32F030M_C_VERSION)
#define PWR_Regulator_ON          ((uint32_t)0x00000000)
#define IS_PWR_REGULATOR(REGULATOR) (((REGULATOR) == PWR_Regulator_LowPower)||((REGULATOR)==PWR_Regulator_ON))
#else
#define IS_PWR_REGULATOR(REGULATOR) ((REGULATOR) == PWR_Regulator_LowPower)
#endif

/** @defgroup PWR_mode_entry 
  * @{
  */

#define PWR_Entry_WFI         ((uint8_t)0x01)
#define PWR_Entry_WFE         ((uint8_t)0x02)
#if defined(HK32F030M_C_VERSION)
#define PWR_Entry_SLEEPONEXIT ((uint8_t)0x03)
#define IS_PWR_ENTRY(ENTRY) (((ENTRY) == PWR_Entry_WFI) || ((ENTRY) == PWR_Entry_WFE) || \
                             ((ENTRY) == PWR_Entry_SLEEPONEXIT))
#else
#define IS_PWR_ENTRY(ENTRY) (((ENTRY) == PWR_Entry_WFI) || ((ENTRY) == PWR_Entry_WFE))
#endif

#if defined(HK32F030M_C_VERSION)
/** @defgroup PWR_Flag
  * @{
  */
#define PWR_FLAG_LDORDY           ((uint32_t)0x00000008)
#define IS_PWR_GET_FLAG(FLAG) ((FLAG) == PWR_FLAG_LDORDY)
#endif
 
/** @defgroup PWR_LDO_VREF
  * @{
  */
#define ADC_VREF_0D8              ((uint16_t)0x0008)  
#define ADC_VREF_LDO              ((uint16_t)0x000C)  
#define VTEST_SET_MASK            ((uint16_t)0x3FFF)
#define IS_PWR_VTEST_SET(VTEST_SET) (((VTEST_SET) == ADC_VREF_0D8) || ((VTEST_SET) == ADC_VREF_LDO))




void PWR_DeInit(void);

void PWR_EnterSleepMode(uint8_t PWR_Entry);
#if defined(HK32F030M_2K_VERSION)
void PWR_EnterDeepSleepMode(uint8_t PWR_Entry);
#endif
void PWR_EnterStopMode(uint32_t PWR_Regulator, uint8_t PWR_Entry);

#if defined(HK32F030M_2K_VERSION)
void PWR_SetLDO_RefVolToADC(uint16_t Vref_Set);
#endif
#if defined(HK32F030M_C_VERSION)
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG);
#endif


#ifdef __cplusplus
}
#endif

#endif

