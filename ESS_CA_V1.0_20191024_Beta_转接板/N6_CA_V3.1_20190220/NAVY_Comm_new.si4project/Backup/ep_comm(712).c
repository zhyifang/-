#include "ep_comm.h"
#include "ep_rs485.h"
#include "ep_tick.h"
#include "ep_temp.h"
#include "ep_gps.h"
#include "ep_motor.h"
#include "ep_battery.h"
#include "ep_pstorage.h"
#include "ep_crypt_host.h"
#include "ep_protection.h"
#include "ep_version.h"
#include "ep_spi_slave.h"

uint8_t  ControllerMcuID;

extern Comm_Handle_t Comm_Handle;
extern Motor_Handle_t Motor_Handle;
extern GPS_Handle_t Gps_Handle;
extern Battery_Handle_t Battery_Handle;

static uint8_t Comm_CheckSum(uint8_t* data, uint8_t size);
uint8_t* Comm_PrepareRet(Comm_Handle_t* handle);

void Comm_Init(Comm_Handle_t* handle)
{
	RS485_Init();
	handle->State = Comm_State_Disconnected;
    handle->MotorState = Comm_MotorState_Off;
	handle->TxPtr = 0;
	handle->RxPtr = 0;
	handle->CommFailCnt = 0;
	handle->Mode = Comm_Mode_RS485;
	handle->MotorRpm = 0;
	handle->MotorPower = 0;
    handle->ThrottlePercentageVolt = 100;       // 100%
    handle->ThrottlePercentageTemp = 100;       // 100%   
    handle->ThrottlePercentageBatt = 100;       // 100%       
	handle->RetCmdIndex = 0;
	handle->BatteryOnOff = 0;
	handle->Mode = Comm_Mode_RS485;
	for(uint8_t i = 0; i < MAINSYSTEM_CMD_LEN; i++)
    {
        handle->Rec.Raw[i] = 0;
        handle->Ret.Raw[i] = 0;
    }
    RS485_LockMode(RS485_REMOTE);
}
uint16_t TxData = 0;
uint8_t* Comm_PrepareRet(Comm_Handle_t* handle)
{
	static uint16_t TxData = 0;
	
	handle->Ret.Data.Addr = ADDR_BRIDGESYSTEM;
	
	if(Comm_ReadSetupType(handle) != ReadMotorStatusType)
	{
		if(Comm_ReadSetupType(handle) == ReadGzllEnable)
		{
			handle->RetCmdIndex = 20;
		}
        else if(Comm_ReadSetupType(handle) == ReadGzllAddrHType)
		{
			handle->RetCmdIndex = 21;
		}
		else if(Comm_ReadSetupType(handle) == ReadGzllAddrLType)
		{
			handle->RetCmdIndex = 22;
		}
        else if(Comm_ReadSetupType(handle) == ReadGzllFinish)
		{
			handle->RetCmdIndex = 23;
		}
		else if(Comm_ReadSetupType(handle) == VoltageNeedSetType)
		{
			handle->RetCmdIndex = 24;
		}
		else if(Comm_ReadSetupType(handle) == SetNeutralType)
		{
			handle->RetCmdIndex = 25;
		}		
		else if(Comm_ReadSetupType(handle) == SetHandleSideType)
		{
			handle->RetCmdIndex = 26;
		}
		else if(Comm_ReadSetupType(handle) == SetVoltageType)
		{
			handle->RetCmdIndex = 27;
		}
		else if(Comm_ReadSetupType(handle) == SetErrClearType)
		{
			handle->RetCmdIndex = 28;
		}
		else if(Comm_ReadSetupType(handle) == SysOnRetType)
		{
			handle->RetCmdIndex = 29;
		}
		else if(Comm_ReadSetupType(handle) == SysOffRetType)
		{
			handle->RetCmdIndex = 30;
		}
	}	

	switch(handle->RetCmdIndex)
	{
		case 0:

			break;
		case 1://motor statue
			handle->Ret.Data.ReturnType = Comm_Ret_MotorStatus;
			TxData = Motor_Handle.MotorObj.ErrorFlag;
			break;
		case 2://motor statue
			handle->Ret.Data.ReturnType = Comm_Ret_MotorPosition;
			TxData = Motor_Handle.MotorPosition2Steering;
			break;
		case 3://motor statue
			handle->Ret.Data.ReturnType = Comm_Ret_MotorRst;
			TxData = Motor_Handle.MotorSysState;
			break;
		//===============================pair===============================
        case 20:		
             handle->Ret.Data.ReturnType = Comm_Ret_PairEnable;    
		     TxData = ControllerMcuID; 
             handle->RetCmdIndex = 1;		
			 break;  		
        case 21:		
             handle->Ret.Data.ReturnType = Comm_Ret_GzllAddrH;    
		     TxData = GZLL_Get_gzllAddr1(&GZLL_Handle)>>14; 
             handle->RetCmdIndex = 1;		
			 break;         
        case 22:
			 handle->Ret.Data.ReturnType = Comm_Ret_GzllAddrL;
		     TxData = GZLL_Get_gzllAddr1(&GZLL_Handle)& 0x3FFF; 
             handle->RetCmdIndex = 1;			
			 break;	
        case 23:
			 handle->Ret.Data.ReturnType = Comm_Ret_ReadGzllAdrRecSuc;
             handle->RetCmdIndex = 1;			
			 break;	
	    case 24:
			 handle->Ret.Data.ReturnType = Comm_RetVoltageNeedSet;//需要设置电压标志
			 TxData = (SetBatteryVoltageNeeded==1?1:0);//Motor_GetVoltageSetFlag(&Motor_Handle);
             //handle->RetCmdIndex = 1;			
			 break;	
	    case 25:
			 handle->Ret.Data.ReturnType = Comm_RecMotorNeutralSuc;
			 TxData = (DriverCalibrationStatus==1?0:1);
             //handle->RetCmdIndex = 1;			
			 break;	
	    case 26:
			 handle->Ret.Data.ReturnType = Comm_RetHandleSideSet;
			 TxData = UserHandleSideSetStatus==1?0:1;
             //handle->RetCmdIndex = 1;			
			 break;		 
	    case 27:
			 handle->Ret.Data.ReturnType = Comm_RecMotorVolSuc;
			 TxData = (VoltageStatus==0?0:1);
             //handle->RetCmdIndex = 1;			
			 break;		
		case 28:
			 handle->Ret.Data.ReturnType = Comm_RetErrClear;
			 TxData = (VoltageStatus==1?0:1);
             //handle->RetCmdIndex = 1;			
			 break;		
		case 29:
			 handle->Ret.Data.ReturnType = Comm_Ret_CrSysOn;
			 TxData = true;
             //handle->RetCmdIndex = 1;			
			 break;		
 		case 30:
			 handle->Ret.Data.ReturnType = Comm_Ret_CrSysOff;
			 TxData = true;
             //handle->RetCmdIndex = 1;			
			 break;		
		//===================================================================
		//read the drv and the commrealy msg	
		default:			
			handle->Ret.Data.ReturnType = Comm_Ret_NULL;
			TxData = 0;
			break;
	}
	
	handle->Ret.Data.MsgHi = (TxData >> 7) & 0x7f;
	handle->Ret.Data.MsgLo = TxData & 0x7f;
	handle->Ret.Data.CheckSum  = Comm_CheckSum(handle->Ret.Raw, MAINSYSTEM_RET_LEN - 1) & 0x7f;
    	#if 1
    if((Comm_GetState(handle) != Comm_State_Connected))
    {
        handle->RetCmdIndex = 0;
    }
    else
    {
        handle->RetCmdIndex = handle->RetCmdIndex%3+1;
    }
       #endif 
	return (uint8_t*)(handle->Ret.Raw);
}

