#include "dht11_bsp.h"


static void dht11_bsp_power_set(uint8_t state)
{
    if(state == DHT11_POWER_ON)
    {
        HAL_GPIO_WritePin(DHT11_VCC_GPIO_Port,DHT11_VCC_Pin,GPIO_PIN_SET);
    }
    else if(state == DHT11_POWER_OFF)
    {
        HAL_GPIO_WritePin(DHT11_VCC_GPIO_Port,DHT11_VCC_Pin,GPIO_PIN_RESET);
    }
}

static void dht11_bsp_set_sda_mode(uint8_t mode)
{
    if(mode == DHT11_PIN_MODE_INPUT)
    {
        DHT11_DATA_GPIO_Port->CRH &= ~(0xf);
        DHT11_DATA_GPIO_Port->CRH |= 0x04;
    }
    else if(mode == DHT11_PIN_MODE_OUTPUT)
    {
        DHT11_DATA_GPIO_Port->CRH &= ~(0xf);
        DHT11_DATA_GPIO_Port->CRH |= 0x05;
    }
    
}

static void dht11_bsp_write_sda(uint8_t state)
{
    HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port,DHT11_DATA_Pin,state);
}

uint8_t dht11_bsp_read_sda()
{
    return HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port,DHT11_DATA_Pin);
}

static void dht11_bsp_dly_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

//8MHz
static void dht11_bsp_dly_us(uint16_t us)
{
    while(us--)
    {
        __NOP();__NOP();__NOP();__NOP();
        __NOP();__NOP();__NOP();__NOP();
    }
}

dht11_bspTypedef dht11_bsp = {
    .power_set      = dht11_bsp_power_set,
    .set_sda_mode   = dht11_bsp_set_sda_mode,
    .write_sda      = dht11_bsp_write_sda,
    .read_sda       = dht11_bsp_read_sda,
    .dly_ms         = dht11_bsp_dly_ms,
    .dly_us         = dht11_bsp_dly_us
};