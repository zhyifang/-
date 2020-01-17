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
#include "ep_throttle.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_adc.h"
#include "pstorage.h"
#include "ep_lcd_6kw.h"
#include "ep_sys.h"
#include "ep_pstorage.h"
#include "ep_motor.h"
#include "ep_version.h"
#include "ep_led.h" 

//int32_t thetaAng;
uint8_t start_flag = 0;


//0-45 tan0 = 0  tan45 = 1
//table value is arctan[(0-1)/256] * 1024
//arctan[1/256] * 1024 = 0.223 * 1024 = 229
//arctan[2/256] * 1024 = 0.447 * 1024 = 458
//
#if 0
static const uint16_t tanTable[256] = {
	0, 	   229,   458,   687,   916,   1145,  1374,  1603,
	1832,  2061,  2290,  2519,  2748,  2976,  3205,  3433,
	3662,  3890,  4118,  4346,  4574,  4802,  5029,  5257,
	5484,  5711,  5938,  6165,  6391,  6618,  6844,  7070,
	7296,  7521,  7746,  7971,  8196,  8421,  8645,  8869,
	9093,  9317,  9540,  9763,  9986,  10208, 10431, 10652,
	10874, 11095, 11316, 11537, 11757, 11977, 12197, 12416,
	12635, 12853, 13071, 13289, 13507, 13724, 13940, 14157,
	14373, 14588, 14803, 15018, 15232, 15446, 15660, 15873,
	16085, 16297, 16509, 16720, 16931, 17142, 17352, 17561,
	17770, 17979, 18187, 18394, 18601, 18808, 19014, 19220,
	19425, 19630, 19834, 20038, 20241, 20444, 20646, 20848,
	21049, 21250, 21450, 21649, 21848, 22047, 22245, 22443,
	22640, 22836, 23032, 23227, 23422, 23616, 23810, 24003,
	24196, 24388, 24580, 24771, 24961, 25151, 25340, 25529,
	25717, 25905, 26092, 26278, 26464, 26649, 26834, 27018,
	27202, 27385, 27568, 27750, 27931, 28112, 28292, 28471,
	28650, 28829, 29007, 29184, 29361, 29537, 29712, 29887,
	30062, 30236, 30409, 30582, 30754, 30925, 31096, 31266,
	31436, 31605, 31774, 31942, 32109, 32276, 32442, 32608,
	32773, 32938, 33101, 33265, 33428, 33590, 33751, 33913,
	34073, 34233, 34392, 34551, 34709, 34867, 35024, 35180,
	35336, 35492, 35646, 35801, 35954, 36107, 36260, 36412,
	36563, 36714, 36864, 37014, 37163, 37312, 37460, 37607,
	37754, 37901, 38047, 38192, 38337, 38481, 38624, 38768,
	38910, 39052, 39194, 39335, 39475, 39615, 39754, 39893,
	40032, 40169, 40307, 40443, 40580, 40715, 40850, 40985,
	41119, 41253, 41386, 41519, 41651, 41782, 41913, 42044,
	42174, 42303, 42432, 42561, 42689, 42817, 42944, 43070,
	43196, 43322, 43447, 43572, 43696, 43819, 43943, 44065,
	44188, 44309, 44431, 44551, 44672, 44792, 44911, 45030,
	45148, 45266, 45384, 45501, 45618, 45734, 45849, 45965,
};
#endif
 uint16_t cal_sin_max = 0, cal_sin_min = 0x400, cal_cos_max = 0, cal_cos_min = 0x400;
 uint16_t cal_sin_zero = 0, cal_cos_zero = 0;

#define VSIN_PIN				ADC_INPUT_AIN2_P01
#define VCOS_PIN				ADC_INPUT_AIN3_P02

#define THROTTLE_CHANGE_TH			 _Q10(1)
#define THROTTLE_ZERO_WIDTH			 _Q10(10)// _Q10(20)
#define THROTTLE_MAX_WIDTH			 _Q10(20)

THROTTLE_Handle_t THROTTLE_Handle;

