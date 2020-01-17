#ifndef __EP_TEMP_H_
#define __EP_TEMP_H_

#include <stdint.h>
#include "ep_misc.h"
#include "main.h"

typedef enum {B_3435, B_3975}B_Value;                                                                                                                                                          

typedef struct {
	QuickAvg_t MovingAvgStruct;
	int16_t Temperature;
	uint8_t ThrottlePercentage;
	uint16_t Vol;
} Temperature_t;
 
#define MOT_OT_BIT	            0x01  //motor OT protect flag bit
#define MOS1_OT_BIT	            0x02  //power mos protect flag bit
#define MOS2_OT_BIT	            0x04  //driver mos protect flag bit

#define HIGH_TEMP_POWER_LIMIT	70      //limit to 70% output power ,the max limit power = 70% * (6000w/3000w),The value must be 10 multiple
#define POWER_LIMIT_FACTOR   	((100-HIGH_TEMP_POWER_LIMIT)/10)       //(100-HIGH_TEMP_POWER_LIMIT)/10

typedef enum
{
	TMOT,        //motor temperature  temp_row1
	TMOS_B,      //NTC 10K  B phase             temp1_row2									 
	TMOS_P1,     //NTC 10K  power	  temp_row4
	TMOS_A,      //NTC 10K  A phase   temp_row3
	TMOS_P2L,    //LMT87    power	            temp1_row4
	TMOS_CL,	 //LMT87    C phase             temp1_row3
	VDD,         //Voltage  3.3v
	V15V,        //Voltage  15V
	TMOTNEW,     //motor temperature new , receive from control board  temp1_row1
}Temp_Index_e;

extern Temp_Index_e  MotProtectTempIndex,PowMosProtectTempIndex,DrvMosProtectTempIndex;

void Temp_Init(void);
void Temp_TriggerAdcSampling(void);
void Temp_Update(void);
int16_t Temp_Get(uint8_t channel);         //return the temperature for channel with precision 0.1degree
void Temp_Set(uint8_t channel,int16_t temp);
uint8_t Temp_IsOverheated(void);            //return 1 if any overheat events occur
void TempProtectValueCheck(void);

#endif  /*__EP_TEMP_H_*/
