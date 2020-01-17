#ifndef  __EP_BLE_ADMIN_H
#define  __EP_BLE_ADMIN_H

#include  "nrf.h"
#include  "ble.h"
#include  "ble_srv_common.h"
#include  <stdint.h>
#include  <stdbool.h>
#include  "ep_ble_init.h"


#define EP_ADMIN_SERVICE_UUID              0x7000//0x1000 /*ADMIN Control Service*/

#define EP_ADMIN_TEST_DATA1_UUID           0x7001 /*Permission Level*/
#define EP_ADMIN_TEST_DATA2_UUID           0x7002 /*Permission Level*/
#define EP_ADMIN_TEST_DATA3_UUID           0x7003 /*Permission Level*/
#define EP_ADMIN_TEST_DATA4_UUID           0x7004 /*Permission Level*/

typedef struct  ble_admin_s        ble_admin_t;
typedef void (*ble_admin_data_handler_t) (ble_admin_t * p_ADMIN, uint8_t * p_data, uint16_t length);



struct ble_admin_s
{
    uint8_t                     uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                    service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */ 
	
	ble_char_t                  testData1; 
	ble_char_t                  testData2;
	ble_char_t                  testData3; 
	ble_char_t                  testData4;
	
	uint16_t                    conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_admin_data_handler_t    data_handler;            /**< Event handler to be called for handling received data. */

};

void      ADMIN_ble_init(ble_admin_t * p_admin);
uint32_t  ADMIN_service_init(ble_admin_t * p_admin);
void      ble_admin_on_ble_evt(ble_admin_t * p_admin, ble_evt_t * p_ble_evt);
uint32_t  ble_admin_string_send(ble_admin_t * p_admin,uint16_t value_handle, uint8_t * p_string, uint16_t length);
	
#endif

