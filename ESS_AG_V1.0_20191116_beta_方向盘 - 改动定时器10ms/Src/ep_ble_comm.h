#ifndef  __EP_BLE_COMM_H
#define  __EP_BLE_COMM_H

#include "ep_ble_init.h"

typedef enum
{
	Permission_Level_type = 0x01,
	
	Motor_Voltage_type,
	Motor_Current_type,
	Motor_Power_type,	
	Motor_RPM_type,
	Motor_Temps_type,
	Motor_Status_type,
	
	Battery_Remaining_Cap_type,
	Battery_Voltage_type,
	Battery_Current_type,	
	Battery_Power_type,	
	Battery_Temps_type,
    Battery_Status_type,
	
	Remain_TimeAndMile_type,
	Gps_Speed_type,
	
	
	
}tx_type_e;

typedef  struct
{
	volatile  bool         tx_flag;
	tx_type_e              tx_type;
	ble_char_t             tx_char;
    uint8_t                tx_dat_len;
	uint32_t               tx_data;
	
}Ble_Comm_handle_t;

extern Ble_Comm_handle_t  Ble_Comm_handle;

#endif



