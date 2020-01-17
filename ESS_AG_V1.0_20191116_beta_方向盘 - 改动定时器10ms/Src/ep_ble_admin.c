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
#include "ep_ble_admin.h"



static void on_connect(ble_admin_t * p_admin, ble_evt_t * p_ble_evt)
{
    p_admin->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_admin     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_admin_t * p_admin, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_admin->conn_handle = BLE_CONN_HANDLE_INVALID;
}
uint32_t   admin_error_code ;
static void admin_data_handler(ble_admin_t * p_admin, uint8_t * p_data, uint16_t length)
{
    
}




static void on_write(ble_admin_t * p_admin, ble_evt_t * p_ble_evt)
{
	ble_gatts_evt_write_t * p_evt_write;
    //ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

	
	ble_notif_en_check(p_evt_write,&p_admin->testData1);
	ble_notif_en_check(p_evt_write,&p_admin->testData2);
	ble_notif_en_check(p_evt_write,&p_admin->testData3);
	ble_notif_en_check(p_evt_write,&p_admin->testData4);

	
//    if(p_evt_write->handle == p_admin->Available_Mask_handles.value_handle)
//	{
//	   p_admin->data_handler(p_admin, p_evt_write->data, p_evt_write->len);
//	}
//	
//    if(
//	   (p_evt_write->handle == p_admin->Permission_Key_handles.value_handle) &&
//	   (p_evt_write->len == 2)
//	)
//	{
//	    p_admin->data_handler(p_admin, p_evt_write->data, p_evt_write->len);
//	}
}
static uint32_t  ADMIN_char_add(ble_admin_t * p_admin,ble_char_t *ble_char)
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
	

    ble_uuid.type = p_admin->uuid_type;
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

    return sd_ble_gatts_characteristic_add(p_admin->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}
void ble_admin_on_ble_evt(ble_admin_t * p_admin, ble_evt_t * p_ble_evt)
{
    if ((p_admin == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_admin, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_admin, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_admin, p_ble_evt);
            break;
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
			 break;
        default:
            // No implementation needed.
            break;
    }
}
void  ADMIN_ble_init(ble_admin_t * p_admin)
{
	p_admin->data_handler       = admin_data_handler;
	p_admin->conn_handle        = BLE_CONN_HANDLE_INVALID;
	
	p_admin->testData1.char_r_d           = char_read_en;
	p_admin->testData1.char_val_max_len   = 4;
    p_admin->testData1.notif_en           = false;
	p_admin->testData1.uuid               = EP_ADMIN_TEST_DATA1_UUID;
	
	p_admin->testData2.char_r_d           = char_read_en;
	p_admin->testData2.char_val_max_len   = 4;
	p_admin->testData2.notif_en       	  = false;
	p_admin->testData2.uuid               = EP_ADMIN_TEST_DATA2_UUID;
	
	p_admin->testData3.char_r_d           = char_read_en;
	p_admin->testData3.char_val_max_len   = 4;
	p_admin->testData3.notif_en           = false;
	p_admin->testData3.uuid               = EP_ADMIN_TEST_DATA3_UUID;
	
	p_admin->testData4.char_r_d           = char_read_en;
	p_admin->testData4.char_val_max_len   = 4;
	p_admin->testData4.notif_en           = false;
	p_admin->testData4.uuid               = EP_ADMIN_TEST_DATA4_UUID;

}
uint32_t err_code1,err_code2;
uint32_t ADMIN_service_init(ble_admin_t * p_admin)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t admin_base_uuid =  EP_BLE_BASE_UUID;

	p_admin->data_handler = admin_data_handler;
	
    if (p_admin == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.

    p_admin->data_handler            = p_admin->data_handler;


    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&admin_base_uuid, &p_admin->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_admin->uuid_type;
    ble_uuid.uuid = EP_ADMIN_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_admin->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

	err_code = ADMIN_char_add(p_admin,&p_admin->testData1);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = ADMIN_char_add(p_admin,&p_admin->testData2);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
	err_code1 = ADMIN_char_add(p_admin,&p_admin->testData3);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code2 = ADMIN_char_add(p_admin,&p_admin->testData4);
	if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    return NRF_SUCCESS;
}


uint32_t ble_admin_string_send(ble_admin_t * p_admin, uint16_t value_handle, uint8_t * p_string, uint16_t length)
{
	static uint8_t  count;
	count++;
    ble_gatts_hvx_params_t hvx_params;

    if (p_admin == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (p_admin->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > BLE_MAX_CHAR_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

	hvx_params.handle = value_handle;
    hvx_params.p_data = p_string;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_admin->conn_handle, &hvx_params);
}


