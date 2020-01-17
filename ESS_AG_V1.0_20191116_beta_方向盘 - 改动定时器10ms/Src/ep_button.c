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
#include "nrf.h"
#include "nrf_gpio.h"
#include "app_button.h"
#include "app_timer.h"
#include "ep_button.h"
#include "ep_ble_init.h"
#include "ep_lcd_6kw.h"
#include "ep_powerswitch.h"
#include "ep_buzzer.h"
#include "ep_led.h" 
#include "ep_tick.h"

#define BUTTON_DETECTION_DELAY   APP_TIMER_TICKS(20, APP_TIMER_PRESCALER)  

BUTTON_Handle_t  BUTTON_Handle;

void  button_init(void);
void  BUTTON_Press(BUTTON_Handle_t* handle,uint8_t pin_no);
void  BUTTON_Release(BUTTON_Handle_t* handle,uint8_t pin_no);	
void  BUTTON_get_touch_type(button_t* button);
void  BUTTON_pressCountClear(button_t* button);

static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action)
{
	if(button_action == APP_BUTTON_PUSH)
	{
		BUTTON_Press(&BUTTON_Handle,pin_no);  
	}
	if(button_action == APP_BUTTON_RELEASE)
	{
		BUTTON_Release(&BUTTON_Handle,pin_no);
	}
}

void button_init(void)
{
	static const app_button_cfg_t app_buttons[3] =
	{
		{BUTTON_Menu_Button, false, NRF_GPIO_PIN_PULLUP, bsp_button_event_handler} 
	};
	app_button_init((app_button_cfg_t *)app_buttons,1,BUTTON_DETECTION_DELAY);
	app_button_enable();
	
	BUTTON_Handle.Menu_Button.action   = BUTTON_Action_release;
}

void BUTTON_Press(BUTTON_Handle_t* handle,uint8_t pin_no)
{
	if(pin_no == BUTTON_Menu_Button)
	{
		handle->Menu_Button.action = BUTTON_Action_push;

	}
			ConutDown3SecondCnt = 0;
	PowerSwitch_OffCountClear();//clear auto power off count   			
}
void BUTTON_Release(BUTTON_Handle_t* handle,uint8_t pin_no)
{
	if(pin_no == BUTTON_Menu_Button)
	{
		handle->Menu_Button.action = BUTTON_Action_release;
		handle->Menu_Button.state = BUTTON_State_Releasing;
	}	
}
void BUTTON_pressCountClear(button_t* button)
{
	if(button->action == BUTTON_Action_release)
	{
		button->pressCount = 0;
		button->lock = 0;
	}  
}
void BUTTON_UpdateKeyCounter(BUTTON_Handle_t* handle)
{
	if((handle->Menu_Button.action == BUTTON_Action_push) && (handle->Menu_Button.state == BUTTON_State_Releasing))
	{
		handle->Menu_Button.pressCount++;                              //按键加 30ms按键处理函数
	}

	if((handle->Menu_Button.pressCount) && (handle->Menu_Button.state == BUTTON_State_Releasing))
	{
		BUTTON_get_touch_type(&handle->Menu_Button);	
	}
}



void BUTTON_get_touch_type(button_t* button)
{
	if((button->pressCount > 2) &&(button->pressCount <= 29))          //29*30ms=0.8s
	{
		if(button->action == BUTTON_Action_release)
		{
			button->touch_type = Touch_Short;                          //短按标志
		}
	}
	else if((button->pressCount > 30) && (button->pressCount <= 60))  //60*30ms=1.8s
	{
		if(button->action == BUTTON_Action_release)
		{
			button->touch_type = Touch_Mid;                            
		}
	}
	else if((button->pressCount > 60)&&(button->pressCount < 70)&&(button->lock == 0) )//时间从1.8s到2.1s
	{		
		button->lock = 1;
		button->touch_type = Touch_Long;                              //长按标志
	}
    
    LCD_Next_Menu_Config(&LCD_Handle);                                //下一个界面
	BUTTON_pressCountClear(button);                                   //release 清除计数
	button->touch_type = Touch_no;
}

