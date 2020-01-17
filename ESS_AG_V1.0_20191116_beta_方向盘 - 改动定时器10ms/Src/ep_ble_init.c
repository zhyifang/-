/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_gpiote.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "ep_BLE_init.h"
#include "ep_timeslot.h"
#include "main.h"
#include "ep_comm.h"
#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "ep_button.h"
#include "ep_PowerSwitch.h"
#include "ep_button.h"
#include "ep_lcd_6kw.h"
#include "ep_pstorage.h"
#include "ep_ble_access.h"
#include "ep_ble_device_info.h"
#include "ep_ble_motor.h"
#include "nrf_gzp.h"
#include "ep_ble_battery.h"
#include "ep_ble_navigation.h"
#include "ep_ble_admin.h"
#include "ep_ble_steer.h"
#include "ep_battery.h"
#include "ep_gps.h"
#include "ep_tick.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define DEVICE_NAME                     "EP_NAVY"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_ADV_INTERVAL                MSEC_TO_UNITS(1050, UNIT_0_625_MS) //64      /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      5                                         /**< The advertising timeout (in units of seconds). */


#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)   //500          /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(30, UNIT_1_25_MS)    //1000         /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   2                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */




#define  TMIE_HANDER_MS             10
#define  GZLL_PACKET_INTERVAL       APP_TIMER_TICKS(TMIE_HANDER_MS, APP_TIMER_PRESCALER)    //改变量39，define time_hander_ms  30
#define  ADC_START_INTERVAL         APP_TIMER_TICKS(2, APP_TIMER_PRESCALER)

static   app_timer_id_t                 m_gazell_timer_id;
//static   app_timer_id_t                 m_app_timer_id;

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */





ble_admin_t                      m_admin; 



static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
volatile uint8_t ble_evt_id;

static ble_uuid_t                       m_adv_uuids[] = { 
													     {EP_MOTOR_SERVICE_UUID, BLE_UUID_TYPE_BLE},
	                                                     {EP_DEVICE_INFO_SERVICE_UUID, BLE_UUID_TYPE_BLE},
														 {EP_BATTERY_SERVICE_UUID, BLE_UUID_TYPE_BLE},
														 {EP_NAVIG_SERVICE_UUID, BLE_UUID_TYPE_BLE},
//														 {EP_ADMIN_SERVICE_UUID, BLE_UUID_TYPE_BLE},
														 {EP_ACCESS_SERVICE_UUID, BLE_UUID_TYPE_BLE},
														 {EP_STEER_SERVICE_UUID, BLE_UUID_TYPE_BLE}
                                                         };  /**< Universally unique service identifier. */


BLE_Handler_t BLE_Handler;

void Conver_num_16(uint16_t num,uint8_t *p_data)
{
   *p_data = num & 0xFF;
   *(p_data+1) = (num >> 8) & 0xFF;
}
void Conver_num_32(uint32_t num,uint8_t *p_data)
{
   *p_data = num & 0xFF;
   *(p_data + 1) = (num >> 8)  & 0xFF;
   *(p_data + 2) = (num >> 16) & 0xFF;
   *(p_data + 3) = (num >> 24) & 0xFF;
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
#if 0
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
#endif

bool  ble_notif_en_check(ble_gatts_evt_write_t * p_evt_write,ble_char_t * ble_char)
{
    if (
        (p_evt_write->handle == ble_char->handles.cccd_handle)
        &&
        (p_evt_write->len == 2)
       )   
	{
		if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            ble_char->notif_en = true;
			return true;
        }
        else
        {
            ble_char->notif_en = false;
			return false;
        }
	}
	return false;
}
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}



/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */

