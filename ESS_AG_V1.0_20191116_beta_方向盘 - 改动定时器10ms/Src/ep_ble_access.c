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
#include "ep_ble_access.h"
#include "ep_ble_init.h"


ble_access_t      m_access;

static  uint8_t  EP_Mac_Nomal_Key[16]  = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35};
static  uint8_t  EP_Mac_Admin_Key[16]  = {0x31,0x30,0x32,0x33,0x34,0x35,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
static  uint8_t  EP_Mac_Pubilc_Key[16] = {0x23,0x45,0x21,0x65,0x67,0x23,0x7f,0x21,0x75,0x56,0x23,0x12,0x78,0x21,0x89,0x26};

static void on_connect(ble_access_t * p_access, ble_evt_t * p_ble_evt)
{
    p_access->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	p_access->perm_Level_sent_flag = true;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_access     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_access_t * p_access, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_access->conn_handle = BLE_CONN_HANDLE_INVALID;
	ACCESS_notif_disable(&m_access);
}
static void ACCESS_notif_disable(ble_access_t * p_access)
{
	//p_access->char_available_mask.notif_en         = false;
	p_access->char_public_key.notif_en             = false;
	//p_access->char_public_key.notif_en         = false;
	p_access->char_permission_level.notif_en       = false;

}
uint32_t   access_error_code ;
uint8_t rec_data1[16];
uint16_t access_key_num = 0;
static void access_data_handler(ble_access_t * p_access, uint8_t * p_data, uint16_t length)
{
	
	uint8_t return_dat[1];
	access_key_num++;
	for(uint8_t i= 0;i<16;i++)
	{
		rec_data1[i] = *(p_data+i);
	}
    if(key_compare(EP_Mac_Nomal_Key,p_data))   //nomal key ,return nomal  permssion data to app :EP_ACCESS_NORMAL_ACCESS
	{
		p_access->access_perm_Level = EP_ACCESS_NORMAL_ACCESS;
		return_dat[0] = EP_ACCESS_NORMAL_ACCESS;
		ble_access_string_send(p_access,&p_access->char_permission_level,return_dat,1);
	}
	else if(key_compare(EP_Mac_Admin_Key,p_data))//admin key ,return admin  permssion data to app  :EP_ACCESS_ADMIN_ACCESS
	{
	    p_access->access_perm_Level = EP_ACCESS_ADMIN_ACCESS;
		return_dat[0] = EP_ACCESS_ADMIN_ACCESS;
		ble_access_string_send(p_access,&p_access->char_permission_level,return_dat,1);
	}
	else//key is error ,return data to app  :EP_ACCESS_ERROR_KEY
	{
		return_dat[0] = EP_ACCESS_ERROR_KEY;
		ble_access_string_send(p_access,&p_access->char_permission_level,return_dat,1);
	}
   
}

uint8_t access_count;
void ACCESS_perm_Level_sent(ble_access_t * p_access)
{
	uint8_t return_dat[1];
	return_dat[0]  =  EP_ACCESS_ADMIN_ACCESS;
	ble_access_string_send(p_access,&p_access->char_permission_level,return_dat,1);
	access_count++;
}
ble_gatts_evt_write_t * p_evt_write;

uint8_t per_noti_num = 0;

static void on_write(ble_access_t * p_access, ble_evt_t * p_ble_evt)
{

     p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
	if (
        (p_evt_write->handle == p_access->char_public_key.handles.value_handle)
         &&
        (p_access->data_handler != NULL)
        )
    {
        p_access->data_handler(p_access, p_evt_write->data, p_evt_write->len);
		return;
    }
	//ble_notif_en_check(p_evt_write,&p_access->char_available_mask);
	ble_notif_en_check(p_evt_write,&p_access->char_public_key);
	if(ble_notif_en_check(p_evt_write,&p_access->char_permission_level))
	{
		if(p_access->perm_Level_sent_flag == true)
		{
			//ACCESS_perm_Level_sent(p_access);
			p_access->perm_Level_sent_flag = false;
			per_noti_num++;
		}
		
		
	}
	
}
void ble_access_on_ble_evt(ble_access_t * p_access, ble_evt_t * p_ble_evt)
{
    if ((p_access == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_access, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_access, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_access, p_ble_evt);
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			 break;
        default:
            // No implementation needed.
            break;
    }
}

static uint32_t ACCESS_char_add(ble_access_t * p_access,ble_char_t* ble_char)
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
	    char_md.char_props.write  = 1;
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
	

    ble_uuid.type = p_access->uuid_type;
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
    return sd_ble_gatts_characteristic_add(p_access->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}
void  ACCESS_ble_init(ble_access_t * p_access)
{
    p_access->data_handler       = access_data_handler;
	p_access->conn_handle        = BLE_CONN_HANDLE_INVALID;
	
//	p_access->char_available_mask.char_r_d         = char_read_en;
//	p_access->char_available_mask.notif_en         = false;
//	p_access->char_available_mask.uuid             = EP_ACCESS_AVAILABLE_UUID;
//	p_access->char_available_mask.char_val_max_len = 2;
//	
	p_access->char_public_key.char_r_d               = char_read_en;
	p_access->char_public_key.notif_en               = false;
	p_access->char_public_key.uuid                   = EP_ACCESS_PUB_KEY_UUID;
	p_access->char_public_key.char_val_max_len       = 16;
	//ACCESS_pubilc_ket_set(p_access,"1");
	p_access->char_public_key.p_value                = EP_Mac_Pubilc_Key;
	
	
	
	p_access->char_permission_key.char_r_d           = char_write_en;
	p_access->char_permission_key.uuid               = EP_ACCESS_PERM_KEY_UUID;
	p_access->char_permission_key.char_val_max_len   = 16;
	
	p_access->char_permission_level.char_r_d         = char_read_en;
	p_access->char_permission_level.notif_en         = false;
	p_access->char_permission_level.uuid             = EP_ACCESS_PERM_LEVEL_UUID;
	p_access->char_permission_level.char_val_max_len = 1;
}

uint32_t ACCESS_service_init(ble_access_t * p_access)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t access_base_uuid =  EP_BLE_BASE_UUID;

	
    if (p_access == NULL)
    {
        return NRF_ERROR_NULL;
    }
    err_code = sd_ble_uuid_vs_add(&access_base_uuid, &p_access->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_access->uuid_type;
    ble_uuid.uuid = EP_ACCESS_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_access->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

//    err_code = ACCESS_char_add(p_access,&p_access->char_available_mask);
//	if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }
    err_code = ACCESS_char_add(p_access,&p_access->char_public_key);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ACCESS_char_add(p_access,&p_access->char_permission_key);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = ACCESS_char_add(p_access,&p_access->char_permission_level);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}


uint32_t ble_access_string_send(ble_access_t * p_access, ble_char_t *ble_char, uint8_t * p_string, uint16_t length)
{
	static uint8_t  count;
	count++;
    ble_gatts_hvx_params_t hvx_params;

    if ((p_access == NULL) && ble_char->notif_en == false)
    {
        return NRF_ERROR_NULL;
    }

    if (p_access->conn_handle == BLE_CONN_HANDLE_INVALID)
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

    return sd_ble_gatts_hvx(p_access->conn_handle, &hvx_params);
}

static bool key_compare(const uint8_t* compared_key, uint8_t*  input_key)
{
	uint8_t i;
	for(i = 0; i<2; i++)
	{
		if(compared_key[i] != input_key[i])
			return false;
	}
	return true;
}
void ACCESS_pubilc_ket_set(ble_access_t * p_access,uint8_t *key)
{
	for(uint8_t i = 0;i < 16 ;i++)
	{
		p_access->public_key[i] = EP_Mac_Pubilc_Key[i];
	}


}
