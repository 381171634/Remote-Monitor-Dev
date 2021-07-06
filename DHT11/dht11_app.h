#ifndef _DHT11_APP_H
#define _DHT11_APP_H

#include "stm32f1xx_hal.h"
#include "common.h"
#include "dht11_bsp.h"

#define DHT11_AVERAGE_CNT   10

typedef enum{
    DHT11_STEP_START = 0,
    DHT11_STEP_READ,
    DHT11_STEP_POWER_OFF,
    DHT11_STEP_FINISH
}dht11StepTypedef;

typedef struct{
    uint32_t dht11_wet_sum;     //湿度和
    int      dht11_temp_sum;    //温度和
    uint16_t readCnt;           //读的次数
    uint16_t dht11_wet_avg;     //湿度均值
    int      dht11_temp_avg;    //温度均值
}dht11ResTypedef;               //dht11结果结构体

typedef struct{
    uint8_t wet_H;              //湿度高
    uint8_t wet_L;              //湿度低
    uint8_t temp_H;             //温度高
    uint8_t temp_L;             //温度低
    uint8_t sum;                //校验和
}dht11DataTypedef;              //dht11单词读取数据

void dht11_task();
extern taskManageTypedef dht11_tm;
extern dht11ResTypedef dht11Res;

#endif