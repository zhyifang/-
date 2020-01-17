#include "as5600.h" 
#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
#include "nrf_delay.h"

extern void delay_ms(uint16_t time);

void AS5600_Init(void)
{
	nrf_gpio_cfg_output(IIC_CLK_PIN);
	nrf_gpio_cfg_output(IIC_SDA_PIN);
	nrf_gpio_cfg_output(AS5600_PRG_PIN);
//	nrf_gpio_pin_clear(AS5600_PRG_PIN);
	nrf_gpio_pin_set(AS5600_PRG_PIN);
}

u8 AS5600_ReadOneByte(u8 ReadAddr)
{				  
	u8 temp=0;		
	
    IIC_Start();  
	IIC_Send_Byte(0X6C);     				//发送AS5600从设备地址写
	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr);				//发送寄存器地址
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0X6D);                   //发送AS5600从设备地址读			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();                            //产生一个停止条件	    
	return temp;
}

void AS5600_WriteOneByte(u8 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();   
    IIC_Send_Byte(0X6C);            //发送AS5600设备地址写 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr);       //发送寄存器地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();						//产生一个停止条件 
	delay_ms(10);	 				//等待AS5600擦写数据
}

u8 AS5600_Read()
{
	u8 temp0=0;
	IIC_Start();
	IIC_Send_Byte(0X6D);
	IIC_Wait_Ack();
	temp0=IIC_Read_Byte(0);
	IIC_NAck();
	IIC_Stop();
	return temp0;
}

uint8_t AS5600_GetStatus(void)
{
	return AS5600_ReadOneByte(STATUS) & 0x38;	
}

uint16_t AS5600_GetAngle()
{
	return (int)((float)(AS5600_ReadOneByte(ANGLE_L) + (AS5600_ReadOneByte(ANGLE_H) << 8))/4096*360);	
}


uint16_t AS5600_GetRawAngle()
{
	uint16_t AngleVal=AS5600_ReadOneByte(RAWANG_L) + (AS5600_ReadOneByte(RAWANG_H) << 8);
	return AngleVal;	
}

void AS5600_SetHystheresis(uint8_t Hyst)
{
	uint8_t TmpConfHigh=AS5600_ReadOneByte(CONF_H);
	TmpConfHigh&=0xf3;
	TmpConfHigh |= (HYST_MASK & Hyst);	
	AS5600_WriteOneByte(CONF_H,TmpConfHigh);
}



void AS5600_SetOutputStage(uint8_t OutStage)
{
	uint8_t TmpConfHigh=AS5600_ReadOneByte(CONF_L);
	TmpConfHigh&=0xcf;
	TmpConfHigh |= (OUT_STG_MASK & OutStage);
	AS5600_WriteOneByte(CONF_L,TmpConfHigh);
}

void AS5600_SetPWMFreq(uint8_t Freq)
{
	uint8_t TmpConfHigh=AS5600_ReadOneByte(CONF_H);
	TmpConfHigh&=0x3f;
	TmpConfHigh |= (PWMF_MASK & Freq);
	AS5600_WriteOneByte(CONF_H,TmpConfHigh);
}

void AS5600_SetPMMode(uint8_t mode)
{
	uint8_t TmpConfL=AS5600_ReadOneByte(CONF_L);
	TmpConfL&=0xfc;
	TmpConfL |= (PM_MASK & mode);
	AS5600_WriteOneByte(CONF_L,TmpConfL);
}

void AS5600_EnableWd(void)
{
	uint8_t TmpConfH=AS5600_ReadOneByte(CONF_H);
	TmpConfH &= 0xDF;
	TmpConfH |= 0X20;
	AS5600_WriteOneByte(CONF_H,TmpConfH);
}

void AS5600_DisableWd(void)
{
	uint8_t TmpConfH=AS5600_ReadOneByte(CONF_H);
	TmpConfH&=0xDF;
	TmpConfH |= 0X00;
	AS5600_WriteOneByte(CONF_H,TmpConfH);
}


uint8_t AS5600_GetAGC(void)
{
	return AS5600_ReadOneByte(AGC);	
}

uint16_t AS5600_ReadConf(void)
{
	uint16_t conf_tmp = 0;
	
	conf_tmp=AS5600_ReadOneByte(CONF_H);
	conf_tmp <<=8;
	conf_tmp=AS5600_ReadOneByte(CONF_L);

	return conf_tmp;
}
uint16_t AS5600_ReadMagitue(void)
{
	uint16_t conf_tmp = 0;
	
	conf_tmp=AS5600_ReadOneByte(MAGN_H);
	conf_tmp <<=8;
	conf_tmp=AS5600_ReadOneByte(MAGN_L);

	return conf_tmp;
}

