#ifndef _DHT11_H
#define _DHT11_H

#include "stm32f1xx_hal.h"

//board define
//GPIO定义
#define     DHT11_POWER_Port        DHT11_VCC_GPIO_Port
#define     DHT11_POWER_Pin         DHT11_VCC_Pin
#define     DHT11_SDA_Port          DHT11_DATA_GPIO_Port
#define     DHT11_SDA_Pin           DHT11_DATA_Pin

//上电
#define     DHT11_POWER_ON          HAL_GPIO_WritePin(DHT11_POWER_Port,DHT11_POWER_Pin,GPIO_PIN_SET)
//掉电
#define     DHT11_POWER_OFF         HAL_GPIO_WritePin(DHT11_POWER_Port,DHT11_POWER_Pin,GPIO_PIN_RESET)

//数据引脚拉高
#define     DHT11_SDA_WRITE_H       HAL_GPIO_WritePin(DHT11_SDA_Port,DHT11_SDA_Pin,GPIO_PIN_SET)
//数据引脚拉低
#define     DHT11_SDA_WRITE_L       HAL_GPIO_WritePin(DHT11_SDA_Port,DHT11_SDA_Pin,GPIO_PIN_RESET)

//数据引脚 输入模式
#define     DHT11_SDA_MODE_INPUT    do\
                                    {\
                                        DHT11_SDA_Port->CRH &= ~(0xf);\
                                        DHT11_SDA_Port->CRH |= 0x04;\
                                    }while(0);
//数据引脚 输出模式
#define     DHT11_SDA_MODE_OUTPUT   do\
                                    {\
                                        DHT11_SDA_Port->CRH &= ~(0xf);\
                                        DHT11_SDA_Port->CRH |= 0x05;\
                                    }while(0);
//读数据引脚
#define     DHT11_SDA_READ          HAL_GPIO_ReadPin(DHT11_SDA_Port,DHT11_SDA_Pin)

#define     DHT11_PIN_STATE_L       0
#define     DHT11_PIN_STATE_H       1


typedef struct{
    void    (*gpio_init)(void);
    void    (*dly_ms)(uint16_t ms);
    void    (*dly_us)(uint16_t us);
    uint32_t(*getTick)();
}dht11_bspTypedef;

extern dht11_bspTypedef dht11_bsp;

#endif