#ifndef _GPRS_BSP_H
#define _GPRS_BSP_H

#include "stm32f1xx_hal.h"

#define GPRS_RECV_BUF_LEN   1024

typedef struct{
    void (*reset)(void);
    void (*dly_ms)(uint16_t ms);
    uint32_t (*getTickMs)(void);
    uint8_t (*write)(uint8_t *pSrc,uint16_t len);
}gprs_bspTypedef;

typedef struct{
    uint8_t pRecvBuf[GPRS_RECV_BUF_LEN];
    uint16_t pW;
    uint16_t pR;
}gprs_RBTypedef;

extern gprs_RBTypedef gprsRB;
extern gprs_bspTypedef gprs_bsp;
extern uint8_t stm32_hal_uart_irq_rx;
#endif