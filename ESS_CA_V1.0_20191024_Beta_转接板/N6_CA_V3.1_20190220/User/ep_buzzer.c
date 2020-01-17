#include  "ep_buzzer.h"
#include  "nrf.h"
#include  "nrf51.h"
#include "nrf_gpio.h"



#define  BuzzerOnTimeMs      400  //400ms
#define  BuzzerOnCountMax    (BuzzerOnTimeMs/30) 

Buzzer_Handle_t Buzzer_Handle; 
 
void  Buzzer_On(void)
{
	BUZZER_1;
}
void  Buzzer_Off(void)
{
	BUZZER_0;
}
void  Buzzer_Toggle(void)
{
	BUZZER_Toggle;
}
void Buzzer_Init(Buzzer_Handle_t* handle)
{
	handle->BuzzerOnFlag   = true;
	handle->BuzzerOffCount = 0;
	
	nrf_gpio_cfg_output(BUZZER_PIN);
	Buzzer_Off();
}
void Buzzer_OffCheck(Buzzer_Handle_t* handle)
{
	if(handle->BuzzerOnFlag)
	{
		handle->BuzzerOffCount++;
		if(handle->BuzzerOffCount >= BuzzerOnCountMax)
		{
			handle->BuzzerOffCount = 0;
			handle->BuzzerOnFlag   = false;
			Buzzer_Off();
		}
	}
}
void Buzzer_OnEn(Buzzer_Handle_t* handle)
{
	handle->BuzzerOnFlag = true;
	handle->BuzzerOffCount = 0;
	Buzzer_On();
}

