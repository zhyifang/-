#ifndef EP_LCD_H__
#define EP_LCD_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "ep_throttle.h"
#include "ep_battery.h"
#include "ep_sys.h"
#include "ep_lcd_6kw.h"
#include "ep_version.h"
#include "ep_led.h"

typedef enum
{
	VOLTAGE12V = 1,
	VOLTAGE24V = 2,
	VOLTAGE36V = 3,
	VOLTAGE48V = 4,
}Supply_Voltage_e;

typedef enum
{
	WirelessPairModeManual = 0,
	WirelessPairModeAuto
}LCD_WirelessPairMode_e;

typedef enum
{
	LCD_Menu_Main,            //0；lcd main menu 
	LCD_Menu_MotorCalSet,     //1；battery cap/type/voltage setting menu
	LCD_Menu_MiddlePosition,  //2；中位点
	LCD_Menu_ClearErr,        //3；故障清除
	LCD_Menu_HandleSide,      //4；无线连接
	LCD_Menu_GzllSet,         //5；gazell addr setting menu 
	LCD_Menu_ThCal,           //6；Throttle  calibration setting menu
	LCD_Menu_SettingPage,     //7；
}LCD_Menu_e;

typedef struct
{ 
	uint32_t         GzllAddrTemp;	
}LCD_SetDataTemp_t;

typedef struct
{ 
	bool                  DisplayAllFlag;
	uint8_t               Menu_index;	
	uint8_t               flickCount;

	LCD_Menu_e            Menu;
	LCD_SetDataTemp_t     SetDataTemp;

	uint8_t 			  CountDownFlag;
}LCD_Handle_t;

extern  LCD_Handle_t  LCD_Handle;

void LCD_Init(LCD_Handle_t* handle);

LCD_Menu_e LCD_GetMenu(void);

void  LCD_SetMenu(LCD_Menu_e menu);
void LCD_GzllPairCountDown(LCD_Handle_t* handle);
void LCD_Next_Menu_Config(LCD_Handle_t* handle);

#endif /* EP_LCD_H__ */
