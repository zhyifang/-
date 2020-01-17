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
#include "nrf.h"
#include "ep_battery.h"
#include <stdbool.h>
#include  "ep_comm.h"
#include  "ep_lcd_6kw.h"
#include  "ep_ble_init.h"
#include  "ep_ble_motor.h"
#include  "ep_ble_battery.h"
#include  "ep_pstorage.h"
#include  "main.h"
#include  "ep_sys.h"
#include  "ep_motor.h"

BATTERY_Handle_t BATTERY_Handle;

void BATTERY_Init(BATTERY_Handle_t* handle)
{
	handle->Bat_SocState     = bat_soc_more;
	handle->BatVoltage       = 0;
	handle->BatCurrent       = 0;
	handle->BatStatus        = 0;
	handle->useBatVol        = 0;
	handle->useBatRealCap    = 0;
	handle->useBatType       = BatteryType_Pb;
	handle->useBatVol        = 480;
	handle->useBatRealCap    = 130;
	handle->SaveSocFlag      = false;
	handle->State            = Battery_State_Off;
	handle->SendSocFlag      = true;
	handle->BatSoc = handle->SaveSoc;
}
void BATTERY_SetBatSocState(Bat_SocState_e state)
{
	BATTERY_Handle.Bat_SocState = state;
}
Bat_SocState_e BATTERY_GetBatState(BATTERY_Handle_t* handle)
{
	return handle->Bat_SocState;
}	
#if 0
void BATTER_comm_RX_decode(BATTERY_Handle_t* handle,Comm_Handle_t* comm_handle)
{
	uint16_t rx_data;
	switch(comm_handle->Rec.Data.ReturnType)  
	{
		case Comm_Ret_SystemStartup:
			 rx_data = (comm_handle->Rec.Data.MsgLo);
			 BATTERY_DrvSysStartCheck(rx_data);
            // MOTOR_CheckCommFail(&MOTOR_Handle,comm_handle->Rec.Data.MsgHi);	
             MOTOR_CheckStatus(&MOTOR_Handle,comm_handle->Rec.Data.MsgHi);		
		     SYS_Handle.DrvSysStarupCount++;
			break;
		case Comm_Ret_BatteryVoltage: 	    
			handle->BatVoltage = (comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo);	
		    Motor_SetCanRunFlag(true);
		    Comm_DataTypeSetThro(comm_handle);	
            Comm_ReadAddrCheck(comm_handle);			
			break;   
		case Comm_Ret_BatteryCapacity:
			handle->BatSoc = comm_handle->Rec.Data.MsgLo;
    		BATTERY_CheckSoc(handle);//如果使用
			break;
		case Comm_Ret_BatteryStatus:
			handle->Status = (Battery_Status_e)((comm_handle->Rec.Data.MsgHi << 7)|(comm_handle->Rec.Data.MsgLo));
			break;
		default :break;

	}
}
#endif
Battery_Status_e  BATTERY_GetBatStatus(BATTERY_Handle_t * handle)
{
	return  handle->Status;
}
void  BATTERY_SetBatTemp(BATTERY_Handle_t * handle,uint8_t HighTemp,uint8_t LowTemp)
{
	handle->HighestTemp = HighTemp;
	handle->LowestTemp  = LowTemp;
}
#if 0
void  BATTERY_CheckSoc(BATTERY_Handle_t * handle)
{
	static uint8_t count =0 ;
	if(handle->useBatType != Battery_TypeOwnBatt)
	{
		if(handle->Status == Battery_Status_NonEpBatt)//if not use ep battery ,need send soc  
		{
			if(handle->SendSocFlag == true)
			{
				if(count++ > 2)
				{
					if(ABS((handle->BatSoc - handle->SaveSoc)) < 15)
					{
						handle->BatSoc = handle->SaveSoc;
						Comm_DataTypeSet(BatSocSetType);
					}
					count = 0;
					handle->BatSoc = handle->SaveSoc;
					handle->SendSocFlag = false;
				}
			}		
			else
			{
				BATTERY_SaveSoc(handle);
			}
		}
	}
	if(handle->Status == Battery_Status_On)
	{
		handle->useBatType = Battery_TypeOwnBatt;
	}
	
		
}
#endif
void  BATTERY_SetSendSocFlag(BATTERY_Handle_t * handle,bool flag)
{
	handle->SendSocFlag = flag;
	if(handle->useBatType == Battery_TypeOwnBatt)
	{
		handle->SendSocFlag = false;
	}
}
void  BATTERY_SaveSoc(BATTERY_Handle_t * handle)
{
	if(handle->SaveSocFlag  == true) //every 1s to save soc 
	{
		handle->SaveSoc  = handle->BatSoc;
		handle->SaveSocFlag  = false;
	}
}
uint16_t BATTERY_ReadBatSoc(BATTERY_Handle_t * handle)
{
	return handle->BatSoc;
}
BatteryType_e BATTERY_ReadBatType(BATTERY_Handle_t * handle)
{
	return handle->useBatType;
}
#if 0
void BATTERY_DrvSysStartCheck(uint16_t StartupValue)
{
	if(StartupValue == Comm_Ret_SystemStartup+1)
	{
		
		THROTTLE_Handle.state = THROTTLE_State_NeedReset;
		//Comm_DataTypeSet(RestartReplyType); 
	}
}
#endif
BatteryType_e  BATTERY_Get_useBatType(BATTERY_Handle_t * handle)
{
    return handle->useBatType;
}

