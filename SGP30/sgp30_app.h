#ifndef _SGP30_APP_H
#define _SGP30_APP_H

#include "common.h"

#define SGP30_AVERAGE_CNT   10

typedef enum{
    SGP30_STEP_START = 0,
    SGP30_STEP_FIX,
    SGP30_STEP_WAIT_INIT,
    SGP30_STEP_READ,
    SGP30_STEP_LOW_POWER,
    SGP30_STEP_FINISH
}sgp30StepTypedef;

typedef struct{
    uint32_t tvoc_sum;  //甲醛和
    uint32_t co2_sum;   //二氧化碳和
    uint16_t readCnt;   //读次数
    uint32_t tvoc_arg;  //甲醛均值
    uint32_t co2_avg;   //二氧化碳均值
}sgp30ResTypedef;       //sgp30读结果

void sgp30_task();

extern taskManageTypedef sgp30_tm;
extern sgp30ResTypedef sgp30Res;





#endif