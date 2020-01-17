/**************************************
Company       : ePropulsion 
File name     : ep_lcd_6kw.c
Author        : LGJ
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
#include "ep_lcd_6kw.h"
#include "nrf_gpio.h"
#include "ep_button.h"
#include "ep_throttle.h"
#include "ep_hall.h"
#include "ep_pstorage.h"
#include "ep_gps.h"
#include "ep_comm.h"
#include "ep_motor.h"
#include "ep_sys.h"
#include "ep_ble_init.h"
#include "ep_buzzer.h"
#include "main.h"
#include "ep_timerecord.h"
#include "ep_version.h"
#include "ep_led.h" 
#include "ep_tick.h"
#include "ep_PowerSwitch.h"

//extern 	uint8_t start_flag;
LCD_Handle_t  LCD_Handle;

void LCD_Init(LCD_Handle_t* handle)
{
	handle->Menu              = LCD_Menu_Main;
	handle->Menu_index        = LCD_Menu_MotorCalSet;

}

void LCD_SetMenu(LCD_Menu_e menu)
{
	LCD_Handle.Menu = menu;
}

LCD_Menu_e LCD_GetMenu(void)
{
	return LCD_Handle.Menu;
}


void LCD_GzllPairCountDown(LCD_Handle_t* handle)
{
	static  uint8_t SucDisCount = 0;
		
	if(Gzll_ReadCommStatus() == CommStatus_Nomal)
	{		 
		Comm_ClearFailCnt(&Comm_Handle);	
	}
	else if(Gzll_ReadCommStatus() == CommStatus_AutoPair)
	{
		if(Gzll_ReadPairState() == PairSucState)
		{
			SucDisCount++;
			if(SucDisCount > 27)//1000/180*5 = 5s
			{
				SucDisCount = 0;
				Gzll_SetPairState(PairBackupState);
			}
		}
		else if((Gzll_ReadPairState() == PairBackupState) | (Gzll_ReadPairState() == PairIdleState))
		{
			LCD_SetMenu(LCD_Menu_Main);
			Gzll_SetCommStatus(CommStatus_Nomal);
			Gzll_Backup_gzllAddr1(&GZLL_Handle,pstorage_Wdata);
			PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD);
			PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
			Gzll_SetAddrState(addr_HaveSet);
		}
		else 
		{
			if(Gzll_ReadCountDownSec() == 1)
			{
				Gzll_SetCommStatus(CommStatus_Nomal);
				Gzll_SetPairState(PairBackupState);
				Comm_DataTypeSet(ThrottleValSetType);
				GZLL_Handle.GZLL_addr_change = true;
				handle->Menu = LCD_Menu_Main;
			}
			Gzll_PairTimeCountDown();               				
		}						
	}
}

/***************

*****************/
void LCD_Next_Menu_Config(LCD_Handle_t* handle)
{
	/**TEST CODE  END ***/
	/**************LCD_Menu_Main***********************/
	if(handle->Menu == LCD_Menu_Main)
	{
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//if((BUTTON_Handle.Menu_Button.pressCount > 333))//setting page 进入设置界面
		{
			handle->Menu = LCD_Menu_SettingPage;
			LED_AllOff();
			LCD_Handle.CountDownFlag=1;//超时退出标志
			ConutDown3SecondCnt = 0;
		}
	    if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//长按关机
		{
			Comm_DataTypeSet(SysOffType);	
			PowerSwitch_Handle.SysPowerState = PowerOff;
			LED_SysOff();
		}
	}//end of if(handle->Menu == LCD_Menu_Main)
	//----------------setting page
	else if(handle->Menu == LCD_Menu_SettingPage)
	{
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//短按调整参数
		{
			handle->Menu_index++;
			if(handle->Menu_index > LCD_Menu_ThCal)//清除故障
			{
				handle->Menu = LCD_Menu_Main;
				handle->Menu_index = LCD_Menu_MotorCalSet;
			}
			LED_AllOff();
			BUZZER_beep_once();
		}
		if((BUTTON_Handle.Menu_Button.pressCount > 166))
		{
			handle->Menu = (LCD_Menu_e)handle->Menu_index;
			BUZZER_beep_once();
			LED_AllOff();
			if(handle->Menu == LCD_Menu_ThCal)//方向盘 校准
			{
				THROTTLE_cal_state(&THROTTLE_Handle,true);
				THROTTLE_SetState(&THROTTLE_Handle,THROTTLE_State_Cal_Start);
				THROTTLE_SetCalErrFlag(false);
			}
			else if(handle->Menu == LCD_Menu_GzllSet)//无线配对
			{
				handle->SetDataTemp.GzllAddrTemp = GZLL_Handle.config.base_address1;

				Gzll_SetCommStatus(CommStatus_AutoPair);
				Gzll_SetPairState(PairIntoState);
				Gzll_SetCountDownSec(PAIR_COUNTDOWN_SEC_MAX);
				GZLL_Handle.GZLL_addr_change = true;
				Comm_DataTypeSet(GzllPairRequestType);	
				GZLL_Handle.PairBuzzer = true;
			}
			else if(handle->Menu == LCD_Menu_HandleSide)//设置左右舷
			{
				#if 0
				if(Comm_ReadState() == Comm_State_Disconnected)//未连接，慢闪
				{
					handle->Menu = LCD_Menu_Main;
				}
				#endif
			}
			else if(handle->Menu == LCD_Menu_MiddlePosition)//设置中位点
			{
				if(Comm_ReadState() == Comm_State_Disconnected)//未连接，慢闪
				{
					handle->Menu = LCD_Menu_Main;
				}
			}
			else if(handle->Menu == LCD_Menu_MotorCalSet)//设置电压
			{
				if(Comm_ReadState() == Comm_State_Disconnected)//未连接，慢闪
				{
					handle->Menu = LCD_Menu_Main;
				}
			}
		}

	}
	//---------------------------
	/**************LCD_Menu_GzllSet  gazell addr set ***********************/
	else if(handle->Menu == LCD_Menu_GzllSet)
	{
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)
		{
			Gzll_SetCommStatus(CommStatus_Nomal);
			Gzll_SetPairState(PairBackupState);
			Comm_DataTypeSet(ThrottleValSetType);
			GZLL_Handle.GZLL_addr_change = true;
			//handle->Menu = LCD_Menu_ThCal;
					BUZZER_beep_once();
		}
		#if 1
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)
		{
			BUZZER_beep_twice();
			handle->Menu = LCD_Menu_Main; 
//			if(handle->SetDataTemp.GzllAddrTemp == 0)
//			{
//				Gzll_SetAddrState(addr_NoSet);		
//			}
//			else
//			{
//				GZLL_Handle.GZLL_addr_change = true;
////				GZLL_Handle.config.base_address1 = handle->SetDataTemp.GzllAddrTemp;
//				Comm_DataTypeSet(GzllAddr1HSetType);
//				Gzll_Backup_gzllAddr1(&GZLL_Handle,pstorage_Wdata);
//				PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD);
//				PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
//				Gzll_SetAddrState(addr_HaveSet);	
//			}    
		}
		#endif
	}  
	
	/**************LCD_Menu_ThCal 角度校准界面***********************/
	else if(handle->Menu == LCD_Menu_ThCal)
	{
		/**************Menu_Button*************/
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//切换校准位置
		{
			if(THROTTLE_Cal_Next(&THROTTLE_Handle) == 1)//caculation finsh and success
			{
				THROTTLE_Backup_cal(&THROTTLE_Handle,pstorage_Wdata);
				PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_ang_handle,ANG_BLOCK_ADD);
				PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 

				LCD_Handle.Menu = LCD_Menu_Main;
				THROTTLE_cal_state(&THROTTLE_Handle,false);
				THROTTLE_Handle.state = THROTTLE_State_Reseted;	
				THROTTLE_Handle.throttleRstFlag = false;
			}	
			BUZZER_beep_once();
		}
		/*******************Select_Button***********************/
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//返回主界面
		{
			if(THROTTLE_ReadCalErrFlag() == true)
			{
				//return;
			}
			handle->Menu = LCD_Menu_Main;
			THROTTLE_cal_state(&THROTTLE_Handle,false);
			THROTTLE_Handle.state = THROTTLE_State_Reseted;	
			LED_AllOff();
			BUZZER_beep_twice();
		}
