#include "rtc.h"

#define BASE_YEAR	70	//世纪秒的年份基准，BASE_YEAR = 基准年份 - 1900
#define BASE_MON	1	//世纪秒的月份基准，一般是1月
#define BASE_DAY	1	//世纪秒的日基准  ，一般是1日
						//时分秒都默认是0

//是否是UTC时间,单片机中使用为1(因为没有设置时区)，电脑中使用需要为0(因为电脑调用time.h中的相关函数时，调用了系统时区)
#define USE_UTC 1

//月份天数表
unsigned char Montbl[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

//是否闰年
unsigned char IsLeap(int year)
{
	unsigned char res = 0;
	if( (year % 4 == 0) && (year % 4 != 100) )
	{
		res = 1;
	}

	return res;
}

//计算世纪秒 从BASE_YEAR-BASE_MON-BASE_DAY 0时0分0秒到某一时间经过的秒
//注意，tm_mon = [0,11]
//后期要对输入的时间加合法判定
uint32_t my_mktime(struct tm* p)
{
	uint32_t res = 0;

	int year_s = BASE_YEAR, mon_s = 0;
	unsigned char leap = 0;//最新年份是否闰年

	if( IsLeap(p->tm_year + 1900))
	{
		leap = 1;
	}

	//按年累加
	while(year_s < p->tm_year)
	{
		res += 365 * 24 * 3600;
		if(IsLeap(year_s + 1900))
		{
			res += 24 * 3600;
		}

		year_s++;
	}

	//具体到月
	while(mon_s < p->tm_mon)
	{
		//如果月份为2且是闰年
		if(mon_s == 1 && leap)
		{
			res += (Montbl[mon_s] + 1) * 24 * 3600;
		}
		else
		{
			res += Montbl[mon_s] * 24 * 3600;
		}

		mon_s++;
	}

	//具体到日
	res += (p->tm_mday - 1) * 24 * 3600;

	//具体到时分秒
	res += 3600 * p->tm_hour + 60 * p->tm_min + p->tm_sec;

	//如果加上了本地时区（东八区）
	if(!USE_UTC)
	{
		res -= 3600 * 8;
	}

	return res;
}