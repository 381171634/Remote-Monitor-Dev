/*
 ============================================================================
 Name        : lowPower.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : 低功耗处理
 ============================================================================
 */

#include "includes.h"

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
    GPIO_InitTypeDef GPIO_InitStruct;
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

    //GPRS模块的DCDC引脚保持使能，拉低
    //dht11的VCC引脚推挽输出高，同时DATA也被拉高
    //其余引脚配置为模拟输入
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_All & ~(DCDC_ENABLE_Pin);
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DHT11_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_POWER_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DHT11_POWER_Port,DHT11_POWER_Pin,GPIO_PIN_SET);
    

    

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
    //HAL_PWR_EnterSTANDBYMode();
}
