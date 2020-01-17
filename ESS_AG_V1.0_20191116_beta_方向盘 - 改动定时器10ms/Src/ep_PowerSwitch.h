/*************************************
* File    Name:  ep_PowerSwitch.H
* Description :  
*
*
*************************************/

#ifndef  __EP_POWERSWITCH_H_
#define  __EP_POWERSWITCH_H_

#include "nrf.h"

#define   POWER_EN_PIN_NUM      28
//#define   VBAT_OK_PIN           12
#define  BUTTON_Power_Button   7

#define BAT_VOLTAGE_LOW 536  //voltage low 
#define BAT_VOLTAGE_FULL 641 //voltage full
#define BAT_VOLTAGE_NORMAL 565 


typedef enum 
{
	HandleBAT_UV_Off, //handle battery  voltage nomal
	HandleBAT_UV_On     //handle battery under voltage
}HandleBAT_UV_Warning_State_e;

typedef enum 
{
	PowerTurnOn,
	PowerOn,
	PowerOff,
    PowerTurnOffEn
}PowerState_e;

typedef enum 
{
	HandleBAT_nomal, //handle battery  voltage nomal
	HandleBAT_UV     //handle battery under voltage
}HandleBAT_State_e;

typedef struct __PowerSwitch_Handle_t
{
	PowerState_e        SysPowerState;
	uint8_t             SysPowerCheckFlag;
	HandleBAT_State_e   HandleBAT_State;  
	uint16_t            AutoPowerOffCount;    //自动关机计数
	uint16_t            BatUV_count;
	uint16_t 			BatVoltageValue;
	uint16_t 			BatSum;
	uint8_t 			BatReadCnt;
	HandleBAT_UV_Warning_State_e				BatUVWarningFlag;
}PowerSwitch_Handle_t;

extern  PowerSwitch_Handle_t  PowerSwitch_Handle;

extern  void  PowerSwitch_BatUVcountClear(void);
extern  HandleBAT_State_e  PowerSwitch_ReadBatState(PowerSwitch_Handle_t* handle);
extern  void  PowerSwitch_OffCountClear(void);
extern  void  PowerSwitch_OffCountAdd(void);
extern  void  PowerSwitch_Init(PowerSwitch_Handle_t* handle);
extern  void  PowerSwitch_onCheck(PowerSwitch_Handle_t* handle);
extern  void  PowerSwitch_offCheck(PowerSwitch_Handle_t* handle);
static void  PowerSwitch_Off(void);
void  PowerSwitch_CloseSys(void);
void  PowerSwitch_OpenSys(void);
void  GPIO_OutPut(void);
void ADC_GetBatVol(void);

#endif
