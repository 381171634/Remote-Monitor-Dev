#ifndef _SGP30_BSP_H
#define _SGP30_BSP_H

#include "stm32f1xx_hal.h"

#define SGP30_SCL_GPIO_PORT     SGP_IIC_CLK_GPIO_Port
#define SGP30_SCL_GPIO_PIN      SGP_IIC_CLK_Pin

#define SGP30_SDA_GPIO_PORT     SGP30_IIC_SDA_GPIO_Port
#define SGP30_SDA_GPIO_PIN      SGP30_IIC_SDA_Pin


#define SGP30_SCL_H             HAL_GPIO_WritePin(SGP_IIC_CLK_GPIO_Port,SGP_IIC_CLK_Pin,GPIO_PIN_SET)                                
#define SGP30_SCL_L             HAL_GPIO_WritePin(SGP_IIC_CLK_GPIO_Port,SGP_IIC_CLK_Pin,GPIO_PIN_RESET)

#define SGP30_SDA_H             HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT,SGP30_SDA_GPIO_PIN,GPIO_PIN_SET)                                
#define SGP30_SDA_L             HAL_GPIO_WritePin(SGP30_SDA_GPIO_PORT,SGP30_SDA_GPIO_PIN,GPIO_PIN_RESET)

#define SGP30_SDA_READ          HAL_GPIO_ReadPin(SGP30_SDA_GPIO_PORT,SGP30_SDA_GPIO_PIN)

#define SGP30_read  0xb1  //SGP30的读地址
#define SGP30_write 0xb0  //SGP30的写地址

void SGP30_DELAY_US(uint16_t us);
void SGP30_DELAY_MS(uint16_t ms);

void SGP30_IIC_Start(void);				//发送IIC开始信号
void SGP30_IIC_Stop(void);	  			//发送IIC停止信号
void SGP30_IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
uint16_t SGP30_IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
uint8_t SGP30_IIC_Wait_Ack(void); 				//IIC等待ACK信号
void SGP30_IIC_Ack(void);					//IIC发送ACK信号
void SGP30_IIC_NAck(void);				//IIC不发送ACK信号
void SGP30_IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t SGP30_IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	
void SGP30_Init(void);				  
void SGP30_Write(uint8_t a, uint8_t b);
uint8_t SGP30_Read(uint32_t *pDes);



#endif

