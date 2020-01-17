/***************************
* 文件名称: gzll.c
* 文件功能: GAZELL 
*
******************************/
#include "ep_gzll.h"
#include "ep_comm.h"
#include "ep_crypt_host.h" 
#include "ep_battery.h"
#include "main.h"
#include "ep_pstorage.h"


#define   DEFAULT_ADDRESS          12345	

#define   PREFIX_BYTE              0x77


GZLL_Handle_t  GZLL_Handle;
extern Battery_Handle_t Battery_Handle;

uint8_t  table[7] = {0x34,11,1,21,32,36,39};	//通信频道表

/************************
* 函数名称: gzll_param_init
* 函数功能：gazell 初始化设置参数
*
***************************/
void GZLL_param_init(GZLL_Handle_t* handle)
{
	//通信间隔时间 = TIMESLOAT_PERIOD * timeslots_per_channel * channel_table_size
	handle->config.timeslot_period        =  TIMESLOAT_PERIOD;
	handle->config.gzll_mode              =  NRF_GZLL_MODE_HOST; 
	handle->config.timeslots_per_channel  =  2;                   
	handle->config.channel_table_size     =  3;
	handle->config.tx_power               =  NRF_GZLL_TX_POWER_4_DBM;
	handle->config.data_rate              =  NRF_GZLL_DATARATE_1MBIT;
	handle->config.max_tx_attempts        =  4;
	handle->config.base_address1          =  DEFAULT_ADDRESS;
	handle->config.prefix_byte            =  PREFIX_BYTE;
	handle->GzllCommFailCount      =  0;
	handle->gzll_CommState         =  GZLL_State_Disconnected;
	handle->set_prefix_flag        =  false;

	handle->CommStatus             =  CommStatus_Nomal;     
}
/************************
* 函数名称: gzll_init
* 函数功能：gazell 初始化
*
***************************/