extern uint8_t cmdOverTimeCnt;
uint8_t cmd_index = 0;
void Comm_Decode(Comm_Handle_t* handle)
{  
    static uint8_t ReadAddrRecSucCount = 0;	
	//uint16_t msg = 0 ;
	if((Comm_Handle.Rec.Data.Addr != ADDR_BRIDGESYSTEM) || (handle->Rec.Data.CheckSum != (Comm_CheckSum(handle->Rec.Raw, MAINSYSTEM_CMD_LEN -1)&0x7f)))
    {
		return;
	}	
    #if 0
	if(cmd_index < 50)
	{
		cmd_buf[cmd_index++] = handle->Rec.Data.RequestedType;
		if(cmd_index >= 50)
		{
			cmd_index = 0;
		}
	}
	#endif
	switch(handle->Rec.Data.RequestedType)
	{
	#if 1
		case Comm_Cmd_SetSysOn:  
			if(Comm_GetState(handle) != Comm_State_Connected)
			{
					handle->State = Comm_State_Connected;
			}
			Motor_Handle.MotorPosition2Motor = handle->Rec.Data.MsgHi << 7 | handle->Rec.Data.MsgLo;
			Motor_Handle.SetMotorOffFlag = false;
			Motor_Handle.SetMotorOnFlag = true;
			Motor_Handle.MotorSysState  = false;
			cmdOverTimeCnt = 0;
			//===================test
			Comm_SetSetupType(SysOnRetType);//repeat directly
			//=======================
			break;
		case Comm_Cmd_SetSysOff:  
            if(Comm_GetState(handle) != Comm_State_Connected)
            {
                handle->State = Comm_State_Connected;
            }
			//msg = handle->Rec.Data.MsgHi << 7 | handle->Rec.Data.MsgLo;

		    Motor_Handle.SetMotorOffFlag = true;
			Motor_Handle.SetMotorOnFlag = false;
			cmdOverTimeCnt = 0;
			//====================test
			Comm_SetSetupType(SysOffRetType);//repeat directly
			//========================
			break;
	#endif
        /*** Set Cmd   ******/
	    case Comm_Cmd_SetMotorPower:
            handle->MotorPower = handle->Rec.Data.MsgHi << 7 | handle->Rec.Data.MsgLo;
            Comm_SetMotorStatus(handle, Comm_MotorState_On);  
		    Comm_SetupParamSave(handle);
			if(Motor_Handle.VoltageNeedSetFlag == 1)//需要设置电池电压
			{
				Comm_SetSetupType(VoltageNeedSetType);//需要设置电压
				ucAngle = 1;//handle->MotorPower& 0x7F;
				ucSteeringDrection = 0;//(handle->MotorPower >> 7) & 0x7F;
			}
			else
			{	
				Comm_SetSetupType(ReadMotorStatusType);//回复电机状态
				ucAngle = handle->MotorPower& 0x7F;
				ucSteeringDrection = (handle->MotorPower >> 7) & 0x7F;
			}	
			break;
			
		case Comm_Cmd_SetMotorStop:             //normal power is from 0-127, will stop if fall outside the range
            Comm_SetMotorStatus(handle, Comm_MotorState_Stop); 
            handle->MotorRpm = 0;
            handle->MotorPower = 0; 
            Comm_SetupParamSave(handle); 
			Comm_SetSetupType(ReadMotorStatusType);
			break;	
		//-------------------------------------------------pair------
	
		case  Comm_Cmd_PairRequest:
			 if(Gzll_ReadRssi() < PAIR_ENABLE_RSSI)
			 {
				 ControllerMcuID = handle->Rec.Data.MsgHi << 7 | handle->Rec.Data.MsgLo;
				 GZLL_Set_gzllAddr1(&GZLL_Handle,(ControllerMcuID + GZLL_Get_gzllAddr1(&GZLL_Handle)));
			     Comm_SetSetupType(ReadGzllEnable);
			 }			 
			break;
	    case  Comm_Cmd_GetGzllAddrH:
			Comm_SetSetupType(ReadGzllAddrHType);
			break;
	    case  Comm_Cmd_GetGzllAddrL:
			Comm_SetSetupType(ReadGzllAddrLType);
			break;
	    case  Comm_Cmd_GetGzllAddrSuc:
	   	    ReadAddrRecSucCount++;
		    if(ReadAddrRecSucCount>=2)
			{
				Gzll_NomalModeConfig(&GZLL_Handle);
			}				
			Comm_SetSetupType(ReadGzllFinish);
		    
			break;
	//---------------------------------------------------------------		
		//------------------------------------------------------setting------
		case Comm_Cmd_SetVoltage://设置电源电压
			handle->BatVoltage = handle->Rec.Data.MsgLo;
			UserSetBatteryVoltageNeeded = 1;
			ucVoltage = handle->BatVoltage;
			Comm_SetSetupType(SetVoltageType);
			break;
		case Comm_Cmd_SetNeutral://设置中位点
			handle->NeutralPosition = (handle->Rec.Data.MsgHi<<7)|handle->Rec.Data.MsgLo;
			SetBoatFrontNeeded = 1;
			Comm_SetSetupType(SetNeutralType);
			break;
		case Comm_Cmd_SetHandleSide://设置左右舷
			UserHandleSideSetFlag = 1;
			handle->HandleSide = (handle->Rec.Data.MsgHi<<7)|handle->Rec.Data.MsgLo;
			Comm_SetSetupType(SetHandleSideType);
			break;
		case Comm_Cmd_SetErrClear://清除故障
			ucClearFault = 100;
			Comm_SetSetupType(SetErrClearType);
			break;	
		//------------------------------------------------------			
		/***************no  use****************************/
		default:
			break;
	}
    
    handle->CommFailCnt = 0;
    handle->Rec.Data.CheckSum = 0;  //*******************
        
    Comm_PrepareRet(handle);
    Comm_Send(handle);    
}