uint8_t  FORWARD_POWER_MAX_LIMIT;
uint8_t  BACKWARD_POWER_MAX_LIMIT;

void THROTTLE_Init(THROTTLE_Handle_t* handle)
{	
	handle->flagNeedCal = false;

	handle->rpm = 0;
	handle->dir = 1;

	handle->state = THROTTLE_State_Reseted;
	
    handle->PercentTemp = 0;
	handle->HandleSide = HANDLE_SIDE_RIGHT;
	handle->Voltage = VOLTAGE12V;

	handle->forwardMaxVal = HALF_RANGE;
	handle->backwardMaxVal = HALF_RANGE;
	handle->preAngle = handle->angle;
	handle->ADCTimerStateFlag = Throttle_ADC_Timer_State_Stoped;
	handle->SensorDataErrFlag = false;
	THROTTLE_Handle.throttleRstFlag = false;
	handle->throttleDataErrCnt = 0;
	THROTTLE_SetCalErrFlag(true);
}


void THROTTLE_cal_state(THROTTLE_Handle_t* handle,bool flag)
{
	handle->calibrationFlag = flag;
}

uint8_t value_buf[N],i=0;
uint16_t sum=0;

uint8_t PercentFilter(uint8_t data_get)
{
	uint8_t count;
	
	sum=0;
	value_buf[i++] = data_get;
	if ( i >= N )    
	{
		i = 0;
	}
	for(count=0;count<N;count++)
	{
		sum += value_buf[count];
	}
	return (uint8_t)(sum>>4);
}


