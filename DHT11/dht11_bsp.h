#ifndef _DHT11_H
#define _DHT11_H

#include "stm32f1xx_hal.h"

#define     DHT11_POWER_ON          1
#define     DHT11_POWER_OFF         0

#define     DHT11_PIN_MODE_INPUT    0
#define     DHT11_PIN_MODE_OUTPUT   1

#define     DHT11_PIN_STATE_H       1
#define     DHT11_PIN_STATE_L       0

typedef struct{
    void    (*power_set)(uint8_t state);
    void    (*set_sda_mode)(uint8_t mode);
    void    (*write_sda)(uint8_t state);
    uint8_t (*read_sda)(void);
    void    (*dly_ms)(uint16_t ms);
    void    (*dly_us)(uint16_t us);
}dht11_bspTypedef;

extern dht11_bspTypedef dht11_bsp;

#endif