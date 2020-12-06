#include "sgp30_app.h"
#include "sgp30_bsp.h"
#include "main.h"

taskManageTypedef sgp30_tc = {0,0,0};

//由温度、相对湿度-绝对湿度表，获得绝对湿度g/m^3
float getHumidity(float tempture)
{
    return (25.5*exp(0.07718*tempture) - 25.25*exp(0.07731*tempture))/5*0.34;
}

void sgp30_task()
{
    
}

void sgp30test()
{
	uint8_t res;
	uint32_t CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
	uint32_t sgp30_dat;

	SGP30_Init(); //初始化SGP30
	SGP30_DELAY_MS(100);
	SGP30_Write(0x20, 0x08);
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
		SGP30_Write(0x20, 0x08);
		SGP30_Read(&sgp30_dat);				  //读取SGP30的值
		CO2Data = (sgp30_dat & 0xffff0000) >> 16; //取出CO2浓度值
		TVOCData = sgp30_dat & 0x0000ffff;		  //取出TVOC值
		DBG_PRT("CO2:%dppm\r\nTVOC:%dppd\r\n", CO2Data, TVOCData);
		SGP30_DELAY_MS(500);

		//SGP30_Reset();
	}
}
