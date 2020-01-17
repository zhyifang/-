/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  Description*****
1.  Date:
    Author:
    Modification:

*  
*************************************/
#include <stdbool.h>
#include "nrf.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "pstorage.h"
#include "ep_pstorage.h"
#include "pstorage_platform.h"
#include "ep_gzll.h"
#include "ep_comm.h"
#include "nrf_delay.h"
#include "ep_motor.h"
#include "ep_gps.h"
#include "ep_TimeRecord.h"

PSTORAGE_Handle_t PSTORAGE_Handle;

volatile bool flagRest = false;
static pstorage_handle_t  base_handle;

uint8_t   pstorage_Wdata[16];
uint8_t   pstorage_Rdata[16];

bool PSTORAGE_Read_flagRest(void)
{
    return  flagRest;
}
typedef struct
{
	pstorage_ntf_cb_t  ps_cb_handler;                   
} cb_init;

void  PSTORAGE_set_Wflag(PSTORAGE_Handle_t* handle,bool flag)
{
	handle->write_flag = flag;
}
bool  PSTORAGE_read_Wflag(PSTORAGE_Handle_t* handle)
{
	return handle->write_flag;
}
void PSTORAGE_set_CurHandle(pstorage_handle_t ps_handle,uint32_t add)
{
	PSTORAGE_Handle.CurrentHandle =  ps_handle;
	PSTORAGE_Handle.handleAdd     =  add;
}

void PSTORAGE_DataUdate(void)
{
	//PSTORAGE_Read(&PSTORAGE_Handle.ps_unit_handle,UNIT_BLOCK_ADD,pstorage_Rdata);
	//LCD_Update_unit(&LCD_Handle,pstorage_Rdata);

	//PSTORAGE_Read(&PSTORAGE_Handle.ps_battery_handle,BATTERY_BLOCK_ADD,pstorage_Rdata);
	//BATTERY_Update_battery(&BATTERY_Handle,pstorage_Rdata);	
	
	//PSTORAGE_Read(&PSTORAGE_Handle.ps_gps_handle,GPS_BLOCK_ADD,pstorage_Rdata);
	//GPS_Dat_Update(&GPS_Handle,pstorage_Rdata);
	
	//PSTORAGE_Read(&PSTORAGE_Handle.ps_TimeRecord_handle,TIMERECORD_BLOCK_ADD,pstorage_Rdata);
	//TimeRecord_Update(&TimeRecord_Handle,pstorage_Rdata);
	
	PSTORAGE_Read(&PSTORAGE_Handle.ps_ang_handle,ANG_BLOCK_ADD,pstorage_Rdata);
	THROTTLE_Update_cal(&THROTTLE_Handle,pstorage_Rdata);
	
	PSTORAGE_Read(&PSTORAGE_Handle.ps_percent_max_handle,PERCENT_MAX_BLOCK_ADD,pstorage_Rdata);
	THROTTLE_Update_PercentMax(&THROTTLE_Handle,pstorage_Rdata);
	
	PSTORAGE_Read(&PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD,pstorage_Rdata);
	Gzll_Update_gzllAddr1(&GZLL_Handle,pstorage_Rdata);
}
void PSTORAGE_SaveData(PSTORAGE_Handle_t* handle)
{
	PSTORAGE_Erase(&handle->CurrentHandle,handle->handleAdd,false);		
	PSTORAGE_Write(&handle->CurrentHandle,handle->handleAdd,pstorage_Wdata);
	PSTORAGE_set_Wflag(handle,false);
	//befor close sys,save unit -> battery  dat ->save gps dat 
	
	#if 0
	//if(handle->CurrentHandle.block_id == handle->ps_unit_handle.block_id)// save bat dat  
	{
		BATTERY_Backup_battery(&BATTERY_Handle,pstorage_Wdata);
		PSTORAGE_set_CurHandle(handle->ps_battery_handle,BATTERY_BLOCK_ADD);
		PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
	}

	else if(handle->CurrentHandle.block_id == handle->ps_battery_handle.block_id)// save gps dat  
	{
		GPS_Dat_Backup(&GPS_Handle,pstorage_Wdata);
		PSTORAGE_set_CurHandle(handle->ps_gps_handle,GPS_BLOCK_ADD);
		PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
	}
	else if(handle->CurrentHandle.block_id == handle->ps_gps_handle.block_id)// save time record dat  
	{
		TimeRecord_Backup(&TimeRecord_Handle,pstorage_Wdata);
		PSTORAGE_set_CurHandle(handle->ps_TimeRecord_handle,TIMERECORD_BLOCK_ADD);
		PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
	}
	#endif
}
void ps_cb_handler(pstorage_handle_t *  p_handle,
                              uint8_t   op_code,
                              uint32_t  result,
                              uint8_t * p_data,
                              uint32_t  data_len)
{
    switch(op_code)
    {

       case PSTORAGE_STORE_OP_CODE:
            if (result == NRF_SUCCESS)
            {
               flagRest = true;
            }
            else
            {
               // Store operation failed.
				PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
            }
           // Source memory can now be reused or freed.
           break;
			
	   case PSTORAGE_UPDATE_OP_CODE:
            if (result == NRF_SUCCESS)
           {
               // Update operation successful.
           }
           else
           {
               // Update operation failed.
           }
           break;
		   
	    case PSTORAGE_CLEAR_OP_CODE:
           if (result == NRF_SUCCESS)
           {
               // Clear operation successful.
				 flagRest=true;
           }
           else
           {
               // Clear operation failed.
				 PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 
           }
           break;
    }
}


