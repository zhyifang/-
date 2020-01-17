#ifndef __EP_Motor_H__
#define __EP_Motor_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "ep_comm.h"
#include "ep_misc.h"

#define SPI_STE_PIN		23
#define SPI_CLK_PIN		25
#define SPI_MOSI_PIN	22
#define SPI_MISO_PIN	24

#define MOTOR_CMD_RET_LEN			5
#define	ADDR_MOTOR					0xCA

/*  Motor_Ret_Status    */
#define ERR_UVwarn_BIT		        0x0001              //detected by msp430
#define ERR_MOT_OV_BIT			        0x0002
#define ERR_MOT_UV_BIT			        0x0004
#define ERR_MOT_OC_BIT			        0x0008
#define ERR_MOT_STALL_BIT		        0x0010
#define ERR_MOT_OT_BIT		        0x0020
#define ERR_MOT_OTW_BIT               0x0040 
#define ERR_MOT_SENSOR_BIT               0x0080 
#define ERR_COMM_FAIL_DETECTED_BIT		0x1000 


#define ERR_SYS_ENABLE_BIT		        0x2000
#define ERR_UPDATE_BIT		            0x8000

#define MOT_OV_TRIGGER          600
#define MOT_OV_RELEASE          580
#define MOT_UV_TRIGGER          390
#define MOT_UV_RELEASE          420

#define MOT_CCV_TRIGGER_UP          300
#define MOT_CCV_TRIGGER_DW          150

#define MOT_SHUT_VOL_TRIGGER	300

typedef enum
{
	#if 0
	Motor_Cmd_Stop 					= 0x40,
	Motor_Cmd_SetRpm 				= 0x41,
	Motor_Cmd_SetPwr 				= 0x42,
	Motor_Cmd_SetCurP 				= 0x48,
	Motor_Cmd_SetCurI 				= 0x49,
	Motor_Cmd_SetSpdP 				= 0x4A,
	Motor_Cmd_SetSpdI 				= 0x4B,
	Motor_Cmd_SetPwrP 				= 0x4C,
	Motor_Cmd_SetPwrI 				= 0x4D,
	Motor_Cmd_SetSpdRmp 			= 0x50,
	Motor_Cmd_SetPwrRmp 			= 0x51,
	Motor_Cmd_SetMotOTWarning 		= 0x58,
	Motor_Cmd_SetMotOTStop 			= 0x59,
	Motor_Cmd_SetMosOTWarning 		= 0x5A,
	Motor_Cmd_SetMosOTStop 			= 0x5B,
	Motor_Cmd_SetMaxRpm 			= 0x60,
	Motor_Cmd_SetMaxCur 			= 0x61,
	Motor_Cmd_Enable 				= 0x68,
	Motor_Cmd_Disable 				= 0x69,
    MOTOR_Cmd_StartCal			    = 0x70,
	MOTOR_Cmd_SetCal1			    = 0x71,
	MOTOR_Cmd_SetCal2			    = 0x72,
	MOTOR_Cmd_SetCalp			    = 0x73
	#endif
	Motor_Cmd_SetMotorPower					= 0x70,
	Motor_Cmd_SetMotorOriginal 				= 0x71,
	Motor_Cmd_SetNeutral 				= 0x72,
	Motor_Cmd_SetVoltage 				= 0x73,
}Motor_Cmd_e;

typedef enum
{
#if 0
	Motor_Ret_Status				= 0x00,
	Motor_Ret_Temp					= 0x08,
	Motor_Ret_Id					= 0x10,
	Motor_Ret_Iq					= 0x11,
	Motor_Ret_Iu					= 0x12,
	Motor_Ret_Iv					= 0x13,
	Motor_Ret_Iw					= 0x14,
	Motor_Ret_Ud					= 0x18,
	Motor_Ret_Uq					= 0x19,
	Motor_Ret_Uu					= 0x1A,
	Motor_Ret_Uv					= 0x1B,
	Motor_Ret_Uw					= 0x1C,
	Motor_Ret_Ubus					= 0x1D,
	Motor_Ret_Ibus					= 0x1E,
	Motor_Ret_Rpm					= 0x20,
	Motor_Ret_Pow					= 0x21,	
    MOTOR_Ret_Cal1				    = 0x30,
	MOTOR_Ret_Cal2				    = 0x31,
	MOTOR_Ret_Calp				    = 0x32,
	MOTOR_Ret_HS_Ver 			    = 0x33,     // Hardware and software ver 
	MOTOR_Ret_SwDate 			    = 0x34,     // soft Date
	MOTOR_Ret_PM 			        = 0x35,     // product module 
#endif
	Motor_Ret_MotorStatus				= 0x50,
	MOTOR_Ret_MotorOriginal 			        = 0x51,
	MOTOR_Ret_CommStart 			        = 0x52,
	MOTOR_Ret_SetNeutralState 			        = 0x53,
}Motor_Ret_e;

