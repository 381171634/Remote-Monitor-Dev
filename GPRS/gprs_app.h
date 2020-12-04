#ifndef _GPRS_APP_H
#define _GPRS_APP_H

#include "stm32f1xx_hal.h"

uint16_t gprs_ATcmdTx(const uint8_t *cmd,const uint8_t *hopeAck,uint16_t timeoutMs,uint8_t retry );
void gprs_task();
#endif