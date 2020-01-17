#ifndef __EP_COMM_H_
#define __EP_COMM_H_

#include <stdint.h>
#include "nrf_gpio.h"
#include "ep_throttle.h"

#define RS485_TX_PIN	            8
#define RS485_RE_DE_PIN	            9
#define RS485_RX_PIN	            10
#define RS485_CHECK_PIN             6


#define	ADDR_MAIN_SYSTEM		    0xE5
#define MAINSYSTEM_CMD_LEN			5
#define MAINSYSTEM_RET_LEN			5

#define NeedSendCmdCountMax         9 //the max count of the cmd need to be send  when reconnect with the commrelay board

#define COMM_DEBUG_MODE	            1

#define DISCONNECT_CNT_MAX          100 //100*30MS = 3S

typedef enum
{		
	Comm_Cmd_PairRequest                = 1,   //wireless pair request
	Comm_Cmd_GetGzllAddrH               = 2,   //get the wireless address high 14bits
	Comm_Cmd_GetGzllAddrL               = 3,   //get the wireless address low 14bits
	Comm_Cmd_GetGzllAddrSuc             = 4,   //get the wireless address  sucess
	
	Comm_Cmd_SetMotorPower              = 5,
	Comm_Cmd_SetMotorOriginal           = 6,
	Comm_Cmd_SetMotorStop			    = 7,

	Comm_Cmd_SetNeutral                 = 8,   //设置中位点
	Comm_Cmd_SetVoltage                 = 9,   //设置电压
	Comm_Cmd_SetHandleSide              = 10,
	Comm_Cmd_SetErrClear                = 11,

	Comm_Cmd_SetSysOn,
	Comm_Cmd_SetSysOff,


}Comm_Cmd_e;

typedef enum
{
	Comm_Ret_NULL                       = 1,  //null 
	Comm_Ret_MotorStatus                = 2,  //motor status	

	Comm_Ret_PairEnable                 = 3,  //pair enable
	Comm_Ret_GzllAddrH                  = 4,  //the wireless address high 14bits
	Comm_Ret_GzllAddrL                  = 5,  //the wireless address low 14bits
	Comm_Ret_ReadGzllAdrRecSuc          = 6,  //the wireless address read success
	
	Comm_Ret_MotorOriginal			    = 7,  //返回电机原点校准结果
	Comm_Ret_MotorNeutralSuc			= 8,
	Comm_Ret_MotorVolSuc 		        = 9,  //设置电压成功
	Comm_Ret_VoltageNeedSet		        = 10,
	Comm_Ret_HandleSideSet		        = 11,
	Comm_Ret_ErrClear                   = 12,

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
	
}Comm_Cmd_t;
typedef union 
{
	Comm_Cmd_t Data;
	uint8_t Raw[MAINSYSTEM_CMD_LEN];
}Comm_Cmd_u;
typedef struct
{	
	uint8_t 			Addr;			//acknowledge the cmd
	Comm_Ret_e 		    ReturnType;
	uint8_t	 			MsgHi;
	uint8_t 			MsgLo;
	uint8_t 			CheckSum;
}Comm_Rec_t;
typedef union 
{
	Comm_Rec_t Data;
	uint8_t Raw[MAINSYSTEM_CMD_LEN];
}Comm_Rec_u;
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
	NullType,
	SysOnType,
	SysOffType,
    ThrottleValSetType,       //set power  Throttle Value
	GzllPairRequestType,      //request the pair 
	GzllAddr1HReadType,       //read the gzll address high 14bits
	GzllAddr1LReadType,       //read the gzll address low 14bits
	GzllAddrReadSucType,      //read the gzll address address

	NeutralSetType,
	MotorCalSetType,
	HandleSideSetType,
	NeutralConformType,
	ErrorClearType,
}Comm_Data_Type_e;

