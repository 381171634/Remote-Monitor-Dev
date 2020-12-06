#include "sgp30_bsp.h"
#include "gpio.h"
#include "common.h"

static uint8_t sgp30_crc(uint8_t *p,uint8_t len)
{
	uint8_t poly = 0x31;
	volatile uint8_t crc = 0xff;

	uint8_t i,j;
	for(i = 0;i < len; i++)
	{
		crc = p[i] ^ crc;
		for(j = 0;j <8; j++)
		{
			if(crc & 0x80)
			{
				crc = (crc << 1) ^ poly;
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	return crc;
}

void SGP30_DELAY_US(uint16_t us)
{
	while (us--)
	{
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
	}
}

void SGP30_DELAY_MS(uint16_t ms)
{
	HAL_Delay(ms);
}

void SGP30_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, SGP30_SCL_GPIO_PIN | SGP30_SDA_GPIO_PIN, GPIO_PIN_SET);

	/*Configure GPIO pins : PAPin PAPin */
	GPIO_InitStruct.Pin = SGP30_SCL_GPIO_PIN | SGP30_SDA_GPIO_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SDA_OUT(void)
{
	SGP30_SDA_GPIO_PORT->CRH &= ~(0xf << 16);
	SGP30_SDA_GPIO_PORT->CRH |= (0x05 << 16);
}

void SDA_IN(void)
{
	SGP30_SDA_GPIO_PORT->CRH &= ~(0xf << 16);
	SGP30_SDA_GPIO_PORT->CRH |= (0x04 << 16);
}

//����IIC��ʼ�ź�
void SGP30_IIC_Start(void)
{
	SDA_OUT();
	SGP30_SDA_H;
	SGP30_SCL_H;
	SGP30_DELAY_US(20);

	SGP30_SDA_L; //START:when CLK is high,DATA change form high to low
	SGP30_DELAY_US(20);
	SGP30_SCL_L; //ǯסI2C���ߣ�׼�����ͻ��������
}

//����IICֹͣ�ź�
void SGP30_IIC_Stop(void)
{
	SDA_OUT();
	SGP30_SCL_L;
	SGP30_SDA_L; //STOP:when CLK is high DATA change form low to high
	SGP30_DELAY_US(20);
	SGP30_SCL_H;
	SGP30_SDA_H; //����I2C���߽����ź�
	SGP30_DELAY_US(20);
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t SGP30_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	SDA_IN();
	SGP30_SDA_H;
	SGP30_DELAY_US(10);
	SGP30_SCL_H;
	SGP30_DELAY_US(10);
	while (SGP30_SDA_READ)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			SGP30_IIC_Stop();
			return 1;
		}
	}
	SGP30_SCL_L; //ʱ�����0
	return 0;
}

//����ACKӦ��
void SGP30_IIC_Ack(void)
{
	SGP30_SCL_L;
	SDA_OUT();
	SGP30_SDA_L;
	SGP30_DELAY_US(20);
	SGP30_SCL_H;
	SGP30_DELAY_US(20);
	SGP30_SCL_L;
}

//������ACKӦ��
void SGP30_IIC_NAck(void)
{
	SGP30_SCL_L;
	SDA_OUT();
	SGP30_SDA_H;
	SGP30_DELAY_US(20);
	SGP30_SCL_H;
	SGP30_DELAY_US(20);
	SGP30_SCL_L;
}

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void SGP30_IIC_Send_Byte(uint8_t txd)
{
	uint8_t t;
	SDA_OUT();
	SGP30_SCL_L; //����ʱ�ӿ�ʼ���ݴ���
	for (t = 0; t < 8; t++)
	{
		if ((txd & 0x80) >> 7)
			SGP30_SDA_H;
		else
			SGP30_SDA_L;
		txd <<= 1;
		SGP30_DELAY_US(20);
		SGP30_SCL_H;
		SGP30_DELAY_US(20);
		SGP30_SCL_L;
		SGP30_DELAY_US(20);
	}
	SGP30_DELAY_US(20);
}

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
uint16_t SGP30_IIC_Read_Byte(uint8_t ack)
{
	uint8_t i;
	uint16_t receive = 0;
	SDA_IN();
	for (i = 0; i < 8; i++)
	{
		SGP30_SCL_L;
		SGP30_DELAY_US(20);
		SGP30_SCL_H;
		receive <<= 1;
		if (SGP30_SDA_READ)
			receive++;
		SGP30_DELAY_US(20);
	}
	if (!ack)
		SGP30_IIC_NAck(); //����nACK
	else
		SGP30_IIC_Ack(); //����ACK
	return receive;
}

//��ʼ��IIC�ӿ�
void SGP30_Init(void)
{
	SGP30_GPIO_Init();
	SGP30_DELAY_MS(5);
	SGP30_Write(0x20, 0x03);
	//	SGP30_ad_write(0x20,0x61);
	//	SGP30_ad_write(0x01,0x00);
}

void SGP30_Write(uint8_t a, uint8_t b)
{
	SGP30_IIC_Start();
	SGP30_IIC_Send_Byte(SGP30_write); //����������ַ+дָ��
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Send_Byte(a); //���Ϳ����ֽ�
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Send_Byte(b);
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Stop();
	SGP30_DELAY_MS(100);
}

void SGP30_Reset()
{
	SGP30_IIC_Start();
	SGP30_IIC_Send_Byte(0x00); 
	SGP30_IIC_Wait_Ack();
	SGP30_IIC_Send_Byte(0x06);
	SGP30_IIC_Wait_Ack();
}

uint8_t SGP30_Read(uint32_t *pDes)
{
	uint8_t recvBuf[6];
	uint8_t res = TRUE;
	uint8_t i;

	SGP30_IIC_Start();
	SGP30_IIC_Send_Byte(SGP30_read); 	//����������ַ+��ָ��
	SGP30_IIC_Wait_Ack();

	for(i = 0;i < 6; i++)
	{
		if(i < 5)
		{
			recvBuf[i] = SGP30_IIC_Read_Byte(1);
		}
		else
		{
			recvBuf[i] = SGP30_IIC_Read_Byte(0);
		}
	}

	SGP30_IIC_Stop();

	//check crc
	if(recvBuf[2] == sgp30_crc(recvBuf,2) && recvBuf[5] == sgp30_crc(recvBuf + 3,2))
	{
		*pDes = (recvBuf[0] << 24) | (recvBuf[1] << 16) | (recvBuf[3] << 8) | (recvBuf[4] << 0);
		res = TRUE;
	}
	else
	{
		res = FALSE;
	}
	
	return res;
}


