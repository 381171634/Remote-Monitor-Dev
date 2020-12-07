#include "sgp30_app.h"
#include "sgp30_bsp.h"
#include "main.h"
#include "dht11_app.h"

taskManageTypedef sgp30_tm = {0,0,0};
sgp30ResTypedef sgp30Res;

//由温度、相对湿度-绝对湿度表，获得绝对湿度g/m^3,然后写入sgp30
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

void sgp30_task()
{
	uint8_t res;
	uint8_t i;
	uint16_t cmd; 
	uint32_t CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
	uint32_t sgp30_dat;

    //运行时间未到 返回
    if(SGP30_GET_TICK() - sgp30_tm.execuTick > 0xffffffff / 2
        || sgp30_tm.step == SGP30_STEP_FINISH)
    {
        return;
    }

	switch(sgp30_tm.step)
	{
		case SGP30_STEP_START:
			memset((void *)&sgp30Res,0,sizeof(sgp30Res));
			SGP30_Init();
			SGP30_DELAY_MS(100);
			cmd = 0x0820;
			SGP30_Write((uint8_t *)&cmd,2);
			res = SGP30_Read(&sgp30_dat);
			if(res == TRUE)
			{
				DBG_PRT("sgp30 start OK!\n");
				sgp30_tm.execuTick = SGP30_GET_TICK() + 15000;//等15s稳定
				sgp30_tm.step++;
			}
			else
			{
				sgp30_tm.errCnt++;
				DBG_PRT("sgp30 start ERR!\n");
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
		case SGP30_STEP_READ:
			for(i = 0;i < 3; i++)
			{
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
					sgp30Res.co2_avg = sgp30Res.tvoc_sum / SGP30_AVERAGE_CNT;
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

void sgp30test()
{
	uint8_t res;
	uint16_t cmd;
	uint32_t CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
	uint32_t sgp30_dat;

	SGP30_Init(); //初始化SGP30
	SGP30_DELAY_MS(100);
	cmd = 0x0820;
	SGP30_Write((uint8_t *)&cmd,2);
	res = SGP30_Read(&sgp30_dat); //读取SGP30的值
	CO2Data = (sgp30_dat & 0xffff0000) >> 16;
	TVOCData = sgp30_dat & 0x0000ffff;
	//SGP30模块开机需要一定时间初始化，在初始化阶段读取的CO2浓度为400ppm，TVOC为0ppd且恒定不变，因此上电后每隔一段时间读取一次
	//SGP30模块的值，如果CO2浓度为400ppm，TVOC为0ppd，发送“正在检测中...”，直到SGP30模块初始化完成。
	//初始化完成后刚开始读出数据会波动比较大，属于正常现象，一段时间后会逐渐趋于稳定。
	//气体类传感器比较容易受环境影响，测量数据出现波动是正常的，可自行添加滤波函数。
	// while (CO2Data == 400 && TVOCData == 0)
	// {
	// 	SGP30_Write(0x20, 0x08);
	// 	SGP30_Read(&sgp30_dat);				  //读取SGP30的值
	// 	CO2Data = (sgp30_dat & 0xffff0000) >> 16; //取出CO2浓度值
	// 	TVOCData = sgp30_dat & 0x0000ffff;		  //取出TVOC值
	// 	DBG_PRT("test ing...\r\n");
	// 	SGP30_DELAY_MS(500);
	// }
	SGP30_DELAY_MS(15000);
	while (1)
	{
		cmd = 0x0820;
		SGP30_Write((uint8_t *)&cmd,2);
		SGP30_Read(&sgp30_dat);				  //读取SGP30的值
		CO2Data = (sgp30_dat & 0xffff0000) >> 16; //取出CO2浓度值
		TVOCData = sgp30_dat & 0x0000ffff;		  //取出TVOC值
		DBG_PRT("CO2:%dppm\r\nTVOC:%dppd\r\n", CO2Data, TVOCData);
		SGP30_DELAY_MS(500);

	}
}
