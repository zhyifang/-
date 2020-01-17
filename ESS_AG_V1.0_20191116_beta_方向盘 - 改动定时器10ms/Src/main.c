/*********************************************************
  * Copyright (C), 2017,Dongguan ePropulsion Intelligence Technology Limited.
    All Rights Reserved
  * MCU                : NRF51822
  * MDK                : KEIL5
  * SofeDevice         : s110_nrf51_8.0.0_softdevice
  * Description        : 
  * Note               :
***********************************************************/
#include <stdint.h>
#include "stdio.h"
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "nrf51.h"
#include "app_error.h"
#include "nrf_delay.h"
/*******user header file************/ 
#include "ep_timeslot.h"
#include "ep_throttle.h"
#include "simple_uart.h"
#include "ep_throttle.h"
#include "ep_lcd_6kw.h"
#include "ep_pstorage.h"
#include "pstorage.h"
#include "ep_comm.h"
#include "ep_ble_init.h"
#include "ep_PowerSwitch.h"
#include "ep_hall.h"
#include "ep_button.h"
#include "ep_battery.h"
#include "ep_motor.h"
#include "ep_gzll.h"
#include "ep_sys.h"
#include "ep_wdt.h"
#include "ep_gps.h"
#include "ep_crypt_device.h"
#include "nrf_soc.h"
#include "nrf_gzp.h"
#include "ep_buzzer.h"
#include "ep_ble_access.h"
#include "ep_ble_battery.h"
#include "ep_ble_motor.h"
#include "main.h"
#include "ep_TimeRecord.h"
#include "ep_version.h"
#include "ep_tick.h"
#include "ep_led.h"
#include "as5600.h"

uint32_t  gzll_addr1,gzll_addr0;
void delay_ms(uint16_t time);
//static void  GPIO_Init(void);
void POWER_Disable(void);

uint8_t MCU_ID[1];
uint16_t RawAngle = 0;
uint16_t res = 0;
uint16_t Angle = 0,gAngle = 0;
uint8_t gAGC = 0;
uint16_t Config = 0;
uint16_t Magitue = 0;

