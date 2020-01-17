#ifndef __EP_BUTTON_H
#define __EP_BUTTON_H

#include "nrf.h"


#define BUTTON_1      7
#define BUTTON_2      //2
#define BUTTON_3     // 29
#define BUTTON_4     // 27

extern void button_init(void);

typedef enum
{
	BUTTON_State_Released,
	BUTTON_State_Releasing,
	BUTTON_State_Pressed,
	BUTTON_State_Pressing
}BUTTON_State_e;

typedef enum
{
	BUTTON_Action_push,
	BUTTON_Action_release
}BUTTON_Action_e;

typedef enum
{
	//BUTTON_Select_Button  =  BUTTON_2,
	//BUTTON_Power_Button   =  BUTTON_4,
	BUTTON_Menu_Button    =  BUTTON_1,
	//BUTTON_Ok_Button      =  BUTTON_3 
}BUTTON_Name_e;
typedef enum
{
	Touch_no,
	Touch_Short,
	Touch_Mid,
	Touch_Long
}BUTTON_touch_type_e;
typedef struct
{
	uint8_t              lock;
	BUTTON_Action_e		 action;
	BUTTON_State_e	  	 state;
	BUTTON_touch_type_e  touch_type;
	uint16_t			 pressCount;
}button_t;
typedef struct
{
	//button_t    	Power_Button;
	//button_t		Select_Button;
	button_t		Menu_Button;
	//button_t		Ok_Button;
}BUTTON_Handle_t;
extern BUTTON_Handle_t  BUTTON_Handle;

extern void BUTTON_UpdateKeyCounter(BUTTON_Handle_t* handle);
#endif 
