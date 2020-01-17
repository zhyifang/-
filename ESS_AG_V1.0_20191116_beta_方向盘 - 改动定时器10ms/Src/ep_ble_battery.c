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


#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "ep_ble_init.h"
#include "ep_ble_battery.h"
#include  "ep_ble_motor.h"

ble_battery_t                    m_battery;

static void on_connect(ble_battery_t * p_battery, ble_evt_t * p_ble_evt)
{
    p_battery->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_battery     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_battery_t * p_battery, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_battery->conn_handle = BLE_CONN_HANDLE_INVALID;
	BATTERY_notif_disable(&m_battery);
}
static void BATTERY_notif_disable(ble_battery_t * p_battery)
{
	p_battery->char_bat_num.notif_en           = false;
	p_battery->char_remain_cap.notif_en        = false;
	p_battery->char_vol.notif_en               = false;
	p_battery->char_cur.notif_en	              = false;
	p_battery->char_pow.notif_en 	          = false;
	p_battery->char_temp_num.notif_en          = false;
	p_battery->char_temp_value.notif_en	      = false;
	p_battery->char_status.notif_en 	          = false;
}
uint32_t   battery_error_code ;
static void battery_data_handler(ble_battery_t * p_battery, uint8_t * p_data, uint16_t length)
{
}




uint8_t battery_data[2];

static void on_write(ble_battery_t * p_battery, ble_evt_t * p_ble_evt)
{
	ble_gatts_evt_write_t * p_evt_write;
    //ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_srv_is_indication_enabled(p_evt_write->data);

	ble_notif_en_check(p_evt_write,&p_battery->char_bat_num);
	ble_notif_en_check(p_evt_write,&p_battery->char_remain_cap);
	ble_notif_en_check(p_evt_write,&p_battery->char_vol);
	ble_notif_en_check(p_evt_write,&p_battery->char_cur);
	ble_notif_en_check(p_evt_write,&p_battery->char_pow);
    ble_notif_en_check(p_evt_write,&p_battery->char_temp_num);
	ble_notif_en_check(p_evt_write,&p_battery->char_temp_value);
	ble_notif_en_check(p_evt_write,&p_battery->char_status);
//    if(p_evt_write->handle == p_battery->Available_Mask_handles.value_handle)
//	{
//	   p_battery->data_handler(p_battery, p_evt_write->data, p_evt_write->len);
//	}
//	
//    if(
//	   (p_evt_write->handle == p_battery->Permission_Key_handles.value_handle) &&
//	   (p_evt_write->len == 2)
//	)
//	{
//	    p_battery->data_handler(p_battery, p_evt_write->data, p_evt_write->len);
//	}
}
uint8_t  batt_data[5];
static uint32_t  BATTERY_char_add(ble_battery_t * p_battery,ble_char_t *ble_char)
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
	

    ble_uuid.type = p_battery->uuid_type;
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
    attr_char_value.init_len  = ble_char->char_val_max_len;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = ble_char->char_val_max_len;
    attr_char_value.p_value   = ble_char->p_value;
    return sd_ble_gatts_characteristic_add(p_battery->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}