uint16_t  BATTERY_Get_BatCap(BATTERY_Handle_t * handle)
{
    return handle->useBatRealCap;
}
uint16_t  BATTERY_Get_BatUseVol(BATTERY_Handle_t * handle)
{
    return handle->useBatVol;
}

void  BATTERY_Set_BatCapType(BATTERY_Handle_t * handle,uint16_t useBatCapType)
{
    handle->useBatRealCap = useBatCapType & 0x7FF;
	handle->useBatType    = (BatteryType_e)(useBatCapType >> 11);
	
}
void  BATTERY_Set_BatUseVol(BATTERY_Handle_t * handle,uint16_t useBatvol )
{
     handle->useBatVol = useBatvol;
}
void  BATTERY_CheckBatType(BATTERY_Handle_t * handle)
{
	static  BatteryType_e  pre_Type = Battery_TypeE ;
	
	if((pre_Type == BatteryType_LiOn ) && (handle->useBatType == BatteryType_Pb))
	{
	    handle->useBatVol = 480;	
	}
	else if((pre_Type == BatteryType_Pb ) && (handle->useBatType == Battery_TypeLiFe))
	{
	    handle->useBatVol = 480;	
	}
	else if((pre_Type == Battery_TypeLiFe ) && (handle->useBatType == BatteryType_LiOn))
	{
	    handle->useBatVol = 481;	
	}
	pre_Type = handle->useBatType;
}
void  BATTERY_CheckVolRange(BATTERY_Handle_t * handle)
{
	 if(handle->useBatType == BatteryType_LiOn)
	 {
		if(handle->useBatVol > 532)
		{
			handle->useBatVol = 532;
		}
		if(handle->useBatVol < 432)
		{
			handle->useBatVol = 432;
		}
	 }
	 else if(handle->useBatType == BatteryType_Pb)
	 {
		if(handle->useBatVol > 540)
		{
			handle->useBatVol = 540;
		}
		if(handle->useBatVol < 440)
		{
			handle->useBatVol = 440;
		}
	 }
	 else if(handle->useBatType == Battery_TypeLiFe)
	 {
		if(handle->useBatVol > 512)
		{
			handle->useBatVol = 512;
		}
		if(handle->useBatVol < 448)
		{
			handle->useBatVol = 448;
		}
	 }
}
void BATTERY_AddVol(BATTERY_Handle_t * handle)
{
    uint16_t  LiBatVolBuf[9] = {432,444,456,468,481,494,504,518,532};
	
	if(handle->useBatType == BatteryType_LiOn)
	{
		uint8_t i;
		for(i = 0;i < 8;i++)
		{
			if(handle->useBatVol == LiBatVolBuf[i])
			{
				handle->useBatVol = LiBatVolBuf[i+1];
				return;
			}
			
		}
		if(handle->useBatVol == LiBatVolBuf[8])
		{
			handle->useBatVol = LiBatVolBuf[0];
		}
	}
	else if(handle->useBatType == BatteryType_Pb)
	{
		handle->useBatVol += 20;
		if(handle->useBatVol > 540)
		{
			handle->useBatVol = 440;
		}
	}
	else if(handle->useBatType == Battery_TypeLiFe)
	{
		handle->useBatVol += 32;
		if(handle->useBatVol > 512)
		{
			handle->useBatVol = 448;
		}
	}
}
	
////////////////////////////////////
void BATTERY_Backup_battery(BATTERY_Handle_t * handle,uint8_t* data)
{
	*(data+0) = (uint8_t) ( (handle->useBatType) & 0x0F );   
	*(data+1) = (uint8_t) ( ((handle->useBatRealCap) >> 8) & 0xFF );   
	*(data+2) = (uint8_t) ( (handle->useBatRealCap) & 0xFF );   
	*(data+3) = (uint8_t) ( ((handle->useBatVol) >> 8) & 0xFF );   
	*(data+4) = (uint8_t) ( (handle->useBatVol) & 0xFF );  
	*(data+5) = (uint8_t) ( (handle->SaveSoc) & 0xFF );  
    *(data+6) = PSTORAGE_CheckSum(data, 6);	
}
bool BATTERY_Update_battery(BATTERY_Handle_t * handle,uint8_t* data)
{
	if(data[6] == PSTORAGE_CheckSum(data, 6)) 
	{ 
		handle->useBatType     = (BatteryType_e)(*(data+0) & 0x0F);  
//		if((handle->useBatType < BatteryType_LiOn) || (handle->useBatType > Battery_TypeLiFe)) 
//		{
//			handle->useBatType = BatteryType_LiOn;
//		}
		handle->useBatRealCap  = (uint16_t)(*(data+1) << 8 ) + (uint16_t)( *(data+2) ); 
        handle->useBatVol      = (uint16_t)(*(data+3) << 8 ) + (uint16_t)( *(data+4) ); 
        handle->SaveSoc        = (uint8_t)*(data+5);
        handle->BatSoc         =  handle->SaveSoc;	
       //if((handle->useBatType < BatteryType_LiOn) || (handle->useBatType > Battery_TypeLiFe)) 
	    if((handle->useBatType > Battery_TypeLiFe)) 
		{
			handle->useBatType = BatteryType_LiOn;
			handle->useBatVol  = 481;
		}		
		if(handle->useBatType == Battery_TypeOwnBatt)
		{
			handle->SendSocFlag = false;
		}
		return true;
	}
	return false;
}

uint16_t BATTERY_ReadVol(void)
{
    return  BATTERY_Handle.BatVoltage;
}



