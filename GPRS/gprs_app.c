/*
 ============================================================================
 Name        : gprs_app.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : gprs应用层
 ============================================================================
 */
#include "includes.h"

//gprs_ATcmdTx中不需要第二个期望应答
#define GPRS_NO_HOPEACK2    0
//gprs_ATcmdTx中不需要返回AT指令字符串
#define GPRS_NO_BACK        0
//服务器地址
#define SERVER_IP           "106.13.1.239"
//服务器端口
#define SERVER_PORT         8888
//栈中at字符串长度
#define AT_STR_STACK_LEN    256

taskManageTypedef gprs_tm = {0,0,0};

/*============================================================================
 通用AT指令发送函数
 cmd：AT指令
 hopeAck1：期望应答1
 hopeAck2：期望应答2
 pBack：返回指针
 timeoutMs：超时 毫秒
 retry：重试次数
 return：成败
 ============================================================================*/
static uint16_t gprs_ATcmdTx(const uint8_t *cmd,const uint8_t *hopeAck1,const uint8_t *hopeAck2,uint8_t *pBack,uint16_t timeoutMs,uint8_t retry )
{
    uint8_t res = FALSE;
    uint8_t atBuf[AT_STR_STACK_LEN] = {0};
    uint8_t atBufpW = 0;
    uint8_t i;
    uint32_t endTick;

    for(i = 0;i < retry; i++)
    {
        endTick = gprs_bsp.getTickMs() + timeoutMs;
        memset(atBuf,0,sizeof(atBuf));
        atBufpW = 0;
        if(gprs_bsp.write((uint8_t *)cmd,strlen((char const *)cmd)) == TRUE)
        {
            while(1)
            {
                atBufpW += gprs_bsp.read(atBuf + atBufpW,128 - atBufpW,100);
                
                if(hopeAck1 == 0)
                {
                    res = FALSE;
                    break;
                }

                if(strstr((char const *)atBuf,(char const *)hopeAck1) != 0)
                {
                    res = TRUE;
                    break;
                }
                else if(hopeAck2 != 0)
                {
                    if(strstr((char const *)atBuf,(char const *)hopeAck2) != 0)
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

            }
        }

        if(res == TRUE)
        {
            if(pBack != 0)
            {
                memcpy(pBack,atBuf,AT_STR_STACK_LEN);
            }
            break;
        }
    }
	
	return res;
}

/*============================================================================
 gprs任务
 ============================================================================*/
void gprs_task()
{
    uint8_t res = 0;
    uint8_t atbufBack[AT_STR_STACK_LEN] = {0};
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
            if(dht11_tm.step == DHT11_STEP_FINISH)
            {
                res = gprs_ATcmdTx("AT+CPIN?\r","OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,1000,10);
                if(res == TRUE)
                {
                    DBG_PRT("gprs AT+CPIN OK!\n");
                    gprs_tm.step++;
                    gprs_tm.execuTick = gprs_bsp.getTickMs() + 10000;
                }
                else
                {
                    DBG_PRT("gprs AT+CPIN ERR!\n");
                    gprs_tm.errCnt++;
                    gprs_tm.step = GPRS_STEP_RESET;
                }
            }
            
            break;
        case GPRS_STEP_CGREG:
            res = gprs_ATcmdTx("AT+CGREG?\r","CGREG: 0,5","CGREG: 0,1",GPRS_NO_BACK,2000,5);  
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
        case GPRS_STEP_PPP:
            res = gprs_ATcmdTx("AT+XIIC?\r","+XIIC:    1","+XIIC:    0",atbufBack,1000,10);  
            if(res == TRUE)
            {
                if(strstr((char const *)atbufBack,"+XIIC:    1") != 0)
                {
                    DBG_PRT("gprs PPP OK!\n");
                    gprs_tm.step++;
                }
                else
                {
                    DBG_PRT("gprs PPP need to be connected!\n");
                    res = gprs_ATcmdTx("AT+XIIC=1\r","OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,1000,10);
                    if(res == TRUE)
                    {
                        DBG_PRT("gprs PPP OK!\n");
                        gprs_tm.execuTick = gprs_bsp.getTickMs() + 3000;
                    }
                    else
                    {
                        DBG_PRT("gprs PPP ERR!\n");
                        gprs_tm.errCnt++;
                        gprs_tm.step = GPRS_STEP_RESET;
                    }
                }
                
            }
            else
            {
                DBG_PRT("gprs PPP ERR!\n");
                gprs_tm.errCnt++;
                gprs_tm.step = GPRS_STEP_RESET;
            }
            break;
        case GPRS_STEP_UPDATE_TIME:
            res = gprs_ATcmdTx("AT+UPDATETIME=1,time.windows.com,10\r","Update To",GPRS_NO_HOPEACK2,atbufBack,11000,3);  
            if(res == TRUE)
            {
                DBG_PRT("gprs AT+UPDATETIME OK!\n");
                gprs_getTime(atbufBack);
                gprs_tm.step++;
            }
            else
            {
                DBG_PRT("gprs AT+UPDATETIME ERR!\n");
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
            if(dht11_tm.step == DHT11_STEP_FINISH && sgp30_tm.step == SGP30_STEP_FINISH)
            {
                snprintf((char*)cmd,128,"AT+TCPTRANS=%s,%d\r",SERVER_IP,SERVER_PORT);
                res = gprs_ATcmdTx(cmd,"OK",GPRS_NO_HOPEACK2,GPRS_NO_BACK,5000,3);  
                if(res == TRUE)
                {
                    DBG_PRT("gprs %s OK!\n",cmd);
                    gprs_tm.execuTick = gprs_bsp.getTickMs() + 2000;
                    gprs_tm.step++;
                }
                else
                {
                    DBG_PRT("gprs %s ERR!\n",cmd);
                    gprs_tm.errCnt++;
                    gprs_tm.step = GPRS_STEP_RESET;
                }
            }
            
            break;
        case GPRS_STEP_IN_TRANS:
            if(proc_tm.step == PROC_STEP_FINISH)
            {
                DBG_PRT("transport finish!\n");
                gprs_tm.step++;
            }
            proc_task();
            break;
        case GPRS_STEP_POWER_OFF:
            GPRS_POWER_OFF;
            DBG_PRT("gprs power off!\n");
            gprs_tm.step++;
            break;
        default:
            break;

    }

        
}