void THROTTLE_Calc(THROTTLE_Handle_t* handle)
{	
	if(THROTTLE_Handle.throttleRstFlag == false)
	{
		THROTTLE_Handle.preAngle = THROTTLE_Handle.angle;
		THROTTLE_Handle.throttleRstFlag = true;
	}
	
	if(handle->Cal_BackwardAngle < handle->Cal_ForwardAngle)//f<360 b>0
	{
		if(ABS(handle->angle-handle->preAngle)>MAX_ANGLE_TURN)
		{
			if((!((handle->preAngle <= MAX_ANGLE_TURN) && (handle->angle <= MAX_ANGLE && handle->angle >= MAX_ANGLE-MAX_ANGLE_TURN)&&(handle->preAngle+MAX_ANGLE-handle->angle<=MAX_ANGLE_TURN)))&&
				(!((handle->preAngle <= MAX_ANGLE && handle->preAngle >= MAX_ANGLE-MAX_ANGLE_TURN) && (handle->angle <= MAX_ANGLE_TURN)&&(handle->angle+MAX_ANGLE-handle->preAngle<=MAX_ANGLE_TURN))))
			{
				if(handle->throttleDataErrCnt++>5)
				{
					handle->SensorDataErrFlag = true;
					handle->throttleDataErrCnt = 0;
				}
				return;
			}
		}

		handle->preAngle = handle->angle;
		
		if((handle->angle < handle->Cal_ForwardAngle)&&(handle->angle > handle->Cal_BackwardAngle))
		{
			if(handle->Cal_ForwardAngle - handle->angle <= 5)
			{
				handle->angle = handle->Cal_ForwardAngle;
			}
			else if(handle->angle-handle->Cal_BackwardAngle <= 5)
			{
				handle->angle = handle->Cal_BackwardAngle;
			}
			else
			{
				return;
			}
		}
		
		if(handle->Cal_ForwardAngle <= 225)//zero->f
		{
			if((handle->angle <= handle->zeroAngle) && (handle->angle >= handle->Cal_ForwardAngle))
			{
				handle->dir = 1;
				handle->PercentTemp = (127L*(handle->zeroAngle - handle->angle))/HALF_RANGE;
			}
			else if((handle->angle > handle->zeroAngle) && (handle->angle <= MAX_ANGLE))
			{
				handle->dir = 0;
				handle->PercentTemp = (127L*(handle->angle - handle->zeroAngle))/HALF_RANGE;
			}
			else
			{
				handle->dir = 0;
				handle->PercentTemp = (127L*(handle->angle + MAX_ANGLE - handle->zeroAngle))/HALF_RANGE;
			}
		}
		else//zero->b
		{	
			if((handle->angle > handle->zeroAngle)&&(handle->angle <= handle->Cal_BackwardAngle))
			{
				handle->dir = 0;
				handle->PercentTemp = (127L*(handle->angle - handle->zeroAngle))/HALF_RANGE;
			}
			else if((handle->angle <= handle->zeroAngle))
			{
				handle->dir = 1;
				handle->PercentTemp = (127L*(handle->zeroAngle - handle->angle))/HALF_RANGE;
			}
			else
			{
				handle->dir = 1;
				handle->PercentTemp = (127L*(MAX_ANGLE - handle->angle + handle->zeroAngle))/HALF_RANGE;
			}
		}
	}
	else if(handle->Cal_BackwardAngle > handle->Cal_ForwardAngle)//0-360
	{
		if((ABS(handle->angle-handle->preAngle)>=MAX_ANGLE_TURN)||((handle->preAngle-handle->angle)>=MAX_ANGLE_TURN))
		{
			if(handle->throttleDataErrCnt++>5)
			{
				handle->SensorDataErrFlag = true;
				handle->throttleDataErrCnt = 0;
			}
			return;
		}
		handle->preAngle = handle->angle;
		
		if(handle->angle < handle->Cal_ForwardAngle || handle->angle > handle->Cal_BackwardAngle)
		{
			if(handle->angle < handle->Cal_ForwardAngle)
			{
				if(handle->Cal_BackwardAngle!=360)
				{
					handle->angle = handle->Cal_ForwardAngle;
				}
				else if(handle->Cal_BackwardAngle==360)
				{
					handle->angle = handle->Cal_BackwardAngle;
				}
			}	
			else if(handle->angle > handle->Cal_BackwardAngle)
			{
				if(handle->Cal_ForwardAngle!=0)
				{
					handle->angle = handle->Cal_BackwardAngle;
				}
				else if(handle->Cal_ForwardAngle==0)
				{
					handle->angle = handle->Cal_ForwardAngle;
				}
			}
		}
		
		if((handle->angle <= handle->zeroAngle) && (handle->angle >= handle->Cal_ForwardAngle))
		{
			handle->dir = 1;
			handle->PercentTemp = (127L*(handle->zeroAngle - handle->angle))/HALF_RANGE;
		}
		else if((handle->angle <= handle->Cal_BackwardAngle)&&(handle->angle > handle->zeroAngle))
		{
			handle->dir = 0;
			handle->PercentTemp = (127L*(handle->angle - handle->zeroAngle))/HALF_RANGE;
		}

	}
	else 
	{
		THROTTLE_SetCalErrFlag(true);
	}

	handle->percent = PercentFilter((uint8_t)handle->PercentTemp);
	if(handle->percent>127)handle->percent = 127;
	handle->PrePercent = handle->percent;		
}


void  THROTTLE_SaveLimitValue(THROTTLE_Handle_t* handle)
{	
	if(handle->state == THROTTLE_State_Cal_For)
	{
		handle->Cal_ForwardAngle = handle->angle;
	}
	else if(handle->state == THROTTLE_State_Cal_Bak)
	{
		handle->Cal_BackwardAngle = handle->angle;
	}
}


//////////////////////////////////////////////////////
int32_t THROTTLE_GetAngle(THROTTLE_Handle_t* handle)
{
	return handle->angle;
}

int16_t THROTTLE_GetRpm(THROTTLE_Handle_t* handle)
{
	return handle->rpm;
}

int16_t THROTTLE_GetPercent(THROTTLE_Handle_t* handle)
{
	return handle->percent;
}
void THROTTLE_SetPercent(uint8_t percent)
{
	 THROTTLE_Handle.percent = percent;
}
int16_t THROTTLE_GetDir(THROTTLE_Handle_t* handle)
{
	int16_t dir_tmp = 0;
	
	#if 1
	if(THROTTLE_Handle.HandleSide == HANDLE_SIDE_RIGHT)
	{
		if(handle->dir == 0)dir_tmp = 1;
		else if(handle->dir == 1)dir_tmp = 0;
	}
	else if(THROTTLE_Handle.HandleSide == HANDLE_SIDE_LEFT)
	{
		dir_tmp = handle->dir;
	}
	#endif
	
	return dir_tmp;
}

