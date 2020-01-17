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
#include "ep_sys.h"
#include "nrf.h"
#include "ep_motor.h"
#include "ep_battery.h"
#include "ep_lcd_6kw.h"
#include "ep_hall.h"
#include "ep_powerswitch.h"
#include "ep_buzzer.h"
#include "ep_gzll.h"
#include "main.h"
#include "ep_gps.h"
#include "as5600.h"

SYS_Handle_t SYS_Handle;

void SYS_Init(SYS_Handle_t* handle)
{
	handle->CommDisconCount = 0;
	handle->err_code        = Err_None;
	handle->sys_state       = SYS_State_Normal;
	handle->DrvSysStarupCount = 0;
}

void SYS_SetErrorCode(Err_code_e  error_code)
{
    SYS_Handle.sys_state = SYS_State_Error;
    SYS_Handle.err_code  = error_code;  
}

void SYS_SetSysNormal(SYS_Handle_t* handle)
{
    SYS_Handle.sys_state = SYS_State_Normal ;
    SYS_Handle.err_code  = Err_None ;  
}

#if 1
void SYS_ErrCheck(SYS_Handle_t* handle)
{
	if((LCD_Handle.Menu == LCD_Menu_GzllSet) )//gazell addr set menu 
	{
		return;
	}
	
	if(LCD_Handle.Menu == LCD_Menu_ThCal)
	{
		if(THROTTLE_ReadCalErrFlag() == true)//校准失败
		{
			SYS_SetErrorCode(Err_Cal);
		}
	}

	if(PowerSwitch_ReadBatState(&PowerSwitch_Handle)==HandleBAT_UV)
	{
		SYS_SetErrorCode(Err_BatLow);
	}
	else if(THROTTLE_ReadCalErrFlag() == true)//校准失败
	{
		SYS_SetErrorCode(Err_Cal);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_over_vol)//电机过压
	{
		SYS_SetErrorCode(Err_MOTOR_OV);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_over_cur)//电机过流
	{
		SYS_SetErrorCode(Err_MOTOR_OC);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_under_vol)//电机欠压
	{
		SYS_SetErrorCode(Err_MOTOR_UV);
	}
	
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_stall)//电机堵转
	{
		SYS_SetErrorCode(Err_MOTOR_Stall);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == Motor_Sensor)//电机控制错误
	{
		SYS_SetErrorCode(Err_MOTOR_Sensor);
	}	

	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_over_temp_warn)//电机控制错误
	{
		SYS_SetErrorCode(Err_MOTOR_OTWarn);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_over_temp)//电机控制错误
	{
		SYS_SetErrorCode(Err_MOTOR_OT);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_comm_fail)//电机控制错误
	{
		SYS_SetErrorCode(Err_MOTOR_Comm);
	}
	else if(MOTOR_ReadMotorStatus(&MOTOR_Handle) == motor_under_voltage_warm)//电机低压告警
	{
		SYS_SetErrorCode(Err_MOTOR_UV_Warm);
	}
//	else if(MOTOR_ReadMotorVoltageNeedSetFlag(&MOTOR_Handle) == 1)//电池电压需要设置
//	{
//		SYS_SetErrorCode(Err_MotorNeedVoltageSet);
//	}
	else if(AS5600_GetStatus() == MAGNET_LOW||THROTTLE_Handle.SensorDataErrFlag == true)
	{
		SYS_SetErrorCode(Err_As6500);
	}
	else //正常状态
	{
		if(handle->sys_state != SYS_State_Normal)
		{
			THROTTLE_On();
			handle->sys_state = SYS_State_Normal; 
			handle->CommDisconCount = 0;
		}	
	}
    
	if(handle->sys_state == SYS_State_Error)	//system occur error
	{
	   MOTOR_Off();
	   THROTTLE_Off();	   
	}
	else //sys  is nomal 
	{ 		
	    MOTOR_On();
	    THROTTLE_On(); 
	}
	
	if((Comm_ReadState() == Comm_State_Disconnected))
	{
		Comm_DataTypeSet(SysOnType);
		THROTTLE_Off();
	    MOTOR_Stop();
		SYS_DataInit();
	}
}
#endif

sys_mode_e  SYS_ReadMode(SYS_Handle_t* handle)
{
	return handle->sys_mode ;
}

SYS_state_e  SYS_ReadState(SYS_Handle_t* handle)
{
	return handle->sys_state;
}

void SYS_DataInit(void)
{	
	Comm_Handle.ReadAddrEnable = true;
	
	MOTOR_SetStatus(0);
	MOTOR_Handle.motor_status = motor_nomal;
	Gzll_SetCommStatus(CommStatus_Nomal);
}