uint32_t PSTORAGE_Init(void)
{
	uint32_t err_code;

	PSTORAGE_Handle.write_flag = false;
	
	
	err_code = pstorage_init();
	if (err_code != NRF_SUCCESS)
	{
			return err_code;
	}
	
	pstorage_module_param_t param;  
	
	param.block_size  = BLOCK_SIZE;
	param.block_count = BLOCK_COOUNT;
	param.cb          = ps_cb_handler;
	
	err_code = pstorage_register(&param, &base_handle);
	
	err_code = pstorage_block_identifier_get(&base_handle, UNIT_BLOCK_ADD, &PSTORAGE_Handle.ps_unit_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = pstorage_block_identifier_get(&base_handle, BATTERY_BLOCK_ADD, &PSTORAGE_Handle.ps_battery_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = pstorage_block_identifier_get(&base_handle, GZLLADDR1_BLOCK_ADD,&PSTORAGE_Handle.ps_gzll_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
	err_code = pstorage_block_identifier_get(&base_handle, PERCENT_MAX_BLOCK_ADD,&PSTORAGE_Handle.ps_percent_max_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
	err_code = pstorage_block_identifier_get(&base_handle, ANG_BLOCK_ADD,&PSTORAGE_Handle.ps_ang_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = pstorage_block_identifier_get(&base_handle, GPS_BLOCK_ADD,&PSTORAGE_Handle.ps_gps_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
	err_code = pstorage_block_identifier_get(&base_handle, TIMERECORD_BLOCK_ADD,&PSTORAGE_Handle.ps_TimeRecord_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
	//PSTORAGE_Erase(&base_handle,0,true);//Erase ALL
	if (err_code != NRF_SUCCESS)
	{
			return err_code;
	}
	
	return NRF_SUCCESS;
}
uint32_t PSTORAGE_Erase(pstorage_handle_t* pstorage_handle ,uint32_t block_add,bool all)
{
	uint32_t  err_code; 
	if(all)
	{
		err_code = pstorage_clear(pstorage_handle, BLOCK_SIZE * BLOCK_COOUNT);
	}
	else
	{
		err_code = pstorage_clear(pstorage_handle, BLOCK_SIZE);
	}
	if (err_code == NRF_SUCCESS)
	{
		flagRest = false;
	}
	else
	{
			// Failed to request clear, take corrective action.
		return err_code;
	}
	while(flagRest == 0);
	return NRF_SUCCESS;
}

uint32_t PSTORAGE_Read(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *rData)
{
	uint32_t  err_code;
	
	err_code = pstorage_load(rData,pstorage_handle,BLOCK_SIZE,0);
	if (err_code == NRF_SUCCESS)
	{
		// Get Block Identifier successful.
		flagRest = false;
	}
	else
	{
		return err_code;
	}
	return NRF_SUCCESS;
}
uint32_t PSTORAGE_Write(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *wData)
{
	uint32_t  err_code;  
	err_code = pstorage_store(pstorage_handle,wData,BLOCK_SIZE,0);
	if (err_code == NRF_SUCCESS)
	{
			// Get Block Identifier successful.
			flagRest = false;
	}
	else
	{
			return err_code;
	}
	while(flagRest == 0);
	return err_code;
}
uint8_t PSTORAGE_CheckSum(uint8_t * data, uint8_t length)
{
	uint8_t sum = 0;
	uint8_t i = 0;
	for(i = 0; i < length; i++)
	{
		sum += data[i];
	}
	return sum;
}
//PSTORAGE_CheckWsuc()