void THROTTLE_SetZeroAngle(THROTTLE_Handle_t* handle,uint32_t angle)
{
	handle->zeroAngle = angle;
}

void THROTTLE_SetCosMid(THROTTLE_Handle_t* handle,uint16_t mid)
{
	handle->cosMid = mid;
}

void THROTTLE_SetSinMid(THROTTLE_Handle_t* handle,uint16_t mid)
{
	handle->sinMid = mid;
}

void THROTTLE_RequireReset(void)
{
	THROTTLE_Handle.state = THROTTLE_State_NeedReset;
}

bool THROTTLE_GetReset(THROTTLE_Handle_t* handle)
{
	if(handle->state == THROTTLE_State_NeedReset)
		return true;
	else
		return false;
}
void THROTTLE_Off(void)
{
	//THROTTLE_Handle.percent = 0;
	switch(THROTTLE_Handle.state)
	{
		case THROTTLE_State_Reseted:
		case THROTTLE_State_Forward:
		case THROTTLE_State_Backward:
			 THROTTLE_Handle.state = THROTTLE_State_Off;
			 break;
		default:
			break;
	}
}

void THROTTLE_On(void)
{
	if(THROTTLE_Handle.state == THROTTLE_State_Off)
	{
	    THROTTLE_Handle.state = THROTTLE_State_Reseted;//上电不需要回0
	}	
}
#if 0
void THROTTLE_Cal_Start(THROTTLE_Handle_t* handle)
{
	cal_sin_max = 0;
	cal_sin_min = 0x400;
	cal_cos_max = 0;
	cal_cos_min = 0x400;
}
void THROTTLE_CalRes(THROTTLE_Handle_t* handle)
{
	int32_t rotAngle = 0;//与中间角度偏差
  
    if((handle->state == THROTTLE_State_NeedReset)
	||(handle->state == THROTTLE_State_Off) )
	{
		//THROTTLE_SetPercent(0);
		THROTTLE_angleUpdate(handle,THROTTLE_Ad2angle(handle->sinVal,handle->cosVal));

		rotAngle = handle->zeroAngle - handle->angle;
		if(rotAngle < 0) rotAngle += _Q10(360);
		if((THROTTLE_isZero(handle,rotAngle) == true)&&(handle->calibrationFlag == true))
		{
			handle->state = THROTTLE_State_Cal_Start;
		}
	} 
}
#endif
void  THROTTLE_CalErrCheck(THROTTLE_Handle_t* handle)
{
	if(handle->Cal_BackwardAngle<handle->Cal_ForwardAngle)//f<360 b>0
		{
			if(handle->Cal_ForwardAngle<=225)//zero->f
				handle->zeroAngle = HALF_RANGE + handle->Cal_ForwardAngle;
			else//zero->b
				handle->zeroAngle = HALF_RANGE - (MAX_ANGLE-handle->Cal_ForwardAngle);

			if((handle->Cal_ForwardAngle-handle->Cal_BackwardAngle <= ANGLE_RANGE_MIN)||
				(handle->Cal_ForwardAngle-handle->Cal_BackwardAngle >= ANGLE_RANGE_MAX))
			{
				THROTTLE_SetCalErrFlag(true);	
			}
		}
		else if(handle->Cal_BackwardAngle>handle->Cal_ForwardAngle)//0-360
		{
			handle->zeroAngle = handle->Cal_ForwardAngle+HALF_RANGE;
			if((handle->Cal_BackwardAngle-handle->Cal_ForwardAngle <= ANGLE_RANGE_LONG_MIN)||
				(handle->Cal_BackwardAngle-handle->Cal_ForwardAngle >= ANGLE_RANGE_LONG_MAX))
			{
				THROTTLE_SetCalErrFlag(true);	
			}
		}
		else 
		{
			THROTTLE_SetCalErrFlag(true);
		}
}

