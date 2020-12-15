#ifndef _GPRS_BSP_H
#define _GPRS_BSP_H

#include "stm32f1xx_hal.h"
#include "main.h"

//底层串口环形缓存长度
#define GPRS_RECV_BUF_LEN   1024
//定义管脚
#define GPRS_POWER_Port     DCDC_ENABLE_GPIO_Port
#define GPRS_POWER_Pin      DCDC_ENABLE_Pin  
#define GPRS_RESET_Port     GPRS_RST_GPIO_Port
#define GPRS_RESET_Pin      GPRS_RST_Pin 
//上电
#define GPRS_POWER_ON       HAL_GPIO_WritePin(DCDC_ENABLE_GPIO_Port,DCDC_ENABLE_Pin,GPIO_PIN_SET)
//掉电
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
    uint8_t pRecvBuf[GPRS_RECV_BUF_LEN];    //缓冲区
    uint16_t pW;                            //写指针
    uint16_t pR;                            //读指针
    uint8_t uart_idle_flag;                 //串口空闲中断标志
}gprs_RBTypedef;                            //串口通道结构体

extern gprs_RBTypedef gprsRB;
extern gprs_bspTypedef gprs_bsp;

#endif