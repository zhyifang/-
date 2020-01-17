#ifndef __EP_Battery_H__
#define __EP_Battery_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"

//ep battery communication settings
#define EP_BATT_ADDR                    0x90
#define EP_BATT_FUNC                    0x03
#define EP_BATT_CMD                     0xA0
#define EP_BATT_LEN                     0x15
#define EP_BATT_CRC1                    0xC0
#define EP_BATT_CRC2                    0x25

#define EP_BATT_CMD_LEN				    6
#define EP_BATT_RET_LEN				    27              
#define EP_BATT_RET_DATA_LEN			21
#define EP_BATT_RET_DATA_POS			4

#define EP_BATT_UPDATE_INTERVAL			1000             //update batt info every 1s 


//general batteries properties
#define BATTERY_LION_FULL               410
#define BATTERY_LION_NOMINAL            370
#define BATTERY_LION_LINEAR             330
#define BATTERY_LION_EMPTY              300
#define BATTERY_LION_LOAD_VOLT_DIFF     23
   
#define BATTERY_PB_FULL                 215
#define BATTERY_PB_NOMINAL              200
#define BATTERY_PB_LINEAR               195
#define BATTERY_PB_EMPTY                162
#define BATTERY_PB_LOAD_VOLT_DIFF       19

#define BATTERY_LFE_FULL                365
#define BATTERY_LFE_NOMINAL             320
#define BATTERY_LFE_LINEAR              300
#define BATTERY_LFE_EMPTY               200
#define BATTERY_LFE_LOAD_VOLT_DIFF      40



typedef enum
{
	Battery_DataUpdate				    = 0xA0,
}Battery_Command_e;

typedef enum
{
	Battery_Read						= 0x0B,
	Battery_Word_Write					= 0xF1,
	Battery_Block_Write					= 0xF2
}Battery_Command_Type_e;

typedef struct
{	
	uint8_t								BattAddr;
	uint8_t								FuncCode;
	Battery_Command_e					Cmd;
	uint8_t								DataLen;
	uint8_t								CS1;
	uint8_t								CS2;
//	uint8_t								data[Battery_CMD_DATA_LEN + 4];  
}Battery_Cmd_t;

typedef struct 
{
	uint8_t								BattAddr;
	uint8_t								FuncCode;
	Battery_Command_e					Cmd;
	uint8_t								DataLen;
	uint8_t								Data[EP_BATT_RET_DATA_LEN];  
	uint8_t								CS1;
	uint8_t								CS2;
}Battery_Ret_t;

typedef union
{
	Battery_Cmd_t 						Data;
	uint8_t   							Raw[EP_BATT_CMD_LEN];
}Battery_Cmd_u;

typedef union
{
	Battery_Ret_t 						Data;
	uint8_t   							Raw[EP_BATT_RET_LEN];
}Battery_Ret_u;

typedef enum
{
	Battery_Status_NonEpBatt = 0,
	Battery_Status_CommFailed,
	Battery_Status_FunctionalError,
 	Battery_Status_On,
}Battery_Status_e;

typedef enum
{
	Battery_Warning_True                        = 0x01,
	Battery_Warning_UnderVolt                   = 0x02,
	Battery_Warning_OverCurrent                 = 0x04,
	Battery_Warning_OverTemp                    = 0x08,
	Battery_Warning_LowTemp                     = 0x10,
	Battery_Warning_OverVolt                    = 0x20,
	Battery_Warning_HighTempCurrentLimiting     = 0x40,
	Battery_Warning_LowSoc                      = 0x80,
}Battery_Warning_e;


typedef enum 
{
	Battery_TypeOwnBatt = 0,
	Battery_TypeLiOn,
	Battery_TypePb,
	Battery_TypeLiE,
	Battery_TypeA,
	Battery_TypeB,
	Battery_TypeC,
	Battery_TypeD,
}Battery_Type_e;

typedef struct
{
	uint16_t                    FullVoltage;
	uint16_t                    NominalVoltage;
	uint16_t                    LinearVoltage;
	uint16_t                    EmptyVoltage;
    uint8_t                     LinearRange;
    uint8_t                     ExpoRange;
	uint8_t						NumberOfCells;
}Cell_Spec_Obj;