void ble_battery_on_ble_evt(ble_battery_t * p_battery, ble_evt_t * p_ble_evt)
{
    if ((p_battery == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_battery, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_battery, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_battery, p_ble_evt);
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			 break;
        default:
            // No implementation needed.
            break;
    }
}
void  BATTERY_ble_init(ble_battery_t * p_battery)
{
	
	p_battery->data_handler       = battery_data_handler;
	p_battery->conn_handle        = BLE_CONN_HANDLE_INVALID;
	//Number of Batteries
	p_battery->char_bat_num.char_r_d               = char_r_w_en;
	p_battery->char_bat_num.char_val_max_len       = 1;
	p_battery->bat_num_data[0]                     = BATTERY_NUM;
	p_battery->char_bat_num.p_value                = p_battery->bat_num_data;
	p_battery->char_bat_num.notif_en               = false;
	p_battery->char_bat_num.uuid                   = EP_BATTERY_BAT_NUM_UUID;
	//Remaining Capacity
	p_battery->char_remain_cap.char_r_d           = char_write_en;
	p_battery->char_remain_cap.char_val_max_len   = 1;
	p_battery->char_remain_cap.uuid               = EP_BATTERY_REMAIN_CAP_UUID;
	//Battery Voltage
	p_battery->char_vol.char_r_d                  = char_read_en;
	p_battery->char_vol.char_val_max_len          = 2;
	p_battery->char_vol.notif_en                  = false;
	p_battery->char_vol.uuid                      = EP_BATTERY_VOL_UUID;
	//Battery Current
	p_battery->char_cur.char_r_d                  = char_read_en;
	p_battery->char_cur.char_val_max_len          = 2;
	p_battery->char_cur.notif_en                  = false;
	p_battery->char_cur.uuid                      = EP_BATTERY_CUR_UUID;
	//Battery Power
	p_battery->char_pow.char_r_d                  = char_read_en;
	p_battery->char_pow.char_val_max_len          = 2;
	p_battery->char_pow.notif_en                  = false;
	p_battery->char_pow.uuid                      = EP_BATTERY_POW_UUID;
	//Number of temp sensors
	p_battery->char_temp_num.char_r_d             = char_read_en;
	p_battery->char_temp_num.char_val_max_len     = 1;
	p_battery->temp_num_data[0]                   = BATTERY_TEMP_NUM;
	p_battery->char_temp_num.p_value              = p_battery->temp_num_data;
	p_battery->char_temp_num.notif_en             = false;
	p_battery->char_temp_num.uuid                 = EP_BATTERY_TEMP_NUM_UUID;
	//Temps for battery
	p_battery->char_temp_value.char_r_d           = char_read_en;
	p_battery->char_temp_value.char_val_max_len   = 2;
	p_battery->char_temp_value.notif_en           = false;
	p_battery->char_temp_value.uuid               = EP_BATTERY_CUR_UUID;
	//Status
	p_battery->char_status.char_r_d               = char_read_en;
	p_battery->char_status.char_val_max_len       = 1;
	p_battery->char_status.notif_en               = false;
	p_battery->char_status.uuid                   = EP_BATTERY_TEMP_UUID;
	
	//Max Current Set
	p_battery->char_max_cur_set.char_r_d           = char_write_en;
	p_battery->char_max_cur_set.char_val_max_len   = 2;
	p_battery->char_max_cur_set.uuid               = EP_BATTERY_MAX_CUR_SET_UUID;
	//Min Voltage Set
	p_battery->char_min_vol_set.char_r_d           = char_write_en;
	p_battery->char_min_vol_set.char_val_max_len   = 1;
	p_battery->char_min_vol_set.uuid               = EP_BATTERY_MIN_VOL_SET_UUID;
	
//	//testData3
//	p_battery->char_testData3.char_r_d           = char_read_en;
//	p_battery->char_testData3.char_val_max_len   = 4;
//	p_battery->char_testData3.notif_en	        = false;
//	p_battery->char_testData3.uuid     	  		= EP_BATTERY_STATUS_UUID;
//	//testData4
//	p_battery->char_testData4.char_r_d           = char_read_en;
//	p_battery->char_testData4.char_val_max_len   = 4;
//	p_battery->char_testData4.notif_en 	  		= false;
//	p_battery->char_testData4.uuid         		= EP_BATTERY_TEST_DATA4_UUID;
	
}
uint32_t BATTERY_service_init(ble_battery_t * p_battery)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t battery_base_uuid =  EP_BLE_BASE_UUID;

	p_battery->data_handler = battery_data_handler;
	
    if (p_battery == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    p_battery->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_battery->data_handler            = p_battery->data_handler;


    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&battery_base_uuid, &p_battery->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_battery->uuid_type;
    ble_uuid.uuid = EP_BATTERY_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_battery->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = BATTERY_char_add(p_battery,&p_battery->char_bat_num);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_remain_cap);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_vol);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_cur);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_pow);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_temp_num);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_temp_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_status);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_max_cur_set);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	err_code = BATTERY_char_add(p_battery,&p_battery->char_min_vol_set);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
//err_code = BATTERY_char_add(p_battery,&p_battery->testData3);
//if (err_code != NRF_SUCCESS)
//{
//	return err_code;
//}
//err_code = BATTERY_char_add(p_battery,&p_battery->testData4);
//if (err_code != NRF_SUCCESS)
//{
//	return err_code;
//}
	return NRF_SUCCESS;
}


uint32_t ble_battery_string_send(ble_battery_t * p_battery,ble_char_t* bat_char, uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if ((p_battery == NULL) && (bat_char->notif_en == false))
    {
        return NRF_ERROR_NULL;
    }

    if (p_battery->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_MAX_CHAR_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

	hvx_params.handle = bat_char->handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_battery->conn_handle, &hvx_params);
}
void BATTERY_ble_data_send(ble_battery_t * p_battery,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len)
{
	uint8_t ble_dat[4];
	ble_dat[0] =  send_data & 0xFF;
	ble_dat[1] = (send_data >> 8)  & 0xFF;
	ble_dat[2] = (send_data >> 16) & 0xFF;
	ble_dat[3] = (send_data >> 24) & 0xFF;
	
	ble_battery_string_send(p_battery,&ble_char,ble_dat,dat_len);  
}