uint8_t THROTTLE_Cal_Next(THROTTLE_Handle_t* handle)
{	
	LED_AllOff();
	if(handle->state == THROTTLE_State_Cal_Start)
	{
		handle->state = THROTTLE_State_Cal_For;
				#ifdef CALL_CHECK
		if((ABS(handle->angle - LEFT_ANG) > MAX_ANG_CHECK_VAL))
		{
			 THROTTLE_SetCalErrFlag(true);
			 return 0;
		}
		#endif
		THROTTLE_SaveLimitValue(handle);
		
		return 0;
	}
	else if(handle->state == THROTTLE_State_Cal_For)
	{
		handle->state = THROTTLE_State_Cal_Mid;
		#ifdef CALL_CHECK
		if((ABS(handle->angle - ZERO_ANG) > MAX_ANG_CHECK_VAL))
		{
			 THROTTLE_SetCalErrFlag(true);
			 return 0;
		}
		#endif
		handle->zeroAngle = handle->angle;

		return 0;
	}
	else if(handle->state == THROTTLE_State_Cal_Mid)
	{
		handle->state = THROTTLE_State_Cal_Bak;
		#ifdef CALL_CHECK
		if((ABS(handle->angle - RIGHT_ANG) > MAX_ANG_CHECK_VAL))
		{
			 THROTTLE_SetCalErrFlag(true);
			 return 0;
		}
		#endif
		THROTTLE_SaveLimitValue(handle);
	//	return 0;
	}
	if(handle->state == THROTTLE_State_Cal_Bak)
	{ 

		LCD_Handle.Menu = LCD_Menu_Main;
		THROTTLE_cal_state(&THROTTLE_Handle,false);
		THROTTLE_Handle.state = THROTTLE_State_Reseted;	
		
		handle->forwardMaxVal = HALF_RANGE;//360-(handle->Cal_ForwardAngle-handle->zeroAngle);
		handle->backwardMaxVal = HALF_RANGE;//(handle->Cal_BackwardAngle-handle->zeroAngle);

		//THROTTLE_CalErrCheck(handle);

#if 1
		if(handle->Cal_BackwardAngle < handle->Cal_ForwardAngle)//f<360 b>0
		{
			if(handle->Cal_ForwardAngle <= 225)//zero->f
				handle->zeroAngle = HALF_RANGE + handle->Cal_ForwardAngle;
			else//zero->b
				handle->zeroAngle = HALF_RANGE - (MAX_ANGLE - handle->Cal_ForwardAngle);

			if((handle->Cal_ForwardAngle - handle->Cal_BackwardAngle <= ANGLE_RANGE_MIN)||
				(handle->Cal_ForwardAngle - handle->Cal_BackwardAngle >= ANGLE_RANGE_MAX))
			{
				THROTTLE_SetCalErrFlag(true);	
				return 0;
			}
		}
		else if(handle->Cal_BackwardAngle > handle->Cal_ForwardAngle)//0-f-b-360
		{
			handle->zeroAngle = handle->Cal_ForwardAngle + HALF_RANGE;
			if((handle->Cal_BackwardAngle - handle->Cal_ForwardAngle <= ANGLE_RANGE_LONG_MIN)||
				(handle->Cal_BackwardAngle - handle->Cal_ForwardAngle >= ANGLE_RANGE_LONG_MAX))
			{
				THROTTLE_SetCalErrFlag(true);	
				return 0;
			}
		}
		else 
		{
			THROTTLE_SetCalErrFlag(true);
		}
#endif		
		THROTTLE_cal_state(&THROTTLE_Handle,false);
	}
	return 1;
}

void THROTTLE_SetState(THROTTLE_Handle_t* handle,THROTTLE_State_e state)
{
	handle->state = state;
}

THROTTLE_State_e THROTTLE_GetState(THROTTLE_Handle_t* handle)
{
	return handle->state;
}

void THROTTLE_Request_Cal(THROTTLE_Handle_t* handle)
{
	handle->flagNeedCal = true;
}

