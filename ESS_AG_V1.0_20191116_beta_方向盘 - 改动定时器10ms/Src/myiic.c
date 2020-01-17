#include "myiic.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK��ӢSTM32������
//IIC���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
void delay_us(uint32_t t)
{
		nrf_delay_us(t);
}
//��ʼ��IIC
void IIC_Init(void)
{	
#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//ʹ��GPIOBʱ��
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 �����
	#endif
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;	  	  
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
	delay_us(4);
 	NRF_GPIO->OUTCLR = 1 << IIC_SDA_PIN;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
	NRF_GPIO->OUTCLR = 1 << IIC_SDA_PIN;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN; 
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;//����I2C���߽����ź�
	delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;delay_us(1);	   
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;delay_us(1);	 
	while(nrf_gpio_pin_read(IIC_SDA_PIN))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
	SDA_OUT();
	NRF_GPIO->OUTCLR = 1 << IIC_SDA_PIN;
	delay_us(2);
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
	delay_us(2);
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
	SDA_OUT();
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;
	delay_us(2);
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
	delay_us(2);
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        nrf_gpio_pin_write(IIC_SDA_PIN,(txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
		delay_us(2); 
		NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;	
		delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN; 
        delay_us(2);
		NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
        receive<<=1;
        if(nrf_gpio_pin_read(IIC_SDA_PIN))receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}



























