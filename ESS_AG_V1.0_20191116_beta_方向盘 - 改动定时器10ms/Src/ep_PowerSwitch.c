/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  Description*****
1.  Date:
    Author:
    Modification:
*************************************/
#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
#include "ep_PowerSwitch.h"
#include "ep_button.h"
#include "ep_lcd_6kw.h"
#include "softdevice_handler.h"
#include "ep_pstorage.h"
#include "main.h"
#include "ep_buzzer.h"
#include "nrf_delay.h"
#include "ep_led.h" 

//power button press time for sys on/off
#define   SWITCH_ON_TIME             100                           //30ms * SWITCH_ON_TIME  = 1.2s ,
#define   SWITCH_OFF_TIME            40                          //30ms * SWITCH_OFF_TIME = 1.2s 
#define   POWER_OFF_COUNT_MAX_MINU   60                           //自动关机计数s
#define   POWER_OFF_COUNT_MAX_SEC    (30*POWER_OFF_COUNT_MAX_MINU/10)//(30*POWER_OFF_COUNT_MAX_MINU/10)  //自动关机计数 30min

PowerSwitch_Handle_t  PowerSwitch_Handle;

void  PowerSwitch_OffCountClear(void)
{
	PowerSwitch_Handle.AutoPowerOffCount = 0;
}

void  PowerSwitch_OffCountAdd(void)
{
	if( (Comm_ReadState() == Comm_State_Disconnected))     //无按键操作且通信出错
	{
		 PowerSwitch_Handle.AutoPowerOffCount++;
	}
}

void ADC_GetBatVol(void)
{		
	uint8_t i = 0;

	PowerSwitch_Handle.BatSum = 0;
	for(i=0;i<5;i++)
	{
		NRF_ADC->TASKS_START = 1;
		while(NRF_ADC->EVENTS_END == 0);
		  NRF_ADC->EVENTS_END = 0;
		PowerSwitch_Handle.BatSum += NRF_ADC->RESULT;
	}
    
	PowerSwitch_Handle.BatVoltageValue = PowerSwitch_Handle.BatSum/5;
	if(PowerSwitch_Handle.BatVoltageValue < BAT_VOLTAGE_LOW)
	{
		PowerSwitch_Handle.HandleBAT_State = HandleBAT_UV;	
	}
	else if(PowerSwitch_Handle.BatVoltageValue >= BAT_VOLTAGE_NORMAL)
	{
		PowerSwitch_Handle.HandleBAT_State = HandleBAT_nomal;	
	}
	
}

#if 0
void ADC_IRQHandler(void)
{
	PowerSwitch_Handle.BatSum += NRF_ADC->RESULT;
	if(PowerSwitch_Handle.BatReadCnt > 5)
	NRF_ADC->EVENTS_END = 0;
	NRF_ADC->CONFIG &= ~ADC_CONFIG_PSEL_Msk;
	NRF_ADC->CONFIG |= (1 << BAT_VOLTAGE_READ_PIN) << ADC_CONFIG_PSEL_Pos;

	NRF_ADC->TASKS_START = 1;
}
#endif
void  ADC_init(void)
{
	NRF_ADC->POWER = 1;
	//config adc to 10bit, input 1/3 prescaling, reference supply 1/3 prescaling, pin select to vsin
	NRF_ADC->CONFIG = (ADC_RES_10bit << ADC_CONFIG_RES_Pos)
                      |(ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) 
	                  |(ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling << ADC_CONFIG_REFSEL_Pos) 
	                  |((BAT_VOLTAGE_READ_PIN) << ADC_CONFIG_PSEL_Pos);

	//enable adc
	NRF_ADC->ENABLE = 1;    
	NRF_ADC->INTENCLR = 1;	
//	//enable interrupt
//	NRF_ADC->INTENSET = ADC_INTENSET_END_Msk;
	//set priority to lowest priority
//	NVIC_SetPriority(ADC_IRQn, 3);
	//enable adc interrupt
///	NVIC_EnableIRQ(ADC_IRQn);
}

/*******************
* 函数名称：PowerOnInit
* 函数功能：开关机初始化
*********************/
void PowerSwitch_Init(PowerSwitch_Handle_t* handle)
{
	nrf_gpio_cfg_output(POWER_EN_PIN_NUM);
	nrf_gpio_cfg_input(BUTTON_Power_Button,NRF_GPIO_PIN_NOPULL);
	//nrf_gpio_cfg_input(VBAT_OK_PIN,NRF_GPIO_PIN_NOPULL);

	nrf_gpio_pin_clear(POWER_EN_PIN_NUM);
	ADC_init();
	
	handle->SysPowerState     = PowerTurnOn;
	handle->AutoPowerOffCount = 0;
	handle->BatUV_count       = 0;
	handle->HandleBAT_State   = HandleBAT_nomal;
	 
}
/*******************
* name        ：PowerSwitch_onCheck
* Description : use for sys power on check
*********************/
void PowerSwitch_onCheck(PowerSwitch_Handle_t* handle)
{
	static  uint16_t ONcount;
	while((handle->SysPowerState == PowerTurnOn))
	{
		if((handle->SysPowerCheckFlag)&&(!nrf_gpio_pin_read(BUTTON_Power_Button)))//开机长按检测
		{
			handle->SysPowerCheckFlag = 0;
			ONcount++;

			if(ONcount >= SWITCH_ON_TIME)//200*33MS = 5S
			{
				ONcount = 0;
				PowerSwitch_OpenSys();				
				handle->SysPowerState = PowerOn;
				Comm_DataTypeSet(SysOnType);		
				//----------------
				BUZZER_On();
				nrf_delay_ms(300);
				BUZZER_Off();
				//----------------	
				BUTTON_Handle.Menu_Button.state = BUTTON_State_Pressing;
				LED_SysON();
			}				 
		}		
	}
}
/*******************
* name        ：PowerSwitch_offCheck
* Description : use for sys power off check
*********************/
//uint16_t OFFcount = 0;
 void PowerSwitch_offCheck(PowerSwitch_Handle_t* handle)
{
	if(handle->AutoPowerOffCount >= POWER_OFF_COUNT_MAX_SEC)
	{		
		  handle->SysPowerState = PowerOff; //turn off handle   		
	}
	PowerSwitch_Off();
}
void PowerSwitch_BatUVcountClear(void)//stop warning until wire comm
{
    PowerSwitch_Handle.BatUV_count = 0;
}
HandleBAT_State_e PowerSwitch_ReadBatState(PowerSwitch_Handle_t* handle)
{
    return  handle->HandleBAT_State;
}
#if 1
static void PowerSwitch_Off(void)
{
	static uint8_t sys_delay_off;

	if(PowerSwitch_Handle.SysPowerState == PowerOff)
	{
		sys_delay_off++;
		if(sys_delay_off==2)
		{
			PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); //save motor position
			PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_percent_max_handle,PERCENT_MAX_BLOCK_ADD);
			THROTTLE_Backup_PercentMax(&THROTTLE_Handle,pstorage_Wdata);//掉电存储
		}
		else if(sys_delay_off>3)
		{
			PowerSwitch_CloseSys();
		}
	}	
}
#endif

void PowerSwitch_CloseSys(void)
{
	if(PSTORAGE_read_Wflag(&PSTORAGE_Handle) == false)
	{
		nrf_gpio_pin_clear(POWER_EN_PIN_NUM);//close sys
				NVIC_SystemReset();
	}
}

void PowerSwitch_OpenSys(void)
{
	nrf_gpio_pin_set(POWER_EN_PIN_NUM);//使能LDO
}