#if 0
		else if(BUTTON_Handle.Select_Button.touch_type == Touch_Long)
		{
			THROTTLE_Handle.state = THROTTLE_State_Cal_Start;
			THROTTLE_cal_state(&THROTTLE_Handle,true);
			THROTTLE_SetCalErrFlag(false);
			SYS_SetSysNormal(&SYS_Handle);
		}
#endif
	}
	else if(handle->Menu == LCD_Menu_MotorCalSet)//丝杆校准
	{
		/**************Menu_Button*************/
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//
		{
			THROTTLE_Handle.Voltage = VOLTAGE12V;
			BUZZER_beep_once();
		}
		/*******************Select_Button***********************/
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//返回主界面
		{
			BUZZER_beep_twice();
			Comm_DataTypeSet(MotorCalSetType);//按下确认才发送指令
		}
	}
	else if(handle->Menu == LCD_Menu_HandleSide)
	{
		/**************Menu_Button*************/
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//
		{
			if(THROTTLE_Handle.HandleSide == HANDLE_SIDE_RIGHT)
				THROTTLE_Handle.HandleSide = HANDLE_SIDE_LEFT;
			else
				THROTTLE_Handle.HandleSide = HANDLE_SIDE_RIGHT;
			
			//Comm_DataTypeSet(HandleSideSetType);
					BUZZER_beep_once();
		}
		/*****
		**************Select_Button***********************/
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//返回主界面
		{
			BUZZER_beep_twice();
			
			PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
			PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_percent_max_handle,PERCENT_MAX_BLOCK_ADD);
			THROTTLE_Backup_PercentMax(&THROTTLE_Handle,pstorage_Wdata);//掉电存储
			
			Comm_DataTypeSet(ThrottleValSetType);
			handle->Menu = LCD_Menu_Main;
			LED_AllOff();
		}
	}
	else if(handle->Menu == LCD_Menu_MiddlePosition)
	{
		/**************Menu_Button*************/
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//短按开始定位中位点
		{
			Comm_DataTypeSet(NeutralSetType);	
			BUZZER_beep_once();
		}
		/*******************Select_Button***********************/
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//返回主界面
		{
			BUZZER_beep_twice();
			handle->Menu = LCD_Menu_Main;
			Comm_DataTypeSet(ThrottleValSetType);
			LED_AllOff();
		}
	}
	else if(handle->Menu == LCD_Menu_ClearErr)
	{
		/**************Menu_Button*************/
		if(BUTTON_Handle.Menu_Button.touch_type == Touch_Short)//短按开始定位中位点
		{
			Comm_DataTypeSet(ErrorClearType);	
			BUZZER_beep_once();
		}
		/*******************Select_Button***********************/
		else if(BUTTON_Handle.Menu_Button.touch_type == Touch_Long)//返回主界面
		{
			BUZZER_beep_twice();
			handle->Menu = LCD_Menu_Main;
			Comm_DataTypeSet(ThrottleValSetType);
			LED_AllOff();
		}
	}
}

//#endif
