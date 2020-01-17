#ifndef __MYIIC_H
#define __MYIIC_H

#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
//#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//IIC���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
#define u8 unsigned char
//IO��������
 #define IIC_CLK_PIN 29
 #define IIC_SDA_PIN 30
 
#define SDA_IN()  {nrf_gpio_cfg_input(IIC_SDA_PIN,NRF_GPIO_PIN_PULLUP); }         //PA7 ����/����������
#define SDA_OUT() {nrf_gpio_cfg_output(IIC_SDA_PIN); }						      //PA7  ͨ��������� 50MHZ

//IO��������	 
#define IIC_SCL    {nrf_gpio_cfg_output(IIC_CLK_PIN)} //SCL
#define IIC_SDA    {nrf_gpio_cfg_output(IIC_SDA_PIN)} //SDA	 
#define READ_SDA   {nrf_gpio_cfg_input(IIC_SDA_PIN,NRF_GPIO_PIN_PULLUP);nrf_gpio_pin_read(IIC_SDA_PIN);}  //����SDA 

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















