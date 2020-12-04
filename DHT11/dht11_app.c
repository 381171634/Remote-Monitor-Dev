#include "dht11_bsp.h"
#include "dht11_app.h"

uint8_t app_dht11Read(dht11DataTypedef *pSrc)
{
    uint8_t res = TRUE;
    uint16_t i,j,highCnt,lowCnt;
    uint8_t recvData[5] = {0};
    int32_t timeout_us;

    dht11_bsp.power_set(DHT11_POWER_ON);                //上电

    dht11_bsp.set_sda_mode(DHT11_PIN_MODE_OUTPUT);      //sda输出高
    dht11_bsp.write_sda(DHT11_PIN_STATE_H);
    dht11_bsp.dly_ms(1500);                             //上电稳定1秒,电压越低，需要稳定的时间越长
    dht11_bsp.write_sda(DHT11_PIN_STATE_L);             //起始信号
    dht11_bsp.dly_ms(18);
    dht11_bsp.write_sda(DHT11_PIN_STATE_H);
    dht11_bsp.set_sda_mode(DHT11_PIN_MODE_INPUT);       //转为输入 等待应答信号

    timeout_us = 35;                                    //等待响应低
    while(timeout_us-- && (dht11_bsp.read_sda() != DHT11_PIN_STATE_L))
    {
        dht11_bsp.dly_us(1);
    }

    if(timeout_us <= 0)                                 //未响应直接返回
    {
        res = FALSE;
        goto exit;
    }

    timeout_us = 88;                                    //等待响应高    
    while(timeout_us-- && (dht11_bsp.read_sda() != DHT11_PIN_STATE_H))                                 
    {
        dht11_bsp.dly_us(1);
    }

    if(timeout_us <= 0)                                 //未响应直接返回
    {
        res = FALSE;
        goto exit;
    }

    timeout_us = 56;                                    //响应结束，等待开始发送数据
    while(timeout_us-- && (dht11_bsp.read_sda() != DHT11_PIN_STATE_L))
    {
        dht11_bsp.dly_us(1);
    }   

    if(timeout_us <= 0)                                 //未响应直接返回
    {
        res = FALSE;
        goto exit;
    }

    for(i = 0;i < 5; i++)
    {
        for(j = 0;j < 8;j ++)
        {
            highCnt = 0;
			lowCnt = 0;
            
            while(dht11_bsp.read_sda() == DHT11_PIN_STATE_L && lowCnt <= 60 )
            {
                lowCnt++;
                dht11_bsp.dly_us(1);
            }
            while(dht11_bsp.read_sda() == DHT11_PIN_STATE_H && highCnt <= 70 )
            {
                highCnt++;
                dht11_bsp.dly_us(1);
            }
            if(highCnt >= lowCnt)
            {
                recvData[i] |= (1 << (7 - j));
            }
			
        }
    }

    //判校验和
    if(recvData[0] + recvData[1] + recvData[2] + recvData[3] != recvData[4] )
    {
        res = FALSE;
        goto exit;
    }
    else
    {
        pSrc->wet_H = recvData[0];
        pSrc->wet_L = recvData[1];
        pSrc->temp_H = recvData[2];
        pSrc->temp_L = recvData[3];
        pSrc->sum = recvData[4];
    }

exit:
    dht11_bsp.power_set(DHT11_POWER_OFF);                //断电
    return res;
}