typedef struct _Motor_Cmd_
{
	uint8_t 			addr;			//addr
	Motor_Cmd_e			cmd;
	uint8_t 			msgHi;
	uint8_t 			msgLo;
	uint8_t 			sum;
}Motor_Cmd_t;

typedef struct _Motor_Ret_
{
	uint8_t 		addr;			//acknowledge the cmd
	Motor_Ret_e 	type;
	uint8_t	 		msgHi;
	uint8_t 		msgLo;
	uint8_t 		sum;
}Motor_Ret_t;

typedef union _CTRL_Motor_Cmd_
{
	Motor_Cmd_t data;
	uint8_t   raw[MOTOR_CMD_RET_LEN];
}Motor_Cmd_u;

typedef union _CTRL_Motor_Ret_
{
	Motor_Ret_t data;
	uint8_t   raw[MOTOR_CMD_RET_LEN];
}Motor_Ret_u;

typedef enum 
{
	Motor_Status_Off,
	Motor_Status_Stop,
	Motor_Status_On
}Motor_Status_e;

typedef struct 
{
	uint16_t 					ErrorFlag;
	Motor_Status_e  			status;   			//0 : stop, 1 : run
	#if 0
	int16_t  					rpm;				//actual rpm
	uint16_t 					power;              // unit: 1W, from 0-6000    
	uint16_t					ubus;               // unit: 0.1V   
	uint16_t					ibus;               // unit: 0.1A    
	int16_t 					uq;
	int16_t 					iq;
	int16_t 					iu;
	int16_t 					iv;
	int16_t 					iw;
    int16_t                     UbusDiff;
    uint16_t                    CmdValue;
    Motor_CalValue_t            Cal;
	uint16_t                    temp;
	#endif
}Motor_Obj;

typedef struct
{	
	uint8_t								TransferIndex;
	uint8_t*							TxBufPtr;
	uint8_t*							RxBufPtr;
	uint8_t								TransferSize;
	Motor_Cmd_u 						Cmd;
	Motor_Ret_u 						Ret;
	uint8_t  							CommFailCnt;		//+1 for every transmission and cleared at each decode	   
	Motor_Obj							MotorObj;
	uint8_t 							VoltageNeedSetFlag;
	uint8_t								SetMotorOnFlag;
	uint8_t								SetMotorOffFlag;
	uint8_t 							MotorSysState;
	uint16_t 							MotorPosition2Steering;
	uint16_t 							MotorPosition2Motor;
}Motor_Handle_t;

void Motor_Init(Motor_Handle_t* Handle);

uint8_t* Motor_PrepareCmd(Motor_Handle_t* Handle, Motor_Cmd_e cmd, volatile uint16_t msg);                       //prepare command for tx
void Motor_Transfer(Motor_Handle_t* Handle);                                                            //trigger the spi transmission
void Motor_RxDecode(Motor_Handle_t* Handle);                                                            //decode the received msg

bool Motor_CheckCalValue(Motor_Handle_t* Handle, uint8_t* data);
void Motor_BackupCalValue(Motor_Handle_t* Handle,uint8_t* data);
uint16_t Motor_GetCal(Motor_Handle_t* Handle, uint8_t index);

int16_t Motor_GetUq(Motor_Handle_t* Handle);
int16_t Motor_GetIq(Motor_Handle_t* Handle);

uint16_t Motor_GetUBus(Motor_Handle_t* Handle);
uint16_t Motor_GetIBus(Motor_Handle_t* Handle);
int16_t Motor_GetUbusDiff(Motor_Handle_t* Handle);

uint16_t Motor_GetPower(Motor_Handle_t* Handle);
uint16_t Motor_GetRPM(Motor_Handle_t* Handle);

uint16_t Motor_GetErrorFlag(Motor_Handle_t* Handle);
void Motor_SetErrorFlag(Motor_Handle_t* Handle, uint16_t Flag);
void Motor_ClearErrorFlag(Motor_Handle_t* Handle, uint16_t Flag);

uint16_t Motor_GetStatus(Motor_Handle_t* Handle);
void Motor_SetStatus(Motor_Handle_t* Handle, Motor_Status_e state);

uint32_t Motor_GetLastUpdateTime(Motor_Handle_t* Handle);
void Motor_SetLastUpdateTime(Motor_Handle_t* Handle, uint32_t Time);

uint8_t Motor_GetFailCount(Motor_Handle_t* Handle);
uint8_t Motor_GetFailCountCummulated(Motor_Handle_t* Handle);

uint16_t  Motor_ReadMotTemp(void);
void Motor_SetMotTemp(uint16_t temp);

uint8_t Motor_GetVoltageNeedSetFlag(Motor_Handle_t* Handle);
void Motor_VoltageNeedSetFlag(Motor_Handle_t* Handle, uint16_t Flag);

void Motor_ClearFailCount(void);
void Motor_CheckComm(void);

extern Motor_Handle_t Motor_Handle;


#endif /*__EP_Motor_H__*/

