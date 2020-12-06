#include "gprs_bsp.h"
#include "usart.h"
#include "stm32f1xx_it.h"

gprs_RBTypedef gprsRB = {{0},0,0,0};

void USART1_IRQHandler()
{
    uint8_t idle_clr;
    uint32_t isrflags   = READ_REG(huart1.Instance->SR);
    if(isrflags & (UART_FLAG_RXNE | UART_FLAG_IDLE) )
    {
        gprsRB.pRecvBuf[(gprsRB.pW++) % GPRS_RECV_BUF_LEN] = huart1.Instance->DR;
    }
    else
    {
        HAL_UART_IRQHandler(&huart1);
    }
}

static void gprs_bsp_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPRS_RESET_Port, GPRS_RESET_Pin, GPIO_PIN_RESET);
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPRS_POWER_Port, GPRS_POWER_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPRS_POWER_Pin|GPRS_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
}

static void gprs_bsp_dly_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

static void gprs_bsp_reset(void)
{
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_SET);
    gprs_bsp_dly_ms(150);
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_RESET);

}

static uint8_t gprs_bsp_write(uint8_t *pSrc,uint16_t len)
{
    if(HAL_UART_Transmit(&huart1,pSrc,len,100) == HAL_OK)
    {
        return TRUE;
    }
    
    return FALSE;
}

static uint32_t gprs_bsp_getTick(void)
{
    return HAL_GetTick();
}

gprs_bspTypedef gprs_bsp = {
    .init       =   gprs_bsp_init,
    .reset      =   gprs_bsp_reset,
    .dly_ms     =   gprs_bsp_dly_ms,
    .getTickMs  =   gprs_bsp_getTick,
    .write      =   gprs_bsp_write
};