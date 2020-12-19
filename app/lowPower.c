/*
 ============================================================================
 Name        : lowPower.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 低功耗处理
 ============================================================================
 */

#include "rtc.h"
#include "lowPower.h"
#include "dht11_app.h"
#include "dht11_bsp.h"
#include "gprs_app.h"
#include "sgp30_app.h"
#include "gprs_bsp.h"
#include "sgp30_bsp.h"
#include "gpio.h"
#include "usart.h"

/*============================================================================
 低功耗RTC唤醒中断
 ============================================================================*/
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    HAL_NVIC_SystemReset();
}

/*============================================================================
 低功耗进入函数
 ============================================================================*/
void enter_lowPwr()
{
    RTC_AlarmTypeDef alarm;
    RTC_TimeTypeDef time;
    memset((void *)&alarm, 0, sizeof(alarm));

    //任务未完成，因2分超时进入低功耗时，需要对各模块做低功耗处理
    if(dht11_tm.step != DHT11_STEP_FINISH)
    {
        DHT11_POWER_OFF;
    }

    if(sgp30_tm.step != SGP30_STEP_FINISH)
    {
        SGP30_Reset();
    }

    if(gprs_tm.step != GPRS_STEP_FINISH)
    {
        GPRS_POWER_OFF;
    }

    //除了控制GPRS模块的DCDC使能，其余引脚全配置成浮空输入
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All /* & ~(GPIO_PIN_13 | GPIO_PIN_14) */);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All & ~(DCDC_ENABLE_Pin | DHT11_POWER_Pin));
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);

    

    

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);

    DBG_PRT("time->%02d:%02d:%02d\n",time.Hours,time.Minutes,time.Seconds);
    
    // time.Seconds = 0;
    // time.Minutes += 5;
    // if(time.Minutes > 59)
    // {
    //     time.Hours += 1;
    //     time.Minutes %= 60;
    //     if (time.Hours > 23)
    //         time.Hours = 0;
    // }

    // time.Seconds += 5;
    // if(time.Seconds > 59)
    // {
    //     time.Seconds %= 60;
    //     time.Minutes += 1;
    //     if(time.Minutes > 59)
    //     {
    //         time.Hours += 1;
    //         time.Minutes = 0;
    //         if (time.Hours > 23)
    //             time.Hours = 0;
    //     }
    // }


    time.Seconds = 0;
    time.Minutes = 0;
    time.Hours += 1;
    if (time.Hours > 23)
    {
        time.Hours = 0;
    }
    
    

    alarm.AlarmTime.Hours = time.Hours;
    alarm.AlarmTime.Minutes = time.Minutes;
    alarm.AlarmTime.Seconds = time.Seconds;

    HAL_RTC_SetAlarm_IT(&hrtc,&alarm,RTC_FORMAT_BIN);
    
    SysTick->CTRL = 0x00;   //关闭定时器
    SysTick->VAL = 0x00;    //清空val,清空定时器
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);
}
