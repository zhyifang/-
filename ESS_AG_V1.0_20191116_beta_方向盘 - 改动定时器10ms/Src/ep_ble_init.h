#ifndef __EP_BLE_INIT_H
#define __EP_BLE_INIT_H

#include "nrf.h"
#include "nrf51.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "ep_ble_init.h"
#include "ble_nus.h"



#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            4                                           /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */


#define EP_BLE_BASE_UUID              {{0x40, 0x42, 0x69, 0x13, 0x62, 0xdf, 0x21, 0x7f, 0x49, 0xa6, 0x27, 0xf3, 0x00,  0x00, 0x7a,  0xcc,  }}

#define BLE_MAX_CHAR_LEN              (GATT_MTU_SIZE_DEFAULT - 3)

//extern  ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */



extern volatile  uint8_t  timeout_flag;
extern volatile  uint8_t  ble_evt_id;
typedef enum
{
    BLE_Adv = 0,
	BLE_Connected,
	BLE_Disconnected,
}BleStatus_e;
typedef enum
{
    char_read_en,
	char_write_en,
	char_r_w_en
}char_r_d_e;

typedef enum
{
	EP_ACCESS_NO_ACCESS = 0x0E,
	EP_ACCESS_NORMAL_ACCESS,
	EP_ACCESS_ADMIN_ACCESS,
	EP_ACCESS_ERROR_KEY
} Permission_Level_e;
typedef enum
{
   AdvEnable,
   AdvDisable,
} ble_AdvEnStatus_e;
typedef  struct 
{
	
	volatile bool  notif_en;
	Permission_Level_e  perm_level;
	char_r_d_e     char_r_d;
	uint8_t        *p_value; 
	uint16_t       char_val_max_len;
	uint16_t       uuid;
	ble_gatts_char_handles_t  handles;
}ble_char_t;

typedef struct
{  
	ble_AdvEnStatus_e   AdvEnStatus;
    
	volatile uint8_t    app_time_out_flag;
    
    volatile uint8_t    app_time_out_flag_ms;
	volatile uint8_t    sysLcdRefresh;
	BleStatus_e         status; 
	
}BLE_Handler_t;

extern BLE_Handler_t BLE_Handler;

extern void send_dat(void);
extern void BLE_Init(void);
extern void power_manage(void);
void ble_data_update(void);
void BLE_SetStatus(BleStatus_e status);
bool  ble_notif_en_check(ble_gatts_evt_write_t * p_evt_write,ble_char_t * ble_char);
BleStatus_e  BLE_ReadStatus(BLE_Handler_t* handle);
void   ble_data_update1(void);
void Conver_num_16(uint16_t num,uint8_t *p_data);
void Conver_num_32(uint32_t num,uint8_t *p_data);
void gap_params_update(uint16_t m_conn_handle);

#endif 

