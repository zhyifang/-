
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
#include "softdevice_handler.h"
#include "ep_ble_stack.h"
#include "ep_battery.h"
#include "ep_gps.h"
#include "ep_motor.h"
#include "ep_tick.h"

extern Battery_Handle_t Battery_Handle;
extern GPS_Handle_t Gps_Handle;
extern Motor_Handle_t Motor_Handle;

PSTORAGE_Handle_t PSTORAGE_Handle;
//PSTORAGE_Obj StoredTandCData;	

volatile bool flagRest = false;
static pstorage_handle_t  base_handle;

uint8_t   pstorage_Wdata[16];
uint8_t   pstorage_Rdata[16];

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
void PSTORAGE_set_CurHandle(pstorage_handle_t ps_handle,uint8_t add)
{
    PSTORAGE_Handle.CurrentHandle = ps_handle;
    PSTORAGE_Handle.handleAdd     =  add;
}



void MOTOR_Backup_PowerLimit(uint8_t* data)
{
    *(data+0) =  0xD0;
    *(data+1) =  0xD1;
	
//	   *(data+2) = (uint8_t) ( ((Comm_Handle.mot_power_lim) >> 8) & 0xFF );   
//	   *(data+3) = (uint8_t) ( (Comm_Handle.mot_power_lim) & 0xFF );  
}
//bool MOTOR_Update_PowerLimit(uint8_t* data)
//{
//    if((*(data+0) ==  0xD0) && (*(data+1) ==  0xD1))
//    {
//    //			   Comm_Handle.mot_power_lim = (uint16_t)(*(data+2) << 8 ) + (uint16_t)( *(data+3) );
//    return  true;			 
//    }
//    return  false;
//}
void PSTORAGE_DataUdate(void)
{

   PSTORAGE_Read(&PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD,pstorage_Rdata);
   GZLL_Update_gzllAddr1(&GZLL_Handle,pstorage_Rdata);

    
//    PSTORAGE_Read(&PSTORAGE_Handle.ps_MotCal_handle,MotCal_BLOCK_ADD,pstorage_Rdata);
//    Motor_CheckCalValue(&Motor_Handle, pstorage_Rdata);	  
}

void ps_cb_handler(pstorage_handle_t *  p_handle,
                              uint8_t              op_code,
                              uint32_t             result,
                              uint8_t *            p_data,
                              uint32_t             data_len)
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
               while(1);// Store operation failed.
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
	
	err_code = pstorage_block_identifier_get(&base_handle, GZLLADDR1_BLOCK_ADD,&PSTORAGE_Handle.ps_gzll_handle);
	if (err_code != NRF_SUCCESS)
	{
			return err_code;
	}

    err_code = pstorage_block_identifier_get(&base_handle, MotCal_BLOCK_ADD, &PSTORAGE_Handle.ps_MotCal_handle);
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
	  err_code = pstorage_load(rData ,pstorage_handle,BLOCK_SIZE,0);
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
uint32_t PSTORAGE_Write(pstorage_handle_t* pstorage_handle,uint32_t block_add,uint8_t *data)
{
	uint32_t          err_code;  
	err_code = pstorage_block_identifier_get(&base_handle, block_add, pstorage_handle);
	if (err_code != NRF_SUCCESS)
	{
			return err_code;
	}
	err_code = pstorage_store(pstorage_handle,data,BLOCK_SIZE,0);
	if (err_code == NRF_SUCCESS)
	{
			// Get Block Identifier successful.
			flagRest = false;
	}
	else
	{
			return err_code;
	}
	while(flagRest == false);
	return err_code;
}
static  volatile bool EraseFlag = false;	
void PSTORAGE_Save(void)
{
    if(PSTORAGE_read_Wflag(&PSTORAGE_Handle) == true)
    {   //storage gazell address /  batter type and soc/motor power limit
        //befor use pstorage need to disable gazell
        if(!EraseFlag)
        {
            EraseFlag = true;
            Gzll_Disable();//212us
        	NVIC_DisableIRQ(ADC_IRQn);           
            ble_stack_init();//662us
            PSTORAGE_Erase(&PSTORAGE_Handle.CurrentHandle ,PSTORAGE_Handle.handleAdd,false);
        }
        if(flagRest == true)
        {
            EraseFlag = false;         
            PSTORAGE_set_Wflag(&PSTORAGE_Handle,false);
            PSTORAGE_Write(&PSTORAGE_Handle.CurrentHandle,PSTORAGE_Handle.handleAdd,pstorage_Wdata);
            PSTORAGE_Read(&PSTORAGE_Handle.CurrentHandle,PSTORAGE_Handle.handleAdd,pstorage_Rdata);
            softdevice_handler_sd_disable();//276us
            Gzll_NomalModeConfig(&GZLL_Handle);; //952us
        	  NVIC_EnableIRQ(ADC_IRQn);
        }		
    }
}

