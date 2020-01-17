#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include <stdint.h>
#include "ep_tick.h"

volatile uint32_t SysSecond=0;
volatile uint32_t SysMicroSecond=0;
void Systick_Init(void)
{
	NRF_TIMER1->TASKS_STOP = 1;	
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;  
	NRF_TIMER1->BITMODE = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);
	NRF_TIMER1->PRESCALER = 4;	
	NRF_TIMER1->TASKS_CLEAR = 1; 
	NRF_TIMER1->CC[0] = 1000 ; //1ms
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos; 
	NRF_TIMER1->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
	
	NVIC_SetPriority(TIMER1_IRQn,1);
	NVIC_EnableIRQ(TIMER1_IRQn);
	NRF_TIMER1->TASKS_START = 1;
}

void TIMER1_IRQHandler(void)   
{   
	if (NRF_TIMER1->EVENTS_COMPARE[0] != 0)
	{		
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;
		if((SysMicroSecond++ % 1000) == 0)
		{
            SysSecond++;     
        }            
	}
}

uint32_t Systick_GetSecond(void)
{
	return SysSecond;  
	
}

uint32_t Systick_GetTick(void)
{
	return SysMicroSecond;  
}

void Systick_Delay(__IO uint32_t Delay)
{
	uint32_t StartTime = Systick_GetTick();
	while((Systick_GetTick() - StartTime) < Delay);
}
