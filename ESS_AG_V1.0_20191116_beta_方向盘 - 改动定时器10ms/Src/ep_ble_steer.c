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

#include "EP_ble_steer.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "ep_ble_init.h"
#include "ep_battery.h"
#include "ep_ble_steer.h"

#define BLE_STEER_MAX_RX_CHAR_LEN      20 // BLE_steer_MAX_DATA_LEN        /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_STEER_MAX_TX_CHAR_LEN      20 // BLE_steer_MAX_DATA_LEN        /**< Maximum length of the TX Characteristic (in bytes). */

ble_steer_t                      m_steer;

static void steer_data_handler(ble_steer_t * p_steer, uint8_t * p_data, uint16_t length)
{

}
static void on_connect(ble_steer_t * p_steer, ble_evt_t * p_ble_evt)
{
    p_steer->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_steer     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_steer_t * p_steer, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_steer->conn_handle = BLE_CONN_HANDLE_INVALID;
	STEER_notif_disable(&m_steer);
}
static void STEER_notif_disable(ble_steer_t * p_steer)
{
	//p_steer->char_available_mask.notif_en       = false;
	p_steer->char_steer_available.notif_en      = false;
	p_steer->char_angle.notif_en                = false;
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_steer     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_steer_t * p_steer, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	ble_notif_en_check(p_evt_write,&p_steer->char_steer_available);
	ble_notif_en_check(p_evt_write,&p_steer->char_angle);
}

ble_evt_t  *steer_evt;
void ble_steer_on_ble_evt(ble_steer_t * p_steer, ble_evt_t * p_ble_evt)
{
    if ((p_steer == NULL) || (p_ble_evt == NULL))
    {
        return;
    }
    steer_evt = p_ble_evt;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_steer, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_steer, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_steer, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

static uint32_t STEER_char_add(ble_steer_t * p_steer,ble_char_t* ble_char)
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
	

    ble_uuid.type = p_steer->uuid_type;
    ble_uuid.uuid = ble_char->uuid;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = ble_char->char_val_max_len;

    return sd_ble_gatts_characteristic_add(p_steer->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}




void STEER_ble_init(ble_steer_t * p_steer)
{

    p_steer->data_handler       = steer_data_handler;
	p_steer->conn_handle        = BLE_CONN_HANDLE_INVALID;
//	//Characteristic Available Mask
//	p_steer->char_available_mask.char_r_d              = char_read_en;
//	p_steer->char_available_mask.notif_en              = false;
//	p_steer->char_available_mask.uuid                  = EP_STEER_CHAR_AVAILABLE_UUID;
//	p_steer->char_available_mask.char_val_max_len      = 1;
	//Electric Steering Available
	p_steer->char_steer_available.char_r_d             = char_read_en;
	p_steer->char_steer_available.notif_en             = false;
	p_steer->char_steer_available.uuid                 = EP_STEER_STEER_AVAILABLE_UUID;
	p_steer->char_steer_available.char_val_max_len     = 1;
	//Steering Angle
	p_steer->char_angle.char_r_d                       = char_read_en;
	p_steer->char_angle.notif_en                       = false;
	p_steer->char_angle.uuid                           = EP_STEER_ANGLE_UUID;
	p_steer->char_angle.char_val_max_len               = 2;
	//Steering App Control
	p_steer->char_app_ctr.char_r_d                     = char_write_en;
	p_steer->char_app_ctr.uuid                         = EP_STEER_APP_CTR_UUID;
	p_steer->char_app_ctr.char_val_max_len             = 1;
	//Steering Angle control
	p_steer->char_angle_ctr.char_r_d                   = char_write_en;
	p_steer->char_angle_ctr.uuid                       = EP_STEER_ANGLE_CTR_UUID;
	p_steer->char_angle_ctr.char_val_max_len           = 2;
	//Steering Set Middle
	p_steer->char_set_mid.char_r_d                     = char_write_en;
	p_steer->char_set_mid.uuid                         = EP_STEER_SET_MID_UUID;
	p_steer->char_set_mid.char_val_max_len             = 1;
	//Steering Set Left Most
	p_steer->char_set_left.char_r_d                    = char_write_en;
	p_steer->char_set_left.uuid                        = EP_STEER_SET_LEFT_UUID;
	p_steer->char_set_left.char_val_max_len            = 1;
	//Steering Set Right Most
	p_steer->char_set_right.char_r_d                   = char_write_en;
	p_steer->char_set_right.uuid                       = EP_STEER_SET_RIGHT_UUID;
	p_steer->char_set_right.char_val_max_len           = 1;
	
	
}

uint32_t STEER_service_init(ble_steer_t * p_steer)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t steer_base_uuid =  EP_BLE_BASE_UUID;
    if (p_steer == NULL)
    {
        return NRF_ERROR_NULL;
    }
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&steer_base_uuid, &p_steer->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_steer->uuid_type;
    ble_uuid.uuid = EP_STEER_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_steer->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	// Add the Characteristic
//    err_code = STEER_char_add(p_steer, &p_steer->char_available_mask);
//    if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
	err_code = STEER_char_add(p_steer, &p_steer->char_steer_available);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    err_code = STEER_char_add(p_steer, &p_steer->char_angle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = STEER_char_add(p_steer, &p_steer->char_app_ctr);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
   
    err_code = STEER_char_add(p_steer, &p_steer->char_angle_ctr);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = STEER_char_add(p_steer, &p_steer->char_set_mid);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = STEER_char_add(p_steer, &p_steer->char_set_left);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = STEER_char_add(p_steer, &p_steer->char_set_right);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	return NRF_SUCCESS;
}
uint32_t ble_steer_string_send(ble_steer_t * p_steer, ble_char_t * ble_char, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_steer == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_steer->conn_handle == BLE_CONN_HANDLE_INVALID) || (ble_char->notif_en == false))
    {
        return NRF_ERROR_INVALID_STATE;
    }



    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = ble_char->handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_steer->conn_handle, &hvx_params);
}

uint32_t STEER_ble_data_send(ble_steer_t * p_steer,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len)
{
	uint32_t err_code;
	uint8_t ble_dat[4];
	ble_dat[0] =  send_data & 0xFF;
	ble_dat[1] = (send_data >> 8)  & 0xFF;
	ble_dat[2] = (send_data >> 16) & 0xFF;
	ble_dat[3] = (send_data >> 24) & 0xFF;
	
	err_code = ble_steer_string_send(p_steer,&ble_char,ble_dat,dat_len);
	return err_code;
}

