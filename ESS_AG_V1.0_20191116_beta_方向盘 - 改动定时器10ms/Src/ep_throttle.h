#ifndef __EP_THROTTLE_H__
#define __EP_THROTTLE_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "main.h"

#define N 16


#define BAT_VOLTAGE_READ_PIN		ADC_CONFIG_PSEL_AnalogInput1
//#define CALL_CHECK 
#define MAX_ANGLE_TURN 30

#define MAX_ANGLE 359

#if 1
#define LEFT_ANG    347
#define ZERO_ANG	121
#define RIGHT_ANG   260
#else 
#define LEFT_ANG    280 //40
#define ZERO_ANG	62
#define RIGHT_ANG   196 // 311

#endif
#define HALF_RANGE 135

#define ZERO_RANGE 7
#define MAX_ANG_CHECK_VAL 10

#define ANGLE_RANGE_MAX 100//280
#define ANGLE_RANGE_MIN 80//260

#define ANGLE_RANGE_LONG_MAX 280
#define ANGLE_RANGE_LONG_MIN 260


#define  ANG_EN_PIN    11

#define  _Q10(x) (uint32_t)(x << 10)

#define  THROTTLE_FORWARD_Counterclockwise    //往前时磁铁相对传感器逆时针旋转single navy mode or Double navy mode  the left board
//#define  THROTTLE_FORWARD_Clockwise          //往前时磁铁相对传感器顺时针旋转 Double navy mode  the right board
#define  DIRECTION_FORWARD 0
#define  DIRECTION_BACKWARD 1

#define  NAVY30_DEFAULT_FORWARD_POWER_MAX         100 // 100% * 3000W
#define  NAVY30_FORWARD_POWER_MAX_LIMIT           100 // 100% * 3000W 
#define  NAVY30_DEFAULT_BACKWARD_POWER_MAX        100 // 100% * 3000W
#define  NAVY30_BACKWARD_POWER_MAX_LIMIT          100 // 100% * 3000W 


#define  NAVY60_DEFAULT_FORWARD_POWER_MAX         100 // 100% * 6000W 
#define  NAVY60_FORWARD_POWER_MAX_LIMIT           100 // 100% * 6000W 
#define  NAVY60_DEFAULT_BACKWARD_POWER_MAX        50  // 50% * 6000W
#define  NAVY60_BACKWARD_POWER_MAX_LIMIT          100 // 100% * 6000W 

typedef enum
{
	HANDLE_SIDE_RIGHT,
	HANDLE_SIDE_LEFT,
}THROTTLE_HandleSide_e;


typedef enum
{
    Throttle_ADC_Timer_State_Started,
  	Throttle_ADC_Timer_State_Stoped,
  	Throttle_ADC_Timer_State_End
}THROTTLE_ADC_Timer_State_e;

typedef enum
{
    ADC_RES_8bit = 0, 
    ADC_RES_9bit, 
    ADC_RES_10bit
} ADC_Res_t;

typedef enum 
{
    ADC_INPUT_AIN0_P26 = 0, 
    ADC_INPUT_AIN1_P27 = 1, 
    ADC_INPUT_AIN2_P01 = 2, 
    ADC_INPUT_AIN3_P02 = 3, 
    ADC_INPUT_AIN4_P03 = 4, 
    ADC_INPUT_AIN5_P04 = 5, 
    ADC_INPUT_AIN6_P05 = 6, 
    ADC_INPUT_AIN7_P06 = 7
} ADC_input_selection_t;

typedef enum 
{
    ADC_INT_DISABLED = 0,
    ADC_INT_ENABLED
} ADC_interrupt_enabled_t;

typedef enum
{
	THROTTLE_State_Off = 0,
	THROTTLE_State_NeedReset, //1
	THROTTLE_State_Reseted,   //2
	THROTTLE_State_Forward,   //3
	THROTTLE_State_Backward,  //4
	THROTTLE_State_Cal_Start, //5
	THROTTLE_State_Cal_For,   //6
	THROTTLE_State_Cal_Mid,   //7
	THROTTLE_State_Cal_Bak,   //8
	THROTTLE_State_Cal,       //9
	THROTTLE_State_NoCal      //10
}THROTTLE_State_e;

