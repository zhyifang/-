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

#include "ep_ble_navigation.h"
#include "ep_ble_motor.h"

ble_navig_t                      m_navig;

static void on_connect(ble_navig_t * p_navig, ble_evt_t * p_ble_evt)
{
    p_navig->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_navig     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_navig_t * p_navig, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_navig->conn_handle = BLE_CONN_HANDLE_INVALID;
	NAVIG_notif_disable(&m_navig);
}
static void NAVIG_notif_disable(ble_navig_t * p_navig)
{
	p_navig->char_device_avail.notif_en      = false;
	p_navig->char_gps_latit.notif_en         = false;
	p_navig->char_gps_longit.notif_en        = false;
	p_navig->char_gps_altit.notif_en         = false;
	p_navig->char_gps_row.notif_en           = false;
	p_navig->char_gps_yaw.notif_en           = false;
	p_navig->char_gps_pitch.notif_en         = false;
	p_navig->char_gps_speed.notif_en         = false;
	p_navig->char_gps_dop.notif_en           = false;
	p_navig->char_gps_accura.notif_en        = false;
}
uint32_t   navig_error_code ;
static void navig_data_handler(ble_navig_t * p_navig, uint8_t * p_data, uint16_t length)
{
    
}


static void on_write(ble_navig_t * p_navig, ble_evt_t * p_ble_evt)
{
	ble_gatts_evt_write_t * p_evt_write;
    //ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    ble_srv_is_indication_enabled(p_evt_write->data);

	ble_notif_en_check(p_evt_write,&p_navig->char_remain_tAndm);
	ble_notif_en_check(p_evt_write,&p_navig->char_device_avail);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_latit);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_longit);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_altit);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_row);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_pitch);
	
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_yaw);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_speed);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_dop);
	ble_notif_en_check(p_evt_write,&p_navig->char_gps_accura);
//	ble_notif_en_check(p_evt_write,&p_navig->testData3);
//	ble_notif_en_check(p_evt_write,&p_navig->testData4);

	
//    if(p_evt_write->handle == p_navig->Available_Mask_handles.value_handle)
//	{
//	   p_navig->data_handler(p_navig, p_evt_write->data, p_evt_write->len);
//	}
//	
//    if(
//	   (p_evt_write->handle == p_navig->Permission_Key_handles.value_handle) &&
//	   (p_evt_write->len == 2)
//	)
//	{
//	    p_navig->data_handler(p_navig, p_evt_write->data, p_evt_write->len);
//	}
}
static uint32_t  ble_char_add(ble_navig_t * p_navig,ble_char_t *ble_char)
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
		char_md.p_cccd_md         = &cccd_md;
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
	

    ble_uuid.type = p_navig->uuid_type;
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
    return sd_ble_gatts_characteristic_add(p_navig->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}
