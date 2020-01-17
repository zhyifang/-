#ifndef __EP_COMM_H_
#define __EP_COMM_H_

#include <stdint.h>
#include "nrf_gpio.h"

//#define RS485_TX_PIN	9
//#define RS485_RE_DE_PIN	10
//#define RS485_RX_PIN	11

#define	ADDR_BRIDGESYSTEM			0xE5
#define MAINSYSTEM_CMD_LEN			5
#define MAINSYSTEM_RET_LEN			5

#define COMM_DEBUG_MODE	1

#define DISCONNECT_CNT_MAX          100 //100*30MS = 3S

typedef enum
{
	ReadMotorStatusType,
	ReadGzllEnable,
	ReadGzllAddrHType,
	ReadGzllAddrLType,
	ReadGzllFinish,

	VoltageNeedSetType,
	SetVoltageType,
	SetNeutralType,
	SetHandleSideType,
	SetErrClearType,

	SysOnRetType,
	SysOffRetType,
	
}SetupType_e;

typedef enum
{
	Comm_Cmd_PairRequest                = 1,   //wireless pair request
	Comm_Cmd_GetGzllAddrH               = 2,   //get the wireless address high 14bits
	Comm_Cmd_GetGzllAddrL               = 3,   //get the wireless address low 14bits
	Comm_Cmd_GetGzllAddrSuc             = 4,   //get the wireless address  sucess
	
	Comm_Cmd_SetMotorPower              = 5,
	Comm_Cmd_SetMotorOriginal           = 6,
	Comm_Cmd_SetMotorStop			= 7,

	Comm_Cmd_SetNeutral               = 8,//设置中位点
	Comm_Cmd_SetVoltage              = 9,//设置电压
	Comm_Cmd_SetHandleSide             = 10,
		Comm_Cmd_SetErrClear             = 11,

	Comm_Cmd_SetSysOn,
	Comm_Cmd_SetSysOff,
}Comm_Cmd_e;

typedef enum
{
	Comm_Ret_NULL                       = 1, //null 
	Comm_Ret_MotorStatus                = 2,	//motor status	

	Comm_Ret_PairEnable                 = 3, //pair enable
	Comm_Ret_GzllAddrH                  = 4, //the wireless address high 14bits
	Comm_Ret_GzllAddrL                  = 5, //the wireless address low 14bits
	Comm_Ret_ReadGzllAdrRecSuc          = 6, //the wireless address read success
	Comm_RetMotorOriginal			= 7,//返回电机原点校准结果

	Comm_RecMotorNeutralSuc			 = 8,
	Comm_RecMotorVolSuc 		  = 9,//设置电压成功
	Comm_RetVoltageNeedSet		  = 10,
	Comm_RetHandleSideSet		  = 11,
	Comm_RetErrClear,

		Comm_Ret_CrSysOn,
	Comm_Ret_CrSysOff,

	Comm_Ret_MotorPosition,
	Comm_Ret_MotorRst,
}Comm_Ret_e;

typedef struct
{
	uint8_t 			Addr;			//addr
	Comm_Cmd_e		    RequestedType;
	uint8_t 			MsgHi;
	uint8_t 			MsgLo;
	uint8_t 			CheckSum;
}Comm_Rec_t;

typedef struct
{	
	uint8_t 			Addr;			//acknowledge the cmd
	Comm_Ret_e 		    ReturnType;
	uint8_t	 			MsgHi;
	uint8_t 			MsgLo;
	uint8_t 			CheckSum;
}Comm_Ret_t;
//received  Msg

typedef union 
{
	Comm_Rec_t Data;
	uint8_t Raw[MAINSYSTEM_CMD_LEN];
}Comm_Rec_u;

//returning Msg
typedef union 
{
	Comm_Ret_t Data;
	uint8_t Raw[MAINSYSTEM_RET_LEN];
}Comm_Ret_u;

typedef enum 
{
	Comm_State_Disconnected,
	Comm_State_Connected,
	Comm_State_Error,
}Comm_State_e;

typedef enum 
{
	Comm_Mode_RS485,
	Comm_Mode_Gzll,
}Comm_Mode_e;

typedef enum 
{
	Comm_MotorState_On,
	Comm_MotorState_Off,
	Comm_MotorState_Stop,
}Comm_MotorState_e;

typedef enum 
{
	Comm_485RemoteState_Connected,
	Comm_485RemoteState_Disconnected,
}Comm_485RemoteState_e;


typedef struct
{	
	uint8_t								TxPtr;
	uint8_t								RxPtr;
	Comm_State_e 						State;	
	Comm_MotorState_e				    MotorState;
	Comm_485RemoteState_e				Remote485State;
	uint8_t                             Remote485CommFailCount;
	uint8_t  							CommFailCnt;		//+1 for every transmission and cleared at each decode	
	Comm_Rec_u 							Rec;                //Rec  Msg From handle MCU
	Comm_Ret_u 							Ret;                //return Msg to handle MCU
	Comm_Mode_e          				Mode;  
    uint8_t                             ThrottlePercentageVolt;
    uint8_t                             ThrottlePercentageTemp;
    uint8_t                             ThrottlePercentageBatt;
	uint8_t  							MotorRpm;						
	uint8_t 							MotorPower;			
	uint8_t 							RetCmdIndex;
	uint8_t 							BatteryOnOff;
	uint16_t                            gzll_addr1[2];
	SetupType_e                         SetupType;

	 uint8_t                            BatVoltage;
	 uint8_t                            HandleSide;
	 uint16_t 							NeutralPosition;
}Comm_Handle_t;

extern Comm_Handle_t Comm_Handle;

void Comm_SetupParamSave(Comm_Handle_t* handle);
void Comm_SetSetupType(SetupType_e  type);
SetupType_e Comm_ReadSetupType(Comm_Handle_t* handle);
void Comm_Init(Comm_Handle_t* handle);
void Comm_Send(Comm_Handle_t* handle);
void Comm_Decode(Comm_Handle_t* handle);
uint8_t Comm_GetFailCount(Comm_Handle_t* handle);
void Comm_CheckDisconnected(Comm_Handle_t* handle);

Comm_Mode_e Comm_GetMode(Comm_Handle_t* handle);
void Comm_SetMode(Comm_Handle_t* handle, Comm_Mode_e Mode);
Comm_State_e Comm_GetState(Comm_Handle_t* handle);
uint16_t Comm_GetMotorStatus(Comm_Handle_t* Handle);

uint8_t Comm_GetThrottlePercentageVolt(Comm_Handle_t* Handle);
void Comm_SetThrottlePercentageVolt(Comm_Handle_t* Handle, uint8_t Percentage);

uint8_t Comm_GetThrottlePercentageTemp(Comm_Handle_t* Handle);
void Comm_SetThrottlePercentageTemp(Comm_Handle_t* Handle, uint8_t Percentage);

uint8_t Comm_GetThrottlePercentageBatt(Comm_Handle_t* Handle);
void Comm_SetThrottlePercentageBatt(Comm_Handle_t* Handle, uint8_t Percentage);
    
void Comm_SetMotorStatus(Comm_Handle_t* Handle, Comm_MotorState_e Status);

void Comm_CheckMode(Comm_Handle_t* handle);
void  Remote485_CommFailCheck(void);

#endif  /*__EP_COMM_H_*/
