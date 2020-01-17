#ifndef  __EP_BATTERY_H
#define  __EP_BATTERY_H

#include "nrf.h"
#include <stdbool.h>
#include  "ep_comm.h"

#define   USE_BAT_MAX_VOL   600
#define   USE_BAT_MIN_VOL   390

typedef enum
{
	Battery_State_Off = 0,
	Battery_State_OffCfm,
	Battery_State_TurnOff,
	Battery_State_On,
	Battery_State_OnCfm,
	Battery_State_TurnOn,
	Battery_State_Idle,
	Battery_State_Run,
	Battery_State_CommFailed,
	Battery_State_Error,
}Battery_State_e;
typedef enum
{
	Battery_Status_NonEpBatt = 0,
	Battery_Status_CommFailed,
	Battery_Status_FunctionalError,
	Battery_Status_On,
}Battery_Status_e;

typedef enum 
{
	Battery_TypeOwnBatt,
	BatteryType_LiOn,
	BatteryType_Pb,
	Battery_TypeLiFe,
	BatteryType_Max,
	Battery_TypeA,
	Battery_TypeB,
	Battery_TypeC,
	Battery_TypeD,
	Battery_TypeE
}BatteryType_e;
typedef enum 
{
	bat_soc_more,
	bat_soc_less
}Bat_SocState_e;
typedef   struct
{
	bool              SendSocFlag;
	bool              SaveSocFlag;
	Bat_SocState_e    Bat_SocState;
	Battery_Status_e  Status;
	Battery_State_e   State;
	uint16_t          BatVoltage;
	int               BatSoc;
	int               SaveSoc;
	uint16_t          BatCurrent;
	uint16_t          BatStatus;
	
    uint8_t           HighestTemp;
	uint8_t           LowestTemp;
	
	uint16_t          useBatVol; //使用电池的标称电压
	uint16_t          useBatRealCap;//使用电池的标称容量
	BatteryType_e     useBatType;   //使用电池的类型
}BATTERY_Handle_t;

extern  BATTERY_Handle_t BATTERY_Handle;

BatteryType_e BATTERY_ReadBatType(BATTERY_Handle_t * handle);
uint8_t  BATTERY_Get_BatNum(BATTERY_Handle_t * handle);
void BATTERY_SetBatSocState(Bat_SocState_e state);
Bat_SocState_e BATTERY_GetBatState(BATTERY_Handle_t* handle);
void BATTERY_DrvSysStartCheck(uint16_t StartupValue);
BatteryType_e  BATTERY_Get_useBatType(BATTERY_Handle_t * handle);
uint16_t  BATTERY_Get_BatCap(BATTERY_Handle_t * handle);
void BATTER_comm_RX_decode(BATTERY_Handle_t* handle,Comm_Handle_t* comm_handle);
void BATTERY_Backup_battery(BATTERY_Handle_t * handle,uint8_t* data);
bool BATTERY_Update_battery(BATTERY_Handle_t * handle,uint8_t* data);
extern void BATTERY_Init(BATTERY_Handle_t* handle);
uint16_t  BATTERY_Get_BatUseVol(BATTERY_Handle_t * handle);
//uint16_t BATTERY_CheckVolRange(uint16_t  useBatVol);
void  BATTERY_Set_BatCapType(BATTERY_Handle_t * handle,uint16_t useBatCapType);
void  BATTERY_Set_BatUseVol(BATTERY_Handle_t * handle,uint16_t useBatvol );
void  BATTERY_CheckVolRange(BATTERY_Handle_t * handle);
void  BATTERY_AddVol(BATTERY_Handle_t * handle);
void  BATTERY_CheckBatType(BATTERY_Handle_t * handle);
uint16_t BATTERY_ReadBatSoc(BATTERY_Handle_t * handle);
void  BATTERY_CheckSoc(BATTERY_Handle_t * handle);
void  BATTERY_SaveSoc(BATTERY_Handle_t * handle);
uint16_t BATTERY_ReadState(BATTERY_Handle_t * handle);
void  BATTERY_SetSendSocFlag(BATTERY_Handle_t * handle,bool flag);
void  BATTERY_SetBatTemp(BATTERY_Handle_t * handle,uint8_t HighTemp,uint8_t LowTemp);
Battery_Status_e  BATTERY_GetBatStatus(BATTERY_Handle_t * handle);
uint16_t BATTERY_ReadVol(void);
#endif
