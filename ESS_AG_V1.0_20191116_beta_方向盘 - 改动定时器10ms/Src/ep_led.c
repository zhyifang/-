#include "nrf_gpio.h"
#include "nrf51.h"
#include "nrf.h"
#include "ep_PowerSwitch.h"
#include "ep_button.h"
#include "ep_lcd_6kw.h"
#include "softdevice_handler.h"
#include "ep_pstorage.h"
#include "main.h"
#include "ep_led.h"
#include "nrf_delay.h"
#include "ep_buzzer.h"

void LED_init(void);
void LED_trigger(uint8_t ColorPin);
void LED_blink(uint8_t ColorPin);
void LED_AllOn(void);
void LED_DisplayNum(uint8_t num);

LED_Handle_t LED_Handle;


void LED_init(void)
{
	nrf_gpio_cfg_output(LED1);
	nrf_gpio_cfg_output(LED2);
	nrf_gpio_cfg_output(LED3);
	nrf_gpio_cfg_output(LED4);
	nrf_gpio_cfg_output(LED5);
	nrf_gpio_cfg_output(LED6);
	nrf_gpio_cfg_output(LED7);
	nrf_gpio_cfg_output(LED8);
	nrf_gpio_cfg_output(LED9);

	LED_Handle.led_flick_cnt = 0;
}

void LED_ON(uint8_t led_pin)
{
    NRF_GPIO->OUTSET = 1 << led_pin;
}

void LED_OFF(uint8_t led_pin)
{
	NRF_GPIO->OUTCLR = 1 << led_pin;
}

void LED_SysON(void)
{
	uint8_t i = 0;
	
	LED_AllOff();
    for(i=1;i<10;i++)
	{
		LED_DisplayNum(i);
		delay_ms(200);
	}	
}

void LED_SysOff(void)
{
	uint8_t i = 0;
	
	LED_AllOn();
	for(i=1;i<10;i++)
	{
		LED_DisplayNum(10-i);   //点亮10-i个灯
		delay_ms(200);
	}
}
//点亮某个灯LED
void LED_DisplayPosition(uint8_t led_num)
{
	uint8_t led_tmp[] = {0,LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9};
	
	LED_OFF(LED1);
	LED_OFF(LED2);
	LED_OFF(LED3);
	LED_OFF(LED4);
	LED_OFF(LED5);
	LED_OFF(LED6);
	LED_OFF(LED7);
	LED_OFF(LED8);
	LED_OFF(LED9);
	LED_ON(led_tmp[led_num]);
}

//熄灭所有的灯
void LED_AllOff(void)
{
	LED_OFF(LED1);
	LED_OFF(LED2);
	LED_OFF(LED3);
	LED_OFF(LED4);
	LED_OFF(LED5);
	LED_OFF(LED6);
	LED_OFF(LED7);
	LED_OFF(LED8);
	LED_OFF(LED9);
}

