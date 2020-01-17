#include "nrf_gpio.h"
#include "ep_tick.h"
#include "ep_protection.h"
#include "ep_motor.h"
#include "ep_buzzer.h"

#define error (-1)

extern Motor_Handle_t Motor_Handle;
static uint8_t ProtectionSwtich_State = 0;
uint8_t power_shut_flag = 0;
void ProtectionSwtich_Start(uint8_t DelayOn)
{
	nrf_gpio_pin_write(PROTECTIONSWITCH_PIN, 0);
    nrf_gpio_cfg_output(PROTECTIONSWITCH_PIN);

	if(DelayOn)
	{
        Systick_Delay(4000);
    }
    
    nrf_gpio_pin_write(PROTECTIONSWITCH_PIN, 1);
    Motor_SetStatus(&Motor_Handle, Motor_Status_On);   
}

void ProtectionSwtich_Init(void)
{
	nrf_gpio_pin_write(PROTECTIONSWITCH_PIN, 0);
    nrf_gpio_cfg_output(PROTECTIONSWITCH_PIN);
	nrf_gpio_cfg_output(12); 
}

uint16_t GetChange(uint16_t *num1,uint16_t *num2)
{
	uint16_t Value = 0;
	
	if(((*num1) < 0) | ((*num2) < 0))
		return error;
	
	Value = (((*num1)>(*num2))?((*num1)-(*num2)):((*num2)-(*num1)));
	return(Value);
}

void ProtectionSwtich_TriggerOn(void)
{
	static bool FirstNormalFlag = true;    
    static uint16_t PrevUbus = 0;
    static uint32_t PrevUbusTick = 0;
    
    uint32_t TimeIntervel = Systick_GetTick() - PrevUbusTick;
    uint32_t UbusRateOfChange = 0;
	static uint16_t ProtectCnt = 0;
	
	  UbusRateOfChange = GetChange(&PrevUbus,&Motor_Handle.MotorObj.ubus);// * 1000 / TimeIntervel;
    if(UbusRateOfChange < 0)
			return;
		
    PrevUbusTick = Systick_GetTick();

	PrevUbus = Motor_Handle.MotorObj.ubus;
	
	if(power_shut_flag == 1)
	{
		nrf_gpio_pin_toggle(12);
	}
	if(ProtectionSwtich_State)
	{
		#if 1
		//掉电速率高，且电压在反充范围内则认为是电源切断，需要关闭PROTECTIONSWITCH_PIN
		if((Motor_Handle.MotorObj.ubus <= MOT_CCV_TRIGGER_UP))
		{
			if(ProtectCnt++ > 3)
			{
				ProtectCnt = 0;
				nrf_gpio_pin_write(PROTECTIONSWITCH_PIN, 0);
				Motor_SetStatus(&Motor_Handle, Motor_Status_Off);
				ProtectionSwtich_State = 0; 
				power_shut_flag = 1;
			}
		}
		else
			ProtectCnt = 0;
		
		return;
		#endif
		
	}
	
    if((UbusRateOfChange <= 2) && (Motor_Handle.MotorObj.ubus >= MOT_UV_TRIGGER))//电压稳定，且正常则打开输出？
    {
        nrf_gpio_pin_write(PROTECTIONSWITCH_PIN, 1);
        Motor_SetStatus(&Motor_Handle, Motor_Status_On);
        ProtectionSwtich_State = 1;	
		if(FirstNormalFlag == true)
		{
			Buzzer_OnEn(&Buzzer_Handle);
			FirstNormalFlag = false;
		}
    }
	     	
}

uint8_t ProtectionSwtich_GetState(void)
{
   return  ProtectionSwtich_State;
}

void ProtectionSwtich_SetState(uint8_t ucState)
{
    ProtectionSwtich_State = ucState;
}

