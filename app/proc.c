/*
 ============================================================================
 Name        : proc.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 协议处理
            head_low    head_high   len_l len_h ctrl    data    sum_check
              0xa5         0x5a       xx    xx   xx    xx...xx     xx
 ============================================================================
 */

#include "proc.h"
#include "rtc.h"
#include "dht11_app.h"
#include "sgp30_app.h"

#define DEV_ID  "0535202012080001"  //4位区号+4位年+4位月日+4位顺序号

static int proc_makeAproc(uint8_t *pSrc,unsigned short len);
uint8_t proc_getProc(procChannelTypedef *procRBuf,uint16_t timeout);

static uint8_t proc_online(uint16_t timeout);
static uint8_t proc_publish(uint16_t timeout);

static uint8_t tranBuf[PROC_MAX_LEN] = {0};

taskManageTypedef proc_tm;
procChannelTypedef procChan;

static void proc_init()
{
    memset((void *)&proc_tm,0,sizeof(proc_tm));
    proc_tm.step = PROC_STEP_START;
    procChan.rb = &gprsRB;
    procChan.state = PROC_FIND_1ST_HEAD;
}

void proc_task()
{
    uint8_t res =TRUE;

    switch(proc_tm.step)
    {
        case PROC_STEP_START:
            proc_init();
            proc_tm.step++;
            break;
        case PROC_STEP_ONLINE:
            res = proc_online(10000);
            if(res == TRUE)
            {
                DBG_PRT("get server response OK!\n");
                proc_tm.step++;
            }
            else
            {
                DBG_PRT("get server response ERR!\n");
                proc_tm.errCnt++;
            }
            break;
        case PROC_STEP_PUBLISH:
            res = proc_publish(10000);
            if(res == TRUE)
            {
                DBG_PRT("get server response OK!\n");
                proc_tm.step++;
            }
            else
            {
                DBG_PRT("get server response ERR!\n");
                proc_tm.errCnt++;
            }
            break;
        default:
            break;
    }
}

static uint8_t proc_online(uint16_t timeout)
{
    uint8_t res = TRUE;
    uint16_t len_t = 4;
    uint32_t tick = 0;

    tick = getUnixTick();
    memset(tranBuf,0,sizeof(tranBuf));
    tranBuf[len_t++] = 0x00;
    memcpy(tranBuf + len_t,(void *)&tick,4);
    len_t += 4;
    memcpy(tranBuf + len_t,DEV_ID,16);
    len_t += 16;
    
    len_t = proc_makeAproc(tranBuf,len_t - 4);
    
    if(gprs_bsp.write(tranBuf,len_t) == TRUE)
    {
        res = proc_getProc(&procChan,timeout);
        if(res == TRUE)
        {
            if(procChan.contentBuf[0] == 0x00)
            {
                res = TRUE;
            }
            else
            {
                res = FALSE;
            }
        }
    }
    else
    {
        DBG_PRT("gprs write ERR!\n");
        res = FALSE;
    }
    
    return res;
}

static uint8_t proc_publish(uint16_t timeout)
{
    uint8_t res = TRUE;
    uint16_t len_t = 4;
    uint32_t tick = 0;
    SampleDataTypedef data;

    data.timeTick = getUnixTick();
    data.tempture = dht11Res.dht11_temp_avg;
    data.humidity = dht11Res.dht11_wet_avg;
    data.HCHO     = sgp30Res.tvoc_arg;
    data.CO2      = sgp30Res.co2_avg;
    data.cellVoltage = 0;//wait for cpl

    memset(tranBuf,0,sizeof(tranBuf));
    tranBuf[len_t++] = 0x02;
    memcpy(tranBuf + len_t,(void *)&data.timeTick,4);
    len_t += 4;
    memcpy(tranBuf + len_t,DEV_ID,16);
    len_t += 16;
    memcpy(tranBuf + len_t,(void *)&data,sizeof(SampleDataTypedef));
    len_t += sizeof(SampleDataTypedef);
    
    len_t = proc_makeAproc(tranBuf,len_t - 4);
    
    if(gprs_bsp.write(tranBuf,len_t) == TRUE)
    {
        res = proc_getProc(&procChan,timeout);
        if(res == TRUE)
        {
            if(procChan.contentBuf[0] == 0x02)
            {
                res = TRUE;
            }
            else
            {
                res = FALSE;
            }
        }
    }
    else
    {
        DBG_PRT("gprs write ERR!\n");
        res = FALSE;
    }
    
    return res;
}

