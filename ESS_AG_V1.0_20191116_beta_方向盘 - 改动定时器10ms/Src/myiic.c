#include "myiic.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//IIC驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
void delay_us(uint32_t t)
{
		nrf_delay_us(t);
}
//初始化IIC
void IIC_Init(void)
{	
#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 输出高
	#endif
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;	  	  
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
	delay_us(4);
 	NRF_GPIO->OUTCLR = 1 << IIC_SDA_PIN;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;
	NRF_GPIO->OUTCLR = 1 << IIC_SDA_PIN;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN; 
	NRF_GPIO->OUTSET = 1 << IIC_SDA_PIN;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
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
	NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
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
//不产生ACK应答		    
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
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        nrf_gpio_pin_write(IIC_SDA_PIN,(txd&0x80)>>7);
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		NRF_GPIO->OUTSET = 1 << IIC_CLK_PIN;
		delay_us(2); 
		NRF_GPIO->OUTCLR = 1 << IIC_CLK_PIN;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
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
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}



























