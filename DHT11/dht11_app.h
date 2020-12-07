#ifndef _DHT11_APP_H
#define _DHT11_APP_H

#include "stm32f1xx_hal.h"
#include "common.h"

#define DHT11_AVERAGE_CNT   10

typedef enum{
    DHT11_STEP_START = 0,
    DHT11_STEP_READ,
    DHT11_STEP_POWER_OFF,
    DHT11_STEP_FINISH
}dht11StepTypedef;

typedef struct{
    uint32_t dht11_wet_sum;
    int      dht11_temp_sum;
    uint16_t readCnt;
    uint16_t dht11_wet_avg;
    int      dht11_temp_avg;
}dht11ResTypedef;

typedef struct{
    uint8_t wet_H;
    uint8_t wet_L;
    uint8_t temp_H;
    uint8_t temp_L;
    uint8_t sum;
}dht11DataTypedef;

void dht11_task();
extern taskManageTypedef dht11_tm;
extern dht11ResTypedef dht11Res;

#endif