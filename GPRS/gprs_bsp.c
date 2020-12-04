#include "gprs_bsp.h"
#include "usart.h"

gprs_RBTypedef gprsRB = {{0},0,0};
uint8_t stm32_hal_uart_irq_rx = 0xff;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        gprsRB.pRecvBuf[(gprsRB.pW++) % GPRS_RECV_BUF_LEN] = stm32_hal_uart_irq_rx;
        HAL_UART_Receive_IT(&huart1,&stm32_hal_uart_irq_rx,1);
    }
}

static void gprs_bsp_dly_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

static void gprs_bsp_reset(void)
{
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_RESET);
    gprs_bsp_dly_ms(150);
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_SET);

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
    .reset      =   gprs_bsp_reset,
    .dly_ms     =   gprs_bsp_dly_ms,
    .getTickMs  =   gprs_bsp_getTick,
    .write      =   gprs_bsp_write
};