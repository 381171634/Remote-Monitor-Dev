#ifndef _GPRS_BSP_H
#define _GPRS_BSP_H

#include "stm32f1xx_hal.h"
#include "main.h"

#define GPRS_RECV_BUF_LEN   1024
//board
#define GPRS_POWER_Port     DCDC_ENABLE_GPIO_Port
#define GPRS_POWER_Pin      DCDC_ENABLE_Pin  
#define GPRS_RESET_Port     GPRS_RST_GPIO_Port
#define GPRS_RESET_Pin      GPRS_RST_Pin 
//bsp fill
#define GPRS_POWER_ON       HAL_GPIO_WritePin(DCDC_ENABLE_GPIO_Port,DCDC_ENABLE_Pin,GPIO_PIN_SET)
#define GPRS_POWER_OFF      HAL_GPIO_WritePin(DCDC_ENABLE_GPIO_Port,DCDC_ENABLE_Pin,GPIO_PIN_RESET)

typedef struct{
    void (*init)(void);
    void (*reset)(void);
    void (*dly_ms)(uint16_t ms);
    uint32_t (*getTickMs)(void);
    uint8_t (*write)(uint8_t *pSrc,uint16_t len);
    uint16_t (*read)(uint8_t *pDes,uint16_t len,uint16_t timeout);
}gprs_bspTypedef;

typedef struct{
    uint8_t pRecvBuf[GPRS_RECV_BUF_LEN];
    uint16_t pW;
    uint16_t pR;
    uint8_t uart_idle_flag;
}gprs_RBTypedef;

extern gprs_RBTypedef gprsRB;
extern gprs_bspTypedef gprs_bsp;

#endif