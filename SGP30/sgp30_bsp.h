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

#define SGP30_read  0xb1  //SGP30�Ķ���ַ
#define SGP30_write 0xb0  //SGP30��д��ַ

void SGP30_DELAY_US(uint16_t us);
void SGP30_DELAY_MS(uint16_t ms);
uint32_t SGP30_GET_TICK();
uint8_t sgp30_crc(uint8_t *p,uint8_t len);
uint8_t SGP30_Reset();

void SGP30_IIC_Start(void);				//����IIC��ʼ�ź�
void SGP30_IIC_Stop(void);	  			//����IICֹͣ�ź�
void SGP30_IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint16_t SGP30_IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t SGP30_IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void SGP30_IIC_Ack(void);					//IIC����ACK�ź�
void SGP30_IIC_NAck(void);				//IIC������ACK�ź�
void SGP30_IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t SGP30_IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	
uint8_t SGP30_Init(void);				  
uint8_t SGP30_Write(uint8_t *pSrc,uint16_t len);
uint8_t SGP30_Read(uint32_t *pDes);



#endif

