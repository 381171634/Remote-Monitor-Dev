#ifndef _PROC_H
#define _PROC_H

#include "common.h"
#include "gprs_bsp.h"

#define PROC_MAX_LEN    512

typedef struct{
    uint32_t        timeTick;   //时间戳
    int             tempture;   //温度
    int             humidity;   //湿度
    int             HCHO;       //甲醛
    int             CO2;        //二氧化碳
    int             cellVoltage;//锂电池电压
}SampleDataTypedef;//整数，除以1000保留到小数点后三位

typedef enum{
    PROC_FIND_1ST_HEAD = 0,
    PROC_FIND_2ND_HEAD,
    PROC_GET_LEN_L,
    PROC_GET_LEN_H,
    PROC_GET_DATA,
    PROC_CHECK_SUM
}ProcStateTypedef;//协议状态机

typedef enum{
    PROC_STEP_START = 0,
    PROC_STEP_ONLINE,
    PROC_STEP_PUBLISH,
    PROC_STEP_FINISH
}procStepTypedef;

typedef struct{
    gprs_RBTypedef *rb;                 //gprs底层通讯通道

    uint8_t contentBuf[PROC_MAX_LEN];   //报文内容
    uint16_t pWrite_contentBuf;         //报文内容写指针
    uint16_t proclen;                   //报文长度
    uint8_t state;                      //状态机状态
}procChannelTypedef;                    //协议通道

extern taskManageTypedef proc_tm; 
void proc_task();

#endif