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
 * @defgroup ble_sdk_srv_motor Nordic UART Service
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
 *       by calling the ble_motor_on_ble_evt() function from the ble_stack_handler callback.
 */

#ifndef EP_BLE_MOTOR_H__
#define EP_BLE_MOTOR_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>
#include "ep_ble_init.h"


#define  EP_MOTOR_SERVICE_UUID           0x3000

#define  EP_MOTOR_AVAILABLE_UUID         0x3001 /*Characteristic Available Mask*/
#define  EP_MOTOR_NUM_UUID               0x3002 /*Number of Motors*/
#define  EP_MOTOR_VOL_UUID               0x3004 /*Motor Voltage*/
#define  EP_MOTOR_CUR_UUID               0x3005 /*Motor Current*/
#define  EP_MOTOR_POWER_UUID             0x3006 /*Motor Power*/
#define  EP_MOTOR_RPM_UUID               0x3007 /*Motor RPM*/
#define  EP_MOTOR_MAX_POWER_SET_UUID     0x3008 /*Max Power Set*/
#define  EP_MOTOR_MAX_RPM_SET_UUID       0x3009 /*Max RPM Set*/
#define  EP_MOTOR_TEST_MODE_ON_UUID      0x300a /*Test Mode On*/
#define  EP_MOTOR_SET_POWER_UUID         0x300b /*Set Power	*/
#define  EP_MOTOR_SET_RPM_UUID           0x300c /*Set RPM	*/
#define  EP_MOTOR_TEMP_NUM_UUID          0x300d /*Number of temp sensors*/
#define  EP_MOTOR_MOT_TEMP_UUID          0x300e /*Temps for motor*/
#define  EP_MOTOR_DRV_TEM_UUID           0x3010 /*Temps for driver*/
#define  EP_MOTOR_STATUS_UUID            0x3011 /*Status*/
#define  EP_MOTOR_TEST_DATA1_UUID        0x3012 /**/
#define  EP_MOTOR_TEST_DATA2_UUID        0x3013 /**/
#define  EP_MOTOR_TEST_DATA3_UUID        0x3014 /**/
#define  EP_MOTOR_TEST_DATA4_UUID        0x3015 /**/



#define  MOT_NUM         1
#define  MOT_TEMP_NUM    1
#define  DRV_TEMP_NUM    2
/* Forward declaration of the ble_motor_t type. */
typedef struct ble_motor_s ble_motor_t;
extern ble_motor_t                      m_motor;
/**@brief Nordic UART Service event handler type. */
typedef void (*ble_motor_data_handler_t) (ble_motor_t * p_motor, uint8_t * p_data, uint16_t length);


struct ble_motor_s
{
    uint8_t                  uuid_type;               /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    
	uint8_t                  mot_num_data[4];
	uint8_t                  temp_num_data[4];
	
    ble_char_t               char_available_mask;
	ble_char_t               char_mot_num;            /*Number of Motors*/
	ble_char_t               char_voltage;            /*Motor Voltage*/
	ble_char_t               char_current;            /*Motor Current*/
	ble_char_t               char_power;              /*Motor Power*/
	ble_char_t               char_rpm;                /*Motor RPM*/
	
	ble_char_t               char_max_power_set;      /*Max Power Set*/
	ble_char_t               char_max_rpm_set;        /*Max RPM Set*/
	ble_char_t               char_test_mode_on;       /*Test Mode On*/
	ble_char_t               char_set_power;          /*Set Power	*/
	ble_char_t               char_set_rpm;            /*Set RPM	*/
	
	ble_char_t               char_temp_num;           /*Number of temp sensors*/
	ble_char_t               char_mot_temp;           /*Temps for motor*/
	ble_char_t               char_drv_temp;           /*Temps for driver*/
	ble_char_t               char_status;             /*Status*/
	ble_char_t               char_test_data1;
	ble_char_t               char_test_data2;
	ble_char_t               char_test_data3;
	ble_char_t               char_test_data4;
	
	uint16_t                   conn_handle;             /**< Handle of the char_current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_motor_data_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
};

void MOTOR_ble_data_update(ble_motor_t * p_motor);
void     MOTOR_ble_init(ble_motor_t * p_motor);
uint32_t MOTOR_service_init(ble_motor_t * p_motor);
void     ble_motor_on_ble_evt(ble_motor_t * p_motor, ble_evt_t * p_ble_evt);
uint32_t ble_motor_string_send(ble_motor_t * p_motor, ble_char_t * ble_char, uint8_t * p_string, uint16_t length);

uint32_t     MOTOR_ble_data_send(ble_motor_t * p_motor,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len);
static void  MOTOR_notif_disable(ble_motor_t * p_motor);
#endif // BLE_motor_H__

/** @} */
