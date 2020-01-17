/**************************************
Company       : ePropulsion 
File name     : 
Author        :   
Version       :
Date          :
Description   :
Others        :
Function List :
***Update  Description*****
1.  Date:
    Author:
    Modification:

*  
*************************************/
 
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf_assert.h"
#include "nrf_soc.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "ep_timeslot.h"
#include "app_util_platform.h"
#include "main.h"
#include "ep_ble_init.h"
#include "ep_comm.h" 
#include "ep_gzll.h"
#include "ep_pstorage.h"
#include "ep_motor.h"
#include "nrf_gzp.h"
#include "ep_crypt_device.h"

Timeslot_Handle_t Timeslot_Handle;

static nrf_radio_request_t  m_timeslot_request;
static uint32_t             m_slot_length;
static volatile bool        m_cmd_received = false;
static volatile bool        m_gzll_initialized = false;

static nrf_radio_signal_callback_return_param_t signal_callback_return_param;

void RADIO_IRQHandler(void);


/*************************
* XTAL is always started when using the Timeslot API. When timeslots are requested with the NRF_RADIO_HFCLK_CFG_DEFAULT option, 
  the XTAL is started at the same time as the timeslot START signal is given, unless it already runs. If the NRF_RADIO_HFCLK_CFG_FORCE_XTAL 
  is used, the XTAL is started earlier if not already running, so that it is already running and stable when the timeslot START signal is given
*
*
*
**************************/
void m_configure_next_event_init(void)
{
    m_slot_length                                  = 16000;//10000; 
    m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_DEFAULT;//use DEFAULT more saving power than  FORCE_XTAL
    m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_HIGH;
    m_timeslot_request.params.earliest.length_us   = m_slot_length; //ranges from 100 us to 100 ms.
    m_timeslot_request.params.earliest.timeout_us  = 20000;//10000; //max(128000000UL - 1UL)us

}
void m_configure_next_event(void)
{
    m_slot_length                                  = 16000;//10000; //us 
    m_timeslot_request.request_type                = NRF_RADIO_REQ_TYPE_EARLIEST;
    m_timeslot_request.params.earliest.hfclk       = NRF_RADIO_HFCLK_CFG_DEFAULT;//use DEFAULT more saving power than  FORCE_XTAL
    m_timeslot_request.params.earliest.priority    = NRF_RADIO_PRIORITY_HIGH;//NRF_RADIO_PRIORITY_NORMAL;
    m_timeslot_request.params.earliest.length_us   = m_slot_length; //ranges from 100 us to 100 ms.
    m_timeslot_request.params.earliest.timeout_us  = 20000; //15000max(128000000UL - 1UL)us

}     
void timeslot_sys_event_handler(uint32_t evt_id)
{
    switch (evt_id)
    {
        case NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN:
             break;
        
        case NRF_EVT_RADIO_SESSION_IDLE:  //7
             break;

        case NRF_EVT_RADIO_SESSION_CLOSED:
             break;

        case NRF_EVT_RADIO_BLOCKED://4		    
             break;

        case NRF_EVT_RADIO_CANCELED:
             break;

        default:   break;
    }
	Timeslot_Handle.evt_id = evt_id;
}

void start_timer(void)
{		
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer;  // Set the timer in Counter Mode
	NRF_TIMER0->TASKS_CLEAR = 1;               // clear the task first to be usable for later
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_16Bit;//0x01UL;										//Set counter to 16 bit resolution
	NRF_TIMER0->CC[0] = m_slot_length - 1000;
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
	NVIC_EnableIRQ(TIMER0_IRQn);	
	NRF_TIMER0->TASKS_START = 1;               // Start timer
}
static void m_on_start(void)
{	
    signal_callback_return_param.params.request.p_next = NULL;  //
    signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
	 
    if (!m_gzll_initialized)
    {	
		Gzll_Init(&GZLL_Handle);
        m_gzll_initialized = true;
    }
    else
    {		  
       nrf_gzll_set_mode(NRF_GZLL_MODE_DEVICE);
    }
    start_timer();		

}
void request_next(void)
{
	m_configure_next_event();
	signal_callback_return_param.params.request.p_next = &m_timeslot_request;
	signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
	sd_radio_request(&m_timeslot_request);
}
static void m_on_multitimer(void)
{
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
    
    if (nrf_gzll_get_mode() != NRF_GZLL_MODE_SUSPEND)
    {   
        signal_callback_return_param.params.request.p_next = NULL;
        signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
        nrf_gzll_set_mode(NRF_GZLL_MODE_SUSPEND);
//        NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
//        NRF_TIMER0->CC[0] = m_slot_length - 1000;
    }
    else
    { 
       // ASSERT(nrf_gzll_get_mode() == NRF_GZLL_MODE_SUSPEND);
       // m_configure_next_event();
       // signal_callback_return_param.params.request.p_next = &m_timeslot_request;
       // signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
    }	 
}
uint16_t gzll_fail_count = 0;
nrf_radio_signal_callback_return_param_t * m_radio_callback(uint8_t signal_type)
{   
    switch(signal_type)
    {   
        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:  //0
			m_on_start(); 
	        Comm_GzllModeSend(&Comm_Handle);
			if(GZLL_Handle.GZLL_addr_change == true)
			{
				static uint8_t change_count;
				change_count++;
				if(change_count > 5)//make sure  the address sent to the host 
				{
					GZLL_Handle.GZLL_addr_change =  false;
					m_gzll_initialized = false;
					change_count = 0;
				}
			}
			break;
          
        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO: //radio 数据发送会进入  2
            signal_callback_return_param.params.request.p_next = NULL;
            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
            RADIO_IRQHandler();
          // (void)nrf_gzll_set_mode(NRF_GZLL_MODE_SUSPEND);
            break;

        case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0: //1
//				signal_callback_return_param.params.request.p_next = &m_timeslot_request;
//	            signal_callback_return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND;

				m_on_multitimer();

			NRF_TIMER0->TASKS_STOP = 1;  
			
            break;
		case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
			nrf_gzll_set_mode(NRF_GZLL_MODE_DEVICE);
		    
			break;
    }
		
    return (&signal_callback_return_param);
}

uint32_t gazell_sd_radio_init(void)
{
    uint32_t err_code;
    m_configure_next_event_init();
    err_code = sd_radio_session_open(m_radio_callback);
    if (err_code != NRF_SUCCESS)
        return err_code;
    m_configure_next_event_init();
    err_code = sd_radio_request(&m_timeslot_request);
    if (err_code != NRF_SUCCESS)
    {
        (void)sd_radio_session_close();
        return err_code;
    }
    return NRF_SUCCESS;
}
