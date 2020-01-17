#include  <string.h>
#include  "nordic_common.h"
#include  "ble_srv_common.h"
#include  "ep_ble_init.h"
#include  "ep_ble_device_info.h"

ble_device_info_t                m_device_info;

static void device_info_data_handler(ble_device_info_t * p_device_info, uint8_t * p_data, uint16_t length)
{
}
static void on_connect(ble_device_info_t * p_device_info, ble_evt_t * p_ble_evt)
{
    p_device_info->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the @ref BLE_GAP_EVT_DISCONNECTED event from the S110 SoftDevice.
 *
 * @param[in] p_device_info     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_disconnect(ble_device_info_t * p_device_info, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_device_info->conn_handle = BLE_CONN_HANDLE_INVALID;
}

//
/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_device_info     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_device_info_t * p_device_info, ble_evt_t * p_ble_evt)
{
   // ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
//  ble_notif_en_check(p_evt_write,&p_device_info->Available_Mask);
//	ble_notif_en_check(p_evt_write,&p_device_info->char_model_number);
//	ble_notif_en_check(p_evt_write,&p_device_info->char_mechanical_revision);
//	ble_notif_en_check(p_evt_write,&p_device_info->char_hardware_revision);
//	ble_notif_en_check(p_evt_write,&p_device_info->char_software_revision);
//	ble_notif_en_check(p_evt_write,&p_device_info->Serial_Numbers);
}
void ble_device_info_on_ble_evt(ble_device_info_t * p_device_info, ble_evt_t * p_ble_evt)
{
    if ((p_device_info == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_device_info, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_device_info, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_device_info, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
static uint32_t DeviceInfo_char_add(ble_device_info_t * p_device_info,ble_char_t* ble_char)
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
		//char_md.char_props.notify = 1;
		char_md.p_char_user_desc  = NULL;
		char_md.p_char_pf         = NULL;
		char_md.p_user_desc_md    = NULL;
		//char_md.p_cccd_md         = &cccd_md;
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
	

    ble_uuid.type = p_device_info->uuid_type;
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
    return sd_ble_gatts_characteristic_add(p_device_info->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_char->handles);
}

void DeviceInfo_ble_init(ble_device_info_t * p_device_info)
{
    p_device_info->data_handler       = device_info_data_handler;
	p_device_info->conn_handle        = BLE_CONN_HANDLE_INVALID;
	
	
	p_device_info->char_model_number.char_r_d                = char_read_en;
	p_device_info->char_model_number.notif_en                = false;
	p_device_info->char_model_number.uuid                    = EP_DEVICE_INFO_MOD_NUM_UUID;
	ble_srv_ascii_to_utf8(&p_device_info->Model_Num, EP_MODEL_NUM);
	p_device_info->char_model_number.char_val_max_len        = p_device_info->Model_Num.length;	
	p_device_info->char_model_number.p_value                 = p_device_info->Model_Num.p_str;
	
	p_device_info->char_mechanical_revision.char_r_d         = char_read_en;
	p_device_info->char_mechanical_revision.notif_en         = false;
	p_device_info->char_mechanical_revision.uuid             = EP_DEVICE_INFO_MECH_REV_UUID;
	ble_srv_ascii_to_utf8(&p_device_info->Mech_Rev, MECH_REV);
	p_device_info->char_mechanical_revision.char_val_max_len = p_device_info->Mech_Rev.length;
	p_device_info->char_mechanical_revision.p_value          = p_device_info->Mech_Rev.p_str;
	
	p_device_info->char_hardware_revision.char_r_d           = char_read_en;
	p_device_info->char_hardware_revision.notif_en           = false;
	p_device_info->char_hardware_revision.uuid               = EP_DEVICE_INFO_HARD_REV_UUID;
	ble_srv_ascii_to_utf8(&p_device_info->Hard_Rev, HARDWARE_REV);
	p_device_info->char_hardware_revision.char_val_max_len   = p_device_info->Hard_Rev.length;
	p_device_info->char_hardware_revision.p_value            = p_device_info->Hard_Rev.p_str;
	
	p_device_info->char_software_revision.char_r_d           = char_read_en;
	p_device_info->char_software_revision.notif_en           = false;
	p_device_info->char_software_revision.uuid               = EP_DEVICE_INFO_SOFT_REV_UUID;
	ble_srv_ascii_to_utf8(&p_device_info->Soft_Rev, SOFTWARE_REV);
	p_device_info->char_software_revision.char_val_max_len   = p_device_info->Soft_Rev.length;
	p_device_info->char_software_revision.p_value            = p_device_info->Soft_Rev.p_str;
	
	p_device_info->char_handle_serial_number.char_r_d               = char_read_en;
	p_device_info->char_handle_serial_number.notif_en               = false;
	p_device_info->char_handle_serial_number.uuid                   = EP_DEVICE_INFO_SERIAL_NUM_UUID;	
	ble_srv_ascii_to_utf8(&p_device_info->Handle_Serial, HANDLE_SERIAL_NUM);
	p_device_info->char_handle_serial_number.char_val_max_len       = p_device_info->Handle_Serial.length;
	p_device_info->char_handle_serial_number.p_value                = p_device_info->Handle_Serial.p_str;
	
	p_device_info->char_drv_sys_serial_number.char_r_d              = char_read_en;
	p_device_info->char_drv_sys_serial_number.notif_en              = false;
	p_device_info->char_drv_sys_serial_number.uuid                  = EP_DEVICE_INFO_DRV_NUM_UUID;	
	ble_srv_ascii_to_utf8(&p_device_info->Drv_sys_Serial, DRV_SYS_SERIAL_NUM);
	p_device_info->char_drv_sys_serial_number.char_val_max_len      = p_device_info->Drv_sys_Serial.length;
	p_device_info->char_drv_sys_serial_number.p_value               = p_device_info->Drv_sys_Serial.p_str;
	
}

uint32_t DeviceInfo_service_init(ble_device_info_t * p_device_info)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t device_info_base_uuid = EP_BLE_BASE_UUID;

    if (p_device_info == NULL)
    {
        return NRF_ERROR_NULL;
    }

    // Initialize the service structure.
    

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&device_info_base_uuid, &p_device_info->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_device_info->uuid_type;
    ble_uuid.uuid = EP_DEVICE_INFO_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_device_info->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

//    // Add the RX Characteristic.
//    err_code = DeviceInfo_char_add(p_device_info,&p_device_info->Available_Mask);
//    if (err_code != NRF_SUCCESS)
//    {
//        return err_code;
//    }

    // Add the TX Characteristic.
    err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_model_number);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

	err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_mechanical_revision);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_hardware_revision);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_software_revision);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_handle_serial_number);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	err_code = DeviceInfo_char_add(p_device_info,&p_device_info->char_drv_sys_serial_number);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	
    return NRF_SUCCESS;
}


uint32_t ble_device_info_string_send(ble_device_info_t * p_device_info,ble_char_t * ble_char,uint8_t * p_string, uint16_t length)
{
    ble_gatts_hvx_params_t hvx_params;

    if (p_device_info == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if ((p_device_info->conn_handle == BLE_CONN_HANDLE_INVALID) || (ble_char->notif_en == false))
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

    return sd_ble_gatts_hvx(p_device_info->conn_handle, &hvx_params);
}