void Comm_SetSetupType(SetupType_e  type)
{
	Comm_Handle.SetupType = type;
}

SetupType_e Comm_ReadSetupType(Comm_Handle_t* handle)
{
	return handle->SetupType;
}

void Comm_SetupParamSave(Comm_Handle_t* handle)
{
	if((handle->SetupType == ReadGzllFinish))
	{
		PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD);
		PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 	
		GZLL_Backup_gzllAddr1(&GZLL_Handle,pstorage_Wdata);
		Gzll_NomalModeConfig(&GZLL_Handle);
	}
}

uint8_t Comm_GetFailCount(Comm_Handle_t* handle)
{
	return handle->CommFailCnt;
}

static uint8_t Comm_CheckSum(uint8_t* data, uint8_t size)
{
	uint8_t i;
	uint8_t result = 0;
	
	for(i = 0;i<size;i++)
	{
		result = result ^ data[i];
	}
	return result;
}

uint16_t Comm_GetMotorStatus(Comm_Handle_t* Handle)
{
	return Handle->MotorState;  
}

void Comm_SetMotorStatus(Comm_Handle_t* Handle, Comm_MotorState_e Status)
{
	Handle->MotorState = Status;  
}

uint8_t Comm_GetThrottlePercentageVolt(Comm_Handle_t* Handle)
{
	return Handle->ThrottlePercentageVolt;  
}
void Comm_SetThrottlePercentageVolt(Comm_Handle_t* Handle, uint8_t Percentage)
{
	Handle->ThrottlePercentageVolt = Percentage > 100 ? 100 : Percentage;  
}