static void rstProcState(procChannelTypedef *procRBuf)
{
    procRBuf->state = PROC_FIND_1ST_HEAD;
}

static uint8_t proc_getProc(procChannelTypedef *procRBuf,uint16_t timeout)
{
    int res = FALSE;
    int i;
    uint8_t byte;
    uint8_t sum = 0;
    uint32_t endTick = gprs_bsp.getTickMs() + timeout;

    while(1)
    {
        if(gprs_bsp.getTickMs() - endTick < 0xffffffff/2 || res == TRUE)
        {
            break;
        }

        if(procRBuf->rb->uart_idle_flag == 0)
        {
            gprs_bsp.dly_ms(100);
            continue;
        }

        procRBuf->rb->uart_idle_flag = 0;
        while(procRBuf->rb->pR - procRBuf->rb->pW != 0)
        {
            byte = procRBuf->rb->pRecvBuf[(procRBuf->rb->pR++) % GPRS_RECV_BUF_LEN];
            switch(procRBuf->state)
            {
                case PROC_FIND_1ST_HEAD:
                    if(byte == 0xa5)
                    {
                        procRBuf->state++;
                    }
                    break;
                case PROC_FIND_2ND_HEAD:
                    if(byte == 0x5a)
                    {
                        procRBuf->state++;
                        procRBuf->proclen  = 0;
                    }
                    else
                    {
                        rstProcState(procRBuf);
                    }
                    break;
                case PROC_GET_LEN_L:
                    procRBuf->proclen &= ~(0xff);
                    procRBuf->proclen |= byte;
                    procRBuf->state++;
                    break;
                case PROC_GET_LEN_H:
                    procRBuf->proclen &= ~(0xff00);
                    procRBuf->proclen |= (byte << 8);
                    if(procRBuf->proclen > GPRS_RECV_BUF_LEN)
                    {
                        rstProcState(procRBuf);
                    }
                    else
                    {
                        procRBuf->pWrite_contentBuf = 0;
                        procRBuf->state++;
                    }
                    break;
                case PROC_GET_DATA:
                    if(procRBuf->pWrite_contentBuf < procRBuf->proclen)
                    {
                        procRBuf->contentBuf[(procRBuf->pWrite_contentBuf) % GPRS_RECV_BUF_LEN] = byte;
                    }

                    if(++procRBuf->pWrite_contentBuf >= procRBuf->proclen)
                    {
                        procRBuf->state++;
                    }
                    break;
                case PROC_CHECK_SUM:
                    for(i = 0;i < procRBuf->proclen; i++)
                    {
                        sum += procRBuf->contentBuf[i];
                    }

                    if(sum == byte)
                    {
                        res = TRUE;
                    }

                    rstProcState(procRBuf);
                    break;
                default:
                    rstProcState(procRBuf);
                    break;
            }

            if(res == TRUE)
            {
                break;
            }
        }

    }

    return res;
}

/*============================================================================
 协议组包
 pSrc：报文缓存指针
 len：控制字+数据域长度
 return：此包报文总长度
 ============================================================================*/
static int proc_makeAproc(uint8_t *pSrc,unsigned short len)
{
    int i;
    int p = 0;
    uint8_t *pContent = pSrc + 4;
    uint8_t sum = 0;

    pSrc[p++] = 0xa5;
    pSrc[p++] = 0x5a;
    memcpy((void *)(pSrc + p),&len,2);
    p += 2;
    p += len;
    for(i = 0; i< len; i++)
    {
        sum += pContent[i];
    }
    pSrc[p] = sum;

    return (len + 2 + 2 + 1);
}


