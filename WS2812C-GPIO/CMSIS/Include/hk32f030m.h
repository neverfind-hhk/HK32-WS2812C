#ifndef HK32F030M_H
#define HK32F030M_H

// 支持HK32F030M 和HK32F0301MxxC 两种单片机的设备头文件，根据设备宏自动选择对应的定义，用来支持HAL 库
#if defined(HK32F030M) || defined (HK32F030MJ4M6) || defined (HK32F030MD4P6) || defined (HK32F030MF4P6) || defined (HK32F030MF4U6)

#ifndef HK32F030M_2K_VERSION
#define HK32F030M_2K_VERSION
#endif

#include "./hk32f030m_2k.h"

#elif defined(HK32F0301MxxC) || defined(HK32F0301MF4P7C)

#ifndef HK32F030M_C_VERSION
#define HK32F030M_C_VERSION
#endif

#include "./hk32f0301mxxc.h"

#elif defined(HK32F0301M) || defined(HK32F030xMxxA)

#ifndef HK32F030M_A_VERSION
#define HK32F030M_A_VERSION
#endif

#include "./hk32f0301mxxa.h"

#else

#error "A device macro must be defined"

#endif

#endif  // HK32F030M_H
