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
#include "ep_wdt.h"
#include "nrf.h"
#include "ep_lcd_6kw.h"

#define  WDT_TimeOutS     6
#define  WDT_TimeOutMs    WDT_TimeOutS*1000UL 

void WDT_Init(void);
void WDT_Feed(void);
bool WDT_ReadWdtRunStatues(void);

void WDT_Init(void)
{
	NRF_WDT->CRV=32768*WDT_TimeOutMs/1000-1;
	NRF_WDT->CONFIG=0;
	NRF_WDT->CONFIG|=0x01;//Keep the watchdog running while the CPU is sleeping.
	NRF_WDT->CONFIG|=0x08;//Keep the watchdog running while the CPU is halted by the debugger.
	NRF_WDT->CONFIG&=0xf7;//Pause watchdog while the CPU is halted by the debugger.
	//NRF_WDT->INTENSET= 1;//WDT_INTENSET_TIMEOUT_Set;//开启看门狗中断
	NRF_WDT->TASKS_START=true;//开启看门狗
	while(false == WDT_ReadWdtRunStatues());//等待看门狗开启
	WDT_Feed();
}
void WDT_Feed(void)
{
	NRF_WDT->RREN=1;
	NRF_WDT->RR[0]=0x6E524635;
	NRF_WDT->RREN=0;
}
bool WDT_ReadWdtRunStatues(void)
{

    if(NRF_WDT->RUNSTATUS)
        return true;
		else
        return false;
}




