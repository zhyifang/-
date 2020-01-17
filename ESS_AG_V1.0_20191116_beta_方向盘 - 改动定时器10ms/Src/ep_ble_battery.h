#ifndef  __EP_BLE_BATTERY_H
#define  __EP_BLE_BATTERY_H

#include  "nrf.h"
#include  "ble.h"
#include  "ble_srv_common.h"
#include  <stdint.h>
#include  <stdbool.h>
#include  "ep_ble_init.h"
#define EP_BATTERY_SERVICE_UUID          0x4000 /*BATTERY Control Service*/

#define EP_BATTERY_AVAILABLE_UUID        0x4001 /*Service Available Mask*/
#define EP_BATTERY_BAT_NUM_UUID          0x4002 /*Number of Batteries*/
#define EP_BATTERY_REMAIN_CAP_UUID       0x4003 /*Remaining Capacity*/
#define EP_BATTERY_VOL_UUID              0x4004 /*Battery Voltage*/
#define EP_BATTERY_CUR_UUID              0x4005 /*Battery Current*/
#define EP_BATTERY_POW_UUID              0x4006 /*Battery Power*/
#define EP_BATTERY_MAX_CUR_SET_UUID      0x4007 /*Max Current Set*/
#define EP_BATTERY_MIN_VOL_SET_UUID      0x4008 /*Min Voltage Set*/
#define EP_BATTERY_TEMP_NUM_UUID         0x4009 /*Number of temp sensors*/
#define EP_BATTERY_TEMP_UUID             0x400a /*Temps for battery*/
#define EP_BATTERY_STATUS_UUID           0x400b /*Status*/
#define EP_BATTERY_TEST_DATA1_UUID       0x400c /*Permission Level*/
#define EP_BATTERY_TEST_DATA2_UUID       0x400d /*Permission Level*/
#define EP_BATTERY_TEST_DATA3_UUID       0x400e /*Permission Level*/
#define EP_BATTERY_TEST_DATA4_UUID       0x400f /*Permission Level*/

#define BATTERY_NUM                      5
#define BATTERY_TEMP_NUM                 2

typedef struct  ble_battery_s        ble_battery_t;
extern  ble_battery_t                m_battery;
typedef void (*ble_battery_data_handler_t) (ble_battery_t * p_BATTERY, uint8_t * p_data, uint16_t length);

struct ble_battery_s
{
    uint8_t                     uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                    service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */ 
	
	ble_char_t                  char_available_mask;     /*Service Available Mask*/
    ble_char_t                  char_bat_num;            /*Number of Batteries*/
    ble_char_t                  char_remain_cap; 		 /*Remaining Capacity*/
	ble_char_t                  char_vol;                /*Battery Voltage*/
	ble_char_t                  char_cur;                /*Battery Current*/
	ble_char_t                  char_pow;                /*Battery Power*/
	ble_char_t                  char_max_cur_set;        /*Max Current Set*/
	ble_char_t                  char_min_vol_set;        /*Min Voltage Set*/
	ble_char_t                  char_temp_num;           /*Number of temp sensors*/
	ble_char_t                  char_temp_value;         /*Temps for battery*/
	ble_char_t                  char_status;             /*Status*/
	ble_char_t                  char_testData1; 
	ble_char_t                  char_testData2;
	ble_char_t                  char_testData3; 
	ble_char_t                  char_testData4;
	
	uint8_t                     bat_num_data[1];
	uint8_t                     temp_num_data[1];
	uint16_t                    conn_handle;             /**< Handle of the char_current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_battery_data_handler_t  data_handler;            /**< Event handler to be called for handling received data. */

};

void     BATTERY_ble_init(ble_battery_t * p_battery);
uint32_t BATTERY_service_init(ble_battery_t * p_battery);
void     ble_battery_on_ble_evt(ble_battery_t * p_battery, ble_evt_t * p_ble_evt);
uint32_t ble_battery_string_send(ble_battery_t * p_battery,ble_char_t* bat_char, uint8_t * p_string, uint16_t length);
void     BATTERY_ble_data_send(ble_battery_t * p_battery,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len);	
static   void  BATTERY_notif_disable(ble_battery_t * p_battery);
#endif

