#ifndef _DHT11_APP_H
#define _DHT11_APP_H

#include "stm32f1xx_hal.h"

typedef struct{
    uint8_t wet_H;
    uint8_t wet_L;
    uint8_t temp_H;
    uint8_t temp_L;
    uint8_t sum;
}dht11DataTypedef;

uint8_t app_dht11Read(dht11DataTypedef *pSrc);

#endif