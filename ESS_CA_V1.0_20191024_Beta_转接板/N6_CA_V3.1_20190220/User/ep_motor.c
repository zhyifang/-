#include "ep_motor.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "ep_gps.h"
#include "ep_battery.h"
#include "ep_temp.h"
#include "ep_tick.h"
#include "ep_protection.h"
#include "ep_pstorage.h"
#include "ep_version.h"

extern Motor_Handle_t Motor_Handle;

void Motor_Init(Motor_Handle_t* Handle)
{
	uint8_t i = 0;

	Handle->TransferIndex = 0;
	Handle->TxBufPtr = 0;
	Handle->RxBufPtr = 0;
	Handle->TransferSize = 0;
	Handle->CommFailCnt = 0;
	Handle->MotorObj.ErrorFlag = 0x0;
	Handle->MotorObj.status = Motor_Status_Off;
	Handle->SetMotorOnFlag  = false;
	Handle->SetMotorOffFlag  = false;
	Handle->MotorSysState  = true;
	
	for(i = 0; i < MOTOR_CMD_RET_LEN; i++)
	{
		Handle->Cmd.raw[i] = 0;
        Handle->Ret.raw[i] = 0;
	}
}

void Motor_SetCommFailCount(Motor_Handle_t* Handle, uint8_t count)
{
	Handle->CommFailCnt = count;
}

uint8_t Motor_GetVoltageNeedSetFlag(Motor_Handle_t* Handle)
{
	return Handle->VoltageNeedSetFlag;
}

void Motor_VoltageNeedSetFlag(Motor_Handle_t* Handle, uint16_t Flag)
{
	Handle->VoltageNeedSetFlag = Flag;
}

uint16_t Motor_GetErrorFlag(Motor_Handle_t* Handle)
{
	return Handle->MotorObj.ErrorFlag;
}

void Motor_SetErrorFlag(Motor_Handle_t* Handle, uint16_t Flag)
{
	Handle->MotorObj.ErrorFlag = Flag;
}

void Motor_ClearErrorFlag(Motor_Handle_t* Handle, uint16_t Flag)
{
	Handle->MotorObj.ErrorFlag &= ~Flag;
}

uint16_t Motor_GetStatus(Motor_Handle_t* Handle)
{
	return Handle->MotorObj.status;  
}

void Motor_SetStatus(Motor_Handle_t* Handle, Motor_Status_e Status)
{
	Handle->MotorObj.status = Status;  
}
void Motor_ClearFailCount(void)
{
	Motor_Handle.CommFailCnt = 0;
}

uint8_t Motor_GetFailCount(Motor_Handle_t* Handle)
{
	return Handle->CommFailCnt;
}

void Motor_CheckComm(void)
{
	if(Motor_Handle.CommFailCnt < 35)
	{
        Motor_Handle.CommFailCnt ++;
    }
	else
	{
        Motor_SetErrorFlag(&Motor_Handle, (Motor_Handle.MotorObj.ErrorFlag|ERR_COMM_FAIL_DETECTED_BIT));
        Motor_SetStatus(&Motor_Handle, Motor_Status_Off);
	}
}

