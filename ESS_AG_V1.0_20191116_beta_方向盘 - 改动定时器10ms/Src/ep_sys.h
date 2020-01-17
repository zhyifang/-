#ifndef  _EP_SYS_H_
#define  _EP_SYS_H_

#include "nrf.h"
#include <stdbool.h>

typedef enum
{
	Err_None      			= 0,      //
	//Err_Mot         = 1,
	//Err_Comm_CA		= 2,          //COMM RELAY Communication Fault
	//Err_Comm_Motor			= 3,  //Motor Driver COMM fail
    Err_BatLow              = 4,
	Err_Cal					= 5,  	  //Throttle Calibration Required
	Err_MotorNeedVoltageSet = 6,
	Err_As6500              = 7,      //

	Err_MOTOR_OV,                     //8
	Err_MOTOR_OC,                     //9
	Err_MOTOR_UV,                     //10
	Err_MOTOR_Stall,                  //11
	Err_MOTOR_Sensor,                 //12
	Err_MOTOR_Comm,                   //13
	Err_MOTOR_OTWarn,                 //14
	Err_MOTOR_OT,                     //15
	Err_MOTOR_UV_Warm,                //16
}Err_code_e;
typedef enum
{
   sys_release_mode,//the release version
   sys_debug_mode,//can look vol ,cur......
   sys_debug_mode1,//not SYS_ErrCheck
}sys_mode_e;
typedef enum
{
	SYS_State_Off = 0,
	SYS_State_TurnOff,
	SYS_State_TurnOn,
	SYS_State_Normal,
	SYS_State_Error,
	SYS_State_Cal
}SYS_state_e;

typedef enum
{
	err_lock_state = 0,
	err_release_state
}err_state_e;

typedef enum
{
	sys_err_dis_on = 0,
	sys_err_dis_off
}Sys_Err_Dis_e;


typedef struct
{  
	uint8_t       DrvSysStarupCount;
	uint8_t       CommDisconCount;
    Err_code_e    err_code;
	SYS_state_e   sys_state;
	sys_mode_e    sys_mode;
	Sys_Err_Dis_e sys_err_dis_flag;
}SYS_Handle_t;
extern  SYS_Handle_t SYS_Handle;
void  SYS_Init(SYS_Handle_t* handle);
void  SYS_SetErrorCode(Err_code_e  error_code);
void  SYS_ErrCheck(SYS_Handle_t* handle);
void  SYS_SetSysNormal(SYS_Handle_t* handle);
bool  SYS_overtemp_warning(uint8_t temp,uint8_t temp_th);
SYS_state_e  SYS_ReadState(SYS_Handle_t* handle);
Err_code_e   SYS_ReadErrCode(SYS_Handle_t* handle);
void  SYS_DataInit(void);
void  SYS_CheckDebugMode(SYS_Handle_t* handle);
sys_mode_e  SYS_ReadMode(SYS_Handle_t* handle);

#endif