#if 0
typedef struct
{
	//if the value == 1,it mean the corresponding  cmd need to be send 
	//if the value == 0,it mean the corresponding cmd had been send 
	uint8_t SysOpenTypeBit                  :1;	
	uint8_t BatTypeCapSetTypeBit            :1;
	uint8_t BatVolSetTypeBit                :1;
	
	uint8_t ReadDrvHSverTypeBit             :1;
	uint8_t ReadDrvSwDateTypeBit            :1;
	uint8_t ReadDrvPMTypeBit                :1;
		
	uint8_t ReadCommrelayHSverTypeBit       :1;
	uint8_t ReadCommrelaySwDateTypeBit      :1;
	uint8_t ReadCommrelayPMTypeBit          :1;

}Comm_NeedSendCmd_Type_t;

typedef union
{
	Comm_NeedSendCmd_Type_t    NeedSendCmd_Type;
	uint8_t                    NeedSendCmdBuff[NeedSendCmdCountMax / 8 + 1];
}Comm_NeedSendCmd_Type_u;
#endif

typedef struct
{	
	uint8_t				     TxPtr;
	uint8_t				     RxPtr;
	uint8_t  			     CommFailCnt;		//+1 for every transmission and cleared at each decode	
	uint8_t                  Crypt_key;  
		
	uint16_t                 tx_msg;
	Comm_State_e 		     State;
    Comm_State_e	         Pre_State;
	Comm_Cmd_u 			     Cmd;
	Comm_Cmd_e               Cmd_type;
	Comm_Rec_u 			     Rec;
	
	Comm_Data_Type_e         Data_Type;
	//Comm_NeedSendCmd_Type_u  NeedSendCmd_Type;
	//uint8_t                  NeedCheckSendCmdFlag;
	
	volatile Comm_Mode_e     Mode;   

    bool                     ReadAddrEnable;
}Comm_Handle_t;

extern Comm_Handle_t Comm_Handle;

extern  void  Comm_RS485_Check(Comm_Handle_t* handle);
Comm_State_e  Comm_ReadState(void);
Comm_Mode_e   Comm_Read_Mode(void);
extern  void  Comm_Init(Comm_Handle_t* handle);
static  void RS485_Init(void);
extern  uint8_t* Comm_PrepareTxData(Comm_Handle_t* handle);
void    Comm_SecKey_Set(uint16_t secKey);
extern  void Comm_TxDatSet(Comm_Handle_t* handle, Comm_Cmd_e cmd_type,uint16_t com_msg);
extern  void Comm_Decode(Comm_Handle_t* handle);
void  Comm_UpdateSendData(void);
void  Comm_DataTypeSet(Comm_Data_Type_e  DataType);
Comm_Data_Type_e  Comm_DataTypeRead(Comm_Handle_t* handle);
static  void RS485_Disable(void);
uint8_t Comm_CheckSum(uint8_t* data, uint8_t size);
uint8_t Comm_GetFailCount(Comm_Handle_t* handle);
//static  inline void RS485_SetRx(void); 
//static  inline void RS485_SetTx(void); 
uint8_t* Comm_PrepareCmd(Comm_Handle_t* handle, Comm_Cmd_e cmd, uint16_t msg);
extern void Comm_Send(Comm_Handle_t* handle);
void RS485_Init(void);
void RS485_Disable(void);
void Comm_ClearFailCnt(Comm_Handle_t* handle);
void Comm_RS485ModeSend(Comm_Handle_t* handle);
void Comm_GzllModeSend(Comm_Handle_t* handle);
void Comm_DataTypeSetThro(Comm_Handle_t* handle);
void Comm_ReadAddrCheck(Comm_Handle_t* handle);

void Comm_InitNeedSendCmdBuff(Comm_Handle_t *handle);
void Comm_CheckNeedSendCmd(Comm_Handle_t *handle);

void SetCommRx(void);
void SetCommTx(void);

#endif  /*__EP_COMM_H_*/