typedef struct
{
    //for both ep & non-ep batteries
	Battery_Type_e 						Type;                                 
	uint16_t 							Voltage;                        //unit: 0.1V
	uint16_t							Current;                        //unit: 0.1A
	uint16_t							Power;                          //unit: 1W
	uint8_t							    SOC;                            //unit: 1%
	uint32_t   					 		AbsCapacity;                    //unit: mAh
	uint32_t			    			RemainingCapacity;              //unit: mAh
	uint16_t							RemainingTime;                  //unit: 
	uint32_t 	                        PowerOutputTime;		        //unit: min
    
    //for ep batteries
	uint8_t							    SOH;                            //unit: 1%
	uint8_t 							HighestTemperature;             //unit: 1degreeC   
	uint8_t 							LowerestTemperature;            //unit: 1degreeC  
	uint16_t 							MaxOutputPower;                 //unit: 1W
	Battery_Status_e					Status;                         
	Battery_Warning_e					Warning;                        
 
    //for non-ep batteries
	uint8_t							    PrevSOC;
	uint16_t							DesignedVolt;
    Cell_Spec_Obj                       Cell;
	uint16_t 							LoadedVoltageDiff;
	float							    Drop;
	float						        CompensationDrop;
	float							    TotalDrop;
	uint16_t							LoadedVolt;
	uint8_t							    socbyvolt;
	float							    RemainingCapacityByVolt;
	float							    RemainingCapacityByCurr;
    float                               RcapDiffWithoutLoad;    
    float                               RcapDiffWithLoad;    
	uint32_t 	                        StoredPowerOutputTime;		    //unit: 0.1seconds
}Battery_Obj;

typedef struct
{	
	Battery_Obj							BatteryObj;
    Battery_Cmd_u                       Cmd;
    Battery_Ret_u                       Ret;
	uint8_t							    TxPtr;
	uint8_t							    RxPtr;
	uint32_t							CommFailCnt;
	uint32_t							TrigCommTime;    
	uint32_t							LastCommTime;                  
}Battery_Handle_t;


void Battery_Init(Battery_Handle_t* Handle);

uint8_t Battery_YD_Trigger(Battery_Handle_t* Handle);
void Battery_YD_Update(Battery_Handle_t* Handle);
void Battery_YD_Decode(Battery_Handle_t* Handle);

void Battery_UpdateSpec(Battery_Handle_t* Handle);

uint16_t Battery_GetStatus(Battery_Handle_t* Handle);

Battery_Type_e Battery_GetType(Battery_Handle_t* Handle);
void Battery_SetType(Battery_Handle_t* Handle, Battery_Type_e type);

uint8_t Battery_GetNumOfCells(Battery_Handle_t* Handle);
void Battery_SetNumberOfCells(Battery_Handle_t* Handle, uint8_t num);

uint16_t Battery_GetDesignedVolt(Battery_Handle_t* Handle);
void Battery_SetDesignedVolt(Battery_Handle_t* Handle, uint16_t volt);

uint8_t Battery_GetSoc(Battery_Handle_t* Handle);
void Battery_SetSoc(Battery_Handle_t* Handle, uint8_t soc);

uint8_t Battery_GetPrevSoc(Battery_Handle_t* Handle);
void Battery_SetPrevSoc(Battery_Handle_t* Handle, uint8_t soc);

float Battery_GetRemainingCapacity(Battery_Handle_t* Handle);
void Battery_SetRemainingCap(Battery_Handle_t* Handle, float Rcap);

uint32_t Battery_GetAbsCapacity(Battery_Handle_t* Handle);
void Battery_SetAbsCapacity(Battery_Handle_t* Handle, uint32_t cap);

uint16_t Battery_GetVoltage(Battery_Handle_t* Handle);
void Battery_SetVoltage(Battery_Handle_t* Handle, uint16_t Voltage);

uint16_t Battery_GetCurrent(Battery_Handle_t* Handle); 
void Battery_SetCurrent(Battery_Handle_t* Handle, uint16_t Current);

uint16_t Battery_GetPower(Battery_Handle_t* Handle);
void Battery_SetPower(Battery_Handle_t* Handle, uint16_t Power);

uint16_t Battery_GetRemainingDistance(Battery_Handle_t* Handle);
uint16_t Battery_GetRemainingTime(Battery_Handle_t* Handle);

void Battery_SetTravelledTimeOffset(Battery_Handle_t* Handle, uint32_t Offset);
uint32_t Battery_GetTravelledTime(Battery_Handle_t* Handle);
void Battery_AddTravelledTime(Battery_Handle_t* Handle, uint32_t Increment);

uint8_t Battery_GetHighestTemp(Battery_Handle_t* Handle);
uint8_t Battery_GetLowestTemp(Battery_Handle_t* Handle);

void Battery_EstimateRemainingCapacity(Battery_Handle_t* Handle);



#endif  /* __EP_Battery_H__ */
