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

#include "EP_ble_motor.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "ep_ble_init.h"
#include "ep_battery.h"
#include "ep_motor.h"

#define BLE_MOTOR_MAX_RX_CHAR_LEN      20 // BLE_motor_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_MOTOR_MAX_TX_CHAR_LEN      20 // BLE_motor_MAX_DATA_LEN        /**< Maximum length of the TX Characteristic (in bytes). */

ble_motor_t                      m_motor;

static void motor_data_handler(ble_motor_t * p_motor, uint8_t * p_data, uint16_t length)
{

}
static void on_connect(ble_motor_t * p_motor, ble_evt_t * p_ble_evt)
{
    p_motor->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_motor     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_motor_t * p_motor, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_motor->conn_handle = BLE_CONN_HANDLE_INVALID;
	MOTOR_notif_disable(&m_motor);
}
static void MOTOR_notif_disable(ble_motor_t * p_motor)
{
    p_motor->char_mot_num.notif_en              = false;
	p_motor->char_voltage.notif_en              = false;
	p_motor->char_current.notif_en              = false;
	p_motor->char_power.notif_en                = false;
	p_motor->char_rpm.notif_en                  = false;
	p_motor->char_mot_temp.notif_en             = false;
	p_motor->char_drv_temp.notif_en             = false;
	p_motor->char_temp_num.notif_en             = false;
	p_motor->char_status.notif_en               = false;

}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_motor     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_motor_t * p_motor, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
	ble_notif_en_check(p_evt_write,&p_motor->char_mot_num);
    ble_notif_en_check(p_evt_write,&p_motor->char_voltage);
	ble_notif_en_check(p_evt_write,&p_motor->char_current);
	ble_notif_en_check(p_evt_write,&p_motor->char_power);
	ble_notif_en_check(p_evt_write,&p_motor->char_rpm);
	ble_notif_en_check(p_evt_write,&p_motor->char_temp_num);
	ble_notif_en_check(p_evt_write,&p_motor->char_mot_temp);
	ble_notif_en_check(p_evt_write,&p_motor->char_drv_temp);
	ble_notif_en_check(p_evt_write,&p_motor->char_status);
}

