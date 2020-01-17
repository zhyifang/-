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
#include "ep_motor.h"
#include <stdbool.h>
#include  "ep_comm.h"
#include  "ep_buzzer.h"
#include  "ep_lcd_6kw.h"
#include  "ep_ble_init.h"
#include  "ep_ble_motor.h"

MOTOR_Handle_t  MOTOR_Handle;

void MOTOR_Init(MOTOR_Handle_t*  handle)
{
	handle->motor_status     = motor_nomal;
	handle->VoltageNeedSetFlag = 0;
	
}
#if 0

bool Motor_ReadCanRunFlag(void)
{
	return MOTOR_Handle.MotorCanRunFlag;
}
void Motor_SetCanRunFlag(bool flag)
{
    MOTOR_Handle.MotorCanRunFlag = flag;
}
void MOTOR_ClearUvCount(MOTOR_Handle_t*  handle)
{
	 handle->MotorUvCount =  0;
}
#endif

void MOTOR_Stop(void)
{
	 MOTOR_Handle.state = MOTOR_State_Stop;
}
void MOTOR_Off(void)
{
	 MOTOR_Handle.state = MOTOR_State_Off;
}
void MOTOR_On(void)
{ 
	 MOTOR_Handle.state = MOTOR_State_On;
}

void MOTOR_comm_RX_decode(MOTOR_Handle_t* handle,Comm_Handle_t* comm_handle)
{
	uint8_t ReturnTmp = 1;

	ReturnTmp = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
	switch(comm_handle->Rec.Data.ReturnType)
	{		
		case Comm_Ret_MotorStatus:     //��ȡ���״̬
			handle->status = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);
		    MOTOR_CheckStatus(handle,handle->status);
			break;
		case Comm_Ret_MotorOriginal:   //����ԭ��ɹ�
			ReturnTmp = comm_handle->Rec.Data.MsgLo; 
			if(ReturnTmp == 0)
			{
				//LCD_SetMenu(LCD_Menu_Main);//��Ҫ�û�ȷ���˳���ֱ���˳��޷��޸�ѡ��
		    	//Comm_DataTypeSet(ThrottleValSetType);
//				LED_AllOff();
			}
			break;
			
		case Comm_Ret_MotorNeutralSuc://������λ��ɹ�
			ReturnTmp |= comm_handle->Rec.Data.MsgLo; 
//			if(ReturnTmp == 0)
			{
				//LCD_SetMenu(LCD_Menu_Main);
		   		//Comm_DataTypeSet(ThrottleValSetType);
//				LED_AllOff();
			}
			break;
			
			#if 1
		case Comm_Ret_MotorVolSuc://���õ�ѹ�ɹ�
			ReturnTmp = comm_handle->Rec.Data.MsgLo; 
			//if(ReturnTmp == 0)
			{
				LCD_SetMenu(LCD_Menu_Main);
		   		Comm_DataTypeSet(ThrottleValSetType);	
				handle->VoltageNeedSetFlag = 0;//ȡ����ѹ���������־
//				LED_AllOff();
			}
			break;
			#endif 
		case Comm_Ret_VoltageNeedSet://���õ�ѹ��ʾ���ϵ�������ȷ�Ϸ���
			ReturnTmp = comm_handle->Rec.Data.MsgLo; 
			if(ReturnTmp == 0)
			{
				//LCD_SetMenu(LCD_Menu_Main);
				handle->VoltageNeedSetFlag = 0;	//����Ҫ���õ�ص�ѹ
	    		//Comm_DataTypeSet(ThrottleValSetType);	
//				LED_AllOff();
			}
			else if(ReturnTmp == 1)
			{
				handle->VoltageNeedSetFlag = 1;	//��Ҫ���õ�ص�ѹ
			}
			break;
		case Comm_Ret_HandleSideSet://����������
			ReturnTmp = comm_handle->Rec.Data.MsgLo; 
			//if(ReturnTmp == true)
			{
				//LCD_SetMenu(LCD_Menu_Main);
	    		//Comm_DataTypeSet(ThrottleValSetType);
//				LED_AllOff();
			}
			break;
		case Comm_Ret_ErrClear://����������
			ReturnTmp = comm_handle->Rec.Data.MsgLo; 
			if(ReturnTmp == 0)
			{
				LCD_SetMenu(LCD_Menu_Main);
	    		Comm_DataTypeSet(ThrottleValSetType);
				LED_AllOff();
			}
			break;
		default : break;
	}
}

uint8_t  MOTOR_ReadMotorVoltageNeedSetFlag(MOTOR_Handle_t* handle)
{
	return  handle->VoltageNeedSetFlag;
}

void MOTOR_CheckCommFail(MOTOR_Handle_t* handle,uint8_t status)
{
	static  uint16_t  CheckCount = 0;
	
//	if(handle->DrvSysStartupFlag == true)
//	{
//		handle->DrvSysStartupFlag = false;
//		CheckCount = 0;
//	}
	if(CheckCount < 500 )// 500*30MS  = 15s
	{
		CheckCount++;
	}	
	else
	{
		if((status & ERR_COMM_FAIL_DETECTED_BIT) >> 0)
		{
			handle->motor_status = motor_comm_fail;
		}
	}
}

void MOTOR_SetStatus(uint16_t status)
{
	MOTOR_Handle.status = status;	
}

uint16_t  MOTOR_ReadStatus(void)
{
	return MOTOR_Handle.status;	
}

void MOTOR_CheckStatus(MOTOR_Handle_t* handle,uint16_t status)
{
#if 1
	if((status & ERR_UVwarn_BIT))
	{
		handle->motor_status = motor_under_voltage_warm;
	}
	else if((status & ERR_MOT_OV_BIT) >> 1)
	{
		handle->motor_status = motor_over_vol;
	}
	else if((status & ERR_MOT_UV_BIT) >> 2)
	{
		handle->motor_status = motor_under_vol;
	}
	else if((status & ERR_MOT_OC_BIT) >> 3)
	{
		handle->motor_status = motor_over_cur;
	}
	else if((status & ERR_MOT_STALL_BIT) >> 4)
	{
		handle->motor_status = motor_stall;
	}
	else if((status & ERR_MOT_OT_BIT) >> 5)
	{
		handle->motor_status = motor_over_temp;
	}
	else if((status & ERR_MOT_OTW_BIT) >> 6)
	{
		handle->motor_status = motor_over_temp_warn;
	}
	else if((status & ERR_MOT_SENSOR_BIT) >> 7)
	{
		handle->motor_status = Motor_Sensor;
	}
	else if((status & ERR_COMM_FAIL_DETECTED_BIT))
	{
		handle->motor_status = motor_comm_fail;
	}
	else
	{
		handle->motor_status = motor_nomal;
	}
	#endif

}

void Motor_SetMotStatus(MOTOR_Status_e status)
{
	MOTOR_Handle.motor_status = status;
}

MOTOR_Status_e 	MOTOR_ReadMotorStatus(MOTOR_Handle_t* handle)
{
	return handle->motor_status;
}
/////////////////////////////////////////