bool THROTTLE_Is_Cal_Needed(THROTTLE_Handle_t* handle)
{
	return handle->flagNeedCal;
}
void THROTTLE_SetCalErrFlag(bool status)
{
	THROTTLE_Handle.CalErrorFlag = status;
}
bool THROTTLE_ReadCalErrFlag(void)
{
	return  THROTTLE_Handle.CalErrorFlag ;
}

///////////////////////////////////////////////////
void THROTTLE_Backup_cal(THROTTLE_Handle_t * handle,uint8_t* data)
{
	*(data+0) = handle->zeroAngle >> 24;
	*(data+1) = handle->zeroAngle >> 16;
	*(data+2) = handle->zeroAngle >> 8;
	*(data+3) = handle->zeroAngle;
#if 1
	*(data+4) = handle->Cal_ForwardAngle >> 24;
	*(data+5) = handle->Cal_ForwardAngle >> 16;
	*(data+6) = handle->Cal_ForwardAngle >> 8;
	*(data+7) = handle->Cal_ForwardAngle;
	
	*(data+8) = handle->Cal_BackwardAngle  >> 24;
	*(data+9) = handle->Cal_BackwardAngle >> 16;
	*(data+10) = handle->Cal_BackwardAngle >> 8;
	*(data+11) = handle->Cal_BackwardAngle;
	
	//*(data+12) = ((handle->forwardMaxVal >> 4) & 0xF0) | ((handle->backwardMaxVal >> 8) & 0x0F);
	//*(data+13) = handle->backwardMaxVal & 0xFF;
	#endif
	*(data+12) = PSTORAGE_CheckSum(data, 12); 
}
bool THROTTLE_Update_cal(THROTTLE_Handle_t* handle,uint8_t* data)
{
	if(data[12] == PSTORAGE_CheckSum(data, 12)) 
	{
		handle->zeroAngle = (uint32_t)(*(data+0) << 24) + (uint32_t)(*(data+1) << 16) + (uint32_t)(*(data+2) << 8) + (uint32_t)(*(data+3));
		handle->Cal_ForwardAngle = (uint32_t)(*(data+4) << 24) + (uint32_t)(*(data+5) << 16) + (uint32_t)(*(data+6) << 8) + (uint32_t)(*(data+7));
		handle->Cal_BackwardAngle = (uint32_t)(*(data+8) << 24) + (uint32_t)(*(data+9) << 16) + (uint32_t)(*(data+10) << 8) + (uint32_t)(*(data+11));
		
		THROTTLE_SetCalErrFlag(false);

		THROTTLE_CalErrCheck(handle);
		return true;
	}
	else 
	{
        handle->zeroAngle = ZERO_ANG;//5567;//160317;
        handle->Cal_ForwardAngle = LEFT_ANG;
		handle->Cal_BackwardAngle = RIGHT_ANG;
		
		THROTTLE_SetCalErrFlag(true);	
		//handle->forwardMaxVal = 360-(handle->Cal_ForwardAngle-handle->zeroAngle);
		//handle->backwardMaxVal = (handle->Cal_BackwardAngle-handle->zeroAngle);
	}

	return false;
}
void THROTTLE_Backup_PercentMax(THROTTLE_Handle_t* handle,uint8_t* data)
{
	*(data+0) = handle->MotorPosition>>8;  
	*(data+1) = handle->MotorPosition;  
	*(data+2) = handle->HandleSide;  
	*(data+3) = handle->Voltage;  
    *(data+4) = PSTORAGE_CheckSum(data, 4); 	
}
bool THROTTLE_Update_PercentMax(THROTTLE_Handle_t* handle,uint8_t* data)
{
	if(data[4] == PSTORAGE_CheckSum(data, 4)) 
	{
	   handle->MotorPosition = *(data+0);
	   handle->MotorPosition<<=8;
	   handle->MotorPosition |= *(data+1); 
		
	   handle->HandleSide = *(data+2);
	   handle->Voltage = *(data+3); 
	   return  true;			 
	}
	else
	{
	   handle->HandleSide = HANDLE_SIDE_RIGHT;
	   handle->Voltage = VOLTAGE24V; 
	}
	return  false;
}

