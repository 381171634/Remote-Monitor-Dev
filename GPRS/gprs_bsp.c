/*
 ============================================================================
 Name        : gprs_bsp.c
 Author      : wy
 Version     :
 Copyright   : Your copyright notice
 Description : gprs板级 包含所有gprs的板级基础操作
 ============================================================================
 */
#include "gprs_bsp.h"
#include "usart.h"
#include "stm32f1xx_it.h"
#include <string.h>

gprs_RBTypedef gprsRB;

/*============================================================================
 gprs对应串口的中断处理函数
 来接收中断后将数据放入gprs环形缓存
 来空闲中断则置位gprs串口通道的标志
 ============================================================================*/
void USART1_IRQHandler()
{
    uint32_t isrflags   = READ_REG(huart1.Instance->SR);
    if(isrflags & (UART_FLAG_RXNE | UART_FLAG_IDLE) )
    {
        gprsRB.pRecvBuf[(gprsRB.pW++) % GPRS_RECV_BUF_LEN] = huart1.Instance->DR;
        if(isrflags & UART_FLAG_IDLE)
        {
            gprsRB.uart_idle_flag = 1;
        }
    }
    else
    {
        HAL_UART_IRQHandler(&huart1);
    }
}

/*============================================================================
 初始化 包括gpio、串口配置等
 ============================================================================*/
static void gprs_bsp_init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPRS_RESET_Port, GPRS_RESET_Pin, GPIO_PIN_RESET);
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPRS_POWER_Port, GPRS_POWER_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPRS_POWER_Pin|GPRS_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);

    memset((void *)&gprsRB,0,sizeof(gprsRB));
}

/*============================================================================
 延时 毫秒
 ============================================================================*/
static void gprs_bsp_dly_ms(uint16_t ms)
{
    HAL_Delay(ms);
}

/*============================================================================
 gprs复位
 ============================================================================*/
static void gprs_bsp_reset(void)
{
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_SET);
    gprs_bsp_dly_ms(150);
    HAL_GPIO_WritePin(GPRS_RST_GPIO_Port,GPRS_RST_Pin,GPIO_PIN_RESET);

}

/*============================================================================
 gprs获取时间戳
 ============================================================================*/
static uint32_t gprs_bsp_getTick(void)
{
    return HAL_GetTick();
}

/*============================================================================
 向gprs模块写数据
 pSrc：源指针
 len：长度
 ============================================================================*/
static uint8_t gprs_bsp_write(uint8_t *pSrc,uint16_t len)
{
    if(HAL_UART_Transmit(&huart1,pSrc,len,100) == HAL_OK)
    {
        return TRUE;
    }
    
    return FALSE;
}

/*============================================================================
 从gprs串口通道中读数据
 pDes：目标指针
 len：长度
 timeout：超时 毫秒
 ============================================================================*/
static uint16_t gprs_bsp_read(uint8_t *pDes,uint16_t len,uint16_t timeout)
{
    uint16_t retval = 0;
    uint32_t endTick = gprs_bsp_getTick() + timeout;
    while(1)
    {
        if(gprsRB.uart_idle_flag == 1)
        {
            gprsRB.uart_idle_flag = 0;
            while((gprsRB.pW - gprsRB.pR) && retval < len)
            {
                pDes[retval++] = gprsRB.pRecvBuf[(gprsRB.pR++) % GPRS_RECV_BUF_LEN];
            }
        }

        if(gprs_bsp_getTick() - endTick < 0xffffffff/2 || retval >= len)
        {
            break;
        }
    }

    return retval;
}

/*============================================================================
 gprs底层封装
 ============================================================================*/
gprs_bspTypedef gprs_bsp = {
    .init       =   gprs_bsp_init,
    .reset      =   gprs_bsp_reset,
    .dly_ms     =   gprs_bsp_dly_ms,
    .getTickMs  =   gprs_bsp_getTick,
    .write      =   gprs_bsp_write,
    .read       =   gprs_bsp_read
};