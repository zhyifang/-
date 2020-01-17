#ifndef  __EP_BLE_NAVIGATION_H
#define  __EP_BLE_NAVIGATION_H

#include  "nrf.h"
#include  "ble.h"
#include  "ble_srv_common.h"
#include  <stdint.h>
#include  <stdbool.h>
#include "ep_ble_init.h"


#define EP_NAVIG_SERVICE_UUID              0x6000//0x1000 /*NAVIG Control Service*/

//#define EP_NAVI_AVAILABLE_UUID           0x6001 /*Characteristic Available Mask*/
#define EP_NAVI_DEVICE_AVAIL_UUID          0x6002 /*Device On Board  Available */
#define EP_NAVI_REMAIN_TANDM_UUID          0x6003 /*Remain TimeAndMile*/
#define EP_NAVI_GPS_LATIT_UUID             0x6007 /*Gps latitude*/
#define EP_NAVI_GPS_LONGIT_UUID            0x6008 /*Gps longitude*/
#define EP_NAVI_GPS_ALTIT_UUID             0x6009 /*Gps altitude*/
#define EP_NAVIG_ROW_UUID                  0x600a /*Row*/
#define EP_NAVIG_PITCH_UUID                0x600b /*pitch*/
#define EP_NAVIG_YAW_UUID                  0x600c /*yaw	*/
#define EP_NAVIG_GPS_SPEED_UUID            0x600d /*Gps Speed*/
#define EP_NAVIG_GPS_DOP_UUID              0x600e /*Gps DOP 2D*/
#define EP_NAVIG_GPS_ACCUR_UUID            0x600f /*Gps Accuracy*/
#define EP_NAVIG_TEST_DATA1_UUID           0x6011 /*TEST_DATA*/
#define EP_NAVIG_TEST_DATA2_UUID           0x6012 /*TEST_DATA*/
#define EP_NAVIG_TEST_DATA3_UUID           0x6013 /*TEST_DATA*/
#define EP_NAVIG_TEST_DATA4_UUID           0x6014 /*TEST_DATA*/

#define DEVICE_AVAIL_SET                   0x11


typedef struct  ble_navig_s        ble_navig_t;
extern  ble_navig_t                m_navig;

typedef void (*ble_navig_data_handler_t) (ble_navig_t * p_NAVIG, uint8_t * p_data, uint16_t length);

struct ble_navig_s
{
    uint8_t                     uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                    service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */ 
	ble_char_t                  char_Available_Mask;     /*Characteristic Available Mask*/
    ble_char_t                  char_device_avail;       /*Device On Board  Available */
	ble_char_t                  char_remain_tAndm;       /*Remain TimeAndMile*/
	ble_char_t                  char_gps_latit;          /*Gps latitude*/
	ble_char_t                  char_gps_longit;         /*Gps longitude*/
	ble_char_t                  char_gps_altit;          /*Gps altitude*/
	ble_char_t                  char_gps_row;            /*Row*/
	ble_char_t                  char_gps_pitch;          /*pitch*/
	ble_char_t                  char_gps_yaw;            /*yaw	*/
	ble_char_t                  char_gps_speed;          /*Gps Speed*/
	ble_char_t                  char_gps_dop;            /*Gps DOP 2D*/
	ble_char_t                  char_gps_accura;         /*Gps Accuracy*/
	ble_char_t                  char_testData1; 
	ble_char_t                  char_testData2;
	ble_char_t                  char_testData3; 
	ble_char_t                  char_testData4;
	uint8_t                     device_avail_data[1];
	uint16_t                    conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_navig_data_handler_t  data_handler;            /**< Event handler to be called for handling received data. */

};

void     NAVIG_ble_init(ble_navig_t * p_navig);
uint32_t NAVIG_service_init(ble_navig_t * p_navig);
void     ble_navig_on_ble_evt(ble_navig_t * p_navig, ble_evt_t * p_ble_evt);
uint32_t ble_navig_string_send(ble_navig_t * p_navig,ble_char_t * ble_char, uint8_t * p_string, uint16_t length);
void     NAVIG_ble_data_update(ble_navig_t * p_navig);
void     NAVIG_ble_data_send(ble_navig_t * p_navig,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len);	
static void NAVIG_notif_disable(ble_navig_t * p_navig);
#endif

