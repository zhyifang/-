/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**@file
 *
 * @defgroup ble_sdk_srv_steer Nordic UART Service
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    Nordic UART Service implementation.
 *
 * @details The Nordic UART Service is a simple GATT-based service with TX and RX characteristics.
 *          Data received from the peer is passed to the application, and the data received
 *          from the application of this service is sent to the peer as Handle Value
 *          Notifications. This module demonstrates how to implement a custom GATT-based
 *          service and characteristics using the S110 SoftDevice. The service
 *          is used by the application to send and receive ASCII text strings to and from the
 *          peer.
 *
 * @note The application must propagate S110 SoftDevice events to the Nordic UART Service module
 *       by calling the ble_steer_on_ble_evt() function from the ble_stack_handler callback.
 */

#ifndef EP_BLE_STEER_H__
#define EP_BLE_STEER_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>
#include "ep_ble_init.h"


#define  EP_STEER_SERVICE_UUID           0x5000

#define  EP_STEER_CHAR_AVAILABLE_UUID    0x5001 //Characteristic Available Mask
#define  EP_STEER_STEER_AVAILABLE_UUID   0x5002 //Electric Steering Available
#define  EP_STEER_ANGLE_UUID             0x5003 //Steering Angle
#define  EP_STEER_APP_CTR_UUID           0x5004 //Steering App Control
#define  EP_STEER_ANGLE_CTR_UUID         0x5005 //Steering Angle control
#define  EP_STEER_SET_MID_UUID           0x5006 //Steering Set Middle
#define  EP_STEER_SET_LEFT_UUID          0x5007 //Steering Set Left Most
#define  EP_STEER_SET_RIGHT_UUID         0x5008 //Steering Set Right Most
#define  EP_STEER_TEST_DATA1_UUID        0x5009 //
#define  EP_STEER_TEST_DATA2_UUID        0x500a //
#define  EP_STEER_TEST_DATA3_UUID        0x500b //
#define  EP_STEER_TEST_DATA4_UUID        0x500c //




/* Forward declaration of the ble_steer_t type. */
typedef struct ble_steer_s ble_steer_t;
extern ble_steer_t    m_steer;
/**@brief Nordic UART Service event handler type. */
typedef void (*ble_steer_data_handler_t) (ble_steer_t * p_steer, uint8_t * p_data, uint16_t length);


struct ble_steer_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */

    ble_char_t               char_available_mask;          //Characteristic Available Mask
	ble_char_t               char_steer_available;         //Electric Steering Available
	ble_char_t               char_angle;                   //Steering Angle
	ble_char_t               char_app_ctr;                 //Steering App Control
	ble_char_t               char_angle_ctr;               //Steering Angle control
	ble_char_t               char_set_mid;                 //Steering Set Middle
	ble_char_t               char_set_left;                //Steering Set Left Most
	ble_char_t               char_set_right;               //Steering Set Right Most
	ble_char_t               char_test_data1;
	ble_char_t               char_test_data2;
	ble_char_t               char_test_data3;
	ble_char_t               char_test_data4;
	
	uint16_t                   conn_handle;             /**< Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_steer_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
};

void STEER_ble_data_update(ble_steer_t * p_steer);
void     STEER_ble_init(ble_steer_t * p_steer);
uint32_t STEER_service_init(ble_steer_t * p_steer);
void     ble_steer_on_ble_evt(ble_steer_t * p_steer, ble_evt_t * p_ble_evt);
uint32_t ble_steer_string_send(ble_steer_t * p_steer, ble_char_t * ble_char, uint8_t * p_string, uint16_t length);

uint32_t     STEER_ble_data_send(ble_steer_t * p_steer,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len);
static void  STEER_notif_disable(ble_steer_t * p_steer);
#endif // BLE_steer_H__

/** @} */