//���ԽǶ�
uint16_t Tast_angle = 0;
int main(void)
{
	//Product_Init(&drv_msg,0,0,product_null,year_2017,month_10);
	//Product_Init(&commrelay_msg,0,0,product_null,year_2017,month_11);
	//Product_Init(&handle_msg,SOFTWARE_VER,HARDWARE_VER,product_null,SOFTWARE_YEAR,SOFTWARE_MONTH);
	MCU_ID[0] = NRF_FICR->DEVICEID[0] & 0x7F;	
    POWER_Disable();
	LCD_Init(&LCD_Handle);                    //LCD��ʼ��
	MOTOR_Init(&MOTOR_Handle);                //���������ʼ��
	SYS_Init(&SYS_Handle);                    //ϵͳ��ʼ��
	Gzll_ParamInit(&GZLL_Handle);             //gzll ��ʼ������
	THROTTLE_Init(&THROTTLE_Handle);          //THROTTLE ģ���ʼ��
	PowerSwitch_Init(&PowerSwitch_Handle);    //���ػ���ʼ��
	
	CRYPT_Init();                             //Gzll����ͨ�ż��ܳ�ʼ��
	BLE_Init();                               //������ʼ,����ʱ���ж�
	BUZZER_Init();                            //��������ʼ
	LED_init();                               //LED�ĳ�ʼ��
	PowerSwitch_onCheck(&PowerSwitch_Handle); //�����������  
   
	PSTORAGE_Init();                          //PSTORAGE�洢��ʼ
    PSTORAGE_DataUdate();	                  //��ȡ������PSTORAGE�е�����

	Comm_Init(&Comm_Handle);                  //ͨ�ų�ʼ��
	//GPIO_Init(); 
	AS5600_Init();                            //�Ƕȴ�������ʼ��
	Systick_Init();
	
	AS5600_SetOutputStage(OUT_STG_PWM);       //analog out
	AS5600_SetPMMode(AS5600_PM_MODE_LPM2);    //power lpm2
	AS5600_EnableWd();                        //ʹ��оƬ���Ź�д��
	
    nrf_gpio_cfg_output(15);
	WDT_Init();                               //watch dog  init 
	for (;;)
	{ 
        
        if(BLE_Handler.app_time_out_flag_ms == 1)           //10ms������������ͨ�ź���
        {
        
            BLE_Handler.app_time_out_flag_ms =0;
            //------as5600 test-----
            #if 1
			THROTTLE_Handle.angle= AS5600_GetAngle();               //�Ƕ�--������
			if(THROTTLE_Handle.calibrationFlag == false)
			{
				if(THROTTLE_Handle.angle > 360)
					return 0;
				THROTTLE_Calc(&THROTTLE_Handle);                    //���� --������
			}
            
            Tast_angle=THROTTLE_Handle.angle;
            
            //
            if(Tast_angle!=THROTTLE_Handle.angle)
            {
               nrf_gpio_pin_toggle(15);
            }
            //  
            
            #else                                                      //���Ա����鿴
			Angle &=0x00;
			Angle |=AS5600_ReadOneByte(ANGLE_H);
			Angle<<=8;
			Angle |=AS5600_ReadOneByte(ANGLE_L);
			gAngle = (float)Angle/4096*360;
            
			res = AS5600_GetStatus();
			RawAngle = AS5600_GetRawAngle();
			Config = AS5600_ReadConf();
			gAGC = AS5600_GetAGC();
			Magitue = AS5600_ReadMagitue();
            #endif
		    //----------------------
            if(PSTORAGE_read_Wflag(&PSTORAGE_Handle) == false)    //if  no need storage data 
		   {   
			  if(Comm_Handle.Mode == Comm_Mode_Gzll)             //���߷���
			  {
					CRYPT_Decode(GZLL_Handle.rx_data.data); 
					Comm_UpdateSendData();                         //5us update sent data	���߷���			
					Comm_PrepareTxData(&Comm_Handle);              //6us updata send array 					
					CRYPT_CmdRequest();	
					request_next();	 						
			  }
			  else                                              //���߷���
			  {
					Comm_UpdateSendData();                        
				    Comm_PrepareTxData(&Comm_Handle);              
					Comm_RS485ModeSend(&Comm_Handle);	
			  }				
			/********************/
			 PowerSwitch_offCheck(&PowerSwitch_Handle);	      			
		   }				
		   else 
		   {  
				PSTORAGE_SaveData(&PSTORAGE_Handle);				
		   }
			
      }
        
        
		if(BLE_Handler.app_time_out_flag == 1)              // 30ms access ,need about 400us
		{
		
			BUTTON_UpdateKeyCounter(&BUTTON_Handle);        //30ms����������
			
			BUZZER_Err();
			BLE_Handler.app_time_out_flag = 0;             //need place here ,if not ,when  pstorage data will occrus sth err
		}

   		if (BLE_Handler.sysLcdRefresh == 1)                //30MS*6 = 180MS  LCD display
		{ 
			BLE_Handler.sysLcdRefresh = 0;
			ADC_GetBatVol();
			Comm_RS485_Check(&Comm_Handle);	                       //check  comm type gazell or rs485	
			SYS_ErrCheck(&SYS_Handle);                             //
			
			if(PowerSwitch_Handle.SysPowerState != PowerOff)
			LED_Display();
			
			EventTimeOutCheckProcess();
			WDT_Feed();                                            //feed watch dog	 x	
		}

		__WFI();
   }
}


void delay_ms(uint16_t time) 
{
	uint16_t i, j;
	
	for( i = 0;i < time; i++)
	{
		for( j = 0;j < 2600; j++);
	}
}
void HardFault_Handler(void)
{ 	
	delay_ms(60000);
	while(1)
	{  
		nrf_gpio_pin_clear(POWER_EN_PIN_NUM);//close sys
		NVIC_SystemReset();				
	}
}

//Disable some module power for 
void POWER_Disable(void)
{
	NRF_SPI0->POWER = 0;
	NRF_SPI1->POWER = 0;
	NRF_TEMP->POWER = 0;
	NRF_AAR->POWER  = 0;
	NRF_CCM->POWER  = 0;
	NRF_QDEC->POWER = 0;
	NRF_LPCOMP->POWER = 0;    	
}


