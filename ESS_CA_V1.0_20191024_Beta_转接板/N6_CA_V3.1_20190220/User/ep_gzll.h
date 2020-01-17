#ifndef __EP_GZLL_H
#define __EP_GZLL_H

#include "nrf_gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gzll.h"

#define  TIMESLOAT_PERIOD         900         //us 通信间隔时间 = TIMESLOAT_PERIOD * timeslots_per_channel * channel_table_size
#define  PIPE                     5

#define  PAIR_ENABLE_RSSI         85                         //in pair mode if rssi <  PAIR_ENABLE_RSSI,will enable pair with handle
#define  PAIR_ADDRESS             0xFEFEFEFE                 //pair mode gazell address
#define  PAIR_POWER               NRF_GZLL_TX_POWER_N20_DBM  //pair mode tx power

typedef enum 
{
	GZLL_State_Disconnected,
	GZLL_State_Connected,
}GZLL_CommState_e;
typedef enum 
{
	CommStatus_Nomal,
	CommStatus_AutoPair,
}CommStatus_e;
typedef struct 
{
    uint8_t              max_tx_attempts;
	uint16_t             timeslot_period;         //Timeslots use by the Host and by the Device when communication is in sync
	nrf_gzll_mode_t      gzll_mode;               //gazell 模式 主机/从机
	uint8_t              timeslots_per_channel;   //每个channle 所含有的Timeslot数量
	uint8_t              channel_table_size;      //跳频频道数量
	nrf_gzll_tx_power_t  tx_power;                //TX发射功率  4dBm /0 / -4 ....
	nrf_gzll_datarate_t  data_rate;               //数据发送速率 250KBPS / 1M /2M
	uint32_t             base_address1;
	uint8_t              prefix_byte;
}gzll_config_t;

typedef struct 
{
    gzll_config_t        config;
	
	uint16_t             GzllCommFailCount;
	
	
	uint8_t              Address[4];
	volatile bool        set_prefix_flag;
	GZLL_CommState_e     gzll_CommState;
	
	CommStatus_e         CommStatus;
	
	uint8_t              rssi;
}GZLL_Handle_t;

extern GZLL_Handle_t  GZLL_Handle;

void  Gzll_commState_set(GZLL_CommState_e state);
GZLL_CommState_e  Gzll_commState_read(void); 
void  Gzll_CommFailCountClear(void);
void  GZLL_param_init(GZLL_Handle_t* handle);
bool  GZLL_ReadDisableFlag(void);
void  GZLL_Backup_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data);
bool  GZLL_Update_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data);
void  GZLL_Set_gzllAddr1(GZLL_Handle_t* handle,uint32_t addr);
uint32_t  GZLL_Get_gzllAddr1(GZLL_Handle_t* handle);
extern  void  Gzll_NomalModeConfig(GZLL_Handle_t* Handle);          //gzll 模块初始化  
extern  void  Gzll_Disable(void);
void  GZLL_AddrSet(void);
void  Gzll_CommFailCheck(void);
void  GZll_ChannelTableSet(uint32_t addr);
void  Gzll_set_prefix_byte(uint8_t byte);
void  Gzll_prefix_set(bool flag);
void  Gzll_ResetAddr(uint32_t addr);
void  Gzll_PairIntoCheck(void);
void  Gzll_TimeOutCheck(void);
void  Gzll_PairModeConfig(GZLL_Handle_t* handle);
void  Gzll_NomalModeConfig(GZLL_Handle_t* handle);
uint8_t  Gzll_ReadRssi(void);
#endif