void ble_navig_on_ble_evt(ble_navig_t * p_navig, ble_evt_t * p_ble_evt)
{
    if ((p_navig == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_navig, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_navig, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_navig, p_ble_evt);
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			 break;
        default:
            // No implementation needed.
            break;
    }
}
void  NAVIG_ble_init(ble_navig_t * p_navig)
{
	
	p_navig->data_handler       = navig_data_handler;
	p_navig->conn_handle        = BLE_CONN_HANDLE_INVALID;
	//Device On Board  Available 
	p_navig->char_device_avail.char_r_d           = char_r_w_en;
	p_navig->char_device_avail.char_val_max_len   = 1;
	p_navig->device_avail_data[0]                 = DEVICE_AVAIL_SET;
	p_navig->char_device_avail.p_value            = p_navig->device_avail_data;
	p_navig->char_device_avail.notif_en           = false;
	p_navig->char_device_avail.uuid               = EP_NAVI_DEVICE_AVAIL_UUID;
    //battery remain time and mile 
	p_navig->char_remain_tAndm.char_r_d           = char_read_en;
	p_navig->char_remain_tAndm.char_val_max_len   = 4;
	p_navig->char_remain_tAndm.notif_en           = false;
	p_navig->char_remain_tAndm.uuid               = EP_NAVI_REMAIN_TANDM_UUID;
	//Gps latitude
	p_navig->char_gps_latit.char_r_d              = char_read_en;
	p_navig->char_gps_latit.char_val_max_len      = 1;
	p_navig->char_gps_latit.notif_en              = false;
	p_navig->char_gps_latit.uuid                  = EP_NAVI_GPS_LATIT_UUID;
	//Gps longitude
	p_navig->char_gps_longit.char_r_d             = char_read_en;
	p_navig->char_gps_longit.char_val_max_len     = 4;
	p_navig->char_gps_longit.notif_en             = false;
	p_navig->char_gps_longit.uuid                 = EP_NAVI_GPS_LONGIT_UUID;
	//Gps  altitude
	p_navig->char_gps_altit.char_r_d              = char_read_en;
	p_navig->char_gps_altit.char_val_max_len      = 4;
	p_navig->char_gps_altit.notif_en              = false;
	p_navig->char_gps_altit.uuid                  = EP_NAVI_GPS_ALTIT_UUID;
	//Gps Row
	p_navig->char_gps_row.char_r_d                = char_read_en;
	p_navig->char_gps_row.char_val_max_len        = 2;
	p_navig->char_gps_row.notif_en                = false;
	p_navig->char_gps_row.uuid                    = EP_NAVIG_ROW_UUID;
	//Gps Pitch
	p_navig->char_gps_pitch.char_r_d              = char_read_en;
	p_navig->char_gps_pitch.char_val_max_len      = 2;
	p_navig->char_gps_pitch.notif_en              = false;
	p_navig->char_gps_pitch.uuid                  = EP_NAVIG_PITCH_UUID;
	//Yaw
	p_navig->char_gps_yaw.char_r_d                = char_read_en;
	p_navig->char_gps_yaw.char_val_max_len        = 2;
	p_navig->char_gps_yaw.notif_en                = false;
	p_navig->char_gps_yaw.uuid                    = EP_NAVIG_YAW_UUID;
	//Gps Speed
	p_navig->char_gps_speed.char_r_d              = char_read_en;
	p_navig->char_gps_speed.char_val_max_len      = 2;
	p_navig->char_gps_speed.notif_en              = false;
	p_navig->char_gps_speed.uuid                  = EP_NAVIG_GPS_SPEED_UUID;
	//Gps DOP 2D
	p_navig->char_gps_dop.char_r_d                = char_read_en;
	p_navig->char_gps_dop.char_val_max_len        = 4;
	p_navig->char_gps_dop.notif_en                = false;
	p_navig->char_gps_dop.uuid                    = EP_NAVIG_GPS_DOP_UUID;
	//Gps Accuracy
	p_navig->char_gps_accura.char_r_d             = char_read_en;
	p_navig->char_gps_accura.char_val_max_len     = 4;
	p_navig->char_gps_accura.notif_en             = false;
	p_navig->char_gps_accura.uuid                 = EP_NAVIG_GPS_ACCUR_UUID;
	

	
//	//
//	p_navig->char_testData3.char_r_d              = char_read_en;
//	p_navig->char_testData3.char_val_max_len      = 4;
//	p_navig->char_testData3.notif_en              = false;
//	p_navig->char_testData3.uuid                  = EP_NAVIG_TEST_DATA3_UUID;
//	//
//	p_navig->char_testData4.char_r_d           = char_read_en;
//	p_navig->char_testData4.char_val_max_len   = 4;
//	p_navig->char_testData4.notif_en           = false;
//	p_navig->char_testData4.uuid               = EP_NAVIG_TEST_DATA4_UUID;

}
uint32_t NAVIG_service_init(ble_navig_t * p_navig)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t navig_base_uuid =  EP_BLE_BASE_UUID;

	p_navig->data_handler = navig_data_handler;
	
    if (p_navig == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.

    p_navig->data_handler            = p_navig->data_handler;


    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&navig_base_uuid, &p_navig->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_navig->uuid_type;
    ble_uuid.uuid = EP_NAVIG_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_navig->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	//add char 
	err_code = ble_char_add(p_navig,&p_navig->char_device_avail);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = ble_char_add(p_navig,&p_navig->char_remain_tAndm);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    err_code = ble_char_add(p_navig,&p_navig->char_gps_latit);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = ble_char_add(p_navig,&p_navig->char_gps_longit);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ble_char_add(p_navig,&p_navig->char_gps_altit);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ble_char_add(p_navig,&p_navig->char_gps_row);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ble_char_add(p_navig,&p_navig->char_gps_pitch);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = ble_char_add(p_navig,&p_navig->char_gps_yaw);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ble_char_add(p_navig,&p_navig->char_gps_speed);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code = ble_char_add(p_navig,&p_navig->char_gps_dop);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = ble_char_add(p_navig,&p_navig->char_gps_accura);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
//	err_code = ble_char_add(p_navig,&p_navig->char_testData3);
//	if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
    return NRF_SUCCESS;
}


uint32_t ble_navig_string_send(ble_navig_t * p_navig,ble_char_t * ble_char, uint8_t * p_string, uint16_t length)
{
	static uint8_t  count;
	count++;
    ble_gatts_hvx_params_t hvx_params;

    if (p_navig == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if( (p_navig->conn_handle == BLE_CONN_HANDLE_INVALID) ||(ble_char->notif_en == false))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_MAX_CHAR_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

	hvx_params.handle = ble_char->handles.value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_navig->conn_handle, &hvx_params);
}
void NAVIG_ble_data_send(ble_navig_t * p_navig,ble_char_t ble_char,uint32_t send_data,uint8_t dat_len)
{
	uint8_t ble_dat[4];
	ble_dat[0] =  send_data & 0xFF;
	ble_dat[1] = (send_data >> 8)  & 0xFF;
	ble_dat[2] = (send_data >> 16) & 0xFF;
	ble_dat[3] = (send_data >> 24) & 0xFF;
	
	ble_navig_string_send(p_navig,&ble_char,ble_dat,dat_len);  
}