void Gzll_NomalModeConfig(GZLL_Handle_t* handle)
{
	GZll_ChannelTableSet(handle->config.base_address1);
	nrf_gzll_init(handle->config.gzll_mode);                                   //主从模式设置
	nrf_gzll_set_base_address_1(handle->config.base_address1);                 //地址设置
	nrf_gzll_set_address_prefix_byte(PIPE,handle->config.prefix_byte);
	nrf_gzll_set_tx_power(handle->config.tx_power);                            //发射功率

	nrf_gzll_set_datarate(handle->config.data_rate);                           //数据速率
	nrf_gzll_set_timeslot_period(handle->config.timeslot_period);              //timeslot
	nrf_gzll_set_channel_table(table,handle->config.channel_table_size);       //跳频表和跳频通道数
	nrf_gzll_set_timeslots_per_channel(handle->config.timeslots_per_channel);  // 

	nrf_gzll_set_rx_pipes_enabled(1<<PIPE);
	
	nrf_gzll_enable();	                                                       //使能gazell模块
}
void  Gzll_PairModeConfig(GZLL_Handle_t* handle)
{	
	GZll_ChannelTableSet(PAIR_ADDRESS);
	nrf_gzll_init(handle->config.gzll_mode);                                   //主从模式设置
	nrf_gzll_set_base_address_1(PAIR_ADDRESS);                                 //地址设置
	nrf_gzll_set_address_prefix_byte(PIPE,handle->config.prefix_byte);
	nrf_gzll_set_tx_power(PAIR_POWER);                                         //发射功率

	nrf_gzll_set_datarate(handle->config.data_rate);                           //数据速率
	nrf_gzll_set_timeslot_period(handle->config.timeslot_period);              //timeslot
	nrf_gzll_set_channel_table(table,handle->config.channel_table_size);       //跳频表和跳频通道数
	nrf_gzll_set_timeslots_per_channel(handle->config.timeslots_per_channel);  // 

	nrf_gzll_set_rx_pipes_enabled(1<<PIPE);
	
	nrf_gzll_enable();	
}
void GZll_ChannelTableSet(uint32_t addr)
{

    uint32_t  addr_temp;
	if(addr < 85)
	{
		addr_temp = addr + 100;
	} 
	else
	{
		addr_temp = addr;
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
uint8_t  dat_byte[1];
bool  flag = false;
void Gzll_set_prefix_byte(uint8_t byte)
{
	if(GZLL_Handle.set_prefix_flag)
	{
		nrf_gzll_disable();
		// Wait for Gazell to shut down
		while (nrf_gzll_is_enabled());
//        nrf_gzll_set_base_address_1(GZLL_Handle.base_address1);    		
//		nrf_gzll_set_address_prefix_byte(PIPE,byte);
		(void)nrf_gzll_enable();   
		GZLL_Handle.set_prefix_flag  = false;
	}
	
}
void Gzll_ResetAddr(uint32_t addr)
{
	nrf_gzll_disable();
	while (nrf_gzll_is_enabled());
	
	nrf_gzll_set_base_address_1(addr);
	
	nrf_gzll_enable();	
}
void Gzll_prefix_set(bool flag)
{
	GZLL_Handle.set_prefix_flag  = flag;
}
void Gzll_Disable(void)
{
   	nrf_gzll_disable();
    // Wait for Gazell to shut down
    while (nrf_gzll_is_enabled());
	NVIC_DisableIRQ(RADIO_IRQn);
    NVIC_DisableIRQ(TIMER2_IRQn);  
}


void  Gzll_PairIntoCheck(void)
{
	static uint16_t  CheckTimeCount = 0;
	//30ms * 33 
	if(CheckTimeCount > 300) return;	
	CheckTimeCount++;
	if(CheckTimeCount > 100)
	{
		//if after open  1S,the sys is disconnect,the sys will be into pair status
		if(GZLL_Handle.gzll_CommState == GZLL_State_Disconnected )
		{
			CheckTimeCount = 350;		
			GZLL_Handle.CommStatus =  CommStatus_AutoPair; 	
		    Gzll_PairModeConfig(&GZLL_Handle);			
		}
	}
	if(GZLL_Handle.gzll_CommState == GZLL_State_Connected )
	{
		CheckTimeCount = 350;		
	}
}
void  Gzll_TimeOutCheck(void)
{
	static uint16_t  TimeOutCount = 0;
	if(GZLL_Handle.CommStatus ==  CommStatus_AutoPair)
	{
		TimeOutCount++;
		if(TimeOutCount > 333)      //333*30ms = 10s
		{
			GZLL_Handle.CommStatus =  CommStatus_Nomal;
			Gzll_NomalModeConfig(&GZLL_Handle);
		}
	}
}
void  Gzll_CommFailCheck(void)
{
	if(Comm_Handle.Mode == Comm_Mode_Gzll)
	{
		if(GZLL_Handle.GzllCommFailCount < DISCONNECT_CNT_MAX)
		{
			GZLL_Handle.GzllCommFailCount ++;
		}
		else //100*30 = 3s 在解密无线数据后清零
		{
			Gzll_commState_set(GZLL_State_Disconnected);
			
			Comm_Handle.State = Comm_State_Disconnected;
			Comm_Handle.MotorState = Comm_MotorState_Off;
			Comm_Handle.Mode = Comm_Mode_RS485;
			Comm_Handle.MotorRpm = 0;
			Comm_Handle.MotorPower = 0;
			Comm_Handle.Remote485CommFailCount = 0;
           //Buzzer_OnEn(&Buzzer_Handle);		//测试无线断开	
		}
	}
	
}
void Gzll_CommFailCountClear(void)
{
	GZLL_Handle.GzllCommFailCount = 0;
}
void Gzll_commState_set(GZLL_CommState_e state)
{
	GZLL_Handle.gzll_CommState = state;
}
GZLL_CommState_e  Gzll_commState_read(void) 
{
	return GZLL_Handle.gzll_CommState;
}
/*0************************
*  函数名称：  nrf_gzll_host_rx_data_ready
*  函数功能：  host设备发送数据收到主机ACK回复后的回调函数
*
**************************/	
void  nrf_gzll_host_rx_data_ready(uint32_t 	pipe, nrf_gzll_host_rx_info_t 	rx_info )
{
    uint8_t   rx_data[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];
    uint32_t  len = NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH;
    bool ack;
    
	GZLL_Handle.rssi = -rx_info.rssi;
    //Comm_Handle.Mode = Comm_Mode_Gzll;
    ack = nrf_gzll_fetch_packet_from_rx_fifo(pipe,rx_data,&len);
    if(true == ack)
    {    
    	if(Comm_Mode_Gzll == Comm_Handle.Mode)
		{
	        CRYPT_RxData_Decode(rx_data);   //无线接收的数据解码
	        Comm_Decode(&Comm_Handle);      //
		}
    } 
}
/*************************
*  函数名称：  nrf_gzll_device_tx_success
*  函数功能：  device设备发送数据收到主机ACK回复后的回调函数
*              
**************************/
void  nrf_gzll_device_tx_success(uint32_t pipe, nrf_gzll_device_tx_info_t tx_info)
{
}
/*************************
*  函数名称：  nrf_gzll_device_tx_success
*  函数功能：  device设备发送数据 未收到主机ACK回复的回调函数
*
**************************/
void  nrf_gzll_device_tx_failed(uint32_t 	pipe, nrf_gzll_device_tx_info_t tx_info ){}
/*************************
*  函数名称：  nrf_gzll_disabled
*  函数功能：  gazell disable 后的回调函数
*
**************************/	
void  nrf_gzll_disabled(void)
{
}
void GZLL_Backup_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data)
{
	*(data+0) =  0xC0;
	*(data+1) =  0xC1;

	*(data+2) = (uint8_t) ( (handle->config.base_address1 >> 16) & 0xFF );   
	*(data+3) = (uint8_t) ( (handle->config.base_address1 >>  8) & 0xFF ); 
	*(data+4) = (uint8_t) ( (handle->config.base_address1      ) & 0xFF ); 	 
}
bool GZLL_Update_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data)
{
	if((*(data+0) ==  0xC0) && (*(data+1) ==  0xC1))
	{
		handle->config.base_address1  = (uint32_t)(*(data+2) << 16 ) | (uint32_t)( *(data+3) << 8 ) | (uint32_t)( *(data+4)); 	
		return  true;
	}
	else
	{
		GZLL_Set_gzllAddr1(&GZLL_Handle,mcu_id);
		PSTORAGE_set_CurHandle(PSTORAGE_Handle.ps_gzll_handle,GZLLADDR1_BLOCK_ADD);
		PSTORAGE_set_Wflag(&PSTORAGE_Handle,true); 	
		GZLL_Backup_gzllAddr1(&GZLL_Handle,pstorage_Wdata);
	}
	return  false; 
}
void  GZLL_Set_gzllAddr1(GZLL_Handle_t* handle,uint32_t addr)
{
    handle->config.base_address1 = addr;   
}
uint32_t  GZLL_Get_gzllAddr1(GZLL_Handle_t* handle)
{
    return  handle->config.base_address1;   
}
uint8_t Gzll_ReadRssi(void)
{
	return GZLL_Handle.rssi;
}

