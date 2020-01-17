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
#include "ep_gzll.h"
#include "ep_comm.h"
#include "nrf_gzp.h"
#include "ep_crypt_device.h"
#include "ep_motor.h"
#include <string.h>
#include "ep_pstorage.h"
#include "main.h"


#define   PAIR_ADDRESS                   0xFEFEFEFE
#define   PAIR_POWER                     NRF_GZLL_TX_POWER_N20_DBM

#define   TIMESLOAT_PERIOD               900         //us ¡§a?¡ìD?????¡§o?¨¤?? = TIMESLOAT_PERIOD * timeslots_per_channel * channel_table_size
uint8_t   table[7] = {0x34,11,1,21,32,36,39};	
GZLL_Handle_t   GZLL_Handle;

void Gzll_ParamInit(GZLL_Handle_t* handle)
{
	  //TIMESLOAT_PERIOD * timeslots_per_channel * channel_table_size
	handle->config.timeslot_period            =  TIMESLOAT_PERIOD;
	handle->config.gzll_mode                  =  NRF_GZLL_MODE_DEVICE; 
	handle->config.timeslots_per_channel      =  2;                   
	handle->config.channel_table_size         =  3;
	handle->config.tx_power                   =  NRF_GZLL_TX_POWER_4_DBM;
	handle->config.data_rate                  =  NRF_GZLL_DATARATE_1MBIT;
	handle->config.max_tx_attempts            =  6;
	handle->config.base_address1              =  MAC1;
	handle->config.prefix_byte                =  PREFIX_BYTE;
	
	handle->GZLL_addr_change                  =  false;
	handle->addr_set_state                    =  addr_NoSet;
	handle->CommStatus                        =  CommStatus_Nomal;
	handle->PairTimeCountDownSec              =  PAIR_COUNTDOWN_SEC_MAX;//sec
	handle->fail_count                        =  0;
	handle->PairState                         =  PairIdleState;
}

void Gzll_ChannelTableSet(uint32_t addr)
{
    uint32_t  addr_temp;
	if(addr < 85)
	{
		addr_temp = addr + 100;
	} 
	else
	{
		addr_temp = (addr);
	}
	
	if(addr%3 == 1)
	{
		table[0] = (addr_temp % 75 ) + 2;
	
		table[1] =  (addr_temp % 45 ) + 2;
		if(table[1] == table[0])
		{
			table[1] =  (addr_temp % 70 ) + 2;
		}
	
		table[2] =  (addr_temp % 56 ) + 2;
		if(table[2] == table[0])
		{
			table[2] =  (addr_temp % 60 ) + 2;
		}
		if(table[2] == table[1])
		{
			table[2] =  (addr_temp % 50 ) + 2;
		}
	}
	else if(addr%3 == 0)
	{
		table[0] = (addr_temp % 71 ) + 2;
	    
		table[1] =  (addr_temp % 55 ) + 2;
	    if(table[1] == table[0])
		{
			table[1] =  (addr_temp % 68 ) + 2;
		}
		
		table[2] =  (addr_temp % 50 ) + 2;
		if(table[2] == table[0])
		{
			table[2] =  (addr_temp % 31 ) + 2;
		}
		if(table[2] == table[1])
		{
			table[2] =  (addr_temp % 46 ) + 2;
		}
	}
	else if(addr%3 == 2)//1
	{
		table[0] = (addr_temp % 67 ) + 2;
	    
		table[1] =  (addr_temp % 46 ) + 2;
	    if(table[1] == table[0])
		{
			table[1] =  (addr_temp % 52 ) + 2;
		}
		
		table[2] =  (addr_temp % 39 ) + 2;
		if(table[2] == table[0])
		{
			table[2] =  (addr_temp % 17 ) + 2;
		}
		if(table[2] == table[1])
		{
			table[2] =  (addr_temp % 27 ) + 2;
		}
	}
}
uint32_t address1_temp;
nrf_gzll_tx_power_t  power_temp;
void Gzll_Init(GZLL_Handle_t* handle)
{ 
	handle->PairBuzzer = false;
	nrf_gzll_init(handle->config.gzll_mode);        
	
	if(handle->PairState == PairIdleState)
	{
		address1_temp = handle->config.base_address1;
		power_temp    = handle->config.tx_power;
	}
	else if(handle->PairState == PairIntoState)
	{
		handle->PairState = PairingState;
		address1_temp = PAIR_ADDRESS;
		power_temp    = PAIR_POWER;
	}
	else if(handle->PairState == PairBackupState)
	{
		power_temp    = handle->config.tx_power;
		address1_temp = handle->config.base_address1;
		handle->PairState = PairIdleState;
	}
	else if(handle->PairState == PairSucState)
	{
		power_temp    = handle->config.tx_power;
		address1_temp = handle->config.base_address1;
	}
	
	nrf_gzll_set_address_prefix_byte(PIPE,handle->config.prefix_byte);
	nrf_gzll_set_tx_power(power_temp);                            //
	nrf_gzll_set_max_tx_attempts(handle->config.max_tx_attempts);
    nrf_gzll_set_datarate(handle->config.data_rate);                           //
	nrf_gzll_set_timeslot_period(handle->config.timeslot_period);              //  
	
	Gzll_ChannelTableSet(address1_temp);
    nrf_gzll_set_base_address_1(address1_temp);   

	
	nrf_gzll_set_channel_table(table,handle->config.channel_table_size);       //
	nrf_gzll_set_timeslots_per_channel(handle->config.timeslots_per_channel);  //
	nrf_gzll_set_xosc_ctl(NRF_GZLL_XOSC_CTL_MANUAL); 
	
	nrf_gzll_set_timeslots_per_channel_when_device_out_of_sync(handle->config.channel_table_size * handle->config.timeslots_per_channel);
	nrf_gzll_set_device_channel_selection_policy(NRF_GZLL_DEVICE_CHANNEL_SELECTION_POLICY_USE_SUCCESSFUL);

	nrf_gzll_enable();	                                                    //
}
CommStatus_e  Gzll_ReadCommStatus(void)
{
	return GZLL_Handle.CommStatus;
}
void  Gzll_SetCommStatus(CommStatus_e Status)
{
	GZLL_Handle.CommStatus = Status;
}
void  Gzll_SetPairState(PairState_e state)
{
	GZLL_Handle.PairState = state;
}
PairState_e Gzll_ReadPairState(void)
{
	return GZLL_Handle.PairState;
}
void  Gzll_PairTimeCountDown(void)
{
	static  uint8_t  TimeCount = 0;
	TimeCount++;
	if(TimeCount > 6)//6*180ms = 1080ms
	{
		TimeCount = 0;
		GZLL_Handle.PairTimeCountDownSec--;
		if(GZLL_Handle.PairTimeCountDownSec == 0)
		{
			GZLL_Handle.PairTimeCountDownSec = PAIR_COUNTDOWN_SEC_MAX;
		}
	}
}