typedef struct
{
	bool             CalErrorFlag;
	bool			 flagNeedCal;
	volatile  bool   calibrationFlag;       //calibrationing flag
	
	int16_t      rpm;									//intented rpm
	int16_t	     PercentTemp;
	int16_t	     percent;								//percent of the full throttle from 0~127 without direction
	int16_t	     PrePercent;	
	int16_t      dir;										//direction 1: forward 0: backward
	
	uint8_t      ForwardPowerMax;    //The Forward max power percent 0-100%
	uint8_t      BackwardPowerMax;    //The Backward max power percent 0-100%
	
	uint8_t      ForwardPercentMax;    //The Forward max power percent 0-127
	uint8_t      BackwardPercentMax;    //The Backward max power percent 0-127
	
	//Hall rotrary sensor parameters
	uint16_t      sinMid;
	uint16_t      cosMid;
	
	uint16_t      zeroAngle;
	uint16_t      zeroMin;
	uint16_t      zeroMax;
	uint16_t      forwardMax; 
	uint16_t      backwardMax;
	uint16_t      Cal_ForwardAngle;
	uint16_t      Cal_BackwardAngle;       //
	
	uint16_t     forwardMaxParam;
	uint16_t     backwardMaxParam;
	uint16_t     forwardMaxVal;
	uint16_t     backwardMaxVal;
	uint16_t     ForwardCheckVal;
	uint16_t     backwardCheckVal;
	
	uint16_t angle;		//q10
	uint16_t preAngle;	

    uint8_t     ADCTimerStateFlag;
	uint8_t     HandleSide;
	uint8_t    Voltage;
    THROTTLE_State_e  state;				//0: Initial State when the throttle is not zero
    															//1: Initial State when the throttle is zero or throttle is pushed back to zero
																	//2: Moving forward
																	//3: Moving backward
																	//4: Calibration
	uint16_t 	MotorPosition;
	uint8_t 	SensorDataErrFlag; //sensor error if angle turn > MAX_ANGLE_TURN
	bool        throttleRstFlag;
	uint8_t     throttleDataErrCnt;
} THROTTLE_Handle_t;

typedef enum
{
	Throttle_Angle360_2_0,
	Throttle_Angle0_2_360,
	Throttle_Angle0_2_720,
	Throttle_Angle720_2_0,
	Throttle_AngleCross_End
}THROTTLE_CrossFlag_e;
	
extern THROTTLE_Handle_t THROTTLE_Handle;

extern uint8_t  FORWARD_POWER_MAX_LIMIT;
extern uint8_t  BACKWARD_POWER_MAX_LIMIT;

void  THROTTLE_Init(THROTTLE_Handle_t* handle);
void  THROTTLE_KMZ60Enable(void);
void  THROTTLE_KMZ60Disable(void);
void  THROTTLE_AngSampleStart(void);
void  THROTTLE_AngSample(THROTTLE_Handle_t* handle);
static bool  THROTTLE_isZero(THROTTLE_Handle_t* handle,int32_t angle);
void  THROTTLE_cal_state(THROTTLE_Handle_t* handle,bool flag);
static inline int32_t THROTTLE_Ad2angle(uint16_t sinVal, uint16_t cosVal);
static inline void THROTTLE_angleUpdate(THROTTLE_Handle_t* handle,int32_t theta);
void THROTTLE_Calc(THROTTLE_Handle_t* handle);
int32_t  THROTTLE_GetAngle(THROTTLE_Handle_t* handle);
int16_t  THROTTLE_GetRpm(THROTTLE_Handle_t* handle);
int16_t  THROTTLE_GetPercent(THROTTLE_Handle_t* handle);
int16_t  THROTTLE_GetDir(THROTTLE_Handle_t* handle);
void  THROTTLE_SetZeroAngle(THROTTLE_Handle_t* handle,uint32_t angle);
void  THROTTLE_SetCosMid(THROTTLE_Handle_t* handle,uint16_t mid);
void  THROTTLE_SetSinMid(THROTTLE_Handle_t* handle,uint16_t mid);
void  THROTTLE_RequireReset(void);
bool  THROTTLE_GetReset(THROTTLE_Handle_t* handle);
void  THROTTLE_Off(void);
void  THROTTLE_On(void);
void  THROTTLE_SetCalErrFlag(bool status);
bool  THROTTLE_ReadCalErrFlag(void);
void  THROTTLE_SetPercent(uint8_t percent);
void  THROTTLE_Cal_Start(THROTTLE_Handle_t* handle);
void  THROTTLE_CalRes(THROTTLE_Handle_t* handle);
void  THROTTLE_CalFail(THROTTLE_Handle_t* handle);
uint8_t  THROTTLE_Cal_Next(THROTTLE_Handle_t* handle);
void  THROTTLE_SetState(THROTTLE_Handle_t* handle,THROTTLE_State_e state);
THROTTLE_State_e THROTTLE_GetState(THROTTLE_Handle_t* handle);
void  THROTTLE_Request_Cal(THROTTLE_Handle_t* handle);
bool  THROTTLE_Is_Cal_Needed(THROTTLE_Handle_t* handle);
bool  THROTTLE_Cal_In_Progress(THROTTLE_Handle_t* handle);
void  THROTTLE_Backup_cal(THROTTLE_Handle_t * handle,uint8_t* data);
bool  THROTTLE_Update_cal(THROTTLE_Handle_t* handle,uint8_t* data);
void  THROTTLE_Backup_PercentMax(THROTTLE_Handle_t* handle,uint8_t* data);
bool  THROTTLE_Update_PercentMax(THROTTLE_Handle_t* handle,uint8_t* data);
void  THROTTLE_CalMaxParam(THROTTLE_Handle_t* handle);
void  THROTTLE_SaveLimitValue(THROTTLE_Handle_t* handle);
void  THROTTLE_CalCheckVal(THROTTLE_Handle_t* handle);
void  THROTTLE_CheckPowerMax(void);
#endif /* __EP_THROTTLE_H__ */

