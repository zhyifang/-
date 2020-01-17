#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include <stdint.h>
#include "ep_tick.h"
#include "ep_throttle.h"
#include "ep_lcd_6kw.h"

volatile uint32_t SysSecond=0;
volatile uint32_t SysMicroSecond=0;
uint32_t Systick_GetTick(void);
uint32_t CountDown5SecondCnt = 0;
uint32_t ConutDown3SecondCnt = 0;
extern uint16_t RawAngle;
extern uint16_t res;
extern uint16_t Angle;

extern uint16_t AS5600_GetRawAngle(void);

void SysTimer1Start(void)
{
	  NRF_TIMER1->TASKS_START = 1;
}

void SysTimer1Stop(void)
{
	  NRF_TIMER1->TASKS_STOP = 1;
}

//界面超时处理
void EventTimeOutCheckProcess(void)
{
	if(LCD_Handle.CountDownFlag == 1)
	{
		if(ConutDown3SecondCnt <= 300)    //180ms*300=54s
		{
			  ConutDown3SecondCnt++;
		}
		else
		{
		    LCD_SetMenu(LCD_Menu_Main);
			LED_AllOff();
			LCD_Handle.CountDownFlag = 0;
			THROTTLE_SetState(&THROTTLE_Handle,THROTTLE_State_Reseted);
		}
	}
}


//系统滴答时钟初始化
void Systick_Init(void)
{
	NRF_TIMER1->TASKS_STOP = 1;	
	NRF_TIMER1->MODE       = TIMER_MODE_MODE_Timer;  
	NRF_TIMER1->BITMODE    = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);
	NRF_TIMER1->PRESCALER  = 4;	
	NRF_TIMER1->TASKS_CLEAR = 1; 
	NRF_TIMER1->CC[0]      = 2000 ;      //2ms
	NRF_TIMER1->INTENSET   = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos; 
	NRF_TIMER1->SHORTS     = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
	
	NVIC_SetPriority(TIMER1_IRQn,3);
	NVIC_EnableIRQ(TIMER1_IRQn);
	//NRF_TIMER1->TASKS_START = 1;
}


void TIMER1_IRQHandler(void)   
{   
	if (NRF_TIMER1->EVENTS_COMPARE[0] != 0)
	{		
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;
		#if 1
		if((SysMicroSecond++ % 500) == 0)
		{
        	SysSecond++;  
    	}   
		#endif
	}
}
#if 0
uint32_t Systick_GetSecond(void)
{
	return SysSecond;  
}

uint32_t Systick_GetTick(void)
{
	return SysMicroSecond;  
}
#endif

