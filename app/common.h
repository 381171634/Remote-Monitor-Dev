#ifndef _COMMON_H
#define _COMMON_H

#include "stm32f1xx_hal.h"

#define ENABLE_DBG      1

#if (ENABLE_DBG == 1)
#define DBG_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)
#else
#define DBG_PRT(fmt...)
#endif

typedef struct{
    uint8_t step;
    uint16_t errCnt;
    uint32_t execuTick;
}taskManageTypedef;


#endif