#ifndef  __EP_BLE_ACCESS_H
#define  __EP_BLE_ACCESS_H


#include  "nrf.h"
#include  <stdint.h>
#include  <stdbool.h>
#include  "ep_ble_init.h"


#define EP_ACCESS_SERVICE_UUID      0x1000//0x1000 /*Access Control Service*/

#define EP_ACCESS_AVAILABLE_UUID    0x1001//0x1001 /*Service Available Mask*/
#define EP_ACCESS_PUB_KEY_UUID      0x1002///0x1002 /*Public Key*/
#define EP_ACCESS_PERM_KEY_UUID     0x1003 /*Permission Key*/
#define EP_ACCESS_PERM_LEVEL_UUID   0x1004 /*Permission Level*/
              
typedef struct  ble_access_s        ble_access_t;
extern ble_access_t                 m_access;
typedef void (*ble_access_data_handler_t) (ble_access_t * p_access, uint8_t * p_data, uint16_t length);

typedef enum
{
	EP_ACCESS_EVT_KEY_NORMAL_WRITEN,
	EP_ACCESS_EVT_KET_ADMIN_WRITEN,
} ep_access_evt_type_t;


struct ble_access_s
{
    uint8_t                     uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    bool                        perm_Level_sent_flag;
	
	uint16_t                    service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */ 
	
	ble_char_t                  char_available_mask;              /**< Handles related to the TX characteristic (as provided by the S110 SoftDevice). */
    ble_char_t                  char_public_key;              /**< Handles related to the RX characteristic (as provided by the S110 SoftDevice). */
    ble_char_t                  char_permission_key; 
	ble_char_t                  char_permission_level; 
	
	uint16_t                    conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_access_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
	
	Permission_Level_e          access_perm_Level;
	
	uint8_t                     public_key[16];

};

void      ACCESS_ble_init(ble_access_t * p_access);
uint32_t  ACCESS_service_init(ble_access_t * p_access);
void      ble_access_on_ble_evt(ble_access_t * p_access, ble_evt_t * p_ble_evt);
uint32_t  ble_access_string_send(ble_access_t * p_access, ble_char_t *ble_char, uint8_t * p_string, uint16_t length);
static bool key_compare(const uint8_t* compared_key, uint8_t*  input_key);
static void  ACCESS_notif_disable(ble_access_t * p_access);	
void      ACCESS_perm_Level_sent(ble_access_t * p_access);
void      ACCESS_pubilc_ket_set(ble_access_t * p_access,uint8_t *key);

#endif

