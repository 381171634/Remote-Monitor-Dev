#ifndef _GPRS_APP_H
#define _GPRS_APP_H

#include "stm32f1xx_hal.h"
#include "common.h"
#include "gprs_bsp.h"

typedef enum{
    GPRS_STEP_POWER_ON = 0,
    GPRS_STEP_RESET,
    GPRS_STEP_AT,
    GPRS_STEP_CPIN,
    GPRS_STEP_CGREG,
    GPRS_STEP_PPP,
    GPRS_STEP_UPDATE_TIME,
    GPRS_STEP_MYNETACT,
    GPRS_STEP_TRANS,
    GPRS_STEP_IN_TRANS,
    GPRS_STEP_POWER_OFF,
    GPRS_STEP_FINISH,
}gprsStepTypedef;

void gprs_task();
extern taskManageTypedef gprs_tm;

#endif