uint8_t  Gzll_ReadCountDownSec(void)
{
    return GZLL_Handle.PairTimeCountDownSec;
}

void  Gzll_SetCountDownSec(uint8_t sec)
{
    GZLL_Handle.PairTimeCountDownSec = sec;
}

void Gzll_prefix_set(bool flag)
{
	GZLL_Handle.set_prefix_flag  = flag;	
}

void Gzll_SetTxData(uint8_t* txData,uint32_t data_len)
{
    memcpy(GZLL_Handle.tx_data.data, (void const*)txData, data_len);
	GZLL_Handle.tx_data.len = data_len; 
}

void Gzll_SetRxData(uint8_t* rxData,uint32_t data_len)
{
    memcpy(GZLL_Handle.rx_data.data, (void const*)rxData, data_len);
	GZLL_Handle.rx_data.len = data_len; 
}

void Gzll_AddrSet(uint32_t addr0)
{
	GZLL_Handle.GZLL_addr_change = true;
}

__INLINE void nrf_gzp_disable_gzll(void)
{
	if(nrf_gzll_is_enabled())
	{
		nrf_gzll_disable();
		__WFI();
		while(nrf_gzll_is_enabled());
	}
}

void Gzll_Disable(void)
{
//	   if(nrf_gzll_is_enabled())
//		 {
   	nrf_gzll_disable();
    
    // Wait for Gazell to shut down
//    while (nrf_gzll_is_enabled())
//    {
//        // Do nothing.
//    }
    
    // Clean up after Gazell.
    NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_DisableIRQ(TIMER2_IRQn); 
//	}		
}

void  nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
	bool ack;
	
	uint8_t  rx_data[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];  
	uint32_t len = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH; 
	ack = nrf_gzll_fetch_packet_from_rx_fifo(pipe, rx_data, &len);
    #if  DEBUG_MODE //debug mode 
	GZLL_Handle.fail_count = 0;
	#endif
	if( (true == ack))
	{
		
		Gzll_SetRxData(rx_data,len);
		Comm_ClearFailCnt(&Comm_Handle);
	}
	
}

void  nrf_gzll_device_tx_failed(uint32_t 	pipe, nrf_gzll_device_tx_info_t tx_info )
{
	#if DEBUG_MODE //debug mode 
	if(GZLL_Handle.fail_count++ > GZLL_Handle.fail_count_max)
	{
		GZLL_Handle.fail_count_max = GZLL_Handle.fail_count;
	}	
	#endif
}

void  nrf_gzll_host_rx_data_ready(uint32_t 	pipe, nrf_gzll_host_rx_info_t 	rx_info ){}
void Gzll_Backup_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data)
{
	*(data+0) = (uint8_t) ( (handle->config.base_address1 >> 16) & 0xFF );   
	*(data+1) = (uint8_t) ( (handle->config.base_address1 >>  8) & 0xFF ); 
	*(data+2) = (uint8_t) ( (handle->config.base_address1      ) & 0xFF ); 
    *(data+3) = PSTORAGE_CheckSum(data, 3);	
}
bool Gzll_Update_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data)
{
	if(data[3] == PSTORAGE_CheckSum(data, 3)) 
	{
	   handle->config.base_address1  = (uint32_t)(*(data+0) << 16 ) | (uint32_t)( *(data+1) << 8 ) | (uint32_t)( *(data+2));
       Gzll_SetAddrState(addr_HaveSet);		
	   return  true;
	}
	else
	{
	   Gzll_SetAddrState(addr_NoSet);
	}
	return  false;
}
void Gzll_SetAddrState(addr_set_state_e  state)
{
	GZLL_Handle.addr_set_state = state;
}
addr_set_state_e Gzll_ReadAddrState(GZLL_Handle_t* handle)
{
	return  handle->addr_set_state;
}
void  Gzll_SetGzllAddr1(uint32_t addr)
{
    GZLL_Handle.config.base_address1 = addr;   
}
uint32_t  Gzll_GetGzllAddr1(void)
{
    return (GZLL_Handle.config.base_address1);   
}	

void  nrf_gzll_disabled(void){}   

