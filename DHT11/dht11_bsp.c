/*
 ============================================================================
 Name        : dht11_bsp.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : dht11板级 包含所有dht11的板级基础操作
 ============================================================================
 */
#include "dht11_bsp.h"
#include "gpio.h"

/*============================================================================
 gpio初始化
 ============================================================================*/
static void dht11_bsp_gpio_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(DHT11_SDA_Port, DHT11_SDA_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DHT11_POWER_Port, DHT11_POWER_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = DHT11_POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_POWER_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DHT11_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_SDA_Port, &GPIO_InitStruct);
}

/*============================================================================
 ms延时
 ============================================================================*/
static void dht11_bsp_dly_ms(uint16_t ms)
{
    HAL_Delay(ms);
}


/*============================================================================
 us延时->主频8MHz时
 ============================================================================*/
static void dht11_bsp_dly_us(uint16_t us)
{
    while(us--)
    {
        __NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();
    }
}

/*============================================================================
 获得时间戳，ms
 ============================================================================*/
static uint32_t dht11_bsp_getTick()
{
    return HAL_GetTick();
}

/*============================================================================
 底层封装
 ============================================================================*/
dht11_bspTypedef dht11_bsp = {
    .gpio_init      = dht11_bsp_gpio_init,
    .dly_ms         = dht11_bsp_dly_ms,
    .dly_us         = dht11_bsp_dly_us,
    .getTick        = dht11_bsp_getTick,
};