#include "gprs_app.h"
#include "gprs_bsp.h"
#include "usart.h"
#include <stdio.h>

#define GPRS_NO_HOPEACK2    0
#define GPRS_NO_BACK        0
#define SERVER_IP           "106.13.1.239"
#define SERVER_PORT         8888

taskManageTypedef gprs_tm = {0,0,0};

static uint16_t gprs_ATcmdTx(const uint8_t *cmd,const uint8_t *hopeAck1,const uint8_t *hopeAck2,uint8_t *pBack,uint16_t timeoutMs,uint8_t retry )
{
    uint8_t res = FALSE;
    uint8_t atBuf[128] = {0};
    uint8_t atBufpW = 0;
    uint8_t i;
    uint32_t endTick;

    for(i = 0;i < retry; i++)
    {
        endTick = gprs_bsp.getTickMs() + timeoutMs;
        memset(atBuf,0,sizeof(atBuf));
        atBufpW = 0;
        if(gprs_bsp.write((uint8_t *)cmd,strlen(cmd)) == TRUE)
        {
            while(1)
            {
                if(gprsRB.uart_idle_flag = 1)
                {
                    gprsRB.uart_idle_flag = 0;
                    while(gprsRB.pW - gprsRB.pR)
                    {
                        atBuf[(atBufpW++) % 128] = gprsRB.pRecvBuf[(gprsRB.pR++) % GPRS_RECV_BUF_LEN];
                    }
                }
                
                if(hopeAck1 == 0)
                {
                    res = FALSE;
                    break;
                }

                if(strstr(atBuf,hopeAck1) != 0)
                {
                    res = TRUE;
                    break;
                }
                else if(hopeAck2 != 0)
                {
                    if(strstr(atBuf,hopeAck2) != 0)
                    {
                        res = TRUE;
                        break;
                    }
                }

                //timeout
                if(gprs_bsp.getTickMs() - endTick < 0xffffffff/2)
                {
                    res =FALSE;
                    break;
                }
                else
                {
                    gprs_bsp.dly_ms(100);
                }
            }
        }

        if(res == TRUE)
        {
            if(pBack != 0)
            {
                memcpy(pBack,atBuf,128);
            }
            break;
        }
    }
	
	return res;
}

void gprs_task()
{
    uint8_t res = 0;
    uint8_t atbufBack[128] = {0};
    uint8_t cmd[128] = {0};

    //运行时间未到 返回
    if(gprs_bsp.getTickMs() - gprs_tm.execuTick > 0xffffffff / 2
        || gprs_tm.step == GPRS_STEP_FINISH)
    {
        return;
    }
    
    switch(gprs_tm.step)
    {
        case GPRS_STEP_POWER_ON:
            gprs_bsp.init();

            GPRS_POWER_ON;
            DBG_PRT("gprs power on!\n");
            gprs_tm.execuTick = gprs_bsp.getTickMs() + 100;
            gprs_tm.step++;
            break;
        case GPRS_STEP_RESET:
            gprs_bsp.reset();
            DBG_PRT("gprs reset!\n");
            gprs_tm.execuTick = gprs_bsp.getTickMs() + 5000;
            gprs_tm.step++;
            break;
        case GPRS_STEP_AT:
            res = gprs_ATcmdTx("AT\r","OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,1000,10);
            if(res == TRUE)
            {
                DBG_PRT("gprs AT OK!\n");
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs AT ERR!\n");
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;
        case GPRS_STEP_CPIN:
            res = gprs_ATcmdTx("AT+CPIN?\r","OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,1000,10);
            if(res == TRUE)
            {
                DBG_PRT("gprs AT+CPIN OK!\n");
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs AT+CPIN ERR!\n");
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;
        case GPRS_STEP_CGREG:
            res = gprs_ATcmdTx("AT+CGREG?\r","CGREG: 0,5","CGREG: 0,1",GPRS_NO_BACK,2000,10);  
            if(res == TRUE)
            {
                DBG_PRT("gprs AT+CGREG? OK!\n");
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs AT+CGREG? ERR!\n");
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;
        case GPRS_STEP_MYNETACT:
            res = gprs_ATcmdTx("AT$MYNETACT=0,1\r","OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,10000,3);  
            if(res == TRUE)
            {
                DBG_PRT("gprs AT$MYNETACT=0,1 OK!\n");
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs AT$MYNETACT=0,1 ERR!\n");
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;
        case GPRS_STEP_TRANS:
            snprintf(cmd,128,"AT+TCPTRANS=%s,%d\r",SERVER_IP,SERVER_PORT);
            res = gprs_ATcmdTx(cmd,"OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,5000,3);  
            if(res == TRUE)
            {
                DBG_PRT("gprs %s OK!\n",cmd);
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs %s ERR!\n",cmd);
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;

        default:
            break;

    }

        
}