static void services_init(void)
{
    uint32_t       err_code;

	/***ACCESS service init***/
	ACCESS_ble_init(&m_access);
    err_code = ACCESS_service_init(&m_access);
	APP_ERROR_CHECK(err_code);
	
	/***DeviceInfo service init***/
	DeviceInfo_ble_init(&m_device_info);
	err_code = DeviceInfo_service_init(&m_device_info);
	APP_ERROR_CHECK(err_code);
	
    /***MOTOR service init***/
	MOTOR_ble_init(&m_motor);
	err_code = MOTOR_service_init(&m_motor);
	APP_ERROR_CHECK(err_code);

	/***BATTERY service init***/
	BATTERY_ble_init(&m_battery);
	err_code = BATTERY_service_init(&m_battery);
    APP_ERROR_CHECK(err_code);
	
	/***steer service init***/
    STEER_ble_init(&m_steer);
	err_code = STEER_service_init(&m_steer);
    APP_ERROR_CHECK(err_code);
	
	/***navigation service init***/
	NAVIG_ble_init(&m_navig);
	err_code = NAVIG_service_init(&m_navig);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
		case BLE_ADV_EVT_SLOW:
			break;
        case BLE_ADV_EVT_IDLE:
            break;
        default:
            break;
    }
  
}


/**@brief Function for the Application's S110 SoftDevice event handler.
 *
 * @param[in] p_ble_evt S110 SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
   
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
			m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

			
		    err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, BLE_GATTS_SYS_ATTR_FLAG_USR_SRVCS );
		   // err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
			BLE_SetStatus(BLE_Connected);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
			app_timer_start(m_gazell_timer_id, GZLL_PACKET_INTERVAL, NULL);
		 	BLE_SetStatus(BLE_Disconnected);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_GATTS_EVT_WRITE:

				    break;
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, BLE_GATTS_SYS_ATTR_FLAG_USR_SRVCS);
            break;
        case BLE_EVT_TX_COMPLETE:
		case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        default:
            // No implementation needed.
            break;
    }
	ble_evt_id = p_ble_evt->header.evt_id;
    	
}
void BLE_SetStatus(BleStatus_e status)
{
	BLE_Handler.status = status;
}
BleStatus_e BLE_ReadStatus(BLE_Handler_t* handle)
{
	return handle->status;
}

/**@brief Function for dispatching a S110 SoftDevice event to all modules with a S110 SoftDevice 
 *        event handler.
 *
 * @details This function is called from the S110 SoftDevice event interrupt handler after a S110 
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  S110 SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
	
	ble_access_on_ble_evt(&m_access,p_ble_evt);
	ble_device_info_on_ble_evt(&m_device_info, p_ble_evt);
	ble_motor_on_ble_evt(&m_motor, p_ble_evt);
	ble_battery_on_ble_evt(&m_battery, p_ble_evt);
	ble_navig_on_ble_evt(&m_navig, p_ble_evt);
	//ble_admin_on_ble_evt(&m_admin, p_ble_evt);
	ble_steer_on_ble_evt(&m_steer, p_ble_evt);
	
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

void sys_evt_dispatch(uint32_t sys_evt)
{
	timeslot_sys_event_handler(sys_evt);
	pstorage_sys_event_handler(sys_evt); 
}

/**@brief Function for the S110 SoftDevice initialization.
 *
 * @details This function initializes the S110 SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_250MS_CALIBRATION, NULL);

    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

	ble_enable_params.gatts_enable_params.attr_tab_size = 0xD00;
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
////	
	err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}
//相当于定时器中断，时间超时处理
static void gazell_send_packet_timeout_handler(void *p_context)
{
	static uint16_t app_time_out_count;
	
	app_time_out_count++;                   //进入中断加一
    
    BLE_Handler.app_time_out_flag_ms = 1; 
    
    if(app_time_out_count % (30/TMIE_HANDER_MS) == 0)
    {
	    BLE_Handler.app_time_out_flag = 1;                 //标准位置一30ms，看看代码if(BLE_Handler.app_time_out_flag == 1)// 30ms access ,need about 400us
	    PowerSwitch_Handle.SysPowerCheckFlag = 1;	
	}
	if(app_time_out_count % (180/TMIE_HANDER_MS) == 0) //if(app_time_out_count % 6 == 0)    30*6  =180ms
	{ 
		BLE_Handler.sysLcdRefresh = 1;   //这部分是 //30MS*6 = 180MS  LCD display
		
	}
	if(app_time_out_count % (10000/TMIE_HANDER_MS) == 0)    //30ms*333  = 9.99s = 10S
	{
		app_time_out_count = 0;
		//BATTERY_Handle.SaveSocFlag = true;
		PowerSwitch_OffCountAdd();
	}

	#if 1
	if(sys_debug_mode1 != SYS_ReadMode(&SYS_Handle))
	{
		if((Throttle_ADC_Timer_State_Stoped == THROTTLE_Handle.ADCTimerStateFlag) && ((THROTTLE_Handle.calibrationFlag == true) //(SYS_ReadErrCode(&SYS_Handle) == Err_Cal) 
			| (Comm_ReadState() == Comm_State_Connected)))//连接状态下或者校准状态下打开定时器读取油门
		{
			//app_timer_start(m_app_timer_id, ADC_START_INTERVAL, NULL);
			//app_timer_start(m_app_timer_id, ADC_START_INTERVAL, NULL);
			SysTimer1Start();
			THROTTLE_Handle.ADCTimerStateFlag = Throttle_ADC_Timer_State_Started;
		}	
		else if((Throttle_ADC_Timer_State_Started == THROTTLE_Handle.ADCTimerStateFlag) && (Comm_ReadState() == Comm_State_Disconnected) 
			&& (THROTTLE_Handle.calibrationFlag == false))//(SYS_ReadErrCode(&SYS_Handle) != Err_Cal)) //断开连接状态而且不是校准状态停止定时器
		{
			//app_timer_stop(m_app_timer_id);
			SysTimer1Stop();
			THROTTLE_Handle.ADCTimerStateFlag = Throttle_ADC_Timer_State_Stoped;
		}
	}
	else //if(Throttle_ADC_Timer_State_Stoped == THROTTLE_Handle.ADCTimerStateFlag)//调试模式下只进来一次
	{
		//app_timer_start(m_app_timer_id, ADC_START_INTERVAL, NULL);
		SysTimer1Start();
		THROTTLE_Handle.ADCTimerStateFlag = Throttle_ADC_Timer_State_Started;
	}
	#endif
} 

//extern uint32_t ulAdcSampleCnt;
//static void app_adc_timeout_handler(void *p_context)
//{
//NRF_GPIO->OUTSET = 1 << 26;//on
  //NRF_GPIO->OUTCLR = 1 << 26;//off
//	THROTTLE_KMZ60Enable();
//	THROTTLE_AngSampleStart();
	//app_timer_start(m_app_timer_id, ADC_START_INTERVAL, NULL);

//} 
//定时器初始化 分频得到RTC
static void timers_init(void)
{
    // Initialize timer module
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
    app_timer_create(&m_gazell_timer_id, APP_TIMER_MODE_REPEATED, gazell_send_packet_timeout_handler);
	//app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, app_adc_timeout_handler);
	//app_timer_create(&m_app_timer_id, APP_TIMER_MODE_SINGLE_SHOT, app_adc_timeout_handler);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_slow_enabled  = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_slow_interval = APP_ADV_INTERVAL;
    options.ble_adv_slow_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

		
		
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for placing the application in low power state while waiting for events.
 */
 void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
   // USER_ASSERT(err_code,__LINE__,__FILE__);
}
void BLE_ParamInit(BLE_Handler_t* handle)
{
	handle->status             = BLE_Disconnected;
	handle->app_time_out_flag  = 0;
	handle->sysLcdRefresh      = 0;
}	
/**@brief  function for BLE initialization .
 */

void BLE_Init(void)
{
	BLE_Handler.AdvEnStatus = AdvDisable;
	
	BLE_ParamInit(&BLE_Handler);
	ble_stack_init();                                       //蓝牙协议栈初始化
	
	timers_init();
	app_timer_start(m_gazell_timer_id, GZLL_PACKET_INTERVAL, NULL);
	//app_timer_start(m_app_timer_id, ADC_PACKET_INTERVAL, NULL);
	button_init();
	gap_params_init();                                                                           //GAP initialization
	services_init();                                                                             //服务初始化
	advertising_init();                                                                          //广播参数初始化

	conn_params_init();     
                                                              //initializing the Connection Parameters module.
	//ble_advertising_start(BLE_ADV_MODE_IDLE);                                         //开始广播
	// USER_ASSERT(err_code,__LINE__,__FILE__);
}
void BLE_AdvEnable(void)
{
	if(BLE_Handler.AdvEnStatus == AdvEnable)
	{
		ble_advertising_start(BLE_ADV_MODE_IDLE);   
	}
}
