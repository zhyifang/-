/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  D4escription*****
1.  Date:```
    Author:
    Modification:

*  
*************************************/
#include "ep_TimeRecord.h"

#include "nrf.h"
#include "nrf51_bitfields.h"
#include "nrf51.h"
#include "ep_throttle.h"
#include "ep_pstorage.h"
#include "ep_motor.h"
#include "ep_motor.h"
#include "stdbool.h"
#include "ep_version.h"



TimeRecord_Handle_t TimeRecord_Handle;



uint16_t Mos1TempArr[TEMP_ARR_LEN_MAX];
uint16_t Mos2TempArr[TEMP_ARR_LEN_MAX];
uint16_t MotTempArr[TEMP_ARR_LEN_MAX];
uint8_t ArrReadIndex = 0;
bool    ReadTempArrFlag = false;

void TimeRecord_ClearTempArr(void);

void TimeRecord_Init(TimeRecord_Handle_t* handle)
{
	handle->ForwardDay       = 0;
	handle->ForwardHour      = 0;
	handle->ForwardMinute    = 0;
		
	handle->BackwardDay      = 0;
	handle->BackwardHour     = 0;
	handle->BackwardMinute   = 0;
	handle->RunTime          = 0;
}

void TimeRecord_TimeAdd(TimeRecord_Handle_t* handle)
{
	static uint16_t TimeCount = 0;
	static uint16_t pre_RunTime = 0;
		
	if(THROTTLE_Handle.percent != 0)
	{
		TimeCount++;// 180ms ++ 
		if(THROTTLE_Handle.dir)//forward 
		{
			if(TimeCount > 333) // 60s * 10000/180 = 333 
			{
				TimeCount = 0;
				handle->ForwardMinute++;
				if(MOTOR_ReadRpm(&MOTOR_Handle) > 100)
				{						
					handle->RunTime++;
					if(pre_RunTime != handle->RunTime)  ReadTempArrFlag = false;
					pre_RunTime = handle->RunTime;
					if((handle->RunTime % RECORD_TERMP_INTERVAL_SEC) == 0)
					{
						uint8_t index = 0;
						index = handle->RunTime / RECORD_TERMP_INTERVAL_SEC;
						Mos1TempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMosPower);
						Mos2TempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMosDriver);
						MotTempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMot);
					}
				}
			
				if(handle->ForwardMinute > 59)
				{
					handle->ForwardMinute = 0;
					handle->ForwardHour++;
					if(handle->ForwardHour > 23)
					{
						handle->ForwardHour = 0;
						handle->ForwardDay++;
					}
				}
				
			}
			
		}
		else//backward
		{
			if(TimeCount > 333) // 60s * 10000/180 = 333 
			{
				TimeCount = 0;
				handle->BackwardMinute++;
				if(MOTOR_ReadRpm(&MOTOR_Handle) > 100)
				{
					handle->RunTime++;
					if((handle->RunTime % 5) == 0)
					{
						uint8_t index = 0;
						index = handle->RunTime / 5;
						Mos1TempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMosPower);
						Mos2TempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMosDriver);
						MotTempArr[index] = MOTOR_ReadTempVal(&MOTOR_Handle.tMot);
					}
				}
				if(handle->BackwardMinute > 59)
				{
					handle->BackwardMinute = 0;
					handle->BackwardHour++;
					if(handle->BackwardHour > 23)
					{
						handle->BackwardHour = 0;
						handle->BackwardDay++;
					}
				}
				
			}
		}
	}
	
}
void TimeRecord_TimeClear(TimeRecord_Handle_t* handle)
{
	handle->ForwardDay       = 0;
	handle->ForwardHour      = 0;
	handle->ForwardMinute    = 0;
		
	handle->BackwardDay      = 0;
	handle->BackwardHour     = 0;
	handle->BackwardMinute   = 0;
}

void TimeRecord_Backup(TimeRecord_Handle_t* handle,uint8_t* data)
{
	*(data+0) = (uint8_t) ( (handle->ForwardDay >> 8) & 0xFF );   
	*(data+1) = (uint8_t) ( handle->ForwardDay & 0xFF ); 
	*(data+2) = handle->ForwardHour; 
	*(data+3) = handle->ForwardMinute; 
	
	*(data+4) = (uint8_t) ( (handle->BackwardDay >> 8) & 0xFF );   
	*(data+5) = (uint8_t) ( handle->BackwardDay & 0xFF ); 
	*(data+6) = handle->BackwardHour; 
	*(data+7) = handle->BackwardMinute; 
	
	*(data+8) = handle_msg.product_model; 
	
    *(data+9) = PSTORAGE_CheckSum(data, 9);	
}

bool TimeRecord_Update(TimeRecord_Handle_t* handle,uint8_t* data)
{
	if(data[9] == PSTORAGE_CheckSum(data, 9)) 
	{
	   handle->ForwardDay    =  (uint16_t)( *(data) << 8 ) | (uint16_t)( *(data+1));
	   handle->ForwardHour   = *(data+2); 
	   handle->ForwardMinute = *(data+3); 
		
	   handle->BackwardDay    =  (uint16_t)( *(data+4) << 8 ) | (uint16_t)( *(data+5));
	   handle->BackwardHour   = *(data+6); 
	   handle->BackwardMinute = *(data+7);
		
	   if((*(data+8) != product_navy_60) && (*(data+8) != product_navy_30))
	   {
			handle_msg.product_model = product_navy_60;
	   }
	   else
	   {
			handle_msg.product_model =(product_model_e)( *(data+8));
	   }
	  
		
	   return  true;
	}
	else
	{
		handle_msg.product_model = product_navy_60;
	}
	return  false;
}
uint16_t  TimeRecord_ReadRunTime(void)
{
	return  TimeRecord_Handle.RunTime;
}
void  TimeRecord_ClearRunTime(void)
{
	  TimeRecord_Handle.RunTime = 0;
	  TimeRecord_ClearTempArr();
}
void TimeRecord_AddArrIndex(void)
{
	 if(ArrReadIndex < (TimeRecord_Handle.RunTime / RECORD_TERMP_INTERVAL_SEC))
	 {
		ArrReadIndex++;
		if(ArrReadIndex == TEMP_ARR_LEN_MAX)
		{
			ArrReadIndex = 0;
		}
		ReadTempArrFlag = true;
	 }
	 else 
	 {
		ReadTempArrFlag = false;
		ArrReadIndex = 0;
	 }
	 
}
bool TimeRecord_ReadTempArrFlag(void)
{
	return ReadTempArrFlag;	 
}

void TimeRecord_ClearTempArr(void)
{
	for(uint8_t i  = 0; i < TEMP_ARR_LEN_MAX;i++)
	{
		Mos1TempArr[i] = 0;
		Mos2TempArr[i] = 0;
		MotTempArr[i] = 0;
		ArrReadIndex = 0;
		ReadTempArrFlag = false;
	}
}
