#ifndef  __EP_BLE_DEVICE_INFO_H
#define  __EP_BLE_DEVICE_INFO_H

#include  "nrf.h"
#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>
#include "ep_ble_init.h"

#define EP_DEVICE_INFO_SERVICE_UUID      0x2000 /*Access Control Service*/

#define EP_DEVICE_INFO_AVAILABLE_UUID    0x2001 /*Characteristic Available Mask*/
#define EP_DEVICE_INFO_MOD_NUM_UUID      0x2002 /*Model Number*/
#define EP_DEVICE_INFO_MECH_REV_UUID     0x2003 /*Mechanical Revision*/
#define EP_DEVICE_INFO_HARD_REV_UUID     0x2004 /*Hardware Revision*/
#define EP_DEVICE_INFO_SOFT_REV_UUID     0x2005 /*Software Revision*/
#define EP_DEVICE_INFO_SERIAL_NUM_UUID   0x2006 /*Serial Number*/
#define EP_DEVICE_INFO_DRV_NUM_UUID      0x2007 /*Serial Number*/

#define  SOFTWARE_REV                    "2.55"
#define  HARDWARE_REV                    "2.22"
#define  MECH_REV                        "3.33"
#define  EP_MODEL_NUM                    "Navy 6.0"
#define  HANDLE_SERIAL_NUM               "S1L316031234"
#define  DRV_SYS_SERIAL_NUM              "S1L316031234"

typedef struct ble_device_info_s         ble_device_info_t;
extern  ble_device_info_t                m_device_info;
typedef void (*ble_device_info_data_handler_t) (ble_device_info_t * p_device_info, uint8_t * p_data, uint16_t length);


typedef struct
{
    uint8_t  len;
	uint8_t  *pdata;
}data_t;
struct ble_device_info_s
{
    uint8_t                     uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                    service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
  
	ble_char_t                  char_char_available_mask;              /**< Handles related to the TX characteristic (as provided by the S110 SoftDevice). */
    ble_char_t                  char_model_number;              /**< Handles related to the RX characteristic (as provided by the S110 SoftDevice). */
    ble_char_t                  char_mechanical_revision; 
	ble_char_t                  char_hardware_revision; 
	ble_char_t                  char_software_revision; 
	ble_char_t                  char_handle_serial_number; 
	ble_char_t                  char_drv_sys_serial_number; 
	uint16_t                    conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_device_info_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
	
	ble_srv_utf8_str_t          Model_Num;
	ble_srv_utf8_str_t          Soft_Rev;
	ble_srv_utf8_str_t          Hard_Rev;
	ble_srv_utf8_str_t          Mech_Rev;
	ble_srv_utf8_str_t          Handle_Serial;
	ble_srv_utf8_str_t          Drv_sys_Serial;
};

void DeviceInfo_ble_init(ble_device_info_t * p_device_info);
uint32_t DeviceInfo_service_init(ble_device_info_t * p_device_info);
void ble_device_info_on_ble_evt(ble_device_info_t * p_device_info, ble_evt_t * p_ble_evt);

#endif

