#include "gprs_app.h"
#include "gprs_bsp.h"
#include "usart.h"

uint16_t gprs_ATcmdTx(const uint8_t *cmd,const uint8_t *hopeAck,uint16_t timeoutMs,uint8_t retry )
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
                while(gprsRB.pW - gprsRB.pR)
                {
                    atBuf[(atBufpW++) % 128] = gprsRB.pRecvBuf[(gprsRB.pR++) % GPRS_RECV_BUF_LEN];
                }

                if(strstr(atBuf,hopeAck) != 0)
                {
                    res = TRUE;
                    break;
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
            break;
        }
    }
}

void gprs_task()
{
    uint8_t res = 0;

    

    gprs_bsp.dly_ms(1000);
    uint8_t data[] = {0xa5,0x5a,0x11,0x00,0x03,0x31
    ,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x4b};
    while(1)
    {
        res = gprs_ATcmdTx("AT+TCPTRANS=106.13.1.239,8888\r","OK",1000,3);
        if(res == TRUE)
        {
            
            printf("gprs ok\n");
        }
        else
        {
            printf("gprs ok\n");
        }
		
		HAL_UART_Transmit(&huart1,data,sizeof(data),100);
        gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
		gprs_bsp.dly_ms(1000);
        
    }
}