uint8_t Comm_GetThrottlePercentageTemp(Comm_Handle_t* Handle)
{
	return Handle->ThrottlePercentageTemp;  
}
void Comm_SetThrottlePercentageTemp(Comm_Handle_t* Handle, uint8_t Percentage)
{
	Handle->ThrottlePercentageTemp = Percentage > 100 ? 100 : Percentage;  
}

uint8_t Comm_GetThrottlePercentageBatt(Comm_Handle_t* Handle)
{
	return Handle->ThrottlePercentageBatt;  
}
void Comm_SetThrottlePercentageBatt(Comm_Handle_t* Handle, uint8_t Percentage)
{
	Handle->ThrottlePercentageBatt = Percentage > 100 ? 100 : Percentage;  
}

void Comm_CheckDisconnected(Comm_Handle_t* handle)
{
	if(handle->CommFailCnt>245)//245*20 =4900ms ??陆芒?枚?锚?????????赂?卯潞贸?氓?茫
	{
        handle->State = Comm_State_Error;
        handle->MotorState = Comm_MotorState_Off;
        handle->MotorRpm = 0;
		handle->MotorPower = 0;
        handle->Rec.Data.MsgHi = 0;
        handle->Rec.Data.MsgLo = 0;  
    }
	else
        handle->CommFailCnt++;    
}

void Comm_Send(Comm_Handle_t* handle)
{

    if(handle->Mode == Comm_Mode_Gzll)
	{
		CRYPT_UserData_TX(handle->Ret.Raw);	 
	}
	else if(handle->Mode == Comm_Mode_RS485)
	{ 
		handle->TxPtr = 0;
		RS485_SetTx();
		NRF_UART0->TXD = handle->Ret.Raw[handle->TxPtr++]; 
	}	
}

Comm_State_e Comm_GetState(Comm_Handle_t* handle)
{
    return handle->State;
}

Comm_Mode_e Comm_GetMode(Comm_Handle_t* handle)
{
    return handle->Mode;
}

void Comm_SetMode(Comm_Handle_t* handle, Comm_Mode_e Mode)
{
    handle->Mode = Mode;
}

//妫?娴?85?氫俊澶辫??
void  Remote485_CommFailCheck(void)
{
	if(Comm_Handle.Mode == Comm_Mode_RS485)
	{
		if(Comm_Handle.Remote485CommFailCount < DISCONNECT_CNT_MAX)
		{
			Comm_Handle.Remote485CommFailCount++;
		}
		else 
		{		
		    Comm_Handle.Remote485State = Comm_485RemoteState_Connected;
			Comm_Handle.State = Comm_State_Disconnected;
			Comm_Handle.MotorState = Comm_MotorState_Off;
			Comm_Handle.Mode = Comm_Mode_Gzll;
			Comm_Handle.MotorRpm = 0;
			Comm_Handle.MotorPower = 0;
			 Gzll_CommFailCountClear();//?氓鲁媒
		}
	}
	
}

void Comm_CheckMode(Comm_Handle_t* handle)
{
	 if(handle->Remote485State == Comm_485RemoteState_Connected)
	 	{
			Comm_Handle.Mode = Comm_Mode_Gzll;
		    Gzll_CommFailCountClear();
	 	}	 	
}
