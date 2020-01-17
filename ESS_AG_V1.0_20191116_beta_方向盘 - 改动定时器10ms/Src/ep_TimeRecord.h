#ifndef  __EP_TIMERECORD_H_
#define  __EP_TIMERECORD_H_

#include "nrf.h"
#include "nrf51_bitfields.h"
#include "nrf51.h"
#include "stdbool.h"


#define   TEMP_ARR_LEN_MAX  30
#define  RECORD_TERMP_INTERVAL_SEC   5//every 5 sec save temperature data 

typedef struct 
{
	uint16_t     ForwardDay;
	uint8_t      ForwardHour;
	uint8_t      ForwardMinute;
	
	uint16_t     BackwardDay;
	uint8_t      BackwardHour;
	uint8_t      BackwardMinute;
	
	
	uint16_t     RunTime;//minute ,if power > 0,RunTime++
	
}TimeRecord_Handle_t;

extern TimeRecord_Handle_t TimeRecord_Handle;
extern uint16_t Mos1TempArr[TEMP_ARR_LEN_MAX];
extern uint16_t Mos2TempArr[TEMP_ARR_LEN_MAX];
extern uint16_t MotTempArr[TEMP_ARR_LEN_MAX];
extern uint8_t ArrReadIndex;

void  TimeRecord_Init(TimeRecord_Handle_t* handle);
void  TimeRecord_TimeAdd(TimeRecord_Handle_t* handle);
void  TimeRecord_TimeClear(TimeRecord_Handle_t* handle);
void  TimeRecord_Backup(TimeRecord_Handle_t* handle,uint8_t* data);
bool  TimeRecord_Update(TimeRecord_Handle_t* handle,uint8_t* data);
uint16_t  TimeRecord_ReadRunTime(void);
void  TimeRecord_ClearRunTime(void);
void  TimeRecord_AddArrIndex(void);
bool  TimeRecord_ReadTempArrFlag(void);
#endif

