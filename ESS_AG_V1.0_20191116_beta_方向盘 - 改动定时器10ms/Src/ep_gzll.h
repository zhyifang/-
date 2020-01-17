#ifndef __EP_GZLL_H
#define __EP_GZLL_H

#include "nrf_gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_gzll.h"

#define   PIPE                     5
#define   MAC1                     1702001 //the MSB of the address  should not be 0x55 or 0xAA
#define   PREFIX_BYTE              0x77  

#define   PAIR_COUNTDOWN_SEC_MAX   60//SEC

typedef struct 
{
    uint32_t  len;
	uint8_t   data[16];
}data_pack_t;
typedef enum
{
    addr_HaveSet  = 0x01,
	addr_NoSet
} addr_set_state_e;
typedef enum 
{
	CommStatus_Nomal = 0x11,
	CommStatus_AutoPair,
}CommStatus_e;
typedef enum 
{
	PairIdleState = 0x21,
	PairIntoState,
	PairingState,
	PairTimeoutState,
	PairSucState,
	PairBackupState,
}PairState_e;
typedef struct 
{
	uint8_t              prefix_byte;
	uint32_t             base_address1;
	uint8_t              max_tx_attempts;
	uint8_t              timeslots_per_channel;   //每个channle 所含有的Timeslot数量
	uint8_t              channel_table_size;      //跳频频道数量
	uint16_t             timeslot_period;         //Timeslots use by the Host and by the Device when communication is in sync
	nrf_gzll_mode_t      gzll_mode;               //gazell 模式 主机/从机
	nrf_gzll_tx_power_t  tx_power;                //TX发射功率  4dBm /0 / -4 ....
	nrf_gzll_datarate_t  data_rate;               //数据发送速率 250KBPS / 1M /2M
	
}Gzll_Config_t;
typedef struct 
{
	 Gzll_Config_t        config;
	

	 volatile  bool       GZLL_addr_change;		
	 volatile  uint8_t    GzllRecDataFlag;
	 volatile  bool       set_prefix_flag;
	
	 uint16_t             tx_fail_count;
	 uint16_t             tx_fail_count_min;
	 uint16_t             gzll_rssi;
	 uint16_t             gzll_addr1;
	 
	 data_pack_t          rx_data;
	 data_pack_t          tx_data;
	 uint16_t             fail_count_sec;
	 uint16_t             fail_count_sum;
	 uint16_t             fail_count_max;
	 uint8_t              fail_count;
	 
	 addr_set_state_e     addr_set_state;
	 
	 CommStatus_e         CommStatus;
	 uint8_t              PairTimeCountDownSec;    
     PairState_e          PairState;
	 uint8_t 			PairBuzzer;
}GZLL_Handle_t;

extern  GZLL_Handle_t  GZLL_Handle;
void    Gzll_init(GZLL_Handle_t* Handle);
extern  void  Gzll_ParamInit(GZLL_Handle_t* handle);
extern  void  Gzll_Init(GZLL_Handle_t* Handle);          //gzll 模块初始化  
extern  void  Gzll_Disable(void);
extern  void  Gzll_Backup_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data);
bool    Gzll_Update_gzllAddr1(GZLL_Handle_t* handle,uint8_t* data);
void    Gzll_SetGzllAddr1(uint32_t addr);
extern  uint32_t   Gzll_GetGzllAddr1(void);
extern  void  Gzll_AddrSet(uint32_t addr0);
void    Gzll_SetTxData(uint8_t* txData,uint32_t data_len);
void    Gzll_ChannelTableSet(uint32_t addr);
void    Gzll_prefix_set(bool flag);
void    Gzll_SetAddrState(addr_set_state_e  state);
addr_set_state_e Gzll_ReadAddrState(GZLL_Handle_t* handle);
uint32_t  Gzll_AddrCheck(uint32_t address);
CommStatus_e  Gzll_ReadCommStatus(void);
void  Gzll_SetCommStatus(CommStatus_e Status);
void  Gzll_PairTimeCountDown(void);
uint8_t  Gzll_ReadCountDownSec(void);
void  Gzll_SetCountDownSec(uint8_t sec);
void  Gzll_SetPairState(PairState_e state);
PairState_e Gzll_ReadPairState(void);
#endif

