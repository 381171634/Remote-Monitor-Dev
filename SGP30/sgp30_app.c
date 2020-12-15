/*
 ============================================================================
 Name        : sgp30_app.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : sgp30应用层
 ============================================================================
 */
#include "sgp30_app.h"
#include "sgp30_bsp.h"
#include "main.h"
#include "dht11_app.h"
#include "math.h"

taskManageTypedef sgp30_tm = {0,0,0};
sgp30ResTypedef sgp30Res;


/*============================================================================
 sgp30湿度补偿
 由当前温度、相对湿度获得绝对湿度g/m^3
 写入sgp30中进行湿度补偿
 T：温度
 R：相对湿度
 ============================================================================*/
static uint8_t sgp30_fixDiff(float T,float R)
{
	float fixDiff;
	uint8_t fixDiff_H,fixDiff_L;	//H 整数部分，L小数部分
	uint8_t sendBuf[5] = {0x20,0x61,0,0,0};
	fixDiff = 216.7 * (R/100 * 6.112 * exp(17.62*T / (243.12 + T)) / (273.15 + T));

	fixDiff_H = (int)fixDiff;		
	fixDiff_L = (int)((fixDiff - fixDiff_H) * 256 + 0.5);

	sendBuf[2] = fixDiff_H;
	sendBuf[3] = fixDiff_L;
	sendBuf[4] = sgp30_crc(&sendBuf[2],2);
	
	return SGP30_Write(sendBuf,5);
}


/*============================================================================
 sgp30任务
 ============================================================================*/
void sgp30_task()
{
	uint8_t res;
	uint8_t i;
	uint16_t cmd; 
	uint32_t CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
	uint32_t sgp30_dat;
	static int test_cnt = 0;

    //运行时间未到 返回
    if(SGP30_GET_TICK() - sgp30_tm.execuTick > 0xffffffff / 2
        || sgp30_tm.step == SGP30_STEP_FINISH)
    {
        return;
    }

	switch(sgp30_tm.step)
	{
		case SGP30_STEP_START:
			test_cnt = 0;
			memset((void *)&sgp30Res,0,sizeof(sgp30Res));
			res = SGP30_Init();
			if(res == TRUE)
			{
				SGP30_DELAY_MS(100);
				cmd = 0x0820;
				SGP30_Write((uint8_t *)&cmd,2);
				SGP30_DELAY_MS(20);
				res = SGP30_Read(&sgp30_dat);
				if(res == TRUE)
				{
					DBG_PRT("sgp30 start OK!\n");
					sgp30_tm.execuTick = SGP30_GET_TICK() + 10000;//等10s稳定
					sgp30_tm.step++;
				}
				else
				{
					sgp30_tm.errCnt++;
					DBG_PRT("sgp30 start ERR!\n");
				}
			}
			else
			{
				DBG_PRT("sgp30 init err!\n");
				sgp30_tm.errCnt++;
			}
			
			break;
		case SGP30_STEP_FIX:
			if(dht11_tm.step == DHT11_STEP_FINISH)
			{
				if(sgp30_fixDiff(dht11Res.dht11_temp_avg / 1000.0,dht11Res.dht11_wet_avg / 1000.0) == TRUE)
				{
					DBG_PRT("sgp30 fixDiff OK!\n");
					sgp30_tm.step++;
				}
				else
				{
					sgp30_tm.errCnt++;
					DBG_PRT("sgp30 fixDiff ERR!\n");
				}
			}
			break;
		case SGP30_STEP_WAIT_INIT:
			for(i = 0;i < 3; i++)
			{
				cmd = 0x0820;
				SGP30_Write((uint8_t *)&cmd,2);
				SGP30_DELAY_MS(20);
				if(TRUE == SGP30_Read(&sgp30_dat))
				{
					break;
				}
			}

			if(i < 3)
			{
				CO2Data = (sgp30_dat & 0xffff0000) >> 16; //取出CO2浓度值
				TVOCData = sgp30_dat & 0x0000ffff;		  //取出TVOC值
				if(CO2Data != 400 || TVOCData !=0)
				{
					sgp30_tm.step++;
					DBG_PRT("sgp30 init ok!\n");
				}
				else
				{
					DBG_PRT("sgp30 is testing...!\n");
					if(++test_cnt >= 10)
					{
						sgp30_tm.step = SGP30_STEP_START;
					}
				}
				sgp30_tm.execuTick = SGP30_GET_TICK() + 1000;
			}
			else
			{
				sgp30_tm.errCnt++;
				DBG_PRT("sgp30 read ERR!:%d\n");
			}
			
			break;
		case SGP30_STEP_READ:
			for(i = 0;i < 3; i++)
			{
				cmd = 0x0820;
				SGP30_Write((uint8_t *)&cmd,2);
				SGP30_DELAY_MS(20);
				if(TRUE == SGP30_Read(&sgp30_dat))
				{
					break;
				}
			}

			if(i < 3)
			{
				DBG_PRT("sgp30 read  ok!:%d\n",sgp30Res.readCnt + 1);
				CO2Data = (sgp30_dat & 0xffff0000) >> 16; //取出CO2浓度值
				TVOCData = sgp30_dat & 0x0000ffff;		  //取出TVOC值
				sgp30Res.tvoc_sum += TVOCData * 1000;
				sgp30Res.co2_sum  += CO2Data * 1000;

				if(++sgp30Res.readCnt >= SGP30_AVERAGE_CNT)
				{
					sgp30Res.tvoc_arg = sgp30Res.tvoc_sum / SGP30_AVERAGE_CNT;
					sgp30Res.co2_avg = sgp30Res.co2_sum / SGP30_AVERAGE_CNT;
					sgp30_tm.step++;
					DBG_PRT("sgp30 read finish!tvoc average:%d co2 average:%d\n",sgp30Res.tvoc_arg,sgp30Res.co2_avg);
				}
				else
				{
					sgp30_tm.execuTick = SGP30_GET_TICK() + 1000;
				}
			}
			else
			{
				DBG_PRT("SGP30 read  fail\n");
                sgp30_tm.errCnt++;
                sgp30_tm.step = SGP30_STEP_START;
			}
			break;
		case SGP30_STEP_LOW_POWER:
			if(SGP30_Reset() == TRUE)
			{
				DBG_PRT("SGP30 goto lowpower OK\n");
				sgp30_tm.step++;
			}
			else
			{
				DBG_PRT("SGP30 goto lowpower ERR\n");
				sgp30_tm.errCnt++;
			}
			break;
		default:
			break;
	}
}