ble_evt_t  *motor_evt;
void ble_motor_on_ble_evt(ble_motor_t * p_motor, ble_evt_t * p_ble_evt)
{
    if ((p_motor == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
    motor_evt = p_ble_evt;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_motor, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_motor, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_motor, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

static uint32_t MOTOR_char_add(ble_motor_t * p_motor,ble_char_t* ble_char)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));
    if(ble_char->char_r_d == char_read_en)
	{
		char_md.char_props.read   = 1;
		char_md.char_props.notify = 1;
		char_md.p_char_user_desc  = NULL;
		char_md.p_char_pf         = NULL;
		char_md.p_user_desc_md    = NULL;
		char_md.p_cccd_md         = &cccd_md;
		char_md.p_sccd_md         = NULL;
	}
	else if(ble_char->char_r_d == char_write_en)
	{
	    char_md.char_props.write   = 1;
		char_md.p_char_user_desc  = NULL;
		char_md.p_char_pf         = NULL;
		char_md.p_user_desc_md    = NULL;
		char_md.p_sccd_md         = NULL;
	}
	else if(ble_char->char_r_d == char_r_w_en)
	{
		char_md.char_props.write  = 1;
	    char_md.char_props.read   = 1;
		char_md.char_props.notify = 1;
		char_md.p_char_user_desc  = NULL;
		char_md.p_char_pf         = NULL;
		char_md.p_user_desc_md    = NULL;
		char_md.p_cccd_md         = &cccd_md;
		char_md.p_sccd_md         = NULL;
	}
	

    ble_uuid.type = p_motor->uuid_type;
    ble_uuid.uuid = ble_char->uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = ble_char->char_val_max_len;
    attr_char_value.p_value   = ble_char->p_value;
    return sd_ble_gatts_characteristic_add(p_motor->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}




void MOTOR_ble_init(ble_motor_t * p_motor)
{
    p_motor->data_handler       = motor_data_handler;
	p_motor->conn_handle        = BLE_CONN_HANDLE_INVALID;
	//Number of Motors
	p_motor->char_mot_num.char_r_d              = char_r_w_en;
	p_motor->char_mot_num.notif_en              = false;
	p_motor->char_mot_num.uuid                  = EP_MOTOR_NUM_UUID;
	p_motor->char_mot_num.char_val_max_len      = 1;
	p_motor->mot_num_data[0]                    = MOT_NUM;
	p_motor->char_mot_num.p_value               = p_motor->mot_num_data;

	//Motor Voltage
	p_motor->char_voltage.char_r_d              = char_read_en;
	p_motor->char_voltage.notif_en              = false;
	p_motor->char_voltage.uuid                  = EP_MOTOR_VOL_UUID;
	p_motor->char_voltage.char_val_max_len      = 2;
	//Motor Current
	p_motor->char_current.char_r_d              = char_read_en;
	p_motor->char_current.notif_en              = false;
	p_motor->char_current.uuid                  = EP_MOTOR_CUR_UUID;
	p_motor->char_current.char_val_max_len      = 2;
	//Motor Power
	p_motor->char_power.char_r_d                = char_read_en;
	p_motor->char_power.notif_en                = false;
	p_motor->char_power.uuid                    = EP_MOTOR_POWER_UUID;
	p_motor->char_power.char_val_max_len        = 2;
	//Motor RPM
	p_motor->char_rpm.char_r_d                  = char_read_en;
	p_motor->char_rpm.notif_en                  = false;
	p_motor->char_rpm.uuid                      = EP_MOTOR_RPM_UUID;
	p_motor->char_rpm.char_val_max_len          = 2;
	//Number of temp sensors motor
	p_motor->char_temp_num.char_r_d             = char_read_en;
	p_motor->char_temp_num.notif_en             = false;
	p_motor->char_temp_num.uuid                 = EP_MOTOR_TEMP_NUM_UUID;
	p_motor->temp_num_data[0]                   = (MOT_TEMP_NUM | ( DRV_TEMP_NUM << 4));
	p_motor->char_temp_num.char_val_max_len     = MOT_TEMP_NUM + DRV_TEMP_NUM;
	p_motor->char_temp_num.p_value              = p_motor->temp_num_data;
	//Temps for motor
	p_motor->char_mot_temp.char_r_d             = char_read_en;
	p_motor->char_mot_temp.notif_en             = false;
	p_motor->char_mot_temp.uuid                 = EP_MOTOR_MOT_TEMP_UUID;
	p_motor->char_mot_temp.char_val_max_len     = 4;
	

	//Temps for driver
	p_motor->char_drv_temp.char_r_d             = char_read_en;
	p_motor->char_drv_temp.notif_en             = false;
	p_motor->char_drv_temp.uuid                 = EP_MOTOR_DRV_TEM_UUID;
	p_motor->char_drv_temp.char_val_max_len     = 4;
	//Status
	p_motor->char_status.char_r_d               = char_read_en;
	p_motor->char_status.notif_en               = false;
	p_motor->char_status.uuid                   = EP_MOTOR_STATUS_UUID;
	p_motor->char_status.char_val_max_len       = 1;
	
	//Max Power Set
	p_motor->char_max_power_set.char_r_d          = char_write_en;
	p_motor->char_max_power_set.uuid              = EP_MOTOR_MAX_POWER_SET_UUID;
	p_motor->char_max_power_set.char_val_max_len  = 1;
	//Max RPM Set
	p_motor->char_max_rpm_set.char_r_d            = char_write_en;
	p_motor->char_max_rpm_set.uuid                = EP_MOTOR_MAX_RPM_SET_UUID;
	p_motor->char_max_rpm_set.char_val_max_len    = 4;
	
	//Test Mode On
	p_motor->char_test_mode_on.char_r_d           = char_write_en;
	p_motor->char_test_mode_on.uuid               = EP_MOTOR_TEST_MODE_ON_UUID;
	p_motor->char_test_mode_on.char_val_max_len   = 1;
	//Set Power
	p_motor->char_set_power.char_r_d              = char_write_en;
	p_motor->char_set_power.uuid                  = EP_MOTOR_SET_POWER_UUID;
	p_motor->char_set_power.char_val_max_len      = 4;
	//Set RPM
	p_motor->char_set_rpm.char_r_d                = char_write_en;
	p_motor->char_set_rpm.uuid                    = EP_MOTOR_SET_RPM_UUID;
	p_motor->char_set_rpm.char_val_max_len        = 1;
	
//	//
//	p_motor->test_data3.char_r_d           = char_read_en;
//	p_motor->test_data3.notif_en           = false;
//	p_motor->test_data3.uuid               = EP_MOTOR_TEST_DATA3_UUID;
//	p_motor->test_data3.char_val_max_len   = 2;
//	//
//	p_motor->test_data4.char_r_d           = char_read_en;
//	p_motor->test_data4.notif_en           = false;
//	p_motor->test_data4.uuid               = EP_MOTOR_TEST_DATA4_UUID;
//	p_motor->test_data4.char_val_max_len   = 2;
	
	
}

uint32_t MOTOR_service_init(ble_motor_t * p_motor)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t motor_base_uuid =  EP_BLE_BASE_UUID;
    if (p_motor == NULL)
    {
        return NRF_ERROR_NULL;
    }

   

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&motor_base_uuid, &p_motor->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_motor->uuid_type;
    ble_uuid.uuid = EP_MOTOR_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_motor->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	// Add the Characteristic
    err_code = MOTOR_char_add(p_motor, &p_motor->char_mot_num);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
//	err_code = MOTOR_char_add(p_motor, &p_motor->mot_ctr_num);
//    if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
	
    err_code = MOTOR_char_add(p_motor, &p_motor->char_voltage);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = MOTOR_char_add(p_motor, &p_motor->char_current);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
   
    err_code = MOTOR_char_add(p_motor, &p_motor->char_power);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = MOTOR_char_add(p_motor, &p_motor->char_rpm);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = MOTOR_char_add(p_motor, &p_motor->char_temp_num);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = MOTOR_char_add(p_motor, &p_motor->char_mot_temp);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
//	err_code = MOTOR_char_add(p_motor, &p_motor->drv_temp_num);
//    if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
	err_code = MOTOR_char_add(p_motor, &p_motor->char_drv_temp);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = MOTOR_char_add(p_motor, &p_motor->char_status);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	
	err_code = MOTOR_char_add(p_motor, &p_motor->char_max_power_set);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = MOTOR_char_add(p_motor, &p_motor->char_max_rpm_set);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = MOTOR_char_add(p_motor, &p_motor->char_test_mode_on);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = MOTOR_char_add(p_motor, &p_motor->char_set_rpm);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = MOTOR_char_add(p_motor, &p_motor->char_set_power);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    return NRF_SUCCESS;
	
//	err_code = MOTOR_char_add(p_motor, &p_motor->test_data3);
//	if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
//	err_code = MOTOR_char_add(p_motor, &p_motor->test_data4);
//	if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
}


uint32_t ble_motor_string_send(ble_motor_t * p_motor, ble_char_t * ble_char, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_motor == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_motor->conn_handle == BLE_CONN_HANDLE_INVALID) || (ble_char->notif_en == false))
    {
        return NRF_ERROR_INVALID_STATE;
    }



    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = ble_char->handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_motor->conn_handle, &hvx_params);
}
uint32_t MOTOR_ble_data_send(ble_motor_t * p_motor,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len)
{
	uint32_t err_code;
	uint8_t ble_dat[4];
	ble_dat[0] =  send_data & 0xFF;
	ble_dat[1] = (send_data >> 8)  & 0xFF;
	ble_dat[2] = (send_data >> 16) & 0xFF;
	ble_dat[3] = (send_data >> 24) & 0xFF;
	
	err_code = ble_motor_string_send(p_motor,&ble_char,ble_dat,dat_len);
	return err_code;
}


