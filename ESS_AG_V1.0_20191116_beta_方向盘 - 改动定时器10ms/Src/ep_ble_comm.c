#include "nrf.h"
#include "nrf51.h"
#include "stdbool.h"
#include "ep_ble_init.h"
#include "ep_ble_comm.h"

Ble_Comm_handle_t  Ble_Comm_handle;

void BLE_COMM_PrepareData(Ble_Comm_handle_t* handle,ble_char_t tx_char,tx_type_e tx_type,uint32_t tx_data,uint8_t tx_data_len)
{
	handle->tx_flag     = true;
	handle->tx_char     = tx_char;
	handle->tx_type     = tx_type;
	handle->tx_data     = tx_data;
	handle->tx_dat_len  = tx_data_len;
	
}

void BLE_COMM_TxData(Ble_Comm_handle_t* handle)
{
	
	switch(handle->tx_type)
	{
		case Motor_Voltage_type:
			 break;
		case Motor_Current_type:
			 break;
		case Motor_Power_type:
			 break;
		case Motor_RPM_type:
			 break;
		case Motor_Temps_type:
			 break;
		case Motor_Status_type:
			 break;
		case Battery_Remaining_Cap_type:
			 break;
		case Battery_Voltage_type:
			 break;
		case Battery_Current_type:
			 break;
		case Battery_Power_type:
			 break;
		case Battery_Temps_type:
			 break;
		case Battery_Status_type:
			 break;
		case Remain_TimeAndMile_type:
			 break;
		case Gps_Speed_type:
			 break;
		default:break;
	}
}

