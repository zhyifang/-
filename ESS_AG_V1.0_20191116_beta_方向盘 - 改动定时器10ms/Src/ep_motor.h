#ifndef     _EP_MOTOR_H
#define     _EP_MOTOR_H


#include "nrf.h"
#include <stdbool.h>
#include  "ep_comm.h"
#include  "main.h"
 
 

#define  NAVY30_TMOT_THRESHOLD         (104*10)  
#define  NAVY30_TMOS_THRESHOLD         (104*10) 

#define  NAVY60_TMOT_THRESHOLD         (119*10) 
#define  NAVY60_TMOS_THRESHOLD         (104*10) 

#define ERR_UVwarn_BIT		        0x0001              //detected by msp430
#define ERR_MOT_OV_BIT			        0x0002
#define ERR_MOT_UV_BIT			        0x0004
#define ERR_MOT_OC_BIT			        0x0008
#define ERR_MOT_STALL_BIT		        0x0010
#define ERR_MOT_OT_BIT		        0x0020
#define ERR_MOT_OTW_BIT               0x0040 
#define ERR_MOT_SENSOR_BIT               0x0080 
#define ERR_COMM_FAIL_DETECTED_BIT		0x1000              //detected by myself

typedef enum 
{
	MOTOR_State_Off,
	MOTOR_State_Stop,
	MOTOR_State_On
}MOTOR_State_e;

typedef enum 
{
	temp_hight_state,
	temp_nomal_state

}Temp_State_e;

typedef enum 
{  
	motor_nomal,
	motor_comm_fail,
	motor_over_vol,
	motor_under_vol,
	motor_over_cur,
	motor_over_temp_warn,
	motor_stall,
	motor_over_temp,
	Motor_Sensor,
	motor_under_voltage_warm,
}MOTOR_Status_e;

typedef enum 
{  
    MotorCalStart,
	MotorCalZeroLoc,
	MotorCalMaxLoc,
	MotorCalFinish,
	MotorCalStop,
	MotorCalQuit
	
}Cal_Status_e;
typedef enum 
{  
	Motor_Need_Cal,
	Motor_NoNeed_Cal
}Motor_Need_Cal_e;
typedef  enum
{
    temp_nomal,
    temp_over,	
	temp_damage//if temp value is -500,the temp sensor is damageing 
	
}temp_state_e;
typedef  struct
{
    uint16_t         value;
	temp_state_e     temp_state;
	
}temp_t;
typedef  struct
{  
	//uint16_t 				MotorPosition;
	uint8_t  			 	commFailCnt;		//+1 for every transmission and cleared at each receive
	uint16_t                MotorPowerLimit;
	MOTOR_State_e  		    state;   			//0 : stop, 1 : run
	uint16_t                status;
	MOTOR_Status_e          motor_status;

	uint8_t 				VoltageNeedSetFlag;

}MOTOR_Handle_t;

extern  MOTOR_Handle_t  MOTOR_Handle;
void  Motor_SetCalNeedState(Motor_Need_Cal_e state);
void  Motor_SetCalNeed(MOTOR_Handle_t* handle);
void  Motor_SetTempState(Temp_State_e state);
Temp_State_e Motor_ReadTempState(MOTOR_Handle_t* handle);
void  MOTOR_Stop(void);
void  MOTOR_On(void);
void  MOTOR_Off(void);
void  MOTOR_CheckTempState(temp_t* temp,uint16_t threshold,uint16_t status);
temp_state_e  MOTOR_ReadTempState(temp_t* temp);
void  MOTOR_comm_RX_decode(MOTOR_Handle_t* handle,Comm_Handle_t* comm_handle);
void  MOTOR_CheckStatus(MOTOR_Handle_t* handle,uint16_t status);
MOTOR_Status_e 	MOTOR_ReadMotorStatus(MOTOR_Handle_t* handle);
void  MOTOR_Init(MOTOR_Handle_t*  handle);
void  MOTOR_ClearUvCount(MOTOR_Handle_t*  handle);
uint16_t  MOTOR_ReadTempVal(temp_t* temp);
uint16_t  MOTOR_ReadRpm(MOTOR_Handle_t* handle);
uint16_t  MOTOR_ReadCheckNoTempCount(MOTOR_Handle_t* handle);
uint16_t  MOTOR_ReadPower(MOTOR_Handle_t* handle);
void  Motor_SetMotStatus(MOTOR_Status_e status);
void  Motor_SetCalStatus(Cal_Status_e status);
void  MOTOR_CheckCommFail(MOTOR_Handle_t* handle,uint8_t status);
bool Motor_ReadCanRunFlag(void);
void Motor_SetCanRunFlag(bool flag);
void MOTOR_SetStatus(uint16_t status);
void Motor_CheckStopTemp(void);
uint8_t  MOTOR_ReadMotorVoltageNeedSetFlag(MOTOR_Handle_t* handle);

#endif 
