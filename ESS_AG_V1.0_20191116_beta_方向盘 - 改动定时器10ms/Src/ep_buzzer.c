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

*  
*************************************/
#include "ep_buzzer.h"
#include "nrf_gpio.h"
#include "ep_comm.h"
#include "ep_sys.h"

BUZZER_Handle_t BuzzerHandle;

void BUZZER_Init(void)
{
	nrf_gpio_cfg_output(BUZZER_PIN); 
}

void BUZZER_On(void)
{
	 NRF_GPIO->OUTSET = 1 << BUZZER_PIN;
}

void BUZZER_Off(void)
{
	NRF_GPIO->OUTCLR = 1 << BUZZER_PIN;
}

void BUZZER_Troggle(void)
{
	nrf_gpio_pin_toggle(BUZZER_PIN);
}

void BUZZER_beep_once(void)
{
	BuzzerHandle.g_buzzer_once_flag = true;
	BuzzerHandle.buzzer_delay_cnt2 = 0;
}

void BUZZER_beep_twice(void)
{
	BuzzerHandle.g_buzzer_twice_flag = true;
	BuzzerHandle.g_buzzer_beep_times = 2;
	BuzzerHandle.g_buzzer_beep_cnt = 0;
	BuzzerHandle.buzzer_delay_cnt3 = 0;
}

#if 1
void BUZZER_Err(void)
{	
	if(BuzzerHandle.g_Buzzer_on_flag == true)
	{
		BuzzerHandle.buzzer_delay_cnt1++;
		if(BuzzerHandle.buzzer_delay_cnt1>100)
		{
			BuzzerHandle.buzzer_delay_cnt1 = 0;
			BUZZER_On();
			BuzzerHandle.g_Buzzer_on_flag = false;

		}
		if(BuzzerHandle.buzzer_delay_cnt1==1)
		{
			BUZZER_Off();
		}
	}
	else if(BuzzerHandle.g_buzzer_once_flag == true)
	{
		BuzzerHandle.buzzer_delay_cnt2++;
		if(BuzzerHandle.buzzer_delay_cnt2>10)
		{
			BuzzerHandle.buzzer_delay_cnt2 = 0;
			BUZZER_Off();
			BuzzerHandle.g_buzzer_once_flag = false;
		}
		if(BuzzerHandle.buzzer_delay_cnt2==1)
		{
			BUZZER_On();
		}
	}
	else if(BuzzerHandle.g_buzzer_twice_flag == true)
	{
		BuzzerHandle.buzzer_delay_cnt3++;
		if(BuzzerHandle.buzzer_delay_cnt3>10)
		{
			BuzzerHandle.buzzer_delay_cnt3 = 0;
			BUZZER_Off();
			if(++BuzzerHandle.g_buzzer_beep_cnt>=BuzzerHandle.g_buzzer_beep_times)
			{
				BuzzerHandle.g_buzzer_twice_flag = false;
				BuzzerHandle.g_buzzer_beep_cnt = 0;
			}
		}
		if(BuzzerHandle.buzzer_delay_cnt3==1)
		{
			BUZZER_On();
		}
	}
	else
	{
		BuzzerHandle.g_buzzer_once_flag = false;
		BuzzerHandle.g_buzzer_twice_flag = false;
		BUZZER_Off();
	}
}
#endif
//¹ÊÕÏÌáÊ¾µÈ
#if 0
void BUZZER_Run(uint8_t soc)
{
	static uint8_t  prev_soc = 0;
	static uint16_t ticks = 0;
	static uint8_t  state = 0;  //0 normal, 1 start 50, 2 50, 3 start 20, 4 20, 5 start 5, 6 5
	if(prev_soc >  soc)
	{
	    if((prev_soc - soc) > 5)
		{
		   prev_soc = 0;
		   BUZZER_Off();
		   state = 0;
		   return;
		}
	}
	
	if(SYS_ReadState(&SYS_Handle) != SYS_State_Normal)
	{
	   prev_soc = 0;
	   BUZZER_Off();
	   state = 0;
	   return;
	}
	//soc < 50
	if(soc > 50)
	{
	    prev_soc = soc;
		BUZZER_Off();
		return;
	}
	if(prev_soc > 50 && soc <= 50)
	{
		ticks = 0;
		state = 1;
	}
	//soc < 20
	else if(prev_soc > 20 && soc <= 20)
	{
		ticks = 0;
		state = 3;
	}
	//soc < 10
	else if(prev_soc > 10 && soc <= 10)
	{
		ticks = 0;
		state = 5;
	}
	prev_soc = soc;
	ticks ++;
	switch(state)
	{
		case 0:
			break;
		case 1:
			ticks = 0;
			state = 2;
			break;
		case 2:
			if(ticks % 20 <10)
				BUZZER_On();
			else
				BUZZER_Off();
			if(ticks >= 150)
			{
				BUZZER_Off();
				state = 0;
			}
			break;
		case 3:
			ticks = 0;
			state = 4;
			break;
		case 4:
			if(ticks % 20 <10)
				BUZZER_On();
			else
				BUZZER_Off();
			if(ticks >= 300)
			{
				BUZZER_Off();
				state = 0;
			}
			break;
		case 5:
			ticks = 0;
		    state = 6;
		    break;
		case 6:
			if(ticks % 20 <10)
				BUZZER_On();
			else
				BUZZER_Off();
			if(ticks >= 300)
			{
				BUZZER_Off();
				state = 0;
			}
			break;
		default:break;
	}
			
}
#endif