//点亮所有灯
void LED_AllOn(void)
{
	LED_ON(LED1);
	LED_ON(LED2);
	LED_ON(LED3);
	LED_ON(LED4);
	LED_ON(LED5);
	LED_ON(LED6);
	LED_ON(LED7);
	LED_ON(LED8);
	LED_ON(LED9);
	
}
//点亮n个灯
void LED_DisplayPercent(uint8_t num,uint8_t dir)
{
	switch(num)
		{
		case 0:

			LED_OFF(LED1);
			LED_OFF(LED2);
			LED_OFF(LED3);
			LED_OFF(LED4);
			LED_ON(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 1:
					
			LED_OFF(LED1);
			LED_OFF(LED2);
			LED_OFF(LED3);
			
			if(dir == For_Dir)   //For_Dir 1
				LED_ON(LED4);
			else
				LED_OFF(LED4);
				
			LED_ON(LED5);
			
			if(dir == For_Dir)
			LED_OFF(LED6);
			else
			LED_ON(LED6);	
			
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
			
		case 2:
			LED_OFF(LED1);
			LED_OFF(LED2);
			if(dir == For_Dir)
			{
				LED_ON(LED3);
				LED_ON(LED4);
			}
			else
			{
				LED_OFF(LED3);
				LED_OFF(LED4);

			}
			LED_ON(LED5);
			if(dir == For_Dir)
			{
				LED_OFF(LED6);
				LED_OFF(LED7);
			}
			else
			{
				LED_ON(LED6);
				LED_ON(LED7);

			}
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 3:
			LED_OFF(LED1);
			if(dir == For_Dir)
			{
				LED_ON(LED2);
				LED_ON(LED3);
				LED_ON(LED4);
			}
			else
			{
				LED_OFF(LED2);
				LED_OFF(LED3);
				LED_OFF(LED4);

			}
			LED_ON(LED5);
			if(dir == For_Dir)
			{
				LED_OFF(LED6);
				LED_OFF(LED7);
				LED_OFF(LED8);
			}
			else
			{
				LED_ON(LED6);
				LED_ON(LED7);
				LED_ON(LED8);

			}
			LED_OFF(LED9);
			break;
		case 4:
			if(dir == For_Dir)
			{
				LED_ON(LED1);
				LED_ON(LED2);
				LED_ON(LED3);
				LED_ON(LED4);
			}
			else
			{
				LED_OFF(LED1);
				LED_OFF(LED2);
				LED_OFF(LED3);
				LED_OFF(LED4);
			}
			LED_ON(LED5);
			if(dir == For_Dir)
			{
				LED_OFF(LED6);
				LED_OFF(LED7);
				LED_OFF(LED8);
				LED_OFF(LED9);
			}
			else
			{
				LED_ON(LED6);
				LED_ON(LED7);
				LED_ON(LED8);
				LED_ON(LED9);
			}
			break;
		
		default:
		break;
						
		}
}

//点亮n个灯
void LED_DisplayNum(uint8_t num)
{
	switch(num)
		{
		case 1:
			LED_ON(LED1);
			LED_OFF(LED2);
			LED_OFF(LED3);
			LED_OFF(LED4);
			LED_OFF(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
			
		case 2:
			LED_ON(LED1);
			LED_ON(LED2);
			LED_OFF(LED3);
			LED_OFF(LED4);
			LED_OFF(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 3:
			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_OFF(LED4);
			LED_OFF(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 4:
			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_OFF(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 5:
			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_ON(LED5);
			LED_OFF(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 6:

			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_ON(LED5);
			LED_ON(LED6);
			LED_OFF(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 7:

			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_ON(LED5);
			LED_ON(LED6);
			LED_ON(LED7);
			LED_OFF(LED8);
			LED_OFF(LED9);
			break;
		case 8:

			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_ON(LED5);
			LED_ON(LED6);
			LED_ON(LED7);
			LED_ON(LED8);
			LED_OFF(LED9);
			break;
		case 9:

			LED_ON(LED1);
			LED_ON(LED2);
			LED_ON(LED3);
			LED_ON(LED4);
			LED_ON(LED5);
			LED_ON(LED6);
			LED_ON(LED7);
			LED_ON(LED8);
			LED_ON(LED9);
			break;
		default:
		break;
						
		}
}

void LED_blink(uint8_t led_pin)
{
	static uint16_t led_r_blink_tick_cnt = 0;
	uint8_t led_tmp[] = {0,LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9};

	led_r_blink_tick_cnt++;

	if(led_r_blink_tick_cnt > LED_BLINK_TIME)
	{
		nrf_gpio_pin_toggle(led_tmp[led_pin]);
		led_r_blink_tick_cnt = 0;
	}
}

void LED_nBlink(uint8_t led_num)
{
	static uint16_t led_r_blink_tick_cnt = 0;
	uint8_t led_tmp[] = {0,LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9};
 	uint8_t i;
	
	led_r_blink_tick_cnt++;

	if(led_r_blink_tick_cnt > LED_BLINK_TIME)
	{
		for(i=1;i<=led_num;i++)
		nrf_gpio_pin_toggle(led_tmp[i]);
		
		led_r_blink_tick_cnt = 0;
	}
}

void LED_nBlink_Slow(uint8_t led_num)
{
	static uint16_t led_r_blink_tick_cnt = 0;
	uint8_t led_tmp[] = {0,LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9};
 	uint8_t i;
	
	led_r_blink_tick_cnt++;
	if(led_r_blink_tick_cnt > LED_BLINK_TIME)
	{
		for(i=9;i>(9-led_num);i--)
		nrf_gpio_pin_toggle(led_tmp[i]);	
		led_r_blink_tick_cnt = 0;
	}
}


void LED_nBlink_Fast(uint8_t led_num)
{
	static uint16_t led_r_blink_tick_cnt = 0;
	uint8_t led_tmp[] = {0,LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9};
 	uint8_t i;
	
	led_r_blink_tick_cnt++;

	if(led_r_blink_tick_cnt > FLICK_FAST_NUM)
	{
		for(i=1;i<=led_num;i++)
		nrf_gpio_pin_toggle(led_tmp[i]);
		
		led_r_blink_tick_cnt = 0;
	}
}


void LED_Display(void)
{
		switch(LCD_Handle.Menu)
		{
			case LCD_Menu_Main:
				#if 1
				if(Comm_ReadState() == Comm_State_Disconnected)//未连接，慢闪
				{
					if(LED_Handle.led_flick_cnt++ > FLICK_SLOW_NUM)
					{
						LED_Handle.led_flick_cnt = 0;
						LED_Handle.led_dir ^= 1;
						LED_DisplayPercent(4,LED_Handle.led_dir);
					}
					LED_Handle.led_connect_flag = 1;//切入连接状态标志
					BuzzerHandle.g_Buzzer_on_flag = false;
				}
				#endif
				else //connected ok
				{
					if(LED_Handle.led_connect_flag == 1)
					{
						LED_Handle.led_connect_flag = 0;
						LED_AllOff();
					}
					
					if(SYS_Handle.sys_state == SYS_State_Normal)
					{
						if(THROTTLE_Handle.state == THROTTLE_State_NeedReset)
						{
							LED_blink(5);
						}
						else
						{
							LED_DisplayPercent(THROTTLE_Handle.percent/31,(1-THROTTLE_Handle.dir));//THROTTLE_Handle.percent=0-127  //将方向调换
						}
						BuzzerHandle.g_Buzzer_on_flag = false;
					}
					else if(SYS_Handle.sys_state == SYS_State_Error)//故障指示
					{
						if(SYS_Handle.err_code<=7)
							LED_nBlink_Slow(SYS_Handle.err_code);
						else
							LED_nBlink_Fast(SYS_Handle.err_code-7);
						LED_Handle.led_connect_flag = 1;              //切入到正常状态标志，先清除显示

						BuzzerHandle.g_Buzzer_on_flag = true;
					}
				}
			break;
			case LCD_Menu_MotorCalSet:
				LED_DisplayPosition(THROTTLE_Handle.Voltage);
			break;
			case LCD_Menu_GzllSet:
				//未配对--慢闪，配对中--快闪，配对成功--常亮，配对失败--慢闪
				LCD_GzllPairCountDown(&LCD_Handle);
				if(Gzll_ReadPairState() == PairIntoState||Gzll_ReadPairState() == PairingState||Gzll_ReadPairState() == PairTimeoutState)//配对中，配对失败
				{
					//左右快闪
					if(LED_Handle.led_flick_cnt++ > FLICK_FAST_NUM)
					{
						LED_Handle.led_flick_cnt = 0;
						LED_Handle.led_dir ^= 1;
						LED_DisplayPercent(2,LED_Handle.led_dir);
					}
				}
				else if(Gzll_ReadPairState() == PairSucState||Gzll_ReadPairState() == PairBackupState)//配对成功
				{
					LED_AllOn();//全亮
					if(GZLL_Handle.PairBuzzer == true)
					{
						GZLL_Handle.PairBuzzer = false;
						BUZZER_beep_twice();
					}
				}
			break;	
			case LCD_Menu_ThCal:
				if(THROTTLE_Handle.state == THROTTLE_State_Cal_For)//向左方向校准
				{
					LED_blink(1);
					#ifdef CALL_CHECK
					if((ABS(THROTTLE_Handle.angle - LEFT_ANG) > MAX_ANG_CHECK_VAL))
					{
						LED_blink(1);
					}
					else
					{
						LED_DisplayPosition(1);
					}
					#endif
				}
				else if(THROTTLE_Handle.state == THROTTLE_State_Cal_Mid)//THROTTLE_State_Cal_For)//零点
				{
					LED_blink(5);
					#ifdef CALL_CHECK
					if((ABS(THROTTLE_Handle.angle - ZERO_ANG) > MAX_ANG_CHECK_VAL))
					{
						LED_blink(5);
					}
					else
					{
						LED_DisplayPosition(5);
					}
					#endif
				}
				else if(THROTTLE_Handle.state == THROTTLE_State_Cal_Start)//THROTTLE_State_Cal_Mid)//向右校准
				{
					LED_blink(9);
					#ifdef CALL_CHECK
					if((ABS(THROTTLE_Handle.angle - RIGHT_ANG) > MAX_ANG_CHECK_VAL))
					{
						LED_blink(9);
					}
					else
					{
						LED_DisplayPosition(9);
					}
					#endif
				}
				else if(THROTTLE_Handle.state == THROTTLE_State_Cal_Bak)//校准完
				{
					LED_AllOff();
				}
			break;	
			case LCD_Menu_HandleSide:
				if(THROTTLE_Handle.HandleSide == HANDLE_SIDE_RIGHT)
				{
					LED_DisplayPercent(4,1);
				}
				else
				{
					LED_DisplayPercent(4,0);
				}
			break;
			case LCD_Menu_MiddlePosition:
				if(Comm_ReadState() == Comm_State_Disconnected)//未连接，慢闪
				{
					if(LED_Handle.led_flick_cnt++ > FLICK_SLOW_NUM)
					{
						LED_Handle.led_flick_cnt = 0;
						LED_Handle.led_dir ^= 1;
						LED_DisplayPercent(4,LED_Handle.led_dir);
					}
				}
				else
					LED_DisplayPercent(THROTTLE_Handle.percent/31,(1-THROTTLE_Handle.dir));    //显示当前的角度挡位
			break;
			case LCD_Menu_SettingPage:
				  LED_blink(LCD_Handle.Menu_index);//LED_DisplayPosition(LCD_Handle.Menu_index);
			break;
			case LCD_Menu_ClearErr://清除故障
				  LED_blink(LCD_Handle.Menu_index);//LED_DisplayPosition(LCD_Handle.Menu_index);//LED_DisplayPosition(LCD_Handle.Menu_index);
			break;
			default:
			break;
		}
}
